#ifndef ILIGHT_BRIDGE_H
#define ILIGHT_BRIDGE_H

extern void iLightBridge_init(byte taskId);
extern uint16 iLightBridge_event_loop(byte taskId, uint16 events);

#endif