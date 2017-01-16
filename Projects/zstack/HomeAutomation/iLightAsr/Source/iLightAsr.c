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
#include "iLightAsr.h"
#include "iLightAsr_measure.h"

#define NWK_LED		HAL_LED_2
#define ASR_LED		HAL_LED_1


static uint8 iLightAsr_taskId = 0;
static uint8 iLightAsr_nwkState = 0;
static uint8 iLightAsr_measure_results[5] = 0;
static uint8 iLightAsr_transId = 0;


const cId_t iLightAsr_inClusterList[] = {0};
const cId_t iLightAsr_outClusterList[] = {0};


static SimpleDescriptionFormat_t iLightAsr_simpleDesc = {
  8, // endpoint
  0, // profile id
  0x0302, // device id
  0, // version
  0, // reserved
  1,
  (cId_t *)iLightAsr_inClusterList,
  1,
  (cId_t *)iLightAsr_outClusterList,
};

static endPointDesc_t iLightAsr_epDesc = {
  8,
  &iLightAsr_taskId,
  &iLightAsr_simpleDesc,
  (afNetworkLatencyReq_t)0,
};

void iLightAsr_feedback(void) {
  afAddrType_t destAddr;
  iLight_appMsg_sensor_asr_feedback_t * pFeedback = NULL;
  afStatus_t sendResult;

  pFeedback = (iLight_appMsg_sensor_asr_feedback_t *)osal_mem_alloc(sizeof(iLight_appMsg_sensor_asr_feedback_t));
  if (!pFeedback) return;

  destAddr.addr.shortAddr = 0;
  destAddr.addrMode = afAddr16Bit;
  destAddr.endPoint = 8;

  pFeedback->cmdId = ILIGHT_APPMSG_SENSOR_ASR_FEEDBACK;
  pFeedback->result0= iLightAsr_measure_results[0];
  
  sendResult = AF_DataRequest(
  	&destAddr,
  	&iLightAsr_epDesc,
  	ILIGHT_APPMSG_CLUSTER,
  	sizeof(iLight_appMsg_sensor_asr_feedback_t),
  	(uint8 *)pFeedback,
  	&iLightAsr_transId,
  	AF_ACK_REQUEST | AF_SUPRESS_ROUTE_DISC_NETWORK,
  	AF_DEFAULT_RADIUS);
  
  (void)sendResult;
  osal_mem_free(pFeedback);
}


void iLightAsr_measure_callback(uint8 msg, uint8 status) {
	switch (msg) {
		case ASR_MSG_FIND:
			// blink the LED twice
			HalLedBlink(ASR_LED, 2, 50, 500);
			iLightAsr_measure_results[0] = status;
			// never feedback unless result>0
			if (status > 0) iLightAsr_feedback();
			break;
		case ASR_MSG_ERROR:
			// turn on the LED
			HalLedSet(ASR_LED, HAL_LED_MODE_ON);
	}
}

void iLightAsr_init(uint8 taskId) {
  iLightAsr_taskId = taskId;
  afRegister(&iLightAsr_epDesc);
	// register measure callback
	asr_register_msg_callback(iLightAsr_measure_callback);
}


void iLightAsr_ProcessAirMsg(afIncomingMSGPacket_t * MSGpkt) {
  uint16 cluster = MSGpkt->clusterId;
  uint8 cmdId = 0;
  uint8 * pData = NULL;
  if (cluster != ILIGHT_APPMSG_CLUSTER) return;
  pData = MSGpkt->cmd.Data;
  cmdId = appMsg_getCmdId(pData);
  switch(cmdId) {
  	case ILIGHT_APPMSG_SENSOR_ASR_FETCH:
			iLightAsr_feedback();
		  break;
  }
}


void iLightAsr_ProcessKeys(uint8 shift, uint8 keys) {}


void iLightAsr_ProcessStateChange(devStates_t state) {
	iLightAsr_nwkState = state;
	switch (state) {
		case DEV_INIT:
			// flash slowly
			HalLedBlink(NWK_LED, 0, 30, 1000);
			break;
		case DEV_NWK_DISC:
		case DEV_NWK_JOINING:
		case DEV_END_DEVICE_UNAUTH:
			// flash quickly
			HalLedBlink(NWK_LED, 0, 30, 500);
			break;
		case DEV_END_DEVICE:
		case DEV_ROUTER:
			// turn on
			HalLedSet(NWK_LED, HAL_LED_MODE_ON);
			break;
	}
}


uint16 iLightAsr_event_loop(uint8 taskId, uint16 events) {
  if ( events & SYS_EVENT_MSG )
  {
  	afIncomingMSGPacket_t * MSGpkt = NULL;
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( iLightAsr_taskId )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case AF_INCOMING_MSG_CMD:
				  iLightAsr_ProcessAirMsg((afIncomingMSGPacket_t * )MSGpkt);
				  break;

				case KEY_CHANGE:
				  iLightAsr_ProcessKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
				  break;
		
        case ZDO_STATE_CHANGE:
          iLightAsr_ProcessStateChange((devStates_t)MSGpkt->hdr.status);
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }
  return 0;
}

