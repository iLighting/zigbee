
// back forward delay

#include "ZComDef.h"
#include <ioCC2530.h>
#include "iLight_iic_driver.h"

extern void iic_hd_init(void);
extern void iic_hd_sda_write(uint8 x);
extern void iic_hd_sda_mode_read(void);
extern void iic_hd_sda_mode_write(void);
extern uint8 iic_hd_sda_read(void);
extern void iic_hd_scl_write(uint8 x);


// iic bus's state
static iic_state_t bus_state = iic_state_init;

#define IIC_IS_STATE(x)			((x) == bus_state)
#define IIC_SET_STATE(x)		(bus_state = (x))


static void delay_us(uint16 microSecs)
{
  while(microSecs--)
  {
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop");
  }
}

iic_err_t iic_op_init(void) {
	iic_hd_init();
	iic_hd_scl_write(1);
	iic_hd_sda_write(1);
	IIC_SET_STATE(iic_state_init);
	return iic_err_success;
}

// sda:1, scl:1 => sda:0, scl:1
iic_err_t iic_op_st(void) {
	if (IIC_IS_STATE(iic_state_init) || IIC_IS_STATE(iic_state_sp)) {
		iic_hd_sda_write(0);
		delay_us(2);
		IIC_SET_STATE(iic_state_st);
		return iic_err_success;
	} else {
		return iic_err_stateErr;
	}
}

// sda:0, scl:1 => sda:0, scl:0
iic_err_t iic_op_write(uint8 x) {
	if (IIC_IS_STATE(iic_state_st) || IIC_IS_STATE(iic_state_sack)) {
		uint8 i = 7;
		do {
			iic_hd_scl_write(0);
			delay_us(2);
			iic_hd_sda_write(x & (1<<i));
			delay_us(2);
			iic_hd_scl_write(1);
			delay_us(5);
		} while(i--);
		iic_hd_scl_write(0);
		delay_us(5);
		IIC_SET_STATE(iic_state_write);
		return iic_err_success;
	} else {
		return iic_err_stateErr;
	}
}

// last state: sack
// sda:0, scl:1 => sda:0, scl:0
iic_err_t iic_op_read(uint8 * p) {
	uint8 i = 7;
	uint8 x = 0;
	if (IIC_IS_STATE(iic_state_sack) || IIC_IS_STATE(iic_state_mack)) {
		iic_hd_sda_mode_read();
		do {
			iic_hd_scl_write(0);
			delay_us(5);
			iic_hd_scl_write(1);
			delay_us(5);
			if (iic_hd_sda_read()) { x |= 1<<i; } else { x &= ~(1<<i); }
		} while(i--);
		iic_hd_scl_write(0);
		delay_us(5);
		*p = x;
		IIC_SET_STATE(iic_state_read);
		return iic_err_success;
	} else {
		return iic_err_stateErr;
	}
}

// last state: write
// sda:0, scl:0 => sda:ack_ret, scl:0
iic_err_t iic_op_sxack(uint8 * ack_ret) {
	uint8 ack = 0;
	if (IIC_IS_STATE(iic_state_write)) {
		iic_hd_sda_mode_read();
		iic_hd_scl_write(1);
		delay_us(2);
		ack = iic_hd_sda_read();
		if (ack_ret) *ack_ret = ack;
		delay_us(2);
		iic_hd_scl_write(0);
		delay_us(5);
		IIC_SET_STATE(ack == 0 ? iic_state_sack : iic_state_snack);
		return iic_err_success;
	} else {
		return iic_err_stateErr;
	}
}

static iic_err_t iic_op_mxack(uint8 ack) {
	if (IIC_IS_STATE(iic_state_read)) {
		iic_hd_sda_write(ack);
		delay_us(2);
		iic_hd_scl_write(1);
		delay_us(5);
		iic_hd_scl_write(0);
		delay_us(5);
		IIC_SET_STATE(ack == 0 ? iic_state_mack : iic_state_mnack);
		return iic_err_success;
	} else {
		return iic_err_stateErr;
	}
}

iic_err_t iic_op_mack(void) { return iic_op_mxack(0); }
iic_err_t iic_op_mnack(void) { return iic_op_mxack(1); }


// sda:0, scl:0 => sda:1, scl:1
iic_err_t iic_op_sp(void) {
	if (
		IIC_IS_STATE(iic_state_sack)
		|| IIC_IS_STATE(iic_state_snack)
		|| IIC_IS_STATE(iic_state_mack)
		|| IIC_IS_STATE(iic_state_mnack)) 
	{
		iic_hd_sda_write(0);
		delay_us(2);
		iic_hd_scl_write(1);
		delay_us(2);
		iic_hd_sda_write(1);
		delay_us(2);
		IIC_SET_STATE(iic_state_sp);
		return iic_err_success;
	} else {
		return iic_err_stateErr;
	}
}


