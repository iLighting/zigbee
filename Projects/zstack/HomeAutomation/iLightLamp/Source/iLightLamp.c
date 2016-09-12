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


static uint8 iLightLamp_taskId = 0;
static uint8 iLightLamp_nwkState = 0;
static uint8 iLightLamp_currentOnOff = 0;
static uint8 iLightLamp_transId = 0;


const cId_t iLightLamp_inClusterList[] = {0};
const cId_t iLightLamp_outClusterList[] = {0};


static SimpleDescriptionFormat_t iLightLamp_simpleDesc = {
  8, // endpoint
  0, // profile id
  0, // device id
  0, // version
  0, // reserved
  1,
  (cId_t *)iLightLamp_inClusterList,
  1,
  (cId_t *)iLightLamp_outClusterList,
};

static endPointDesc_t iLightLamp_epDesc = {
  8,
  &iLightLamp_taskId,
  &iLightLamp_simpleDesc,
  (afNetworkLatencyReq_t)0,
};


void iLightLamp_feedbackOnOff() {
  afAddrType_t destAddr;
  iLight_appMsg_lamp_onOff_feedback_t * pFeedback = NULL;
  afStatus_t sendResult;

  pFeedback = (iLight_appMsg_lamp_onOff_feedback_t *)osal_msg_allocate(sizeof(iLight_appMsg_lamp_onOff_feedback_t));
  if (!pFeedback) return;

  destAddr.addr.shortAddr = 0;
  destAddr.addrMode = afAddr16Bit;
  destAddr.endPoint = 8;

  pFeedback->cmdId = ILIGHT_APPMSG_LAMP_ONOFF_FEEDBACK;
  pFeedback->onOff = iLightLamp_currentOnOff;
  
  sendResult = AF_DataRequest(
  	&destAddr,
  	&iLightLamp_epDesc,
  	ILIGHT_APPMSG_LAMP_CLUSTER,
  	sizeof(iLight_appMsg_lamp_onOff_feedback_t),
  	(uint8 *)pFeedback,
  	&iLightLamp_transId,
  	AF_ACK_REQUEST | AF_SUPRESS_ROUTE_DISC_NETWORK,
  	AF_DEFAULT_RADIUS);
  
  (void)sendResult;
}


void iLightLamp_updateOnOff() {
  HalLedSet(HAL_LED_1, iLightLamp_currentOnOff ? HAL_LED_MODE_ON : HAL_LED_MODE_OFF);
}


void iLightLamp_init(uint8 taskId) {
  iLightLamp_taskId = taskId;
  afRegister(&iLightLamp_epDesc);
}


void iLightLamp_ProcessAirMsg(afIncomingMSGPacket_t * MSGpkt) {
  uint16 cluster = MSGpkt->clusterId;
  uint8 cmdId = 0;
  if (cluster != ILIGHT_APPMSG_LAMP_CLUSTER) return;
  cmdId = appMsg_getCmdId(MSGpkt->cmd.Data);
  switch(cmdId) {
  	case ILIGHT_APPMSG_LAMP_ONOFF:
	  iLightLamp_currentOnOff = ((iLight_appMsg_lamp_onOff_t *) MSGpkt)->onOff;
	  iLightLamp_updateOnOff();
	  break;
  }
}


void iLightLamp_ProcessKeys(uint8 shift, uint8 keys) {
  if (keys & HAL_KEY_SW_1) {
	iLightLamp_currentOnOff = !iLightLamp_currentOnOff;
	iLightLamp_updateOnOff();
	iLightLamp_feedbackOnOff();
  }
}


uint16 iLightLamp_event_loop(uint8 taskId, uint16 events) {
  afIncomingMSGPacket_t * MSGpkt = NULL;
  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( iLightLamp_taskId )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case AF_INCOMING_MSG_CMD:
		  iLightLamp_ProcessAirMsg((afIncomingMSGPacket_t * )MSGpkt);
		  break;

		case KEY_CHANGE:
		  iLightLamp_ProcessKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
		  break;
		
        case ZDO_STATE_CHANGE:
          iLightLamp_nwkState = (devStates_t)(MSGpkt->hdr.status);
          break;

        default:
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

