#ifndef _SENSOR_H_
#define _SENSOR_H_

#include "roaster.h"
#include <pthread.h>
#include "event_queue.h"

struct sensor_descriptor {
  /* Sensor string identifier. */
  char *identifier;
  /* Approximate measurement frequency. */
  int meas_freq_ms;
  /* Sensor operations */
  const struct sensor_operations *sops;
};

struct sensor_operations {
  /* Used for sensor initialization. 
     Return non-negative on success. */
  int (*init) (void);
  /* Used to release resources, etc.
     This is only called if init() has returned a non-negative value. */
  void (*cleanup) (void);
  /* Used to update a sensor measurement in the struct.
     Return 1 if a measurement has been updated; 0 otherwise. */
  int (*update_measurement) (struct measurement*);
};

/* Initializes the sensor queue handler.
   This function must be called before the remaining
   functions can be used.
   The sq_handler_destroy() function must be called
   when the handler is no longer used.
   Returns 0 on success; negative on failure.*/
int sq_handler_init(void);

/* Destroys the sensor queue handler. */
void sq_handler_destroy(void);

/* Notify about a sensor event (e.g. button press).
   event is the id of the event, defined by the sensor. 
   event need not be globally unique.
   data is a pointer to data with length data_len associated with the event.
   sender is the sender of the event.
   Returns 0 on success; negative on failure.
   This function is thread-safe. */
int sq_enqueue(int event, char *data, int data_len, const struct sensor_descriptor *sender);

/* Add a listener for new events.
   Returns 0 on success; negative on failure.
   The function which the argument points to should be short,
   as it blocks other listeners.
   The sender attribute in the event struct can safely be 
   casted to a struct sensor_descriptor*.
   If this function is called, the sq_rm_listener() function 
   must be called when the listener is no longer needed
   in order to deallocate memory.
   This function is thread-safe. */
int sq_add_listener(int (*listener) (struct event*));

/* Remove a listener.
   Returns 0 on removal; negative otherwise.
   This function is thread-safe. */
int sq_rm_listener(int (*listener_fun) (struct event*));

#endif /* _SENSOR_H_ */
