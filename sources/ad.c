/*
    程序实现A/D转换功能，A/D采用中断方式。该程序通过单片机的AN6~AN11通道输入三相电压 三相电流.
*/


#include "main.h"

#include "ad.h"



uint32_t ADResult=0,AD_number=0;
uint8_t AD_count=0,Num;

#pragma udata bigbank

uint32_t AD_Buf[512];
uint32_t ADTemp_buf[128];

#pragma udata


void InterruptHandlerHigh (void);



void ad_init (void)
{
	ADCON0 = 0x01;            //选择A/D通道为AN0，使能A/D转换器
	ADCON1 = 0x00;            //参考电源选择AVDD、AVSS，且把AN0~AN15设置为模拟量输入方式
	ADCON2 = 0x92;        //转换结果右对齐,2个TAD、Fosc/32
	PIR1bits.ADIF = 0;        //清除A/D转换标志
	PIE1bits.ADIE = 1;        //A/D转换中断允许
	IPR1bits.ADIP = 1;        //A/D中断为高优先级
	RCONbits.IPEN = 1;        //使能中断优先级
	TRISFbits.TRISF6 = 1; //设置RF6为输入方式 
}



// 低优先级中断向量

#pragma code InterruptVectorHigh = 0x08

void InterruptVectorHigh (void)
{
	_asm
	goto InterruptHandlerHigh         //跳到中断程序
	_endasm
}

#pragma code


// 低优先级中断服务程序

#pragma interrupt InterruptHandlerHigh

void InterruptHandlerHigh (void)
{
    if (PIR1bits.ADIF == 1)     //AD完成中断
    {
      PIR1bits.ADIF = 0;                //清除中断标志
    ADResult = ADRESH * 256;    //读取并存储A/D转换结果
    ADResult += ADRESL;
      
      if (Num < 16)
      {      
      AD_Buf[AD_number++] = ADResult;
      Num++;
      ADCON0 += 0x04;           //选择下一个通道
          ADCON0bits.GO = 1;              //启动下一次A/D转换
      }
      else
      {
          AD_count++;
          if (AD_count >= 32)
          {
              AD_over = 1;            //一周期采样完成
        AD_count = 0;
        AD_number = 0;
              T1CONbits.TMR1ON = 0;   //关闭Timer1
          }
      }
    }
    
    if (PIR1bits.TMR1IF == 1)     //定时器1溢出中断
    {
        PIR1bits.TMR1IF = 0;
      TMR1H = T1_count >> 8;
    TMR1L = (uint8_t) (T1_count & 0x0ff);
    T1CONbits.TMR1ON = 1; 
        Num = 0;                    //由通道0开始采样
    ADCON0 = 0x01;
        ADCON0bits.GO = 1;                //启动下一次A/D转换
    }
}  

// 取绝对值
static uint32_t Abs (int data)
{
    uint32_t return_data;
    
    if ((data&0x8000) == 0x8000)
    {
        return_data = data ^ 0xffff;
        return_data -=0xffff;
    }
    else
    {
        return_data = data;
    }
    
    return (return_data);
}

// 计算采样的实际电压
static void Abs_ADResult (void)
{
  uint32_t i,temp2;
  uint8_t j;
  int temp1;
  unsigned long temp=0;
  
  for (i=0; i<512; i++) {                    //取绝对值
    temp1 = AD_Buf[i] - gVol.RefVol;     //1.5V  0x1d1    3.3V  0x3ff
    temp2 = Abs(temp1);
    temp = temp2;
    temp = temp * 16600;           //16600  502*3.3*10/1023
    temp = temp >> 10;
`   AD_Buf[i] = (uint32_t)(temp & 0x0ffff);
    }
}

// 计算n路电压
static uint32_t Voltage_Deal (uint8_t n)
{
    uint8_t i;
    uint32_t j, u;
    long sum, temp;
    
     sum = 0;
    for (i=0; i<32; i++)            //sprt((U1*U1 + ....+U32*U32)/32 )       
    {
        j = 16 * i + n;
    temp = AD_Buf[j];
        sum += (temp * temp);
    }
    sum = sum >> 5;
    u = (uint32_t) (sqrt(sum));
    
  return u;
}

// 存储n次采样的计算结果
void ADResult_Deal (uint8_t times)
{
    uint8_t i, j;
  uint32_t temp;
    
    Abs_ADResult();
    
    for (i=0; i<16; i++)
    {
        j = times * 16 + i;
        ADTemp_buf[j] = Voltage_Deal(i);
    }
}


// 计算n路多次采样的平均电压值
static uint32_t Voltage_Average (uint8_t n)
{
    uint8_t i, j;
    uint32_t temp=0, return_data;
    
    for (i=0; i<8; i++)            //8次采样结果排序
    {
        for (j=i+1; j<8; j++)
        {
            if (ADTemp_buf[16*i+n] > ADTemp_buf[16*j+n])
            {
                temp = ADTemp_buf[16*i+n];
                ADTemp_buf[16*i+n] = ADTemp_buf[16*j+n];
                ADTemp_buf[16*j+n] = temp;
            }
        }
    }
    
  for (i=1; i<7; i++)       //去掉最大、最小值  平均
  {
      temp += ADTemp_buf[i*16+n];
  }
    return_data = temp / 6;
    
    return (return_data);
}


// 最后对电压值进行处理
void  ADData_Deal (void)
{
	uint8_t i;
	int temp1;
	uint32_t temp_data, temp2;
	uint32_t long_temp, long_temp1, long_temp2;

	for (i=0; i<16; i++) {
		temp_data = Voltage_Average(i);

		if (S2_Flag == 1) {    //正在校准不做处理
			Voltage[i] = temp_data;
		} else {
			temp1 = temp_data - Voltage[i];
			temp2 = Abs(temp1);    
			if (temp2 > 3) {
				long_temp1 = temp_data;
				long_temp2 = KU[i];
				long_temp = long_temp1 * long_temp2 / 1000;
				Voltage[i] = (uint32_t)(long_temp + Change[i] - 100);
				//if (Voltage[i] < 50)
				if (Voltage[i] < 500) { // 低于20V 清零
				    Voltage[i] = 0;
				}

				if (Voltage[i] < gVol.AlarmVoltage) {
					gVol.AlarmState |= ((uint32_t)1<<i);

				} else {
					gVol.AlarmState &= ~((uint32_t)1<<i);
				}
			} 
		}
	}
}



