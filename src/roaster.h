#ifndef _ROASTER_H_
#define _ROASTER_H_

struct measurement {
  int pop;
  float temp;
  float knop_pct;
};

enum system_state { IDLE, ACTIVE };

#endif /* _ROASTER_H_ */
