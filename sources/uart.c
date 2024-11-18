


#include    "main.h"


uint8_t ReadLen_flag=0;
uint8_t Uart_buf[MAX_BUF] = {0};
uint8_t Tx_buf[MAX_BUF] = {0};
uint32_t Baud;      

uint8_t Baud_n;

struct uart_t gUart;

void InterruptHandlerLow (void);





#define MAX_SUPPORT_BAUD	6

uint16_t baudrate[MAX_SUPPORT_BAUD] = {
	650, 	// 9600
	5207, 	// 1200
	2603, 	// 2400
	1301, 	// 4800
	650, 	// 9600
	325};	// 19200

// 波特率选择
void baudrate_set (uint8_t rate)
{
	uint16_t baud = 0;
	if (rate < MAX_SUPPORT_BAUD) {
		baud = baudrate[rate];
		SPBRGH1 = (baud >> 8) & 0xFF;
		SPBRG1 = baud & 0xFF;
	}
}

void uart_init (void)
{
	BAUDCON1bits.BRG16 = 1;        //选择16位波特率发生器
	TXSTA1bits.BRGH = 1;            //选择异步高速方式
	RCSTA1bits.SPEN = 1;            //使能串口引脚
	TRISCbits.TRISC6 = 0;     //RC6(TX)设置为输出
	TRISCbits.TRISC7 = 1;     //RC7(RX)设置为输入
	IPR1bits.TX1IP = 0;       //低优先级中断
	//IPR1bits.TX1IP = 1;
	RCSTA1bits.CREN = 1;            //接受数据允许
	baudrate_set(0);
	IPR1bits.RC1IP = 0;
	// IPR1bits.RC1IP = 1;
	PIE1bits.RC1IE = 1;              //接收中断使能    
	DE = 0;    
}


// 高优先级中断向量
#pragma code InterruptVectorLow = 0x18
void InterruptVectorLow (void)
{
	_asm
	goto InterruptHandlerLow         //跳到中断程
	_endasm
}

#pragma code

// 高优先级中断服务程序
#pragma interruptlow InterruptHandlerLow

void InterruptHandlerLow (void)
{
    uint32_t i;
	uint8_t read = 0;
	static uint8_t started = 0;
    
    if (PIR1bits.RC1IF == 1) {   // EUSART1 接收中断标志位 接收缓冲已满    //串口接收中断
        read = RCREG1;
        // PIR1bits.RC1IF = 0; // 读完会自动清零

        if (UartRx_over == 0) { // 接收完成标志 0:未完成 1:完成
            if (started == 0) {  // 接收到地址数据 0:未接到 1:接到
                if ((read==Address) || (read==0x00) || (read==0xff)) {   //判断地址
                    started = 1;
                    gUart.cnt_rx = 0;
                    Uart_buf[gUart.cnt_rx++] = read;
                    
                    TMR0H = T0_H;
                    TMR0L = T0_L;
                    T0CONbits.TMR0ON = 1; // 使能定时器0  60ms
                } else {
                    gUart.cnt_rx = 0;
                    started = 0;
                    UartRx_over = 0;
                    ReadLen_flag = 0;
                }
            } else {
                Uart_buf[gUart.cnt_rx++] = read;
                TMR0H = T0_H;
                TMR0L = T0_L;  // 最大间隔 60ms

                switch(gUart.cnt_rx) {
                    case 2:
                    {
                        if ((Uart_buf[1]!=0x03) && (Uart_buf[1]!=0x10))    //判断功能码
                        {
                            gUart.cnt_rx = 0;
                            started = 0;
                            UartRx_over = 0;
                            ReadLen_flag = 0;
                        }
                    }
                    break;

                    case 3: // 与4 共同组成寄存器地址 范围0x0000--0x00B6
                    {
                        if (Uart_buf[2]!=0x00)    // 寄存器地址高位 为零
                        {
                            gUart.cnt_rx = 0;
                            started = 0;
                            UartRx_over = 0;
                            ReadLen_flag = 0;
                        }
                    }
                    break;

                    case 4:
                    {
                        if (Uart_buf[3]>0xB6)    // 寄存器地址低位 目前只用了小于 0xB6个寄存器
                        {
                            gUart.cnt_rx = 0;
                            started = 0;
                            UartRx_over = 0;
                            ReadLen_flag = 0;
                        }
                    }
                    break;

                    case 5: // 与6 共同组成连续取寄存器的个数 范围1-16
                    {
                        if (Uart_buf[4]!=0x00)    // 个数高位 为零
                        {
                            gUart.cnt_rx = 0;
                            started = 0;
                            UartRx_over = 0;
                            ReadLen_flag = 0;
                        }
                    }
                    break;

                    case 6:
                    {
                        if (Uart_buf[5]>0x10)    // 个数低位 
                        {
                            gUart.cnt_rx = 0;
                            started = 0;
                            UartRx_over = 0;
                            ReadLen_flag = 0;
                        }
                    }
                    break;

                    default:
                    {
                        if (gUart.cnt_rx >= 8) {
                            if (Uart_buf[1] == 0x03)
                            {
                                gUart.len_rx = 8;
                                started = 0;
                                gUart.cnt_rx = 0; 
                                UartRx_over = 1;
                                ReadLen_flag = 0;
                                T0CONbits.TMR0ON = 0;  // 退出定时器0
                            }
                            else if (Uart_buf[1] == 0x10)
                            {
                                ReadLen_flag = 1;
                                if((Uart_buf[4]*256+Uart_buf[5])*2==Uart_buf[6])
                                {
                                    gUart.len_rx = Uart_buf[6] + 9;
                                    if (gUart.len_rx > MAX_BUF) {
                                        gUart.cnt_rx = 0;
                                        started = 0;
                                        UartRx_over = 0;
                                        ReadLen_flag = 0;
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
              
                if ((gUart.cnt_rx == gUart.len_rx) && (ReadLen_flag==1)) {    //功能码 0x10 数据接收完成
                    started = 0;
                    gUart.cnt_rx = 0; 
                    UartRx_over = 1;
                    ReadLen_flag = 0;
                    T0CONbits.TMR0ON = 0; // 退出定时器0
                }
            }
        }
    }

    else if (PIR1bits.TX1IF == 1) {   // EUSART1 发送中断标志 发送缓冲为空 已发    //串口发送中断
        if (gUart.cnt_tx < gUart.len_tx) {
            TXREG1 = Tx_buf[gUart.cnt_tx++];
        } else {
            for (i=0; i<4200; i++);
            TXSTA1bits.TXEN = 0;  // 发送使能       禁止                   //禁止发送
            PIE1bits.TX1IE = 0;   // EUSART1 发送中断使能 禁止             //禁止发送中断    
            PIR1bits.RC1IF = 0;   // EUSART1 接收中断标志 接收缓冲为空     //发送中断清零
            PIR1bits.TX1IF = 0;   // EUSART1 发送中断标志 发送缓冲为满 未发
            DE = 0;               // IO口输出 0                            //置485接收
            for (i=0; i<200; i++);// ?
            PIE1bits.RC1IE = 1;   // EUSART1 接收中断使能 使能             //接收中断使能
            Tx_flag = 0;
        }
    }    
  
    if (INTCONbits.TMR0IF == 1) {   // 定制起0 超时中断标志 已超时   //定时器0溢出中断
        INTCONbits.TMR0IF = 0;    // 须由软件清零超时标志

        started = 0;
        gUart.cnt_rx = 0; 
        UartRx_over = 0;
    }
}

static void Tx_Data (void)
{
    uint32_t crc_result=0;
    uint8_t crc_h, crc_l, i, len;

    Tx_buf[0] = Address;
    crc_result = CRC16_Table(&Tx_buf[0], gUart.len_tx-2);
    crc_h = crc_result >> 8;    
    crc_l = crc_result & 0x0ff; 
    if (0 == gVol.switch_crc) {
        Tx_buf[gUart.len_tx-2] = crc_h;
        Tx_buf[gUart.len_tx-1] = crc_l;
    } else {
        Tx_buf[gUart.len_tx-2] = crc_l;
        Tx_buf[gUart.len_tx-1] = crc_h;
    }

    gUart.cnt_tx = 0;
    Tx_flag = 1;
  
    DE = 1;                   // IO口输出 1                       //置485 发送
    Delay_nms(10);
    TXSTA1bits.TXEN = 1;      // 发送使能位 使能                  //发送允许
    PIE1bits.RC1IE = 0;       // EUSART1 接收中断使能位 禁止      //禁止接收中断使能
    PIE1bits.TX1IE = 1;       // EUSART1 发送中断使能位 使能      //发送中断使能
    //PIR1bits.RC1IF = 1;       // EUSART1 接收中断标志 接收缓冲满?  //置发送中断标志
    //PIR1bits.TX1IF = 1;//置发送中断标志
}

//-------------------------------------------------------------//










void RxCom_Deal (void)
{
    uint32_t crc_result, addr1, addr2, addr, num;
    uint8_t crc_h, crc_l, data_h, data_l, i, n;
    uint8_t crc_ok=0;
    
    crc_result = CRC16_Table(&Uart_buf[0], gUart.len_rx-2); 
    crc_h = crc_result >> 8;
    crc_l = crc_result & 0x0ff;

    if ((Uart_buf[gUart.len_rx-2]==crc_l) && (Uart_buf[gUart.len_rx-1]==crc_h)) {
        gVol.switch_crc = 1;    
        crc_ok = 1;
    } else if ((Uart_buf[gUart.len_rx-2]==crc_h) && (Uart_buf[gUart.len_rx-1]==crc_l)) {
        gVol.switch_crc = 0;    
        crc_ok = 1;
    }

    if (crc_ok) {
        if (Uart_buf[0] == Address) {
            if (Uart_buf[1] == 0x03) {          //读寄存器命令
                addr = Uart_buf[2]*256 + Uart_buf[3];    //寄存器起始地址
                num = Uart_buf[4]*256 + Uart_buf[5];     //寄存器个数
                
                if ((addr>=REG_Voltage_BASE) && (addr<=REG_Voltage_END)) {    //读16路电压值
                    if ((addr+num) <= REG_Voltage_END+1) {
                        Tx_buf[1] = 0x03;
                        Tx_buf[2] = num * 2;
                        gUart.len_tx = Tx_buf[2] + 5;
                        for (i=0; i<num; i++) {
                            n = addr + i;
                            data_h = Voltage[n-REG_Voltage_BASE] >> 8;
                            data_l = Voltage[n-REG_Voltage_BASE] & 0x0ff;
                            Tx_buf[i*2+3] = data_h;
                            Tx_buf[i*2+4] = data_l;                        
                        } 
                    } else {
                        Tx_buf[1] = 0x83;
                        Tx_buf[2] = 0x02;
                        gUart.len_tx = 5;
                    }
                }
                else if(addr==REG_AlarmState)
                {
                    Tx_buf[1] = 0x03;
                    Tx_buf[2] = 2;
                    data_h = gVol.AlarmState >> 8;
                    data_l = gVol.AlarmState & 0x0FF;
                    Tx_buf[3] = data_h;
                    Tx_buf[4] = data_l;
                    gUart.len_tx = 7;
                }
                else if ((addr>=REG_V_Rate_BASE) && (addr<=REG_V_Rate_END))    //读电压校正比率
                {
                    if ((addr+num) <= (REG_V_Rate_END+1))
                    {
                        Tx_buf[1] = 0x03;
                        Tx_buf[2] = num * 2;
                        gUart.len_tx = Tx_buf[2] + 5;
                        
                        for (i=0; i<num; i++)
                        {
                            n = addr + i;
                            data_h = KU[n-REG_V_Rate_BASE] >> 8;
                            data_l = KU[n-REG_V_Rate_BASE] & 0x0ff;
                            Tx_buf[i*2+3] = data_h;
                            Tx_buf[i*2+4] = data_l;
                        } 
                    }
                    else 
                    {
                        Tx_buf[1] = 0x83;
                        Tx_buf[2] = 0x02;
                        gUart.len_tx = 5;
                    }
                }
                else if ((addr>=REG_Offset_BASE) && (addr<=REG_Offset_END))    //读电压校正偏移量
                {
                    if ((addr+num) <= (REG_Offset_END+1))
                    {
                        Tx_buf[1] = 0x03;
                        Tx_buf[2] = num * 2;
                        gUart.len_tx = Tx_buf[2] + 5;

                        for (i=0; i<num; i++)
                        {
                            n = addr + i;
                            Tx_buf[i*2+3] = 0x00;
                            Tx_buf[i*2+4] = Change[n-REG_Offset_BASE];
                        } 
                    }
                    else 
                    {
                        Tx_buf[1] = 0x83;
                        Tx_buf[2] = 0x02;
                        gUart.len_tx = 5;
                    }
                }
                else if ((addr>=REG_ADDRESS) && (addr<=REG_AlarmVoltage))     //读系统参数
                {
                    switch (addr)
                    {
                        case REG_ADDRESS:
                            {
                                Tx_buf[1] = 0x03;
                                Tx_buf[2] = 2;
                                Tx_buf[3] = 0x00;
                                Tx_buf[4] = Address;
                                gUart.len_tx = 7;
                            }
                            break;
                  
                        case REG_BAUD:
                            {
                                Tx_buf[1] = 0x03;
                                Tx_buf[2] = 2;
                                Tx_buf[3] = 0x00;
                                Tx_buf[4] = Baud_n;
                                gUart.len_tx = 7;
                            }
                            break;

                        case REG_AlarmVoltage:
                            {
                                Tx_buf[1] = 0x03;
                                Tx_buf[2] = 2;
                                data_h = gVol.AlarmVoltage >> 8;
                                data_l = gVol.AlarmVoltage & 0xFF;
                                Tx_buf[3] = data_h;
                                Tx_buf[4] = data_l;
                                gUart.len_tx = 7;
                            }
                            break;

                        default:
                            break; 
                    }
                }
                else 
                {
                    Tx_buf[1] = 0x83;
                    Tx_buf[2] = 0x02;
                    gUart.len_tx = 5;
                }
                Tx_Data();
            }
            else if (Uart_buf[1] == 0x10)       //写寄存器命令
            {
                unsigned char change_baud=0;
                addr = Uart_buf[2]*256 + Uart_buf[3];      //寄存器起始地址
                num = Uart_buf[4]*256 + Uart_buf[5];       //寄存器个数
                
                if ((addr>=REG_V_Rate_BASE) && (addr<=REG_V_Rate_END))      //写电压校正比率
                {
                    if ((addr+num) <= REG_V_Rate_END+1)
                    {
                        for (i=0; i<num; i++)
                        {
                            n = addr + i -REG_V_Rate_BASE;
                            addr1 = KU_ADDR + (2*n);
                            addr2 = addr1 + 1;
                            EEPROM_Write(addr2, Uart_buf[i*2+7]);
                            Delay_nms(10);
                            EEPROM_Write(addr1, Uart_buf[i*2+8]);
                            Delay_nms(10);
                            data_l = EEPROM_Read(addr1);
                            data_h = EEPROM_Read(addr2);
                            KU[n] = data_h*256 + data_l;
                        }
                        Tx_buf[1] = 0x10;
                        Tx_buf[2] = Uart_buf[2];
                        Tx_buf[3] = Uart_buf[3];
                        Tx_buf[4] = Uart_buf[4];
                        Tx_buf[5] = Uart_buf[5];
                        gUart.len_tx = 8;
                    }
                    else 
                    {
                        Tx_buf[1] = 0x90;
                        Tx_buf[2] = 0x02;
                        gUart.len_tx = 5;
                    }
                }
                else if ((addr>=REG_Offset_BASE) && (addr<=REG_Offset_END))     //写电压校正偏移量
                {
                    if ((addr+num) <= (REG_Offset_END+1))
                    {
                        for (i=0; i<num; i++)
                        {
                            n = addr + i - REG_Offset_BASE;
                            addr1 = Change_ADDR + n;
                            EEPROM_Write(addr1, Uart_buf[i*2+8]);
                            Delay_nms(10);
                            Change[n] = EEPROM_Read(addr1);
                        }

                        Tx_buf[1] = 0x10;
                        Tx_buf[2] = Uart_buf[2];
                        Tx_buf[3] = Uart_buf[3];
                        Tx_buf[4] = Uart_buf[4];
                        Tx_buf[5] = Uart_buf[5];
                        gUart.len_tx = 8;
                    }
                    else 
                    {
                        Tx_buf[1] = 0x90;
                        Tx_buf[2] = 0x02;
                        gUart.len_tx = 5;
                    }
                }
                else if ((addr>=REG_ADDRESS) && (addr<=REG_AlarmVoltage))    //写系统参数
                {
                    switch (addr)
                    {
                        case REG_ADDRESS:
                            {
                                if ((Uart_buf[8]>0) && (Uart_buf[8]<255))
                                {
                                    EEPROM_Write(Addr485_ADDR, Uart_buf[8]);
                                    Delay_nms(10);
                                    Address = EEPROM_Read(Addr485_ADDR);
                                    Tx_buf[1] = Uart_buf[1];
                                    Tx_buf[2] = Uart_buf[2];
                                    Tx_buf[3] = Uart_buf[3];
                                    Tx_buf[4] = Uart_buf[4];
                                    Tx_buf[5] = Uart_buf[5];
                                    gUart.len_tx = 8;
                                }
                                else
                                {
                                    Tx_buf[1] = 0x90;
                                    Tx_buf[2] = 0x03;
                                    gUart.len_tx = 5;
                                }
                            }
                            break;
                  
                        case REG_AlarmVoltage:
                            {
                                if ((Uart_buf[7]>0) && (Uart_buf[7]<0x5))
                                {
                                    EEPROM_Write(AlarmVoltage_ADDR, Uart_buf[8]);
                                    Delay_nms(10);
                                    EEPROM_Write(AlarmVoltage_ADDR+1, Uart_buf[7]);
                                    Delay_nms(10);
                                    data_l = EEPROM_Read(AlarmVoltage_ADDR);
                                    data_h = EEPROM_Read(AlarmVoltage_ADDR+1);
                                    gVol.AlarmVoltage = data_h*256+data_l;
                                    Tx_buf[1] = Uart_buf[1];
                                    Tx_buf[2] = Uart_buf[2];
                                    Tx_buf[3] = Uart_buf[3];
                                    Tx_buf[4] = Uart_buf[4];
                                    Tx_buf[5] = Uart_buf[5];
                                    gUart.len_tx = 8;
                                }
                                else
                                {
                                    Tx_buf[1] = 0x90;
                                    Tx_buf[2] = 0x03;
                                    gUart.len_tx = 5;
                                }
                            }
                            break;

                        case REG_BAUD:
                            {
                                if ((Uart_buf[8]>0) && (Uart_buf[8]<6))
                                {
                                    EEPROM_Write(Baud_ADDR, Uart_buf[8]);
                                    Delay_nms(10);
                                    Baud_n = EEPROM_Read(Baud_ADDR);
                                    Tx_buf[1] = Uart_buf[1];
                                    Tx_buf[2] = Uart_buf[2];
                                    Tx_buf[3] = Uart_buf[3];
                                    Tx_buf[4] = Uart_buf[4];
                                    Tx_buf[5] = Uart_buf[5];
                                    gUart.len_tx = 8;
                                    Tx_Data();
                                    change_baud=1;
                                    Delay_nms(100);

									baudrate_set(Baud_n);
                                }
                                else
                                {
                                    Tx_buf[1] = 0x90;
                                    Tx_buf[2] = 0x03;
                                    gUart.len_tx = 5;
                                }
                            }
                            break;

                        default:
                            break; 
                    }
                }
                else     //非法数据地址
                {
                    Tx_buf[1] = 0x90;            
                    Tx_buf[2] = 0x02;
                    gUart.len_tx = 5;      
                }

                if(change_baud!=1)
                {
                    Tx_Data(); 
                }
            } 
        }
        else if (Uart_buf[0] == 0xff)     //广播设置波特率命令
        {
            addr = Uart_buf[2]*256 + Uart_buf[3];
            num = Uart_buf[4]*256 + Uart_buf[5];

            if ((Uart_buf[1]==0x10)&&(addr==REG_BAUD) && (num==1))
            {
                if ((Uart_buf[8]>0) && (Uart_buf[8]<6))
                {
                    EEPROM_Write(Baud_ADDR, Uart_buf[8]);
                    Delay_nms(10);
                    Baud_n = EEPROM_Read(Baud_ADDR);

					baudrate_set(Baud_n);
                }
            }
        }
        else if (Uart_buf[0]==0x00)     //单机读地址命令
        {
            addr = Uart_buf[2]*256 + Uart_buf[3];
            num = Uart_buf[4]*256 + Uart_buf[5];

            if ((Uart_buf[1]==0x03) && (addr==REG_ADDRESS) && (num==1))
            {
                Tx_buf[1] = 0x03;
                Tx_buf[2] = 2;
                Tx_buf[3] = 0x00;
                Tx_buf[4] = Address;
                gUart.len_tx = 7;
                Tx_Data();
            }
        }
    }
}

//-------------------------------------------------------------//



