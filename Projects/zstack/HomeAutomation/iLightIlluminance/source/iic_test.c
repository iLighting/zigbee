
// SDA: P1_2
// SCL: P1_3

#include "ZComDef.h"
#include <ioCC2530.h>
#include "iLight_iic_driver.h"

#define BH1750_ADDR  0x23

static int16 iLightIlluminance_measure_value = 0;
static uint8 iLightIlluminance_measure_task_id = 0;

void iic_hd_init(void) {
	P1SEL &= ~((1<<3)|(1<<2));
	P1INP &= ~((1<<3)|(1<<2)); // pull up
	P1DIR |= 1<<2; // sda output
	P1DIR |= 1<<3; // scl output
	P1_2 = 1;
	P2_3 = 1;
}

void iic_hd_sda_mode_read(void) {
	P1DIR &= ~(1<<2);
}

void iic_hd_sda_mode_write(void) {
	P1DIR |= 1<<2;
}


uint8 iic_hd_sda_read(void) {
	P1DIR &= ~(1<<2);
	return P1_2 ? 1 : 0;
}

void iic_hd_sda_write(uint8 x) {
	P1DIR |= 1<<2;
	P1_2 = x ? 1 : 0;
}

void iic_hd_scl_write(uint8 x) {
	P1_3 = x ? 1 : 0;
}

void main(void) {
  iic_op_init();
	while(1) {
		iic_op_st();
		iic_op_write((BH1750_ADDR<<1) | 0x00);
		iic_op_sxack(NULL);
		iic_op_write(0x10);
		iic_op_sxack(NULL);
		iic_op_sp();
	}
}

