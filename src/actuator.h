#ifndef _ACTUATOR_H_
#define _ACTUATOR_H_

#include "roaster.h"
#include <time.h>

struct actuator_descriptor {
  /* Actuator string identifier. */
  char *identifier;
  /* Actuator operations */
  const struct actuator_operations *aops;
};

struct actuator_operations {
  /* Used for actuator initialization. 
     Return non-negative on success. */
  int (*init) (void);
  /* Used to release resources, etc.
     This is only called if init() has returned a non-negative value. */
  void (*cleanup) (void);
  /* Called when a new measurement is ready. */
  void (*new_measurement_hook) (struct measurement*);
  /* Called when the timer values have been updated. */
  void (*new_time_hook) (struct timespec abs_time, struct timespec rel_time);
  /* Called when the overall system state has been updated. */
  void (*new_state_hook) (enum system_state old_state, enum system_state new_state);
};

#endif /* _ACTUATOR_H_ */
