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

static uint8 iLightGrayLamp_taskId = 0;
static uint8 iLightGrayLamp_nwkState = 0;
static uint8 iLightGrayLamp_currentLevel = 0;
static uint8 iLightGrayLamp_transId = 0;


const cId_t iLightGrayLamp_inClusterList[] = {0};
const cId_t iLightGrayLamp_outClusterList[] = {0};


static SimpleDescriptionFormat_t iLightGrayLamp_simpleDesc = {
	8, // endpoint
	0, // profile id
	0, // device id
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


void iLightGrayLamp_updateLevel() {
	uint8 level = iLightGrayLamp_currentLevel;
	uint16 gammaCorrectedLevel;

  // gamma correct the level
  gammaCorrectedLevel = (uint16) ( pow( ( (float)level / LEVEL_MAX ), (float)GAMMA_VALUE ) * (float)LEVEL_MAX);

  halTimer1SetChannelDuty(WHITE_LED, (uint16)(((uint32)gammaCorrectedLevel*PWM_FULL_DUTY_CYCLE)/LEVEL_MAX) );
}


void iLightGrayLamp_init(uint8 taskId) {
	iLightGrayLamp_taskId = taskId;
	afRegister(&iLightGrayLamp_epDesc);
	// pwm
	HalTimer1Init( 0 );
  halTimer1SetChannelDuty( WHITE_LED, 0 );
}


void iLightGrayLamp_ProcessAirMsg(afIncomingMSGPacket_t * MSGpkt) {
	uint16 cluster = MSGpkt->clusterId;
	uint8 cmdId = 0;
	if (cluster != ILIGHT_APPMSG_CLUSTER) return;
	cmdId = appMsg_getCmdId(MSGpkt->cmd.Data);
	switch(cmdId) {
		case ILIGHT_APPMSG_GRAY_LAMP_CHANGE:
			iLightGrayLamp_currentLevel = ((iLight_appMsg_gray_lamp_change_t *) MSGpkt)->level;
			iLightGrayLamp_updateLevel();
			break;
	}
}


void iLightGrayLamp_ProcessKeys(uint8 shift, uint8 keys) {}

void iLightGrayLamp_annce(void) {
	uint16 nwk = NLME_GetShortAddr();
	byte *ieeep = NLME_GetExtAddr();
	// router
	uint8 ca = (0x01<<1);
	ZDP_DeviceAnnce(nwk, ieeep, ca, 0);
}

void iLightGrayLamp_ProcessStateChange(devStates_t state) {
	iLightGrayLamp_nwkState = state;
	switch (state) {
		case DEV_INIT:
			// flash slowly
			HalLedBlink(HAL_LED_1, 0, 30, 1000);
			break;
		case DEV_NWK_DISC:
		case DEV_NWK_JOINING:
		case DEV_END_DEVICE_UNAUTH:
			// flash quickly
			HalLedBlink(HAL_LED_1, 0, 30, 500);
			break;
		case DEV_END_DEVICE:
		case DEV_ROUTER:
			// turn on
			HalLedSet(HAL_LED_1, HAL_LED_MODE_ON);
			// annce
			iLightGrayLamp_annce();
			break;
	}
}

uint16 iLightGrayLamp_event_loop(uint8 taskId, uint16 events) {
	afIncomingMSGPacket_t * MSGpkt = NULL;
	if ( events & SYS_EVENT_MSG )
	{
		while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( iLightGrayLamp_taskId )) )
		{
			switch ( MSGpkt->hdr.event )
			{
				case AF_INCOMING_MSG_CMD:
					iLightGrayLamp_ProcessAirMsg((afIncomingMSGPacket_t * )MSGpkt);
					break;

				case KEY_CHANGE:
					iLightGrayLamp_ProcessKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
					break;
		
				case ZDO_STATE_CHANGE:
					iLightGrayLamp_ProcessStateChange((devStates_t)(MSGpkt->hdr.status));
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
