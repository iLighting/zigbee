#ifndef _iLightIlluminance_H_
#define _iLightIlluminance_H_

#include "ZComDef.h"


#define ILIGHT_ILLUMINANCE_CHEAK_THRESHOLD	5  // 5 lux
#define ILIGHT_ILLUMINANCE_CHEAK_INTERVAL		1000 // 1s


// event id
#define ILIGHT_ILLUMINANCE_CHEAK  (1<<0)


extern void iLightIlluminance_init(uint8 taskId);
extern uint16 iLightIlluminance_event_loop(uint8 taskId, uint16 events);


#endif
