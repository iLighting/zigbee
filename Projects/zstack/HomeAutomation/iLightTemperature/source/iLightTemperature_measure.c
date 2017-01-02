#include "iLightTemperature.h"
#include "OSAL.h"
#include "hal_defs.h"
#include <ioCC2530.h>

#include "iLightTemperature_measure.h"

static int16 iLightTemperature_measure_temp = 0;
static uint8 iLightTemperature_measure_task_id = 0;


void delay_us(uint16 microSecs)
{
  while(microSecs--)
  {
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop");
  }
}

#define data_pin_out_mode()  P0DIR |= 0x80
#define data_pin_in_mode()   P0DIR &= ~0x80
#define data_pin_write(x)    P0_7 = (x)
#define data_pin_read()      P0_7

static void dht11_init(void)
{
	P0SEL &= ~(1<<7);
	P0INP &= ~(1<<7);
	P2INP &= ~(1<<5); // pull up
  data_pin_out_mode();
	data_pin_write(1);
}

// data_pin == 0 ¨º¡À??¨¨?
static bool dht11_read_data(uint8 *buf)
{
  uint8 i=0;
  uint8 j=0;
  
  for(i=0;i<5;i++)
  {
    buf[i] = 0;
    for(j=0;j<8;j++)
    {
      while(data_pin_read()==0);
      delay_us(50);
      buf[i] <<= 1;
      if (data_pin_read())
      {
        buf[i] |= 0x01;
        while(data_pin_read());
      }
      else
      {
        buf[i] &= 0xfe;
      }
    }
  }
  // TODO: sum check
  while(data_pin_read()==0);
  return TRUE;
}

static void dht11_parse_data(uint8 *buf, int16 *temperature_p, int16 *humidity_p)
{
  if (humidity_p)
    *humidity_p = buf[0]*100 + buf[1];
  if (temperature_p)
    *temperature_p= buf[2]*100 + buf[3];
}

static void dht11_start_transmission(void)
{
  data_pin_out_mode();
  data_pin_write(0);
  osal_start_timerEx(iLightTemperature_measure_task_id, ILIGHT_TEMPERATURE_START_TRANS_DATA, 20);
}

static void dht11_start_trans_data(void)
{
  uint8 payload[40/8];
  
  data_pin_out_mode();
  data_pin_write(1);
  delay_us(30);
  data_pin_in_mode();
  if (data_pin_read()==0)
  {
    // DHT responsed me
    while(data_pin_read()==0);
    while(data_pin_read());
		// start to trans
    if (dht11_read_data(payload))
    {
      // data is ok
      dht11_parse_data(payload, &iLightTemperature_measure_temp, NULL);
    }
  }
}

void iLightTemperature_measure_init(byte task_id)
{
  iLightTemperature_measure_task_id = task_id;
  dht11_init();
  osal_set_event (task_id, ILIGHT_TEMPERATURE_START_TRANS);
}

int16 iLightTemperature_measure_read_temperature(void)
{
  return iLightTemperature_measure_temp;
}

uint16 iLightTemperature_measure_event_loop( uint8 task_id, uint16 events )
{
	if (events & ILIGHT_TEMPERATURE_START_TRANS)
  {
    dht11_start_transmission();
    return events ^ ILIGHT_TEMPERATURE_START_TRANS;
  }
  if (events & ILIGHT_TEMPERATURE_START_TRANS_DATA)
  {
    dht11_start_trans_data();
		osal_start_timerEx(iLightTemperature_measure_task_id, ILIGHT_TEMPERATURE_START_TRANS, ILIGHT_TEMPERATURE_INTERVAL);
    return events ^ ILIGHT_TEMPERATURE_START_TRANS_DATA;
  }
  return 0;
}
