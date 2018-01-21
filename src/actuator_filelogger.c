#include "actuator_filelogger.h"
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

void actuator_filelogger_new_time_hook(struct timespec abs_time, struct timespec rel_time);
void actuator_filelogger_new_meas_hook(struct measurement *meas);
void actuator_filelogger_new_state_hook(enum system_state old_state, enum system_state new_state);

static char filename_buf[100];
static FILE *logfile;
static enum system_state state;
static struct measurement last_meas;
static int has_pop = 0;
static int has_meas = 0;

int actuator_filelogger_init(void) {
  return 0;
}

void actuator_filelogger_cleanup(void) {
}

void actuator_filelogger_new_meas_hook(struct measurement *meas) {  
  memcpy(&last_meas, meas, sizeof(struct measurement));
  has_pop = last_meas.pop; /* Ensure to register pop */
  has_meas = 1;
}

void actuator_filelogger_new_time_hook(struct timespec abs_time, struct timespec rel_time) {
  /* Write to file */
  if(state == ACTIVE && has_meas) {
          fprintf(logfile, "%0.3lf,%f,%f,%d\n", (double)abs_time.tv_sec + ((double)abs_time.tv_nsec)*1.0e-9,
            last_meas.knop_pct, last_meas.temp, has_pop);
    has_pop = 0;
  }
}

void actuator_filelogger_new_state_hook(enum system_state old_state, enum system_state new_state) {
  if(old_state == IDLE && new_state == ACTIVE) {
    /* Create new file */
    unsigned long curtime = time(NULL);
    sprintf(filename_buf, "roast_log/%ld", curtime);
    if(mkdir(filename_buf, S_IRWXU) == 0) {
      sprintf(filename_buf, "roast_log/%ld/roast.json", curtime);
      FILE *file = fopen(filename_buf, "w");
      if(file != NULL) {
        fprintf(file, "{"\
                "\"date\": %ld," \
                "\"coffee\": \"\"," \
                "\"roast_degree\": \"\"," \
                "\"pre_roast_weight\": 0," \
                "\"post_roast_weight\": 0," \
                "\"rating\": 0," \
                "\"comment\": \"\"" \
                "}\n", curtime);
        /* Force sync */
        fflush(file);
        fsync(fileno(file));
        fclose(file);
      }
      sprintf(filename_buf, "roast_log/%ld/roast.csv", curtime);
      logfile = fopen(filename_buf, "w");
      if(file != NULL) {
        fprintf(logfile, "time,heat,temp,crack\n");
      }
    }
  } else if(old_state == ACTIVE && new_state == IDLE) {
    /* Force sync */
    fflush(logfile);
    fsync(fileno(logfile));
    /* Close file */
    fclose(logfile);
  }
  state = new_state;
}

const struct actuator_operations actuator_filelogger_ops = {
  .init = actuator_filelogger_init,
  .cleanup = actuator_filelogger_cleanup,
  .new_measurement_hook = actuator_filelogger_new_meas_hook,
  .new_time_hook = actuator_filelogger_new_time_hook,
  .new_state_hook = actuator_filelogger_new_state_hook,
};

const struct actuator_descriptor actuator_filelogger_desc = {
  .identifier = "filelogger",
  .aops = &actuator_filelogger_ops,
};
