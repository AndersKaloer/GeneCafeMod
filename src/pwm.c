#include "pwm.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>

int pwm_fd_open(struct pwm_channel *channel, char *sys_dir) {
  FILE *fd;
  char file[50];
  sprintf(file, "%s/run", sys_dir);
  fd = fopen(file, "w");
  if(fd == NULL) {
    fprintf(stderr, "Could not open PWM run file: %s\n", strerror(errno));
    goto err_run;
  }
  channel->pwm_fds.run_fd = fd;
  
  sprintf(file, "%s/polarity", sys_dir);
  fd = fopen(file, "w");
  if(fd == NULL) {
    fprintf(stderr, "Could not open PWM polarity file: %s\n", strerror(errno));
    goto err_pol;
  }
  channel->pwm_fds.polarity_fd = fd;
  
  sprintf(file, "%s/duty", sys_dir);
  fd = fopen(file, "w");
  if(fd == NULL) {
    fprintf(stderr, "Could not open PWM duty file: %s\n", strerror(errno));
    goto err_duty;
  }
  channel->pwm_fds.duty_fd = fd;
  
  sprintf(file, "%s/period", sys_dir);
  fd = fopen(file, "w");
  if(fd == NULL) {
    fprintf(stderr, "Could not open PWM period file: %s\n", strerror(errno));
    goto err_period;
  }
  channel->pwm_fds.period_fd = fd;
  return 0;
  
 err_period:
  fclose(channel->pwm_fds.duty_fd);
 err_duty:
  fclose(channel->pwm_fds.polarity_fd);
 err_pol:
  fclose(channel->pwm_fds.run_fd);
 err_run:
  return -1;
}

void pwm_fd_close(struct pwm_channel *channel) {
  fclose(channel->pwm_fds.run_fd);
  fclose(channel->pwm_fds.polarity_fd);
  fclose(channel->pwm_fds.duty_fd);
  fclose(channel->pwm_fds.period_fd);
}

int pwm_init(struct pwm_channel *channel, char *sys_directory) {
  /* Open files */
  if(pwm_fd_open(channel, sys_directory)) {
    return -1;
  }
  /* Read current settings */
  fscanf(channel->pwm_fds.run_fd, "%" PRIu8, (unsigned int*)&channel->running);
  fseek(channel->pwm_fds.run_fd, 0, SEEK_SET);
  fscanf(channel->pwm_fds.polarity_fd, "%" PRIu8, (unsigned int*)&channel->polarity);
  fseek(channel->pwm_fds.polarity_fd, 0, SEEK_SET);
  fscanf(channel->pwm_fds.duty_fd, "%" PRIu64, &channel->duty);
  fseek(channel->pwm_fds.duty_fd, 0, SEEK_SET);
  fscanf(channel->pwm_fds.period_fd, "%" PRIu64, &channel->period);
  fseek(channel->pwm_fds.period_fd, 0, SEEK_SET);
  if(channel->period > 0) {
    channel->duty_pct = channel->duty/(float)channel->period;
  } else {
      channel->duty_pct = 0.0;
  }
  return 0;
}

void pwm_cleanup(struct pwm_channel *channel) {
  pwm_fd_close(channel);
}

void pwm_set_duty(struct pwm_channel *channel, float duty) {
  if(duty > 1.0) {
    duty = 1.0;
  } else if(duty < 0.0) {
    duty = 0.0;
  }
  channel->duty_pct = duty;
  /* Calculate duty in ns */
  channel->duty = channel->period*duty;
  fprintf(channel->pwm_fds.duty_fd, "%" PRIu64, channel->duty);
  fseek(channel->pwm_fds.duty_fd, 0, SEEK_SET);
  fflush(channel->pwm_fds.duty_fd);
}

void pwm_set_period(struct pwm_channel *channel, uint64_t period_ns) {
  /* Rescale duty */
  channel->period = period_ns;
  channel->duty = channel->duty_pct*period_ns;
  /* Update period and duty */
  fprintf(channel->pwm_fds.period_fd, "%" PRIu64, channel->period);
  fseek(channel->pwm_fds.period_fd, 0, SEEK_SET);
  fflush(channel->pwm_fds.period_fd);
  fprintf(channel->pwm_fds.duty_fd, "%" PRIu64, channel->duty);
  fseek(channel->pwm_fds.duty_fd, 0, SEEK_SET);
  fflush(channel->pwm_fds.duty_fd);
}

void pwm_set_polarity(struct pwm_channel *channel, uint8_t polarity) {
  channel->polarity = polarity;
  fprintf(channel->pwm_fds.polarity_fd, "%" PRIu8, channel->polarity);
  fseek(channel->pwm_fds.polarity_fd, 0, SEEK_SET);
  fflush(channel->pwm_fds.polarity_fd);
}

void pwm_start(struct pwm_channel *channel) {
  channel->running = 1;
  fprintf(channel->pwm_fds.run_fd, "1");
  fseek(channel->pwm_fds.run_fd, 0, SEEK_SET);
  fflush(channel->pwm_fds.run_fd);
}

void pwm_stop(struct pwm_channel *channel) {
  channel->running = 0;
  fprintf(channel->pwm_fds.run_fd, "0");
  fseek(channel->pwm_fds.run_fd, 0, SEEK_SET);
  fflush(channel->pwm_fds.run_fd);
}
