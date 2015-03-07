#include "sensor_thermocouple.h"
#include "max31855.h"
#include <stdio.h>

int sensor_thermocouple_init(void) {
  /* Initialize thermocouple */
  if(max31855_init("/dev/spidev1.0")) {
    fprintf(stderr, "Error: Thermocouple SPI initialization failed\n");
    return -1;
  }
  return 0;
}

void sensor_thermocouple_cleanup(void) {
  max31855_cleanup();
}

int sensor_thermocouple_update_measurement(struct measurement *meas) {
  int err = 0;
  float temp = max31855_get_temp(&err);
  if(!err) {
    meas->temp = temp;
    return 1;
  }
  return 0;
}

const struct sensor_operations sensor_thermocouple_ops = {
  .init = sensor_thermocouple_init,
  .cleanup = sensor_thermocouple_cleanup,
  .update_measurement = sensor_thermocouple_update_measurement,
};

const struct sensor_descriptor sensor_thermocouple_desc = {
  .identifier = "thermocouple",
  .meas_freq_ms = 500,
  .sops = &sensor_thermocouple_ops,
};
