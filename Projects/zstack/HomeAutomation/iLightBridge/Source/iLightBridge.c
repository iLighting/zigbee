#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDConfig.h"
#include "MT.h"
#include "MT_SYS.h"
#include "MT_APP.h"
#include "hal_led.h"

#include "iLightBridge.h"

static byte iLightBridge_taskId = 0;
static byte iLightBridge_nwkState = 0;
static byte iLightBridge_transId = 0;

const cId_t iLightBridge_InClusterList[] = {
  0
};

const cId_t iLightBridge_OutClusterList[] = {
  0
};

static SimpleDescriptionFormat_t iLightBridge_SimpleDesc = {
  8, // end point
  0, // profile id
  0, // device id
  0, // device version
  0, // device flag
  1, // in count
  (cId_t *)iLightBridge_InClusterList,
  1, // out count
  (cId_t *)iLightBridge_OutClusterList,  
};

static endPointDesc_t iLightBridge_EpDesc = {
  8,
  &iLightBridge_taskId,
  (SimpleDescriptionFormat_t *)&iLightBridge_SimpleDesc,
  (afNetworkLatencyReq_t)0
};

// 0: power up
// 1: external
// 2: watch dog
// 3: running
void iLightBridge_SysResetInd(uint8 reason) {
	struct {
		uint8 reason;
		uint8 transportRev;
		uint8 productId;
		uint8 majorRel;
		uint8 minorRel;
		uint8 hwRev;
	} sys_reset_ind;
	sys_reset_ind.reason = reason;
	sys_reset_ind.transportRev = 0;
	sys_reset_ind.productId = 0;
	sys_reset_ind.majorRel = 0;
	sys_reset_ind.minorRel = 0;
	sys_reset_ind.hwRev = 0;
	MT_BuildAndSendZToolResponse(0x41,0x80,sizeof(sys_reset_ind), (uint8 *)&sys_reset_ind);
}

void iLightBridge_init(byte taskId) {
  iLightBridge_taskId = taskId;
  afRegister(&iLightBridge_EpDesc);
  iLightBridge_SysResetInd(3);
}


// inject参数，上传至local server(AppMsgFeedback)
static void iLightBridge_ProcessAirMsg(afIncomingMSGPacket_t * pMsg) {
	uint8 feedbackSize = 2+1+2+2+pMsg->cmd.DataLength;
  // This is a bug?
	// uint8 * pFeedback = osal_mem_alloc(feedbackSize);
  
  uint8 pFeedback[6+20] = {0};

	if (pFeedback != NULL) {
		// remoteNwk
		pFeedback[0] = LO_UINT16(pMsg->srcAddr.addr.shortAddr);
		pFeedback[1] = HI_UINT16(pMsg->srcAddr.addr.shortAddr);
		// remoteEp
		pFeedback[2] = pMsg->endPoint;
		// clusterId
		pFeedback[3] = LO_UINT16(pMsg->clusterId);
		pFeedback[4] = HI_UINT16(pMsg->clusterId);
		// msgLen
		pFeedback[5] = LO_UINT16(pMsg->cmd.DataLength);
		pFeedback[6] = HI_UINT16(pMsg->cmd.DataLength);
		// pData
		osal_memcpy(pFeedback+7, pMsg->cmd.Data, pMsg->cmd.DataLength);
		
	  MT_BuildAndSendZToolResponse(
			0x49, 0,
			feedbackSize,
			pFeedback);
		// osal_mem_free(pFeedback);
	}else {
		;
	}
	
}


static void iLightBridge_ProcessAppMsg(mtSysAppMsg_t *pMsg) {
  uint8 appDataLen = pMsg->appDataLen;
  uint8 * pData = pMsg->appData;
  uint8 selfEp;
  uint16 destNwk;
  uint8 destEp;
  uint16 clusterId;
	uint8 len;
  uint8 * pAppPayload = NULL;
  afAddrType_t destAddr;
  afStatus_t sendResult;
  
  // parse
  // ---------------------
	selfEp = pMsg->endpoint;
	destNwk = BUILD_UINT16(pData[0], pData[1]);
	destEp = pData[2];
	clusterId = BUILD_UINT16(pData[3], pData[4]);
	len = pData[5];
	pAppPayload = &pData[6];

  // send
  // ---------------------
  destAddr.addr.shortAddr = destNwk;
  destAddr.addrMode = afAddr16Bit;
  destAddr.endPoint = destEp;
  // destAddr.panId = 0;
  sendResult = AF_DataRequest(
    (afAddrType_t * )&destAddr,
    (endPointDesc_t *)&iLightBridge_EpDesc,
    clusterId,
    len,
    (uint8 *)pAppPayload,
    (uint8 *)&iLightBridge_transId,
    AF_ACK_REQUEST | AF_SUPRESS_ROUTE_DISC_NETWORK,
    AF_DEFAULT_RADIUS);
  switch (sendResult) {
  	case afStatus_SUCCESS:
      break;
    default:
      break;
  }
}

void iLightBridge_ProcessStateChange(devStates_t state) {
	iLightBridge_nwkState = state;
	switch (state) {
		case DEV_INIT:
			// flash slowly
			HalLedBlink(HAL_LED_1, 0, 30, 1000);
			break;
		case DEV_COORD_STARTING:
		case DEV_ZB_COORD:
			// turn on
			HalLedSet(HAL_LED_1, HAL_LED_MODE_ON);
			break;
	}
}


uint16 iLightBridge_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( iLightBridge_taskId )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case AF_INCOMING_MSG_CMD:
          iLightBridge_ProcessAirMsg((afIncomingMSGPacket_t *)MSGpkt);
          break;
		
        case ZDO_STATE_CHANGE:
          iLightBridge_ProcessStateChange((devStates_t)(MSGpkt->hdr.status));
          break;
        
        case MT_SYS_APP_MSG:
          iLightBridge_ProcessAppMsg( (mtSysAppMsg_t * )MSGpkt );
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

  // Discard unknown events
  return 0;
}