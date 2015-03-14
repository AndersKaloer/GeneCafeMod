#ifndef _LOG_SERVER_H_
#define _LOG_SERVER_H_

#include <sys/socket.h>
#include <stdint.h>

#define LOGSERVER_PORT "12000"

struct log_entry {
  uint8_t pop;
  float temp;
  float knop_val;
  uint32_t time_ms;
};

int logserver_start(void);
void logserver_cleanup(void);
void broadcast_log(struct log_entry *entry);

#endif /* _LOG_SERVER_H_ */
