#include "sensor_button.h"
#include "btn.h"
#include "sensor_events.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

struct btn_channel btn_ch;
pthread_t poll_thread;

void *btn_poll_thread(void *args);

int sensor_button_init(void) {
  if(btn_init(&btn_ch, (0*32+23))) { // P8_13, GPIO0_23
    fprintf(stderr, "Error: Button initialization failed\n");
    return -1;
  }
  if(btn_intrpt_rising_enable(&btn_ch)) {
    fprintf(stderr, "Error: Button interrupt config failed\n");
    btn_cleanup(&btn_ch);
    return -1;
  }

  /* Start button poll thread */
  if(pthread_create(&poll_thread, NULL, btn_poll_thread, NULL) != 0) {
    fprintf(stderr, "Error creating peripheral handler\n");
  }
  
  return 0;
}

void *btn_poll_thread(void *args) {
  while(1) {
#ifdef DEBUG
    printf("Waiting for button interrupt\n");
#endif
    int val = btn_intrpt_poll(&btn_ch, 10000);
    if(val == 1) {
      /* Button pressed */
      int hold_cnt = 1;
      while(hold_cnt > 0 && hold_cnt < 10) {
        usleep(100000); /* Wait 0.1 s */
        if(btn_get_value(&btn_ch) == 1) {
          hold_cnt++;
        } else {
          hold_cnt = 0;
        }
      }
      if(hold_cnt == 0) {
        /* Short press */
        if(sq_enqueue(SENSOR_EVENT_BTN_PRESSED_SHORT, NULL, 0, &sensor_button_desc)) {
          fprintf(stderr, "Error enqueuing button event\n");
        }
      } else {
#ifdef DEBUG
        printf("Button long press\n");
#endif
        /* Long press */
        if(sq_enqueue(SENSOR_EVENT_BTN_PRESSED_LONG, NULL, 0, &sensor_button_desc)) {
          fprintf(stderr, "Error enqueuing button event\n");
        }
      }
    }
  }
}

void sensor_button_cleanup(void) {
  pthread_cancel(poll_thread);
  btn_intrpt_disable(&btn_ch);
  btn_cleanup(&btn_ch);
}

const struct sensor_operations sensor_button_ops = {
  .init = sensor_button_init,
  .cleanup = sensor_button_cleanup,
};

const struct sensor_descriptor sensor_button_desc = {
  .identifier = "button",
  .sops = &sensor_button_ops,
};
