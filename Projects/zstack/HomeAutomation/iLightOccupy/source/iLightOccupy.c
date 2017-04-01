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
#include "iLightOccupy.h"
#include "iLightOccupy_measure.h"

static uint8 iLightOccupy_taskId = 0;
static uint8 iLightOccupy_nwkState = 0;
static uint8 iLightOccupy_lastOccupy = 0;
static uint8 iLightOccupy_transId = 0;

const cId_t iLightOccupy_inClusterList[] = {0};
const cId_t iLightOccupy_outClusterList[] = {0};

static SimpleDescriptionFormat_t iLightOccupy_simpleDesc = {
    8,      // endpoint
    0,      // profile id
    0x0303, // device id
    0,      // version
    0,      // reserved
    1,
    (cId_t *)iLightOccupy_inClusterList,
    1,
    (cId_t *)iLightOccupy_outClusterList,
};

static endPointDesc_t iLightOccupy_epDesc = {
    8,
    &iLightOccupy_taskId,
    &iLightOccupy_simpleDesc,
    (afNetworkLatencyReq_t)0,
};

void iLightOccupy_feedbackOccupy(void)
{
  afAddrType_t destAddr;
  iLight_appMsg_sensor_occupy_feedback_t *pFeedback = NULL;
  afStatus_t sendResult;

  pFeedback = (iLight_appMsg_sensor_occupy_feedback_t *)osal_mem_alloc(sizeof(iLight_appMsg_sensor_occupy_feedback_t));
  if (!pFeedback)
    return;

  destAddr.addr.shortAddr = 0;
  destAddr.addrMode = afAddr16Bit;
  destAddr.endPoint = 8;

  pFeedback->cmdId = ILIGHT_APPMSG_SENSOR_OCCUPY_FEEDBACK;
  pFeedback->occupy = iLightOccupy_measure_read_occupy();

  sendResult = AF_DataRequest(
      &destAddr,
      &iLightOccupy_epDesc,
      ILIGHT_APPMSG_CLUSTER,
      sizeof(iLight_appMsg_sensor_occupy_feedback_t),
      (uint8 *)pFeedback,
      &iLightOccupy_transId,
      AF_ACK_REQUEST | AF_SUPRESS_ROUTE_DISC_NETWORK,
      AF_DEFAULT_RADIUS);

  (void)sendResult;
  osal_mem_free(pFeedback);
}

void iLightOccupy_init(uint8 taskId)
{
  iLightOccupy_taskId = taskId;
  afRegister(&iLightOccupy_epDesc);
}

void iLightOccupy_ProcessAirMsg(afIncomingMSGPacket_t *MSGpkt)
{
  uint16 cluster = MSGpkt->clusterId;
  uint8 cmdId = 0;
  uint8 *pData = NULL;
  if (cluster != ILIGHT_APPMSG_CLUSTER)
    return;
  pData = MSGpkt->cmd.Data;
  cmdId = appMsg_getCmdId(pData);
  switch (cmdId)
  {
  case ILIGHT_APPMSG_SENSOR_OCCUPY_FETCH:
    iLightOccupy_feedbackOccupy();
    break;
  }
}

void iLightOccupy_ProcessKeys(uint8 shift, uint8 keys) {}

void iLightOccupy_ProcessStateChange(devStates_t state)
{
  iLightOccupy_nwkState = state;
  switch (state)
  {
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
    osal_start_timerEx(iLightOccupy_taskId, ILIGHT_OCCUPY_CHEAK, 5000);
    break;
  }
}

uint16 iLightOccupy_event_loop(uint8 taskId, uint16 events)
{
  if (events & SYS_EVENT_MSG)
  {
    afIncomingMSGPacket_t *MSGpkt = NULL;
    while ((MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(iLightOccupy_taskId)))
    {
      switch (MSGpkt->hdr.event)
      {
      case AF_INCOMING_MSG_CMD:
        iLightOccupy_ProcessAirMsg((afIncomingMSGPacket_t *)MSGpkt);
        break;

      case KEY_CHANGE:
        iLightOccupy_ProcessKeys(((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys);
        break;

      case ZDO_STATE_CHANGE:
        iLightOccupy_ProcessStateChange(MSGpkt->hdr.status);
        break;
      }

      // Release the memory
      osal_msg_deallocate((uint8 *)MSGpkt);
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if (events & ILIGHT_OCCUPY_CHEAK)
  {
    uint8 current = iLightOccupy_measure_read_occupy();
    if (current != iLightOccupy_lastOccupy)
    {
      HalLedSet(HAL_LED_1, current == 0 ? HAL_LED_MODE_OFF : HAL_LED_MODE_ON);
      iLightOccupy_lastOccupy = current;
      iLightOccupy_feedbackOccupy();
    }
    osal_start_timerEx(iLightOccupy_taskId, ILIGHT_OCCUPY_CHEAK, ILIGHT_OCCUPY_CHEAK_INTERVAL);
    return events ^ ILIGHT_OCCUPY_CHEAK;
  }
  return 0;
}
