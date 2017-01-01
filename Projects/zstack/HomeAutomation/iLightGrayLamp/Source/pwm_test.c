#include <ioCC2530.h>

/*使用P1_0口为输出、外设端口，来输出PWM波形*/
void init_port(void)
{
    P1DIR |= 0x01;    // p1_0 output
    P1SEL |= 0x01;    // p1_0  peripheral
    P2SEL &= 0xEE;    // Give priority to Timer 1
    PERCFG |= 0x40; // set timer_1 I/O位置为2
    return ;
}

/*
    将基准值放入T1CC0 寄存器, 将被比较值放入T1CC2寄存器
    当T1CC2中的值与T1CC0中的值相等时，则T1CC2 设置or清除
*/

void init_timer(void)
{
    T1CC0L = 0xff;   //PWM duty cycle  周期
    T1CC0H = 0x7f;
    
    T1CC2L = 0x00;  //     PWM signal period 占空比
    T1CC2H = 0x00;
    
    T1CCTL2 = 0x34;    // 等于T1CC0中的数值时候，输出高电平 1； 等于T1CC2中的数值时候，输出低电平 0  ，其实整个占空比就为50%了
    T1CTL |= 0x0f; // divide with 128 and to do i up-down mode
    return ;
}

void start_pwm(void) 
{
    init_port();
    init_timer();
//    IEN1 |=0x02;     //Timer 1 中断使能
//    EA = 1;            //全局中断使能
//    while(1) {;}
    return ;
}

void main() {
  start_pwm();
  while(1);
}
