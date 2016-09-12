#include "zComDef.h"

#include "iLightBridge_mt_appMsg_callback.h"


void MT_APP_feedback(uint8 dataLen, uint8 * pData) {
  MT_BuildAndSendZToolResponse(0x49, 0, dataLen, pData);
}
