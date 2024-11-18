#include "main.h"


// 定时器0初始化  -> uart
void timer0_init (void)
{
	T0CON = 0x07;				// 16位定时器,Fosc/4,256预分频
	INTCONbits.TMR0IE = 1;		// 允许Timer0中断
	INTCONbits.TMR0IF = 0;		// 清除Timer0中断标志
	INTCON2bits.TMR0IP = 0;		// Timer0 中断为低优先级
	TMR0H = T0_H;
	TMR0L = T0_L;
}

// 定时器1初始化 -> ad
void timer1_init (void)
{
	T1CON = 0x80;				// 16位读写,Fosc/4
	PIE1bits.TMR1IE = 1;		// 允许Timer1中断
	PIR1bits.TMR1IF = 0;		// 清除Timer1中断标志
	IPR1bits.TMR1IP = 1;		// Timer1中断为高优先级
	TMR1H = T1_count >> 8;
	TMR1L = (uint8_t) (T1_count&0xFF);
}



