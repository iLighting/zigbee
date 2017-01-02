#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDConfig.h"
#include "MT_SYS.h"
#include "MT_APP.h"
#include "hal_key.h"
#include "hal_led.h"
#include "OnBoard.h"

#include "iLight_appMsg.h"
#include "iLightTemperature.h"
#include "iLightTemperature_measure.h"


static uint8 iLightTemperature_taskId = 0;
static uint8 iLightTemperature_nwkState = 0;
static int16 iLightTemperature_lastTemperature = 0;
static uint8 iLightTemperature_transId = 0;


const cId_t iLightTemperature_inClusterList[] = {0};
const cId_t iLightTemperature_outClusterList[] = {0};


static SimpleDescriptionFormat_t iLightTemperature_simpleDesc = {
  8, // endpoint
  0, // profile id
  0x0301, // device id
  0, // version
  0, // reserved
  1,
  (cId_t *)iLightTemperature_inClusterList,
  1,
  (cId_t *)iLightTemperature_outClusterList,
};

static endPointDesc_t iLightTemperature_epDesc = {
  8,
  &iLightTemperature_taskId,
  &iLightTemperature_simpleDesc,
  (afNetworkLatencyReq_t)0,
};

void iLightTemperature_feedbackTemperature(void) {
  afAddrType_t destAddr;
  iLight_appMsg_sensor_temperature_feedback_t * pFeedback = NULL;
  afStatus_t sendResult;

  pFeedback = (iLight_appMsg_sensor_temperature_feedback_t *)osal_mem_alloc(sizeof(iLight_appMsg_sensor_temperature_feedback_t));
  if (!pFeedback) return;

  destAddr.addr.shortAddr = 0;
  destAddr.addrMode = afAddr16Bit;
  destAddr.endPoint = 8;

  pFeedback->cmdId = ILIGHT_APPMSG_SENSOR_TEMPERATURE_FEEDBACK;
  pFeedback->temperature= iLightTemperature_measure_read_temperature();
  
  sendResult = AF_DataRequest(
  	&destAddr,
  	&iLightTemperature_epDesc,
  	ILIGHT_APPMSG_CLUSTER,
  	sizeof(iLight_appMsg_sensor_temperature_feedback_t),
  	(uint8 *)pFeedback,
  	&iLightTemperature_transId,
  	AF_ACK_REQUEST | AF_SUPRESS_ROUTE_DISC_NETWORK,
  	AF_DEFAULT_RADIUS);
  
  (void)sendResult;
  osal_mem_free(pFeedback);
}


void iLightTemperature_init(uint8 taskId) {
  iLightTemperature_taskId = taskId;
  afRegister(&iLightTemperature_epDesc);
}


void iLightTemperature_ProcessAirMsg(afIncomingMSGPacket_t * MSGpkt) {
  uint16 cluster = MSGpkt->clusterId;
  uint8 cmdId = 0;
  uint8 * pData = NULL;
  if (cluster != ILIGHT_APPMSG_CLUSTER) return;
  pData = MSGpkt->cmd.Data;
  cmdId = appMsg_getCmdId(pData);
  switch(cmdId) {
  	case ILIGHT_APPMSG_SENSOR_TEMPERATURE_FETCH:
			iLightTemperature_feedbackTemperature();
		  break;
  }
}


void iLightTemperature_ProcessKeys(uint8 shift, uint8 keys) {}


void iLightTemperature_ProcessStateChange(devStates_t state) {
	iLightTemperature_nwkState = state;
	switch (state) {
		case DEV_INIT:
			// flash slowly
			HalLedBlink(HAL_LED_2, 0, 30, 1000);
			break;
		case DEV_NWK_DISC:
		case DEV_NWK_JOINING:
		case DEV_END_DEVICE_UNAUTH:
			// flash quickly
			HalLedBlink(HAL_LED_2, 0, 30, 500);
			break;
		case DEV_END_DEVICE:
		case DEV_ROUTER:
			// turn on
			HalLedSet(HAL_LED_2, HAL_LED_MODE_ON);
      // feedback by the time this was connected
      osal_start_timerEx(iLightTemperature_taskId, ILIGHT_TEMPERATURE_CHEAK, 5000);
			break;
	}
}


uint16 iLightTemperature_event_loop(uint8 taskId, uint16 events) {
  if ( events & SYS_EVENT_MSG )
  {
  	afIncomingMSGPacket_t * MSGpkt = NULL;
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( iLightTemperature_taskId )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case AF_INCOMING_MSG_CMD:
				  iLightTemperature_ProcessAirMsg((afIncomingMSGPacket_t * )MSGpkt);
				  break;

				case KEY_CHANGE:
				  iLightTemperature_ProcessKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
				  break;
		
        case ZDO_STATE_CHANGE:
          iLightTemperature_ProcessStateChange(MSGpkt->hdr.status);
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

	if ( events & ILIGHT_TEMPERATURE_CHEAK ) {
		int16 currentTemp = iLightTemperature_measure_read_temperature();
    int16 delta = currentTemp - iLightTemperature_lastTemperature;
    delta = delta < 0 ? -delta : delta;
		if ( delta >= ILIGHT_TEMPERATURE_CHEAK_THRESHOLD ) {
			iLightTemperature_lastTemperature = currentTemp;
			iLightTemperature_feedbackTemperature();
		}
		osal_start_timerEx(iLightTemperature_taskId, ILIGHT_TEMPERATURE_CHEAK, ILIGHT_TEMPERATURE_CHEAK_INTERVAL);
		return events ^ ILIGHT_TEMPERATURE_CHEAK;
	}
  return 0;
}

