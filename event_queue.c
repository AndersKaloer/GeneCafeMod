#include "event_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void broadcast_event(struct event_service *service, struct event *event) {
  #ifdef DEBUG
  printf("BROADCASTING EVENT\n");
  #endif
  if(pthread_mutex_lock(&service->event_listeners_mutex) == 0) {
    struct event_listener *listener = service->event_listeners;
    while(listener != NULL) {
      listener->listener_fun(event);
      listener = listener->next;
    }
    pthread_mutex_unlock(&service->event_listeners_mutex);
  }
}

void *event_handler_thread(void *args) {
  if(args == NULL) {
    fprintf(stderr, "Error: NULL argument to event handler thread\n");
    return NULL;
  }
  struct event_service *service = (struct event_service*)args;
  int retval;
  while(1) {
#ifdef DEBUG
    printf("WAITING FOR EVENT\n");
#endif
    if((retval = sem_wait(&service->event_cnt_sem)) == 0) {
#ifdef DEBUG
      printf("NEW EVENT\n");
#endif
      /* New event */
      struct event *event = NULL;
      if(pthread_mutex_lock(&service->event_list_mutex) == 0) {
        /* Remove from queue */
        event = service->event_list->event;
        service->event_list = service->event_list->next;
        pthread_mutex_unlock(&service->event_list_mutex);
      } else {
        fprintf(stderr, "Error: Could not obtain event list mutex\n");
      }
      if(event != NULL) {
        /* Process event */
        broadcast_event(service, event);
      }
    } else {
      fprintf(stderr, "Error: sem_wait failed: %s\n", strerror(retval));
    }
  }
  return NULL;
}

int event_start_service(struct event_service *service) {
  memset(service, 0, sizeof(struct event_service));
  if(sem_init(&service->event_cnt_sem, 0, 0)) {
    fprintf(stderr, "Error: Could not initialize event semaphore\n");
    goto err_sem_init;
  }
  if(pthread_mutex_init(&service->event_list_mutex, NULL) != 0) {
    fprintf(stderr, "Error: Could not allocate event list mutex\n");
    goto err_mutex_init;
  }
  
  if(pthread_create(&service->handler_thread, NULL, event_handler_thread, service) != 0) {
    fprintf(stderr, "Error creating peripheral event handler\n");
    goto err_pthread;
  }
  return 0;

 err_pthread:
  pthread_mutex_destroy(&service->event_list_mutex);
 err_mutex_init:
  sem_destroy(&service->event_cnt_sem);
 err_sem_init:
  return -1;
}

void event_stop_service(struct event_service *service) {
  pthread_cancel(service->handler_thread);
  sem_destroy(&service->event_cnt_sem);
  pthread_mutex_destroy(&service->event_list_mutex);
}

int event_enqueue(struct event_service *service, int id, char *data, int data_len, const void *sender) {
#ifdef DEBUG
  printf("ENQUEUING EVENT\n");
#endif
  struct event *event = malloc(sizeof(struct event));
  if(event == NULL) {
    fprintf(stderr, "Error: Could not allocate event\n");
    goto err_event_alloc;
  }
  struct event_list *list_item = malloc(sizeof(struct event_list));
  if(list_item == NULL) {
    fprintf(stderr, "Error: Could not allocate event list item\n");
    goto err_event_list_alloc;
  }
  char *data_copy = NULL;
  if(data != NULL && data_len > 0) {
    data_copy = malloc(data_len);
    if(data_copy == NULL) {
      fprintf(stderr, "Error: Could not allocate event data\n");
      goto err_data_alloc;
    }
    memcpy(data_copy, data, data_len);
  }
  event->event_id = id;
  event->data = data_copy;
  event->data_len = data_len;
  event->sender = sender;
  /* Add event to list */
  list_item->event = event;
  list_item->next = NULL;
  if(pthread_mutex_lock(&service->event_list_mutex) != 0) {
    fprintf(stderr, "Error: Could not obtain event list mutex\n");
    goto err_event_list_mtx;
  }
  if(service->event_list == NULL) {
    service->event_list = list_item;
    service->event_list_end = list_item;
  } else {
    service->event_list_end->next = list_item;
    service->event_list_end = list_item;
  }
  pthread_mutex_unlock(&service->event_list_mutex);
  /* Notify new event */
  sem_post(&service->event_cnt_sem);
  return 0;
  
 err_event_list_mtx:
 err_data_alloc:
  free(list_item);
 err_event_list_alloc:
  free(event);
 err_event_alloc:  
  return -1;
}

int event_add_listener(struct event_service *service, int (*listener_fun) (struct event*)) {
  struct event_listener *listener = malloc(sizeof(struct event_listener));
  if(listener == NULL) {
    fprintf(stderr, "Error: Could not allocate event listener struct\n");
    return -1;
  }
  listener->listener_fun = listener_fun;
  if(pthread_mutex_lock(&service->event_listeners_mutex) == 0) {
#ifdef DEBUG
    printf("ADDING EVENT LISTENER\n");
#endif
    listener->next = service->event_listeners;
    service->event_listeners = listener;
    pthread_mutex_unlock(&service->event_listeners_mutex);
  }
  return 0;
}

int event_rm_listener(struct event_service *service, int (*listener_fun) (struct event*)) {
  int retval = -1;
  if(pthread_mutex_lock(&service->event_listeners_mutex) == 0) {
    struct event_listener *prev = NULL, *listener = service->event_listeners;
    while(listener != NULL) {
      if(listener->listener_fun == listener_fun) {
        /* Found listener */
        /* Update linked list */
        if(prev == NULL) {
          service->event_listeners = listener->next;
        } else {
          prev->next = listener->next;
        }
        free(listener);
        retval = 0;
        break;
      }
      prev = listener;
      listener = listener->next;
    }
    pthread_mutex_unlock(&service->event_listeners_mutex);
  }
  return retval;
}
