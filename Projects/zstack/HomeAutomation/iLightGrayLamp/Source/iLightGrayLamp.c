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
#include "hal_timer.h"
#include "OnBoard.h"
#include "math.h"

#include "iLight_appMsg.h"
#include "iLightGrayLamp.h"

#include <ioCC2530.h>

#define MAJOR_PWM_CH HAL_T1_CH2 // -> P1_0
#define MINOR_PWM_CH HAL_T1_CH1 // -> P1_1

#define MINOR_PWM_K 0.5

static uint8 iLightGrayLamp_taskId = 0;
static uint8 iLightGrayLamp_nwkState = 0;
static uint8 iLightGrayLamp_currentLevel = 0;
static uint8 iLightGrayLamp_transId = 0;

const cId_t iLightGrayLamp_inClusterList[] = {0};
const cId_t iLightGrayLamp_outClusterList[] = {0};

static SimpleDescriptionFormat_t iLightGrayLamp_simpleDesc = {
    8, // endpoint
    0, // profile id
    1, // device id
    0, // version
    0, // reserved
    1,
    (cId_t *)iLightGrayLamp_inClusterList,
    1,
    (cId_t *)iLightGrayLamp_outClusterList,
};

static endPointDesc_t iLightGrayLamp_epDesc = {
    8,
    &iLightGrayLamp_taskId,
    &iLightGrayLamp_simpleDesc,
    (afNetworkLatencyReq_t)0,
};

static void iLightGrayLamp_launchMinorPwm(void)
{
  halTimer1SetChannelDuty(MINOR_PWM_CH, 1000 * MINOR_PWM_K);
}

void iLightGrayLamp_updateLevel()
{
  uint8 level = iLightGrayLamp_currentLevel;
  halTimer1SetChannelDuty(MAJOR_PWM_CH, level * 10);
}

void iLightGrayLamp_init(uint8 taskId)
{
  iLightGrayLamp_taskId = taskId;
  afRegister(&iLightGrayLamp_epDesc);
  // pwm
  DISABLE_LAMP
  HalTimer1Init(0);
  iLightGrayLamp_launchMinorPwm();
  iLightGrayLamp_updateLevel();
  ENABLE_LAMP;
}

void iLightGrayLamp_ProcessAirMsg(afIncomingMSGPacket_t *MSGpkt)
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
  case ILIGHT_APPMSG_GRAY_LAMP_CHANGE:
    iLightGrayLamp_currentLevel = ((iLight_appMsg_gray_lamp_change_t *)pData)->level;
    iLightGrayLamp_updateLevel();
    break;
  }
}

void iLightGrayLamp_ProcessKeys(uint8 shift, uint8 keys) {}

void iLightGrayLamp_ProcessStateChange(devStates_t state)
{
  iLightGrayLamp_nwkState = state;
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
    break;
  }
}

uint16 iLightGrayLamp_event_loop(uint8 taskId, uint16 events)
{
  afIncomingMSGPacket_t *MSGpkt = NULL;
  if (events & SYS_EVENT_MSG)
  {
    while ((MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(iLightGrayLamp_taskId)))
    {
      switch (MSGpkt->hdr.event)
      {
      case AF_INCOMING_MSG_CMD:
        iLightGrayLamp_ProcessAirMsg((afIncomingMSGPacket_t *)MSGpkt);
        break;

      case KEY_CHANGE:
        iLightGrayLamp_ProcessKeys(((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys);
        break;

      case ZDO_STATE_CHANGE:
        iLightGrayLamp_ProcessStateChange((devStates_t)(MSGpkt->hdr.status));
        break;

      default:
        break;
      }

      // Release the memory
      osal_msg_deallocate((uint8 *)MSGpkt);
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }
  return 0;
}
