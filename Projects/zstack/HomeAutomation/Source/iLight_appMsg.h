#ifndef _ILIGHT_APPMSG_H_
#define _ILIGHT_APPMSG_H_

#define ILIGHT_APPMSG_LAMP_CLUSTER 0xff00
#define ILIGHT_APPMSG_LAMP_ONOFF 0
#define ILIGHT_APPMSG_LAMP_ONOFF_FEEDBACK 1

typedef struct {
  uint8 cmdId;
  uint8 onOff;
} iLight_appMsg_lamp_onOff_t;

typedef struct {
  uint8 cmdId;
  uint8 onOff;
} iLight_appMsg_lamp_onOff_feedback_t;


extern uint8 appMsg_getCmdId(uint8 * pData);

#endif