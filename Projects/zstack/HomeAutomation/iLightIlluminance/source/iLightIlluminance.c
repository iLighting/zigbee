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
#include "iLightIlluminance.h"
#include "iLightIlluminance_measure.h"

static uint8 iLightIlluminance_taskId = 0;
static uint8 iLightIlluminance_nwkState = 0;
static int16 iLightIlluminance_lastValue = 0;
static uint8 iLightIlluminance_transId = 0;

const cId_t iLightIlluminance_inClusterList[] = {0};
const cId_t iLightIlluminance_outClusterList[] = {0};

static SimpleDescriptionFormat_t iLightIlluminance_simpleDesc = {
    8,      // endpoint
    0,      // profile id
    0x0300, // device id
    0,      // version
    0,      // reserved
    1,
    (cId_t *)iLightIlluminance_inClusterList,
    1,
    (cId_t *)iLightIlluminance_outClusterList,
};

static endPointDesc_t iLightIlluminance_epDesc = {
    8,
    &iLightIlluminance_taskId,
    &iLightIlluminance_simpleDesc,
    (afNetworkLatencyReq_t)0,
};

void iLightIlluminance_feedback(void)
{
  afAddrType_t destAddr;
  iLight_appMsg_sensor_illuminance_feedback_t *pFeedback = NULL;
  afStatus_t sendResult;

  pFeedback = (iLight_appMsg_sensor_illuminance_feedback_t *)osal_mem_alloc(sizeof(iLight_appMsg_sensor_illuminance_feedback_t));
  if (!pFeedback)
    return;

  destAddr.addr.shortAddr = 0;
  destAddr.addrMode = afAddr16Bit;
  destAddr.endPoint = 8;

  pFeedback->cmdId = ILIGHT_APPMSG_SENSOR_ILLUMINANCE_FEEDBACK;
  pFeedback->temperature = iLightIlluminance_measure_read();

  sendResult = AF_DataRequest(
      &destAddr,
      &iLightIlluminance_epDesc,
      ILIGHT_APPMSG_CLUSTER,
      sizeof(iLight_appMsg_sensor_illuminance_feedback_t),
      (uint8 *)pFeedback,
      &iLightIlluminance_transId,
      AF_ACK_REQUEST | AF_SUPRESS_ROUTE_DISC_NETWORK,
      AF_DEFAULT_RADIUS);

  (void)sendResult;
  osal_mem_free(pFeedback);
}

void iLightIlluminance_init(uint8 taskId)
{
  iLightIlluminance_taskId = taskId;
  afRegister(&iLightIlluminance_epDesc);
}

void iLightIlluminance_ProcessAirMsg(afIncomingMSGPacket_t *MSGpkt)
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
  case ILIGHT_APPMSG_SENSOR_ILLUMINANCE_FETCH:
    iLightIlluminance_feedback();
    break;
  }
}

void iLightIlluminance_ProcessKeys(uint8 shift, uint8 keys) {}

void iLightIlluminance_ProcessStateChange(devStates_t state)
{
  iLightIlluminance_nwkState = state;
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
    osal_start_timerEx(iLightIlluminance_taskId, ILIGHT_ILLUMINANCE_CHEAK, 5000);
    break;
  }
}

uint16 iLightIlluminance_event_loop(uint8 taskId, uint16 events)
{
  if (events & SYS_EVENT_MSG)
  {
    afIncomingMSGPacket_t *MSGpkt = NULL;
    while ((MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(iLightIlluminance_taskId)))
    {
      switch (MSGpkt->hdr.event)
      {
      case AF_INCOMING_MSG_CMD:
        iLightIlluminance_ProcessAirMsg((afIncomingMSGPacket_t *)MSGpkt);
        break;

      case KEY_CHANGE:
        iLightIlluminance_ProcessKeys(((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys);
        break;

      case ZDO_STATE_CHANGE:
        iLightIlluminance_ProcessStateChange((devStates_t)MSGpkt->hdr.status);
        break;
      }

      // Release the memory
      osal_msg_deallocate((uint8 *)MSGpkt);
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if (events & ILIGHT_ILLUMINANCE_CHEAK)
  {
    int16 current = iLightIlluminance_measure_read();
    int16 delta = current - iLightIlluminance_lastValue;
    delta = delta < 0 ? -delta : delta;
    if (delta >= ILIGHT_ILLUMINANCE_CHEAK_THRESHOLD)
    {
      iLightIlluminance_lastValue = current;
      iLightIlluminance_feedback();
    }

    osal_start_timerEx(iLightIlluminance_taskId, ILIGHT_ILLUMINANCE_CHEAK, ILIGHT_ILLUMINANCE_CHEAK_INTERVAL);
    return events ^ ILIGHT_ILLUMINANCE_CHEAK;
  }
  return 0;
}
