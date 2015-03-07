#include "sensor.h"
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

struct event_service event_service;

int sq_handler_init(void) {
  return event_start_service(&event_service);
}
void sq_handler_destroy(void) {
  event_stop_service(&event_service);
}

int sq_enqueue(int event, char *data, int data_len, const struct sensor_descriptor *sender) {
  return event_enqueue(&event_service, event, data, data_len, (const void*)sender);
}

int sq_add_listener(int (*listener_fun) (struct event*)) {
  return event_add_listener(&event_service, listener_fun);
}

int sq_rm_listener(int (*listener_fun) (struct event*)) {
  return event_rm_listener(&event_service, listener_fun);
}
