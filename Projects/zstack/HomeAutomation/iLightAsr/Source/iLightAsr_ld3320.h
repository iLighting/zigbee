#ifndef _LD3320_H
#define _LD3320_H

// define
// -------------------------
#define LD3320_INIT_MODE_ASR 0

#define LD3320_STATE_ASR_FREE     0
#define LD3320_STATE_ASR_BUSY     1
#define LD3320_STATE_ASR_ERROR    2
#define LD3320_STATE_ASR_FIND     3

// MIC gen 0x00-0x7f
#define LD3320_MIC_GEN  0x60

extern unsigned char spi_init();
extern unsigned char spi_swap(unsigned char x);

extern void ld3320_write_reg(unsigned char addr, unsigned char data);
extern unsigned char ld3320_read_reg(unsigned char addr);

extern unsigned char ld3320_get_state(void);
extern unsigned char ld3320_asr_run(void);
extern unsigned char ld3320_get_asr_result(unsigned char order);
extern unsigned char ld3320_get_asr_count(void);
extern void ld3320_add_item(unsigned char index, unsigned char * strp);

extern unsigned char ld3320_init(unsigned char mode);

#endif
