#ifndef __TIMER_H__
#define __TIMER_H__

#define T0_H    0xfa            //1536 Fosc/4 1:256  60ms
#define T0_L    0x00
#define T1_count    0xf0bd      //3096 Fosc/4 625us

extern void timer0_init (void);

extern void timer1_init (void);

#endif

