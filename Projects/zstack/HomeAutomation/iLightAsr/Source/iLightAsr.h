#ifndef _iLightAsr_H_
#define _iLightAsr_H_

#include "ZComDef.h"


#define ILIGHT_ILLUMINANCE_CHEAK_THRESHOLD	5  // 5 lux
#define ILIGHT_ILLUMINANCE_CHEAK_INTERVAL		1000 // 1s


// event id
#define ILIGHT_ILLUMINANCE_CHEAK  (1<<0)


extern void iLightAsr_init(uint8 taskId);
extern uint16 iLightAsr_event_loop(uint8 taskId, uint16 events);


#endif
