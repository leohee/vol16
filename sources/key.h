#ifndef __KEY_H__
#define __KEY_H__

#define  S1      PORTHbits.RH0
#define  S2      PORTHbits.RH1
#define  DI1     PORTEbits.RE5
#define  DI2     PORTEbits.RE6

extern void key_init (void);

extern void Key_Scan (void);


#endif


