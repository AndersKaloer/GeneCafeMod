#include "actuator_lcd.h"
#include "lcd.h"
#include <time.h>
#include <stdio.h>

void actuator_lcd_new_time_hook(struct timespec abs_time, struct timespec rel_time);
void actuator_lcd_new_meas_hook(struct measurement *meas);
void actuator_lcd_new_state_hook(enum system_state old_state, enum system_state new_state);

int actuator_lcd_init(void) {
  /* Initialize lcd */
  if(lcd_init()) {
    fprintf(stderr, "Error: LCD initialization failed\n");
    return -1;
  }
  struct timespec time_zero = {0};
  actuator_lcd_new_time_hook(time_zero, time_zero);
  struct measurement meas = {0};
  actuator_lcd_new_meas_hook(&meas);
  actuator_lcd_new_state_hook(IDLE, IDLE);
  return 0;
}

void actuator_lcd_cleanup(void) {
  lcd_cleanup();
}

void actuator_lcd_new_meas_hook(struct measurement *meas) {  
  /* Write temp */
  lcd_goto(0, 7);
  lcd_write("%3.0f""\xDF", meas->temp);
  
  /* Write heater pct */
  lcd_goto(0, 12);
  lcd_write("%3.0f%%", meas->knop_pct*100.0);
}

void actuator_lcd_new_time_hook(struct timespec abs_time, struct timespec rel_time) {
  /* Write time */
  int tot_time_sec = abs_time.tv_sec;
  int tot_time_min = tot_time_sec/60;
  tot_time_sec = (tot_time_sec % 60);
  int rel_time_sec = rel_time.tv_sec;
  int rel_time_min = rel_time_min = rel_time_sec/60;
  rel_time_sec = (rel_time_sec % 60);
  lcd_goto(0, 0);
  lcd_write("%02d:%02d", tot_time_min, tot_time_sec);
  lcd_goto(1, 0);
  lcd_write("%02d:%02d", rel_time_min, rel_time_sec);  
}

void actuator_lcd_new_state_hook(enum system_state old_state, enum system_state new_state) {
  lcd_goto(1, 7);
  switch(new_state) {
  case IDLE:
    lcd_write("%9s", "Start\x7E");
    break;
  case ACTIVE:
    lcd_write("%9s", "Crck/Stp\x7E");
    break;
  }
}
const struct actuator_operations actuator_lcd_ops = {
  .init = actuator_lcd_init,
  .cleanup = actuator_lcd_cleanup,
  .new_measurement_hook = actuator_lcd_new_meas_hook,
  .new_time_hook = actuator_lcd_new_time_hook,
  .new_state_hook = actuator_lcd_new_state_hook,
};

const struct actuator_descriptor actuator_lcd_desc = {
  .identifier = "lcd",
  .aops = &actuator_lcd_ops,
};
