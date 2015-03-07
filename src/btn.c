#include "btn.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>

int btn_init(struct btn_channel *channel, int gpio) {
  channel->gpio = gpio;
  FILE *fd = fopen("/sys/class/gpio/export", "w");
  if(fd == NULL) {
    fprintf(stderr, "Could not open export\n");
    return -1;
  }
  fprintf(fd, "%d", channel->gpio);
  fclose(fd);
  char file[50];
  sprintf(file, "/sys/class/gpio/gpio%d/direction", channel->gpio);
  fd = fopen(file, "w");
  if(fd == NULL) {
    fprintf(stderr, "Could not open direction\n");
    return -1;
  }
  fprintf(fd, "in"); // input
  fclose(fd);
  sprintf(file, "/sys/class/gpio/gpio%d/value", channel->gpio);
  int val_fd = open(file, O_RDONLY);
  if(val_fd == -1) {
    fprintf(stderr, "Could not open value\n");
    return -1;
  }
  channel->val_fd = val_fd;
  return 0;
}

int btn_get_value(struct btn_channel *channel) {
  char buf[1];
  read(channel->val_fd, &buf, 1);
  lseek(channel->val_fd, 0, SEEK_SET);
  int val = (buf[0] == '0' ? 0 : 1);
  return val;
}

int btn_intrpt_poll(struct btn_channel *channel, int timeout_ms) {
  struct pollfd pfd = {
    .fd = channel->val_fd,
    .events = POLLPRI|POLLERR,
    .revents = 0,
  };
  poll(&pfd, 1, timeout_ms);
  lseek(channel->val_fd, 0, SEEK_SET);
  if(pfd.revents & POLLPRI) {
    /* Success */
    return btn_get_value(channel);
  }
  return -1;
}

int btn_intrpt_rising_enable(struct btn_channel *channel) {
  FILE *fd;
  char file[50];
  sprintf(file, "/sys/class/gpio/gpio%d/edge", channel->gpio);
  fd = fopen(file, "w");
  if(fd == NULL) {
    fprintf(stderr, "Could not open edge\n");
    return -1;
  }
  fprintf(fd, "rising"); // Rising edge
  fclose(fd);
  return 0;
}

int btn_intrpt_falling_enable(struct btn_channel *channel) {
  FILE *fd;
  char file[50];
  sprintf(file, "/sys/class/gpio/gpio%d/edge", channel->gpio);
  fd = fopen(file, "w");
  if(fd == NULL) {
    fprintf(stderr, "Could not open edge\n");
    return -1;
  }
  fprintf(fd, "falling"); // Rising edge
  fclose(fd);
  return 0;
}

int btn_intrpt_disable(struct btn_channel *channel) {
  FILE *fd;
  char file[50];
  sprintf(file, "/sys/class/gpio/gpio%d/edge", channel->gpio);
  fd = fopen(file, "w");
  if(fd == NULL) {
    fprintf(stderr, "Could not open edge\n");
    return -1;
  }
  fprintf(fd, "none"); // Rising edge
  fclose(fd);
  return 0;
}

void btn_cleanup(struct btn_channel *channel) {
  close(channel->val_fd);
}
