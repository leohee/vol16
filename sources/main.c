

#include "main.h"

struct vol_t gVol;



uint8_t UartRx_over=0, AD_over=0, AD_times=0, Calibrate1_times=0, Calibrate1_over=0, Calibrate2_times=0, Calibrate2_over=0;

uint8_t Address, PassWord[4], Display_mode, Display_times, Display_page, First_flag, Tx_flag=0;     


uint32_t Voltage[16];    //电压值



uint32_t KU[16];    //比率

uint8_t Change[16];  //偏移量

uint32_t Calibrate1_buf[4];    //基准校正数据区

uint32_t Calibrate2_buf[64];   //比率校正数据区

uint8_t S1_Flag=0, S2_Flag=0, DI1_Flag=0, DI2_Flag=0,S1_Count0=0, S1_Count1=0,            //开关参数标志
      S2_Count0=0, S2_Count1=0, DI1_Count0=0, DI1_Count1=0, DI2_Count0=0, DI2_Count1=0;



static void system_init (void)
{
	INTCON = 0x00;      //禁止全局中断和外围中断
	PIE1 = 0x00;
	PIE2 = 0x00;
	PIE3 = 0x00;

	TRISCbits.TRISC2 = 0;       //RUN灯,输出
	TRISHbits.TRISH5 = 1;       //DI1,输入
	TRISHbits.TRISH6 = 1;       //DI2,输入
	TRISGbits.TRISG6 = 0;       //看门狗,输出
	TRISJbits.TRISJ4 = 0;       //485使能,输出
}


void nop()
{
    _asm
      nop
    _endasm
}

void Delay_nms (uint32_t m)    
{
  uint8_t j;
  uint32_t i;

  for (i=0; i<m; i++)
  {
    for(j=0; j<200; j++)
    {
        nop();
    }
  }
}


// 基准电压校正
void Calibrate1 (uint8_t times)     
{
	uint32_t i = 0;
	uint32_t temp = 0;

	for (i=0; i<512; i++) {
		temp += AD_Buf[i];
	}

	Calibrate1_buf[times] = (uint32_t)(temp >> 9);
}



void Calibrate1_Deal (void)
{
	uint8_t i, addr1, addr2, data_l, data_h;
	uint32_t temp;

	temp = (Calibrate1_buf[0]+Calibrate1_buf[1]+Calibrate1_buf[2]+Calibrate1_buf[3]) / 4;
	data_h = temp >> 8;
	data_l = temp & 0x0ff;
	EEPROM_Write(JiZhun_ADDR, data_l);    
	Delay_nms(10);
	EEPROM_Write(JiZhun_ADDR+1, data_h);
	Delay_nms(10);
	data_l = EEPROM_Read(JiZhun_ADDR);
	data_h = EEPROM_Read(JiZhun_ADDR+1);
	gVol.RefVol = data_h * 256 + data_l;
}

// 电压比率校正
void Calibrate2 (uint8_t times)
{
	uint8_t i, j;

	for (i=0; i<16; i++) {
		j = times * 16 + i;
		Calibrate2_buf[j] = 2200000 / Voltage[i];
	}
}

void Calibrate2_Deal (void)
{
	uint8_t i, addr1, addr2, data_l, data_h;
	uint32_t temp;

	for (i=0; i<16; i++) {
		temp = (Calibrate2_buf[i]+Calibrate2_buf[i+16]+Calibrate2_buf[i+32]+Calibrate2_buf[i+48]) / 4;
		temp += 1;
		addr1 = KU_ADDR + 2*i;
		addr2 = addr1 + 1;
		data_h = temp >> 8;
		data_l = temp & 0x0ff;
	    EEPROM_Write(addr1, data_l);    
	    Delay_nms(10);
	    EEPROM_Write(addr2, data_h);
	    Delay_nms(10);
		data_l = EEPROM_Read(addr1);
		data_h = EEPROM_Read(addr2);
		KU[i] = data_h * 256 + data_l;
	}
}

// 读取系统参数
void Read_Para (void)
{
    uint8_t data_l, data_h, i;
    uint32_t addr1, addr2;
    
    Address = EEPROM_Read(Addr485_ADDR);
    Display_page = EEPROM_Read(DisPage_ADDR);
    Display_mode = EEPROM_Read(DisMode_ADDR);
    PassWord[0] = EEPROM_Read(PassWord_ADDR);
    PassWord[1] = EEPROM_Read(PassWord_ADDR+1);
    PassWord[2] = EEPROM_Read(PassWord_ADDR+2);
    PassWord[3] = EEPROM_Read(PassWord_ADDR+3);
    data_l = EEPROM_Read(JiZhun_ADDR);
    data_h = EEPROM_Read(JiZhun_ADDR+1);
    gVol.RefVol = data_h*256 + data_l;
    Baud_n = EEPROM_Read(Baud_ADDR);
	baudrate_set(Baud_n);

    /**************告警电压比较值******************/
    data_l = EEPROM_Read(AlarmVoltage_ADDR);
    data_h = EEPROM_Read(AlarmVoltage_ADDR+1);
    gVol.AlarmVoltage= data_h*256 + data_l;
    
    for (i=0; i<16; i++) {
		addr1 = KU_ADDR + 2*i;
		addr2 = addr1 + 1;
		data_l = EEPROM_Read(addr1); 
		data_h = EEPROM_Read(addr2);
		KU[i] = data_h*256 + data_l;
	}
  
	for (i=0; i<16; i++) {
		Change[i] = EEPROM_Read(Change_ADDR+i);
	}
}

// 第一次上电参数初始化
void Para_Initial (void)
{
	uint8_t i;
	uint32_t addr1, addr2;

	EEPROM_Write(Addr485_ADDR, 0x01);     //0x01   地址
	Delay_nms(10);
	EEPROM_Write(Baud_ADDR, 0x04);        //0x04   96000   波特率
	Delay_nms(10);
	EEPROM_Write(First_Flag_ADDR, 0x55);      //0x55    第一次上电标志
	Delay_nms(10);
	EEPROM_Write(JiZhun_ADDR, 0xd3);        //0x1d3    基准电压
	Delay_nms(10);
	EEPROM_Write(JiZhun_ADDR+1, 0x01);
	Delay_nms(10);

	EEPROM_Write(AlarmVoltage_ADDR, 0x84);        //0x1d3    基准电压
	Delay_nms(10);
	EEPROM_Write(AlarmVoltage_ADDR+1, 0x03);
	Delay_nms(10);

    for (i=0; i<16; i++) {
		addr1 = KU_ADDR + 2*i;
		addr2 = addr1 + 1;
		EEPROM_Write(addr1, 0xe8);     //1000    0x3e8   电压比率
		Delay_nms(10);
		EEPROM_Write(addr2, 0x03);
		Delay_nms(10);
	}

	for (i=0; i<16; i++) {
		EEPROM_Write(Change_ADDR+i, 0x64);   //100   0x64    电压偏移量
		Delay_nms(10);
	}
}

void  main (void)
{
	uint32_t Run_count=0;
	   
	Delay_nms(20);

	system_init();
	ad_init();
	key_init();
	uart_init();
	i2c_init();
	timer0_init();
	timer1_init();

	INTCONbits.GIE = 1;    //使能全局中断
	INTCONbits.PEIE = 1;   //使能外设中断

	First_flag = EEPROM_Read(First_Flag_ADDR);
	if (First_flag==0x55) {
		Read_Para();
	} else {
		Para_Initial();
		Read_Para();
	}

	T1CONbits.TMR1ON = 1;   //打开Timer1

	while (1)  {
		WDI();     //喂狗

		Run_count++;

		if (Run_count >= 30000) {
			Run_count = 0;
			// 正在校准
			if (((S1_Flag==1)&&(Calibrate1_over==0)) || ((S2_Flag==1)&&(Calibrate2_over==0))) {
				LED = 0;
			} else {
				RunLed();
			}
	      
			Key_Scan();    //按键扫描
		}

		// 串口接收数据处理
		if ((UartRx_over==1) && (Tx_flag==0)) {
			RxCom_Deal();
			UartRx_over = 0; 
		}

		if (AD_over == 1) {     //AD采样结束处理
			AD_over = 0;

			if ((S1_Flag==1) && (Calibrate1_over==0)) {
				if (Calibrate1_times >= 4) {
					Calibrate1_times = 0;
					Calibrate1_Deal();
					Calibrate1_over = 1;
					Delay_nms(100);
				} else {
					Calibrate1(Calibrate1_times++);
				}
			}

			if (AD_times >= 8) {
				AD_times = 0;
				ADData_Deal();

				if ((S2_Flag==1) && (Calibrate2_over==0)) {
					if (Calibrate2_times >= 4) {
						Calibrate2_times = 0;
						Calibrate2_Deal();
						Calibrate2_over = 1;
						Delay_nms(100);
					} else {
						Calibrate2(Calibrate2_times++);
					}
				}
			} else {      
				ADResult_Deal(AD_times++);
			}

			TMR1H = 0x00;
			TMR1L = 0x00;
			T1CONbits.TMR1ON = 1;   //打开Timer1   开始下一次采样
		}
	}    

}    



