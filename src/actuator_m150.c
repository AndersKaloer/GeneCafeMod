#include "actuator_m150.h"
#include "pwm.h"
#include <stdio.h>

struct pwm_channel pwm_ch;

int actuator_m150_init(void) {
  /* Initialize PWM */
  if(pwm_init(&pwm_ch, "/sys/devices/ocp.3/roaster0_m150_pwm.13")) {
    fprintf(stderr, "Error: LCD initialization failed\n");
    return -1;    
  }
  pwm_start(&pwm_ch);
  pwm_set_polarity(&pwm_ch, 0);
  pwm_set_period(&pwm_ch, 200000);
  pwm_set_duty(&pwm_ch, 0.0); /* Disable */
  
  return 0;
}

void actuator_m150_cleanup(void) {
  pwm_cleanup(&pwm_ch);
}

void actuator_m150_new_meas_hook(struct measurement *meas) {  
  /* Set PWM duty cycle */
  pwm_set_duty(&pwm_ch, meas->knop_pct);
}

const struct actuator_operations actuator_m150_ops = {
  .init = actuator_m150_init,
  .cleanup = actuator_m150_cleanup,
  .new_measurement_hook = actuator_m150_new_meas_hook,
};

const struct actuator_descriptor actuator_m150_desc = {
  .identifier = "m150",
  .aops = &actuator_m150_ops,
};
