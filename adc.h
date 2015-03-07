#ifndef _ADC_H_
#define _ADC_H_
#include <stdio.h>
#include <stdint.h>

#define MAX_ADC_VOLTAGE 1.8f

struct adc_channel {
  char fname[50];
  float min_val;
  float max_val;
};

int adc_init(struct adc_channel *channel, char *sys_directory, uint8_t adc_num, float min_val, float max_val);
void adc_cleanup(struct adc_channel *channel);
float adc_get_voltage(struct adc_channel *channel, int *err);
float adc_get_percentage(struct adc_channel *channel, int *err);

#endif /* _ADC_H_ */
