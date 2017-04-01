#ifndef _ILIGHT_OCCUPY_MEASURE_H_
#define _ILIGHT_OCCUPY_MEASURE_H_

#define ILIGHT_OCCUPY_INTERVAL 200

// event id
#define ILIGHT_OCCUPY_PULL (1 << 0)

extern void iLightOccupy_measure_init(byte task_id);
extern int16 iLightOccupy_measure_read_occupy(void);
extern uint16 iLightOccupy_measure_event_loop(uint8 task_id, uint16 events);

#endif