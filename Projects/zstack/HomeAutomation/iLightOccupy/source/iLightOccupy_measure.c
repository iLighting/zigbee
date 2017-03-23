#include "iLightOccupy.h"
#include "OSAL.h"
#include "hal_defs.h"
#include <ioCC2530.h>

#include "iLightOccupy_measure.h"

static uint8 iLightOccupy_measure_occupy = 0;
static uint8 iLightOccupy_measure_task_id = 0;

void iLightOccupy_measure_init(byte task_id)
{
  iLightOccupy_measure_task_id = task_id;
  P0SEL &= ~(1 << 7);
  P0INP &= ~(1 << 7);
  P2INP |= 1 << 5; // P0 pull down
  P0DIR &= ~0x80;  // input mode
  osal_set_event(task_id, ILIGHT_OCCUPY_PULL);
}

int16 iLightOccupy_measure_read_occupy(void)
{
  return iLightOccupy_measure_occupy;
}

uint16 iLightOccupy_measure_event_loop(uint8 task_id, uint16 events)
{
  if (events & ILIGHT_OCCUPY_PULL)
  {
    iLightOccupy_measure_occupy = (uint8)P0_7;
    osal_start_timerEx(iLightOccupy_measure_task_id, ILIGHT_OCCUPY_PULL, ILIGHT_OCCUPY_INTERVAL);
    return events ^ ILIGHT_OCCUPY_PULL;
  }
  return 0;
}
