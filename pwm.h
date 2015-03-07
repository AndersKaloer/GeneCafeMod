#ifndef _PWM_H_
#define _PWM_H_
#include <stdio.h>
#include <stdint.h>

struct pwm_fds {
  FILE *run_fd;
  FILE *polarity_fd;
  FILE *duty_fd;
  FILE *period_fd;
};

struct pwm_channel {
  uint8_t running;
  uint8_t polarity;
  float duty_pct;
  uint64_t duty;
  uint64_t period;
  struct pwm_fds pwm_fds;
};

int pwm_init(struct pwm_channel *channel, char *sys_directory);
void pwm_cleanup(struct pwm_channel *channel);
void pwm_set_duty(struct pwm_channel *channel, float duty);
void pwm_set_period(struct pwm_channel *channel, uint64_t period_ns);
void pwm_set_polarity(struct pwm_channel *channel, uint8_t polarity);
void pwm_start(struct pwm_channel *channel);
void pwm_stop(struct pwm_channel *channel);

#endif /* _PWM_H_ */
