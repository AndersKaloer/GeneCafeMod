#include "actuator_logserver.h"
#include "logserver.h"
#include <stdio.h>
#include <pthread.h>

pthread_t broadcast_thread;
pthread_mutex_t broadcast_cond_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t broadcast_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t latest_entry_mtx = PTHREAD_MUTEX_INITIALIZER;
int started = 0;
struct log_entry latest_entry = {0};
long last_broadcast_time_ms = 0;
int last_pop = 0;

pthread_t broadcast_thread;
void *logserver_broadcast_thread(void *args);

int actuator_logserver_init(void) {
  /* Initialize logserver */
  if(logserver_start()) {
    fprintf(stderr, "Error: logserver initialization failed\n");
    return -1;
  }
  /* Start broadcast thread */
  if(pthread_create(&broadcast_thread, NULL, logserver_broadcast_thread, NULL) != 0) {
    fprintf(stderr, "Error creating logserver broadcast thread\n");
    logserver_cleanup();
    return -1;
  }
  return 0;
}

void actuator_logserver_cleanup(void) {
  pthread_cancel(broadcast_thread);
  logserver_cleanup();
}

void *logserver_broadcast_thread(void *args) {
  struct log_entry entry;
  while(1) {
    if(pthread_cond_wait(&broadcast_cond, &broadcast_cond_mtx) == 0) {
      if(pthread_mutex_lock(&latest_entry_mtx) == 0) {
        entry = latest_entry;
        /* Reset pop value */
        latest_entry.pop = 0;
        pthread_mutex_unlock(&latest_entry_mtx);
      }
      broadcast_log(&entry);
    }
  }
  return NULL;
}

void actuator_logserver_new_meas_hook(struct measurement *meas) {
  int broadcast = 0;
  struct timespec time;
  if(pthread_mutex_lock(&latest_entry_mtx) == 0) {
    latest_entry.knop_val = meas->knop_pct;
    latest_entry.temp = meas->temp;
    if(latest_entry.pop == 0) {
      latest_entry.pop = meas->pop;
    }
    /* Only update every 500 ms */
    if(started && clock_gettime(CLOCK_MONOTONIC, &time) == 0) {
      long time_ms = time.tv_sec*1000 + time.tv_nsec/1000000;
      if(time_ms > last_broadcast_time_ms+500) {
        broadcast = 1;
        last_broadcast_time_ms = time_ms;
      }
    }
    pthread_mutex_unlock(&latest_entry_mtx);
  }
  if(broadcast) {
    /* Notify change */
    if(pthread_cond_signal(&broadcast_cond)) {
      fprintf(stderr, "Could not signal new measurement cond\n");
    }
  }
}

void actuator_logserver_new_time_hook(struct timespec abs_time, struct timespec rel_time) {
  if(pthread_mutex_lock(&latest_entry_mtx) == 0) {
    latest_entry.time_ms = abs_time.tv_sec*1000 + abs_time.tv_nsec/1000000;
    pthread_mutex_unlock(&latest_entry_mtx);
  }
  
}

void actuator_logserver_new_state_hook(enum system_state old_state, enum system_state new_state) {
  if(old_state == IDLE && new_state == ACTIVE) {
    if(pthread_mutex_lock(&latest_entry_mtx) == 0) {
      started = 1;
      pthread_mutex_unlock(&latest_entry_mtx);
    }
  } else if(old_state == ACTIVE && new_state == IDLE) {
    if(pthread_mutex_lock(&latest_entry_mtx) == 0) {
      started = 0;
      pthread_mutex_unlock(&latest_entry_mtx);
    }
  }
}

const struct actuator_operations actuator_logserver_ops = {
  .init = actuator_logserver_init,
  .cleanup = actuator_logserver_cleanup,
  .new_measurement_hook = actuator_logserver_new_meas_hook,
  .new_time_hook = actuator_logserver_new_time_hook,
  .new_state_hook = actuator_logserver_new_state_hook,
};

const struct actuator_descriptor actuator_logserver_desc = {
  .identifier = "logserver",
  .aops = &actuator_logserver_ops,
};
