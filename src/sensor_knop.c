#include "sensor_knop.h"
#include "adc.h"
#include <stdio.h>

struct adc_channel adc6_ch;

int sensor_knop_init(void) {
  /* Initialize ADC for knop */
  if(adc_init(&adc6_ch, "/sys/bus/iio/devices/iio:device0", 6, 0, 1.634)) {
    fprintf(stderr, "Error: Knop ADC initialization failed\n");
    return -1;
  }
  return 0;
}

void sensor_knop_cleanup(void) {
  adc_cleanup(&adc6_ch);
}

int sensor_knop_update_measurement(struct measurement *meas) {
  int err = 0;
  float pct = adc_get_percentage(&adc6_ch, &err);
  if(!err) {
    meas->knop_pct = pct;
    return 1;
  }
  return 0;
}

const struct sensor_operations sensor_knop_ops = {
  .init = sensor_knop_init,
  .cleanup = sensor_knop_cleanup,
  .update_measurement = sensor_knop_update_measurement,
};

const struct sensor_descriptor sensor_knop_desc = {
  .identifier = "knop",
  .meas_freq_ms = 100,
  .sops = &sensor_knop_ops,
};
