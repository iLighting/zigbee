#include "iLightAsr_ld3320.h"
#include <iocc2530.h>
#include "OSAL.h"
#include "stdio.h"


// MISO:P1_7 Input
// MOSI:P1_6 Output
// Clock:P1_5 Output
// Reset:P1_3 Output
// ChipSelect: P1_4 Output

// global define
// ------------------------
#define CLK_IN            22.1184 /* user need modify this value according to clock in */
#define LD_PLL_11         (unsigned char)((CLK_IN/2.0)-1)
#define LD_PLL_ASR_19     (unsigned char)(CLK_IN*32.0/(LD_PLL_11+1) - 0.51)
#define LD_PLL_ASR_1B     0x48
#define LD_PLL_ASR_1D     0x1f

// RPI_GPIO_P1_11
#define GPIO_RESET 0
#define GPIO_CHIP_SELECT 1

// global variable
// ------------------------

// delay functions
// ------------------------

static void delay_us(uint16 microSecs)
{
  while(microSecs--)
  {
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop");
  }
}

inline static void delay_ms(uint16 ms) {
	delay_us(ms<<10); // *1024
}


// spi functions
// ------------------------
unsigned char spi_swap(unsigned char x) {
  U1DBUF = x;
  while(!(U1CSR & (1<<1)));
  U1CSR &= ~(1<<1);
  return U1DBUF;
}

/**
 * [spi_init description]
 * @return 1:success, 0:false
 */
unsigned char spi_init() {
  // using USART1
  // CPOL=1, CPHA=0, MSB
  // baudrate = 115200bps
  U1CSR = 0;
  U1GCR = (1<<7) | (1<<5) | 11;
  U1BAUD = 216;
  // USART1 alt2
  PERCFG |= 1<<1;
  // MISO:P1_7 Input
  // MOSI:P1_6 Output
  // Clock:P1_5 Output
  P1DIR |= (1<<6) | (1<<5);
  P1DIR &= ~(1<<7);
  P1SEL |= (1<<7) | (1<<6) | (1<<5);
  return 1;
}

// gpio functions
// -----------------------
unsigned char gpio_init() {
  // Reset:P1_3 Output
  // ChipSelect: P1_4 Output
  P1DIR |= (1<<4) | (1<<3);
  return 1;
}

void gpio_write(unsigned char gid, unsigned char x) {
  switch (gid) {
    case GPIO_RESET:
      P1_3 = x;
      break;
    case GPIO_CHIP_SELECT:
      P1_4 = x;
      break;
  }
}

// ld3320 functions
// -----------------------
void ld3320_write_reg(unsigned char addr, unsigned char data) {
  gpio_write(GPIO_CHIP_SELECT, 0);
  spi_swap(0x04);
  spi_swap(addr);
  spi_swap(data);
  gpio_write(GPIO_CHIP_SELECT, 1);
}

unsigned char ld3320_read_reg(unsigned char addr) {
  unsigned char temp = 0;
  gpio_write(GPIO_CHIP_SELECT, 0);
  spi_swap(0x05);
  spi_swap(addr);
  temp = spi_swap(0);
  gpio_write(GPIO_CHIP_SELECT, 1);
  return temp;
}

void ld3320_reset() {
  gpio_write(GPIO_RESET, 1);
  delay_ms(100);
  gpio_write(GPIO_RESET, 0);
  delay_ms(100);
  gpio_write(GPIO_RESET, 1);
  delay_ms(100);
  gpio_write(GPIO_CHIP_SELECT, 0);
  delay_ms(100);
  gpio_write(GPIO_CHIP_SELECT, 1);
}

/**
 * [ld3320_setup_hardware description]
 * @return  1:success
 */
unsigned char ld3320_setup_hardware(void) {
  if(!gpio_init()) {return 0;}
  if(!spi_init()) {return 0;}
  return 1;
}

unsigned char ld3320_setup_common(void) {return 1;}

unsigned char ld3320_setup_asr(void) {
  ld3320_read_reg(0x06);
  ld3320_write_reg(0x17, 0x35);
  delay_ms(10);
  ld3320_write_reg(0x89, 0x03);
  delay_ms(5);
  ld3320_write_reg(0xcf, 0x43);
  delay_ms(5);
  ld3320_write_reg(0xcb, 0x02);
  ld3320_write_reg(0x11, LD_PLL_11);
  ld3320_write_reg(0x1e, 0x00);
  ld3320_write_reg(0x19, LD_PLL_ASR_19);
  ld3320_write_reg(0x1b, LD_PLL_ASR_1B);
  ld3320_write_reg(0x1d, LD_PLL_ASR_1D);
  delay_ms(10);
  ld3320_write_reg(0xcd, 0x04);
  ld3320_write_reg(0x17, 0x4c);
  delay_ms(5);
  ld3320_write_reg(0xb9, 0x00);
  ld3320_write_reg(0xcf, 0x4f);
  ld3320_write_reg(0x6f, 0xff);
  //
  ld3320_write_reg(0xbd, 0);
  ld3320_write_reg(0x17, 0x48);
  delay_ms(10);
  ld3320_write_reg(0x3c, 0x80);
  ld3320_write_reg(0x3e, 0x07);
  ld3320_write_reg(0x38, 0xff);
  ld3320_write_reg(0x3a, 0x07);
  ld3320_write_reg(0x40, 0x00);
  ld3320_write_reg(0x42, 0x08);
  ld3320_write_reg(0x44, 0x00);
  ld3320_write_reg(0x46, 0x08);
  delay_ms(1);
  return 1;
}

/**
 * 芯片空闲后才能调用
 * @param index [description]
 * @param strp  [description]
 */
void ld3320_add_item(unsigned char index, unsigned char * strp) {
  unsigned char i = 0;
  ld3320_write_reg(0xc1, index);
  ld3320_write_reg(0xc3, 0);
  ld3320_write_reg(0x08, 0x04);
  delay_ms(1);
  ld3320_write_reg(0x08, 0);
  delay_ms(1);
  for(i = 0; strp[i]!=0; i++) {
    ld3320_write_reg(0x05, strp[i]);
  }
  ld3320_write_reg(0xb9, i);
  ld3320_write_reg(0xb2, 0xff);
  ld3320_write_reg(0x37, 0x04);
}

unsigned char ld3320_get_state(void) {
  unsigned char reg_2b = ld3320_read_reg(0x2b);
  unsigned char reg_b2 = ld3320_read_reg(0xb2);
  unsigned char reg_bf = ld3320_read_reg(0xbf);
	
	// printf("%x, %x, %x\n", (int)reg_2b, (int)reg_b2, (int)reg_bf);

  if (reg_2b&(1<<3)) {
    return LD3320_STATE_ASR_ERROR;
  }
	if (reg_2b==0 && reg_b2==0 && reg_bf==0) {
    return LD3320_STATE_ASR_ERROR;
  }
  if (reg_b2==0x21) {
    if (reg_bf==0x35) {
      return LD3320_STATE_ASR_FIND;
    } else {
      return LD3320_STATE_ASR_FREE;
    }
  } else {
    return LD3320_STATE_ASR_BUSY;
  }
}

unsigned char ld3320_asr_run(void) {
  ld3320_write_reg(0x35, LD3320_MIC_GEN);
  ld3320_write_reg(0x1c, 0x09);
  ld3320_write_reg(0xbd, 0x20);
  ld3320_write_reg(0x08, 0x01);
  delay_ms(1);
  ld3320_write_reg(0x08, 0);
  delay_ms(1);
  if(ld3320_get_state()==LD3320_STATE_ASR_BUSY)
    return 0;
  ld3320_write_reg(0xb2, 0xff);
  ld3320_write_reg(0x37, 0x06);
  delay_ms(5);
  ld3320_write_reg(0x1c, 0x0b);
  // ld3320_write_reg(0x29, 0x10);
  ld3320_write_reg(0xbd, 0x00);
  return 1;
}

unsigned char ld3320_get_asr_count(void) {
  unsigned char count = ld3320_read_reg(0xba);
  if ((1 <= count) && (count <= 4)) {
    return count;
  } else {
    return 0;
  }
}

unsigned char ld3320_get_asr_result(unsigned char order) {
  unsigned char index = 0;
  switch (order) {
    case 0:
      index = ld3320_read_reg(0xc5);
    case 1:
      index = ld3320_read_reg(0xc7);
    case 2:
      index = ld3320_read_reg(0xc9);
    case 3:
      index = ld3320_read_reg(0xcb);
    default:
      index = ld3320_read_reg(0xc5);
  }
  ld3320_write_reg(0x2b, 0);
  ld3320_write_reg(0x1c, 0);
  return index;
}

unsigned char ld3320_init(unsigned char mode) {
  unsigned char state = 1;
  switch (mode) {
    case LD3320_INIT_MODE_ASR:
      if(!ld3320_setup_hardware()) { state = 0; break; }
      ld3320_reset();
      if(!ld3320_setup_common()) { state = 0; break; }
      if(!ld3320_setup_asr()) { state = 0; break; }
      break;
  }
  return state;
}
