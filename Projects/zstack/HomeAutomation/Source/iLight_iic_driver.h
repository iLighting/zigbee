#ifndef _ILIGHT_IIC_DRIVER_H_
#define _ILIGHT_IIC_DRIVER_H_

typedef enum {
	iic_state_init,
	iic_state_st,
	iic_state_write,
	iic_state_read,
	iic_state_sack,
	iic_state_snack,
	iic_state_mack,
	iic_state_mnack,
	iic_state_sp
} iic_state_t;

typedef enum {
	iic_err_success = 0,
	iic_err_stateErr
} iic_err_t;

extern iic_err_t iic_op_init(void);
extern iic_err_t iic_op_st(void);
extern iic_err_t iic_op_write(uint8 x);
extern iic_err_t iic_op_read(uint8 * p);
extern iic_err_t iic_op_sxack(uint8 * ack_ret);
extern iic_err_t iic_op_mack(void);
extern iic_err_t iic_op_mnack(void);
extern iic_err_t iic_op_sp(void);


#endif