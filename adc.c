#include "adc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int adc_init(struct adc_channel *channel, char *sys_directory, uint8_t adc_num, float min_val, float max_val) {
  sprintf(channel->fname, "%s/AIN%u", sys_directory, adc_num);
  channel->max_val = max_val;
  channel->min_val = min_val;
  return 0;
}

void adc_cleanup(struct adc_channel *channel) {
  // Nothing
}

float adc_get_voltage(struct adc_channel *channel, int *err) {
  uint32_t mv = 0; // millivolt
  FILE *fd = fopen(channel->fname, "r");
  if(fd == NULL) {
    fprintf(stderr, "Could not open file %s: %s\n", channel->fname, strerror(errno));
    *err = 1;
  }
  fscanf(fd, "%" PRIu32, &mv);
  fclose(fd);
  return mv/1000.0;
}

float adc_get_percentage(struct adc_channel *channel, int *err) {
  return ((adc_get_voltage(channel, err) - channel->min_val)
          /(channel->max_val - channel->min_val));
}
