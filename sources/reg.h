#ifndef __REG_H__
#define __REG_H__


#define First_Flag_ADDR     0x011
#define Addr485_ADDR        0x012
#define PassWord_ADDR       0x013
#define DisPage_ADDR        0x017
#define DisMode_ADDR        0x018
#define Baud_ADDR           0x019
#define Change_ADDR         0x020     //16 byte  0x020~0x02f
#define KU_ADDR             0x040     //32 byte  0x040~0x05f
#define JiZhun_ADDR         0x060

#define AlarmVoltage_ADDR         0x032    //告警电压值

#define REG_Voltage_BASE     (0x0000)
/*#define REG_Voltage_01       (0x0000)
#define REG_Voltage_02       (0x0001)
#define REG_Voltage_03       (0x0002)
#define REG_Voltage_04       (0x0003)
#define REG_Voltage_05       (0x0004)
#define REG_Voltage_06       (0x0005)
#define REG_Voltage_07       (0x0006)
#define REG_Voltage_08       (0x0007)
#define REG_Voltage_09       (0x0008)
#define REG_Voltage_10       (0x0009)
#define REG_Voltage_11       (0x000A)
#define REG_Voltage_12       (0x000B)
#define REG_Voltage_13       (0x000C)
#define REG_Voltage_14       (0x000D)
#define REG_Voltage_15       (0x000E)
#define REG_Voltage_16       (0x000F)*/
#define REG_Voltage_END      (0x000F)

#define REG_AlarmState       (0x0020)

#define REG_V_Rate_BASE      (0x0080)
/*#define REG_V_Rate_01        (0x0080)
#define REG_V_Rate_02        (0x0081)
#define REG_V_Rate_03        (0x0082)
#define REG_V_Rate_04        (0x0083)
#define REG_V_Rate_05        (0x0084)
#define REG_V_Rate_06        (0x0085)
#define REG_V_Rate_07        (0x0086)
#define REG_V_Rate_08        (0x0087)
#define REG_V_Rate_09        (0x0088)
#define REG_V_Rate_10        (0x0089)
#define REG_V_Rate_11        (0x008A)
#define REG_V_Rate_12        (0x008B)
#define REG_V_Rate_13        (0x008C)
#define REG_V_Rate_14        (0x008D)
#define REG_V_Rate_15        (0x008E)
#define REG_V_Rate_16        (0x008F)*/
#define REG_V_Rate_END       (0x008F)

#define REG_Offset_BASE      (0x0090)
/*#define REG_Offset_01        (0x0090)
#define REG_Offset_02        (0x0091)
#define REG_Offset_03        (0x0092)
#define REG_Offset_04        (0x0093)
#define REG_Offset_05        (0x0094)
#define REG_Offset_06        (0x0095)
#define REG_Offset_07        (0x0096)
#define REG_Offset_08        (0x0097)
#define REG_Offset_09        (0x0098)
#define REG_Offset_10        (0x0099)
#define REG_Offset_11        (0x009A)
#define REG_Offset_12        (0x009B)
#define REG_Offset_13        (0x009C)
#define REG_Offset_14        (0x009D)
#define REG_Offset_15        (0x009E)
#define REG_Offset_16        (0x009F)*/
#define REG_Offset_END       (0x009F)

#define REG_ADDRESS          (0x00B0)
#define REG_BAUD             (0x00B1)
#define REG_PASSWORD         (0x00B2)
#define REG_SHOW_MODE        (0x00B3)
#define REG_Default_Page     (0x00B4)
#define REG_Backlight        (0x00B5)
#define REG_AlarmVoltage     (0x00B6)

#endif

