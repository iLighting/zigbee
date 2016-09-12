#include "ZComDef.h"

#include "iLight_appMsg.h"

uint8 appMsg_getCmdId(uint8 * pData) {
  return pData[0];
}
