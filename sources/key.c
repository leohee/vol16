

/*
    对按键的扫描采用查询方式
*/

#include    "main.h"





// 初始化子程序
void key_init (void)
{
	TRISHbits.TRISH0 = 1;       //SW1,输入
	TRISHbits.TRISH1 = 1;       //SW2,输入
	TRISEbits.TRISE2 = 1;       //BUTTON1,输入
	TRISEbits.TRISE3 = 1;       //BUTTON2,输入 
	TRISEbits.TRISE4 = 1;       //BUTTON3,输入
	TRISEbits.TRISE5 = 1;       //DI1,输入 
	TRISEbits.TRISE5 = 1;       //DI2,输入
}

// 键扫描子程序
void Key_Scan (void)
{    
    if (S1 == 0)
    {
        S1_Count0++;
        S1_Count1 = 0;
        if ((S1_Count0>2) && (S1_Flag==0))
        {
          S1_Count0 = 0;
           S1_Flag = 1;
            
        }
    }
    else
    {
      S1_Count1++;
        S1_Count0 = 0;
        if ((S1_Count1>2) && (S1_Flag==1))
        {
          S1_Count1 = 0;
            S1_Flag = 0;
            
            Calibrate1_times = 0;
            Calibrate1_over = 0;
            
        }
    }
    
    if (S2 == 0)
    {
        S2_Count0++;
        S2_Count1 = 0;
        if ((S2_Count0>2) && (S2_Flag==0))
        {
          S2_Count0 = 0;
           S2_Flag = 1;
           
        }
    }
    else
    {
      S2_Count1++;
        S2_Count0 = 0;
        if ((S2_Count1>2) && (S2_Flag==1))
        {
          S2_Count1 = 0;
            S2_Flag = 0;
            
            
            Calibrate2_times = 0;
            Calibrate2_over = 0;
        }
    }
    
    if (DI1 == 0)
    {
        DI1_Count0++;
        DI1_Count1 = 0;
        if ((DI1_Count0>2) && (DI1_Flag==0))
        {
          DI1_Count0 = 0;
           DI1_Flag = 1;
           
            EEPROM_Write(Addr485_ADDR, 0x01);
      Delay_nms(10);
        EEPROM_Write(Baud_ADDR, 0x04);
        Delay_nms(10);
        Address = EEPROM_Read(Addr485_ADDR);
        Baud_n = EEPROM_Read(Baud_ADDR);

		baudrate_set(Baud_n);
        }
    }
    else
    {
      DI1_Count1++;
        DI1_Count0 = 0;
        if (DI1_Count1>2)
        {
          DI1_Count1 = 0;
            DI1_Flag = 0;
            
            
        }
    }
    
    if (DI2 == 0)
    {
        DI2_Count0++;
        DI2_Count1 = 0;
        if ((DI2_Count0>2) && (DI2_Flag==0))
        {
          DI2_Count0 = 0;
           DI2_Flag = 1;
           
            
        }
    }
    else
    {
      DI2_Count1++;
        DI2_Count0 = 0;
        if (DI2_Count1>2)
        {
          DI2_Count1 = 0;
            DI2_Flag = 0;
            
            
        }
    }
                
}
    
//-------------------------------------------------------------//




