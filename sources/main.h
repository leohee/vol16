#ifndef __main_h__
#define __main_h__

#include "p18f97j60.h"
#include "math.h"
#include "stdio.h"
#include <stdint.h>

#include "ad.h"
#include "timer.h"
#include "uart.h"
#include "key.h"
#include "e2prom.h"
#include "crc16.h"
#include "reg.h"


#define Ulong     unsigned long
#define OK        0x22
#define ERROR     0x55



#define WDI()           LATGbits.LATG6=~LATGbits.LATG6         //外部看门狗
#define RunLed()        LATCbits.LATC2=~LATCbits.LATC2         //运行指示灯
#define LED             LATCbits.LATC2



extern uint32_t Baud;

extern uint8_t UartRx_over, AD_over, Calibrate1_times, Calibrate1_over, Calibrate2_times, Calibrate2_over;

extern uint8_t Address, PassWord[4], Display_mode, Display_times, Display_page, Tx_flag, ReadLen_flag;

extern uint32_t Voltage[16];   



extern uint32_t KU[16];    

extern uint8_t Change[16]; 

extern uint32_t AD_Buf[512];

extern uint8_t S1_Flag, S2_Flag, DI1_Flag, DI2_Flag,S1_Count0, S1_Count1, 
      S2_Count0, S2_Count1, DI1_Count0, DI1_Count1, DI2_Count0, DI2_Count1;


struct vol_t {
	uint32_t		RefVol;			// 基准电压

	uint32_t		AlarmVoltage;	// 告警电压比较值
	uint32_t		AlarmState;		// 16路告警状态 bit0~bit16

	uint8_t			switch_crc;		// crc 高低换位
};



extern struct vol_t gVol;




extern void nop (void);


extern void Delay_nms (uint32_t n);


#endif

