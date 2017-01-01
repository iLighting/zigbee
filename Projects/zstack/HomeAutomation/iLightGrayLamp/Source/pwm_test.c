#include <ioCC2530.h>

/*ʹ��P1_0��Ϊ���������˿ڣ������PWM����*/
void init_port(void)
{
    P1DIR |= 0x01;    // p1_0 output
    P1SEL |= 0x01;    // p1_0  peripheral
    P2SEL &= 0xEE;    // Give priority to Timer 1
    PERCFG |= 0x40; // set timer_1 I/Oλ��Ϊ2
    return ;
}

/*
    ����׼ֵ����T1CC0 �Ĵ���, �����Ƚ�ֵ����T1CC2�Ĵ���
    ��T1CC2�е�ֵ��T1CC0�е�ֵ���ʱ����T1CC2 ����or���
*/

void init_timer(void)
{
    T1CC0L = 0xff;   //PWM duty cycle  ����
    T1CC0H = 0x7f;
    
    T1CC2L = 0x00;  //     PWM signal period ռ�ձ�
    T1CC2H = 0x00;
    
    T1CCTL2 = 0x34;    // ����T1CC0�е���ֵʱ������ߵ�ƽ 1�� ����T1CC2�е���ֵʱ������͵�ƽ 0  ����ʵ����ռ�ձȾ�Ϊ50%��
    T1CTL |= 0x0f; // divide with 128 and to do i up-down mode
    return ;
}

void start_pwm(void) 
{
    init_port();
    init_timer();
//    IEN1 |=0x02;     //Timer 1 �ж�ʹ��
//    EA = 1;            //ȫ���ж�ʹ��
//    while(1) {;}
    return ;
}

void main() {
  start_pwm();
  while(1);
}
