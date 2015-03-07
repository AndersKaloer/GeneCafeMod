#include <stdio.h>


struct btn_channel {
  int gpio;
  int val_fd;
};

int btn_init(struct btn_channel *channel, int gpio);
int btn_get_value(struct btn_channel *channel);
int btn_intrpt_poll(struct btn_channel *channel, int timeout_ms);
int btn_intrpt_rising_enable(struct btn_channel *channel);
int btn_intrpt_falling_enable(struct btn_channel *channel);
int btn_intrpt_disable(struct btn_channel *channel);
void btn_cleanup(struct btn_channel *channel);
