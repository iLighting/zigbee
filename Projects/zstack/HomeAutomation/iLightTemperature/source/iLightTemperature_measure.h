#ifndef _ILIGHT_TEMPERATURE_MEASURE_H_
#define _ILIGHT_TEMPERATURE_MEASURE_H_


#define ILIGHT_TEMPERATURE_INTERVAL  5000

// event id
#define ILIGHT_TEMPERATURE_START_TRANS 					(1<<0)
#define ILIGHT_TEMPERATURE_START_TRANS_DATA			(1<<1)


extern void iLightTemperature_measure_init(byte task_id);
extern int16 iLightTemperature_measure_read_temperature(void);
extern uint16 iLightTemperature_measure_event_loop( uint8 task_id, uint16 events );


#endif