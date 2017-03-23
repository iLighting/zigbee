#ifndef _ILIGHTLAMP_H_
#define _ILIGHTLAMP_H_

#include "ZComDef.h"

#define ILIGHT_OCCUPY_CHEAK_INTERVAL 500 // 0.5s

// event id
#define ILIGHT_OCCUPY_CHEAK (1 << 0)

extern void iLightOccupy_init(uint8 taskId);
extern uint16 iLightOccupy_event_loop(uint8 taskId, uint16 events);

#endif
