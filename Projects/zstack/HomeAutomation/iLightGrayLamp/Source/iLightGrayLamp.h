#ifndef _ILIGHT_GRAY_LAMP_H_
#define _ILIGHT_GRAY_LAMP_H_

#define LEVEL_MAX                 0xFE
#define LEVEL_MIN                 0x0
#define GAMMA_VALUE               2
#define PWM_FULL_DUTY_CYCLE       100

extern void iLightGrayLamp_init(uint8 taskId);
extern uint16 iLightGrayLamp_event_loop(uint8 taskId, uint16 events);


#endif
