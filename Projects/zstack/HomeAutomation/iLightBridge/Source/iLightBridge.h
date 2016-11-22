#ifndef ILIGHT_BRIDGE_H
#define ILIGHT_BRIDGE_H

#include "ZComDef.h"


typedef struct {
  uint16   remoteNwk;
  uint8    remoteEp;
  uint16   clusterId;
  uint8    msgLen;
  uint8 *  pData;
} iLightBridge_feedback_t;


extern void iLightBridge_init(byte taskId);
extern uint16 iLightBridge_event_loop(byte taskId, uint16 events);

#endif