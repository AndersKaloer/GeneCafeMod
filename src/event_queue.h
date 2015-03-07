#ifndef _EVENT_QUEUE_H_
#define _EVENT_QUEUE_H_

#include <pthread.h>
#include <semaphore.h>

struct event {
  /* ID of the event */
  int event_id;
  /* Data associated with the event */
  char *data;
  /* Length of data */
  int data_len;
  /* Sender identification */
  const void *sender;
};

struct event_list {
  struct event *event;
  struct event_list *next;
};

struct event_listener {
  int (*listener_fun) (struct event*);
  struct event_listener *next;
};

struct event_service {
  pthread_t handler_thread;
  sem_t event_cnt_sem;

  pthread_mutex_t event_list_mutex;
  struct event_list *event_list;
  struct event_list *event_list_end;

  pthread_mutex_t event_listeners_mutex;
  struct event_listener *event_listeners;
};


/* Initializes a new event handler.
   The event argument is an output parameter
   used to refer to the service when used.
   Returns 0 on success; negative otherwise. */
int event_start_service(struct event_service *service);

/* Stops the event handler refered to by event. */
void event_stop_service(struct event_service *service);


/* Notify about an event.
   event is the id of the event.
   data is a pointer to data with length data_len associated with the event.
   sender is the sender of the event.
   Returns 0 on success; negative on failure.
   This function is thread-safe. */
int event_enqueue(struct event_service *service, int event, char *data, int data_len, const void *sender);

/* Add a listener for new events.
   Returns 0 on success; negative on failure.
   The function which the argument points to should be short,
   as it blocks other listeners.
   If this function is called, the event_rm_listener() function 
   must be called when the listener is no longer needed
   in order to deallocate memory.
   This function is thread-safe. */
int event_add_listener(struct event_service *service, int (*listener_fun) (struct event*));

/* Remove a listener.
   Returns 0 on removal; negative otherwise.
   This function is thread-safe. */
int event_rm_listener(struct event_service *service, int (*listener_fun) (struct event*));

#endif /* _EVENT_QUEUE_H_ */
