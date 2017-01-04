#ifndef _ILIGHT_ILLUMINANCE_MEASURE_H_
#define _ILIGHT_ILLUMINANCE_MEASURE_H_

#include "ZComDef.h"

#define BH1750_ADDR  0x23  // ADDR_PIN == L. if H, 0x5c instead

// event id
#define ILIGHT_ILLUMINANCE_MEASURE_START				0x0001
#define ILIGHT_ILLUMINANCE_MEASURE_START_DONE		0x0002

extern void iLightIlluminance_measure_init(byte task_id);
extern uint16 iLightIlluminance_measure_event_loop( uint8 task_id, uint16 events );

extern int16 iLightIlluminance_measure_read(void);

#endif