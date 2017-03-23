#ifndef _ILIGHT_APPMSG_H_
#define _ILIGHT_APPMSG_H_

#define ILIGHT_APPMSG_CLUSTER 0xff00

#define ILIGHT_APPMSG_LAMP_ONOFF 						0
#define ILIGHT_APPMSG_LAMP_ONOFF_FEEDBACK 	1

#define ILIGHT_APPMSG_GRAY_LAMP_CHANGE 						0
#define ILIGHT_APPMSG_GRAY_LAMP_CHANGE_FEEDBACK 	1

#define ILIGHT_APPMSG_SENSOR_TEMPERATURE_FETCH 			0
#define ILIGHT_APPMSG_SENSOR_TEMPERATURE_FEEDBACK 	1

#define ILIGHT_APPMSG_SENSOR_ILLUMINANCE_FETCH 			0
#define ILIGHT_APPMSG_SENSOR_ILLUMINANCE_FEEDBACK 	1

#define ILIGHT_APPMSG_SENSOR_ASR_FETCH 			0
#define ILIGHT_APPMSG_SENSOR_ASR_FEEDBACK 	1

#define ILIGHT_APPMSG_SENSOR_OCCUPY_FETCH 0
#define ILIGHT_APPMSG_SENSOR_OCCUPY_FEEDBACK 1

// lamp
// ====================
typedef struct {
  uint8 cmdId;
  uint8 onOff;
} iLight_appMsg_lamp_onOff_t;

typedef struct {
  uint8 cmdId;
  uint8 onOff;
} iLight_appMsg_lamp_onOff_feedback_t;

// gray lamp
// ====================
typedef struct {
  uint8 cmdId;
  uint8 level;
} iLight_appMsg_gray_lamp_change_t;

typedef struct {
  uint8 cmdId;
  uint8 level;
} iLight_appMsg_gray_lamp_change_feedback_t;

// sensor temperature
// ====================
typedef struct {
	uint8 cmdId;
} iLight_appMsg_sensor_temperature_fetch_t;

typedef struct {
	uint8 cmdId;
	int16 temperature;
} iLight_appMsg_sensor_temperature_feedback_t;

// sensor illuminance
// ====================
typedef struct {
	uint8 cmdId;
} iLight_appMsg_sensor_illuminance_fetch_t;

typedef struct {
	uint8 cmdId;
	int16 temperature;
} iLight_appMsg_sensor_illuminance_feedback_t;

// sensor asr
// ===================
typedef struct {
	uint8 cmdId;
	int8 result0;
} iLight_appMsg_sensor_asr_feedback_t;

// sensor occupy
// ===================
typedef struct {
	uint8 cmdId;
} iLight_appMsg_sensor_occupy_fetch_t;

typedef struct {
	uint8 cmdId;
	uint8 occupy;
} iLight_appMsg_sensor_occupy_feedback_t;

extern uint8 appMsg_getCmdId(uint8 * pData);

#endif