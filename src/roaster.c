#include "btn.h"
#include "roaster.h"
#include "sensor.h"
#include "sensor_events.h"
#include "sensor_thermocouple.h"
#include "sensor_knop.h"
#include "sensor_button.h"
#include "actuator_lcd.h"
#include "actuator_m150.h"
#include "actuator_logserver.h"
#include <unistd.h>
#include <inttypes.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <mqueue.h>
#include <errno.h>
#include <string.h>

/* 500 ms between measurement per default */
#define DEFAULT_MEAS_FREQ_MS 500
#define MIN_MEAS_FREQ_MS 50

pthread_cond_t timer_restart_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t timer_restart_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rel_timer_mtx = PTHREAD_MUTEX_INITIALIZER;
volatile int rel_timer_restart = 0;

pthread_cond_t sensor_intrpt_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t sensor_intrpt_mtx = PTHREAD_MUTEX_INITIALIZER;

int meas_freq_ms = DEFAULT_MEAS_FREQ_MS;

enum system_state state = IDLE;

pthread_mutex_t pop_detected_mtx = PTHREAD_MUTEX_INITIALIZER;
int pop_detected = 0;

struct sensor_list_item {
  const struct sensor_descriptor *desc;
  pthread_mutex_t mutex;
  int last_meas_ms;
  struct sensor_list_item *next;
};

struct actuator_list_item {
  const struct actuator_descriptor *desc;
  pthread_mutex_t mutex;
  struct actuator_list_item *next;
};

struct sensor_list_item *sensor_list = NULL;
struct actuator_list_item *actuator_list = NULL;

struct timespec time_diff(struct timespec start, struct timespec end) {
  struct timespec diff;
  if(start.tv_nsec > end.tv_nsec) {
    diff.tv_sec = end.tv_sec - start.tv_sec - 1;
    diff.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } else {
    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return diff;
}

void sig_handler(int sig) {
  /* Nothing */
}

static void *timer_monitor(void *arg) {
  struct timespec start, start_rel, cur, max_time;
  int timer_running = 0;
  while(1) {
    if(pthread_cond_wait(&timer_restart_cond, &timer_restart_mtx) == 0) {
      if(clock_gettime(CLOCK_MONOTONIC, &start)) {
        fprintf(stderr, "Warning: Could not get time: %s\n", strerror(errno));
      }
      timer_running = 1;
      start_rel.tv_sec = 0;
      start_rel.tv_nsec = 0;
#ifdef DEBUG
    printf("Starting timer\n");
#endif
    }
    while(timer_running) {
      /* Check if rel timer restarted */
      if(pthread_mutex_lock(&rel_timer_mtx) == 0) {
        if(rel_timer_restart) {
          rel_timer_restart = 0;
          if(clock_gettime(CLOCK_MONOTONIC, &start_rel)) {
            fprintf(stderr, "Warning: Could not get time: %s\n", strerror(errno));
          }
        }
        pthread_mutex_unlock(&rel_timer_mtx);
      }
      if(!clock_gettime(CLOCK_MONOTONIC, &cur)) {
        /* Calculate time diff */
        int tot_time_sec = (cur.tv_sec - start.tv_sec);
        int rel_time_sec = 0;
        if(start_rel.tv_sec > 0) {
          rel_time_sec = (cur.tv_sec - start_rel.tv_sec);
        }
        /* Notify change */
        struct actuator_list_item *aitem = actuator_list;
        while(aitem != NULL) {
          if(pthread_mutex_lock(&aitem->mutex) == 0) {
            if(aitem->desc->aops->new_time_hook != NULL) {
              aitem->desc->aops->new_time_hook(tot_time_sec, rel_time_sec);
            }
            pthread_mutex_unlock(&aitem->mutex);
          }
          aitem = aitem->next;
        }
      }
      if(clock_gettime(CLOCK_REALTIME, &max_time)) {
        fprintf(stderr, "Warning: Could not get time: %s\n", strerror(errno));
      }
      /* Timeout after at most 1 sec */
      max_time.tv_sec += 1;
      max_time.tv_nsec = 0;
      if(pthread_cond_timedwait(&timer_restart_cond, &timer_restart_mtx, &max_time) == 0) {
        /* Stop timer */
        timer_running = 0;
#ifdef DEBUG
        printf("Stopping timer\n");
#endif
      }
    }
  }
  return NULL;
}

static void *peripheral_handler(void *arg) {
  struct measurement last_meas = {0};
  struct timespec meas_start_time, meas_end_time, diff, tmp;
  while(1) {
    if(clock_gettime(CLOCK_MONOTONIC, &meas_start_time)) {
      fprintf(stderr, "Error getting time: %s\n", strerror(errno));
      meas_start_time.tv_sec = 0;
      meas_start_time.tv_nsec = 0;
    }
    /* Request for measurements */
    struct sensor_list_item *sitem = sensor_list;
    int updated = 0;
    while(sitem != NULL) {
      if(pthread_mutex_lock(&sitem->mutex) == 0) {
        if(sitem->desc->sops->update_measurement != NULL) {
          sitem->last_meas_ms += meas_freq_ms;
          if(sitem->desc->meas_freq_ms <= sitem->last_meas_ms+meas_freq_ms/2) {
#ifdef DEBUG
            printf("Requesting measurements for %s\n", sitem->desc->identifier);
#endif
            if(sitem->desc->sops->update_measurement(&last_meas)) {
              updated = 1;
            }
            /* Reset timer */
            sitem->last_meas_ms = 0;
          }
        }
        pthread_mutex_unlock(&sitem->mutex);
      }
      sitem = sitem->next;
    }
    /* Pop detected?
       No need for force update for this */
    if(pthread_mutex_lock(&pop_detected_mtx) == 0) {
      last_meas.pop = pop_detected;
      pop_detected = 0;
      pthread_mutex_unlock(&pop_detected_mtx);
    }
    if(updated) {
      /* Notify actuators */
      struct actuator_list_item *aitem = actuator_list;
      while(aitem != NULL) {
        if(pthread_mutex_lock(&aitem->mutex) == 0) {
#ifdef DEBUG
          printf("Actuating %s\n", aitem->desc->identifier);
#endif
          if(aitem->desc->aops->new_measurement_hook != NULL) {
            aitem->desc->aops->new_measurement_hook(&last_meas);
          }
          pthread_mutex_unlock(&aitem->mutex);
        }
        aitem = aitem->next;
      }
    }
    if(clock_gettime(CLOCK_MONOTONIC, &meas_end_time)) {
      fprintf(stderr, "Error getting time: %s\n", strerror(errno));
      meas_end_time.tv_sec = 0;
      meas_end_time.tv_nsec = 0;
    }
    /* Calculate delay */
    tmp.tv_sec = (int)(meas_freq_ms/1000);
    tmp.tv_nsec = (meas_freq_ms % 1000)*1000000;
#ifdef DEBUG
    printf("Delay %d, %ld\n", (int)tmp.tv_sec, tmp.tv_nsec);
#endif
    /* Calculate diff */
    if(meas_start_time.tv_sec > 0 && meas_end_time.tv_sec > 0) {
      /* Calculate measurement time */
      diff = time_diff(meas_start_time, meas_end_time);
#ifdef DEBUG
    printf("Meas time %d, %ld\n", (int)diff.tv_sec, diff.tv_nsec);
#endif
      /* Subtract measurement time from measurement freq. */
      diff = time_diff(diff, tmp);
      if(diff.tv_sec < 0 || diff.tv_nsec < 0) {
#ifdef DEBUG
  printf("AAAA\n");
#endif
        diff.tv_sec = 0;
        diff.tv_nsec = 0;
      }
    } else {
      diff = tmp;
    }
#ifdef DEBUG
    printf("Sleeping %d, %ld\n", (int)diff.tv_sec, diff.tv_nsec);
#endif
    nanosleep(&diff, NULL);
  }
  return NULL;
}

int peripheral_event_listener(struct event *event) {
  #ifdef DEBUG
  printf("LISTENER INVOKED\n");
  #endif
  enum system_state oldstate = state;
  int handled = 0;
  switch(event->event_id) {
  case SENSOR_EVENT_BTN_PRESSED_SHORT:
    if(state == IDLE) {
      /* Start roaster */
      state = ACTIVE;
      handled = 1;
      if(pthread_cond_signal(&timer_restart_cond)) {
        fprintf(stderr, "Error: Could not start timer\n");
      }
    } else if(state == ACTIVE) {
      /* Pop detected */
      if(pthread_mutex_lock(&pop_detected_mtx) == 0) {
        pop_detected = 1;
        pthread_mutex_unlock(&pop_detected_mtx);
      }
      /* Reset relative timer */
      if(pthread_mutex_lock(&rel_timer_mtx) == 0) {
        rel_timer_restart = 1;
        pthread_mutex_unlock(&rel_timer_mtx);
      }
      handled = 1;
    }
    break;
  case SENSOR_EVENT_BTN_PRESSED_LONG:
    if(state == ACTIVE) {
      /* Stop roaster */
      if(pthread_cond_signal(&timer_restart_cond)) {
        fprintf(stderr, "Error: Could not start timer\n");
      }
      state = IDLE;
      handled = 1;
    }
    break;
  }
  if(oldstate != state) {
    /* State has changed */
    struct actuator_list_item *aitem = actuator_list;
    while(aitem != NULL) {
      if(pthread_mutex_lock(&aitem->mutex) == 0) {
        if(aitem->desc->aops->new_state_hook != NULL) {
          aitem->desc->aops->new_state_hook(oldstate, state);
        }
        pthread_mutex_unlock(&aitem->mutex);
      }
      aitem = aitem->next;
    }
  }
  return handled;
}
 
int register_sensor(const struct sensor_descriptor *desc) {
  struct sensor_list_item *item = malloc(sizeof(struct sensor_list_item));
  if(item == NULL) {
    fprintf(stderr, "Error: Could not allocate sensor list item for %s\n", desc->identifier);
    return -1;
  }
  if(pthread_mutex_init(&item->mutex, NULL) != 0) {
    fprintf(stderr, "Error: Could not allocate sensor mutex for %s\n", desc->identifier);
    free(item);
    return -1;
  }
  if(desc->sops->init()) {
    fprintf(stderr, "Error: Could not initialize sensor %s\n", desc->identifier);
    free(item);
    return -1;
  }
  item->desc = desc;
  if(desc->meas_freq_ms >= MIN_MEAS_FREQ_MS && desc->meas_freq_ms < meas_freq_ms) {
    meas_freq_ms = desc->meas_freq_ms;
  }
  item->next = sensor_list;
  sensor_list = item;
  return 1;
}

int register_actuator(const struct actuator_descriptor *desc) {
  struct actuator_list_item *item = malloc(sizeof(struct actuator_list_item));
  if(item == NULL) {
    fprintf(stderr, "Error: Could not allocate actuator list item for %s\n", desc->identifier);
    return -1;
  }
  if(pthread_mutex_init(&item->mutex, NULL) != 0) {
    fprintf(stderr, "Error: Could not allocate actuator mutex for %s\n", desc->identifier);
    free(item);
    return -1;
  }
  if(desc->aops->init()) {
    fprintf(stderr, "Error: Could not initialize actuator %s\n", desc->identifier);
    free(item);
    return -1;
  }
  item->desc = desc;
  item->next = actuator_list;
  actuator_list = item;
  return 1;
}

void cleanup_sensors(void) {
  struct sensor_list_item *tmp;
  while(sensor_list != NULL) {
    pthread_mutex_destroy(&sensor_list->mutex);
    sensor_list->desc->sops->cleanup();
    tmp = sensor_list;
    sensor_list = sensor_list->next;
    free(tmp);
  }
}

void cleanup_actuators(void) {
  struct actuator_list_item *tmp;
  while(actuator_list != NULL) {
    pthread_mutex_destroy(&actuator_list->mutex);
    actuator_list->desc->aops->cleanup();
    tmp = actuator_list;
    actuator_list = actuator_list->next;
    free(tmp);
  }
}


int main() {
  if(sq_handler_init()) {
    fprintf(stderr, "Error initializing sensor queue handler\n");
    exit(0);
  }
  sq_add_listener(peripheral_event_listener);
  register_sensor(&sensor_thermocouple_desc);
  register_sensor(&sensor_knop_desc);
  register_sensor(&sensor_button_desc);
  register_actuator(&actuator_lcd_desc);
  register_actuator(&actuator_m150_desc);
  register_actuator(&actuator_logserver_desc);

  pthread_t timer_thread, peripheral_thread;
  if(pthread_create(&timer_thread, NULL, timer_monitor, NULL) != 0) {
    fprintf(stderr, "Error creating timer monitor\n");
  }
  if(pthread_create(&peripheral_thread, NULL, peripheral_handler, NULL) != 0) {
    fprintf(stderr, "Error creating peripheral handler\n");
  }

  struct sigaction action = {
    .sa_handler = sig_handler,
  };
  sigaction(SIGINT, &action, NULL);
  /* Wait for signal */
  pause();
  
#ifdef DEBUG
  printf("Cancelling threads\n");
#endif
  pthread_cancel(timer_thread);
  pthread_cancel(peripheral_thread);
  
  cleanup_sensors();
  cleanup_actuators();
  sq_handler_destroy();

  return 0;
}
