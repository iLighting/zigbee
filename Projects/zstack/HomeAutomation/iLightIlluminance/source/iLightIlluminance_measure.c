
// SDA: P1_2
// SCL: P1_3

#include "iLightIlluminance.h"
#include "OSAL.h"
#include "hal_defs.h"
#include <ioCC2530.h>
#include "iLight_iic_driver.h"
#include "hal_led.h"
#include "OnBoard.h"

#include "iLightIlluminance_measure.h"

static int16 iLightIlluminance_measure_value = 0;
static uint8 iLightIlluminance_measure_task_id = 0;

void iic_hd_init(void)
{
  P1SEL &= ~((1 << 3) | (1 << 2));
  P1INP &= ~((1 << 3) | (1 << 2)); // pull up
  P1DIR |= 1 << 2;		   // sda output
  P1DIR |= 1 << 3;		   // scl output
  P1_2 = 1;
  P2_3 = 1;
}

void iic_hd_sda_mode_read(void)
{
  P1DIR &= ~(1 << 2);
}

void iic_hd_sda_mode_write(void)
{
  P1DIR |= 1 << 2;
}

uint8 iic_hd_sda_read(void)
{
  P1DIR &= ~(1 << 2);
  return P1_2 ? 1 : 0;
}

void iic_hd_sda_write(uint8 x)
{
  P1DIR |= 1 << 2;
  P1_2 = x ? 1 : 0;
}

void iic_hd_scl_write(uint8 x)
{
  P1_3 = x ? 1 : 0;
}

static void bh1750_init(void)
{
  iic_op_init();
}

static void bh1750_continuously_h_mode_start(void)
{
  iic_op_st();
  iic_op_write((BH1750_ADDR << 1) | 0x00);
  iic_op_sxack(NULL);
  iic_op_write(0x10);
  iic_op_sxack(NULL);
  iic_op_sp();
}

static void bh1750_continuously_h_mode_read(void)
{
  uint8 high_byte = 0;
  uint8 low_byte = 0;
  iic_op_st();
  iic_op_write((BH1750_ADDR << 1) | 0x01);
  iic_op_sxack(NULL);
  iic_op_read(&high_byte);
  iic_op_mack();
  iic_op_read(&low_byte);
  iic_op_mnack();
  iic_op_sp();
  iLightIlluminance_measure_value = ((high_byte << 8) | low_byte) / 1.2;
}

void iLightIlluminance_measure_init(byte task_id)
{
  iLightIlluminance_measure_task_id = task_id;
  bh1750_init();
  osal_set_event(task_id, ILIGHT_ILLUMINANCE_MEASURE_START);
}

int16 iLightIlluminance_measure_read(void)
{
  return iLightIlluminance_measure_value;
}

uint16 iLightIlluminance_measure_event_loop(uint8 task_id, uint16 events)
{
  if (events & ILIGHT_ILLUMINANCE_MEASURE_START)
  {
    bh1750_continuously_h_mode_start();
    osal_start_timerEx(
	iLightIlluminance_measure_task_id,
	ILIGHT_ILLUMINANCE_MEASURE_START_DONE,
	180);
    return events ^ ILIGHT_ILLUMINANCE_MEASURE_START;
  }
  if (events & ILIGHT_ILLUMINANCE_MEASURE_START_DONE)
  {
    bh1750_continuously_h_mode_read();
    // set LED1 (for debuging)
    HalLedSet(HAL_LED_1, iLightIlluminance_measure_value < 100 ? HAL_LED_MODE_OFF : HAL_LED_MODE_ON);
    // debuging end
    osal_set_event(iLightIlluminance_measure_task_id, ILIGHT_ILLUMINANCE_MEASURE_START);
    return events ^ ILIGHT_ILLUMINANCE_MEASURE_START_DONE;
  }
  return 0;
}
