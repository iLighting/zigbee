#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDConfig.h"
#include "MT_SYS.h"
#include "MT_APP.h"

#include "iLightBridge.h"

static byte iLightBridge_taskId = 0;
static byte iLightBridge_nwkState = 0;

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

void iLightBridge_init(byte taskId) {
  iLightBridge_taskId = taskId;
  afRegister(&iLightBridge_EpDesc);
}


static void iLightBridge_ProcessAppMsg(mtSysAppMsg_t *pMsg) {
  uint8 appDataLen = pMsg->appDataLen;
  uint8 * pData = pMsg->appData;
  uint8 selfEp;
  uint16 destNwk;
  uint8 destEp;
  uint16 clusterId;
  uint8 * pAppPayload = NULL;
  
  // parse
  // ---------------------
  selfEp = *pData++;
  destNwk = BUILD_UINT16(pData[0], pData[1]);
  pData += 2;
  destEp = *pData++;
  clusterId = BUILD_UINT16(pData[0], pData[1]);
  pData += 2;
  pAppPayload = pData;

  // send
  // ---------------------
  
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
        case ZDO_STATE_CHANGE:
          iLightBridge_nwkState = (devStates_t)(MSGpkt->hdr.status);
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