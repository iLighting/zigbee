#ifndef _ILIGHTLAMP_H_
#define _ILIGHTLAMP_H_

#include "ZComDef.h"


#define ILIGHT_TEMPERATURE_CHEAK_THRESHOLD	(1*100)  // 1¡æ
#define ILIGHT_TEMPERATURE_CHEAK_INTERVAL		1000 // 1s



// event id
#define ILIGHT_TEMPERATURE_CHEAK  (1<<0)


extern void iLightTemperature_init(uint8 taskId);
extern uint16 iLightTemperature_event_loop(uint8 taskId, uint16 events);


#endif
