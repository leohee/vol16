
#include "main.h"
#include "i2c.h"           // Load I2C Header file from defult direct



//***********************************************
//*         Write a Byte to EEPROM              *
//*   - ctrl : Control Byte of EEPROM           *
//*   - addr : Location of EEPROM               *
//*   - data : Data Byte of EEPROM              *
//***********************************************

void EEPROM_Write(uint32_t addr,uint8_t data)
{
  uint8_t addr_h, addr_l;
  
  addr_h = addr >> 8;
  addr_l = (uint8_t) (addr & 0x0ff);
  
  IdleI2C();                // ensure module is idle
  StartI2C();               // Start condition
  I2C_Done();               // Wait Start condition completed and clear SSPIF flag    
  
  WriteI2C(Write_com);      // Write Control+Write to EEPROM & Check BF flag
  //while (SSP1CON2bits.ACKSTAT);  // wait until received the Acknowledge from EEPROM
  nop();
  nop(); 
  I2C_Done();               // Clear SSPIF flag
  
  WriteI2C(addr_h);         // Write Address to EEPROM
  //while (SSPCON2bits.ACKSTAT);  // wait until received the Acknowledge from EEPROM
  nop();
  nop(); 
  I2C_Done();
  
  WriteI2C(addr_l);         // Write Address to EEPROM
  //while (SSPCON2bits.ACKSTAT);  // wait until received the Acknowledge from EEPROM
  nop();
  nop();
  I2C_Done();
  
  WriteI2C(data);           // Write Data to EEPROM
  //while (SSPCON2bits.ACKSTAT);  // wait until received the Acknowledge from EEPROM
  nop();
  nop(); 
  I2C_Done();
  
  StopI2C();                // Stop condition
  I2C_Done();               // Wait the Stop condition completed
}

//***********************************************
//*         Pae Write to EEPROM                
//*
//*   - ctrl : Control Byte of EEPROM  
//*   - addr : Location of EEPROM    
//*   - length : Write counter
//*   - *dptr : RAM point --> EEPROM
//* 
//***********************************************
void EE_Page_Write(uint32_t addr,uint8_t len,uint8_t *data)
{
  uint8_t addr_h, addr_l;
  
  addr_h = addr >> 8;
  addr_l = (uint8_t) (addr & 0x0ff);
  
  IdleI2C();                // ensure module is idle
  StartI2C();               // Start condition
  I2C_Done();               // Wait Start condition completed
  
  WriteI2C(Write_com);      // Write Control+Write to EEPROM & Check BF flag
  while (SSPCON2bits.ACKSTAT);  // wait until received the Acknowledge from EEPROM
  I2C_Done();               // Clear SSPIF flag
  
  WriteI2C(addr_h);         // Write Address to EEPROM
  while (SSPCON2bits.ACKSTAT);  // wait until received the Acknowledge from EEPROM
  I2C_Done();
  
  WriteI2C(addr_l);         // Write Address to EEPROM
  while (SSPCON2bits.ACKSTAT);  // wait until received the Acknowledge from EEPROM
  I2C_Done();
  
  while (len != 0)          // Check write completed 
  {
    WriteI2C(*data);        // Write data to EEPROM
    while (SSPCON2bits.ACKSTAT); // wait until received the Acknowledge from EEPROM
    I2C_Done(); 
    data++;                 // Point to next byte
    len--;
  }
  
  StopI2C();                // Stop condition
  I2C_Done();               // Wait the Stop condition completed
} 
 
//***********************************************
//*         EEPROM Acknowledge Polling          *
//*   --  The routine will polling the ACK      *
//*       response from EEPROM                  *
//*   --  ACK=0 return                          *
//*   --  ACK=1 send Restart & loop check       *
//***********************************************
void EEPROM_ACK(uint8_t ctrl)
{
  uint8_t i;
  
  IdleI2C();                // ensure module is idle
  StartI2C();               // Start condition
  I2C_Done();               // Wait Start condition completed
  
  WriteI2C(ctrl);           // Write Control to EEPROM (WRITE)
  I2C_Done();               // Clear SSPIF flag
  
  while (SSPCON2bits.ACKSTAT)  // test for Acknowledge from EEPROM
  {
    for (i=0; i<100; i++);     // Delay for next Repet-Start
  
    RestartI2C();               // initiate Repet-Start condition
    I2C_Done();             // Wait Repet-Start condition completed
  
    WriteI2C(ctrl);         // Write Control to EEPROM (WRITE)
    I2C_Done();             // Clear SSPIF flag 
  }   
  StopI2C();                // send STOP condition
  I2C_Done();               // wait until stop condition is over 
}

//***********************************************
//*    Random Read a Byte from EEPROM           *
//*   - ctrl : Control Byte of EEPROM (Write)   *
//*     (Ctrl +1 ) : Read Command               *
//*   - addr : Address Byte of EEPROM           *
//*   - Return : Read Data from EEPROM          *
//***********************************************

uint8_t EEPROM_Read(uint32_t addr)
{
  uint8_t data, addr_h, addr_l;
  
  addr_h = addr >> 8;
  addr_l = (uint8_t) (addr & 0x0ff);
  
  IdleI2C();                // ensure module is idle
  StartI2C();               // Start condition
  I2C_Done();               // Wait Start condition completed
  
  WriteI2C(Write_com);      // Write Control to EEPROM    
//  while (SSPCON2bits.ACKSTAT);    // test for ACK condition, if received
  nop();
  nop();
  I2C_Done();               // Clear SSPIF flag
  
  WriteI2C(addr_h);         // Write Address to EEPROM         
//  while (SSPCON2bits.ACKSTAT);   // test for ACK condition, if received  
  nop();
  nop();  
  I2C_Done();               // Clear SSPIF flag
  
  WriteI2C(addr_l);         // Write Address to EEPROM         
//  while (SSPCON2bits.ACKSTAT);   // test for ACK condition, if received  
  nop();
  nop();
  I2C_Done();
  
  RestartI2C();             // initiate Restart condition
  I2C_Done();
  
  WriteI2C(Read_com);       // Write Control to EEPROM              
//  while (SSPCON2bits.ACKSTAT);    // test for ACK condition, if received
  nop();
  nop();
  I2C_Done();               // Clear SSPIF flag
  
  data = ReadI2C();         // Enable I2C Receiver & wait BF=1 until received data
  I2C_Done();               // Clear SSPIF flag
  
  NotAckI2C();              // Genarate Non_Acknowledge to EEPROM
  I2C_Done();    
                      
  StopI2C();                // send STOP condition
  I2C_Done();               // wait until stop condition is over 
  
  return (data);            // Return Data from EEPROM 
}

//***********************************************
//*         Sequential Read from EEPROM                
//*
//*   - ctrl : Control Byte of EEPROM  
//*   - addr : Location of EEPROM    
//*   - length : Read counter
//*   - *dptr : Store EEPROM data to RAM 
//* 
//***********************************************

void EE_Page_Read(uint32_t addr, uint8_t len, uint8_t *data)
{
  uint8_t addr_h, addr_l;
  
  addr_h = addr >> 8;
  addr_l = (uint8_t) (addr & 0x0ff);
  
  IdleI2C();                // ensure module is idle
  StartI2C();               // Start condition
  I2C_Done();               // Wait Start condition completed
  
  WriteI2C(Write_com);      // Write Control to EEPROM    
  while (SSPCON2bits.ACKSTAT);    // test for ACK condition, if received
  I2C_Done();               // Clear SSPIF flag
  
  WriteI2C(addr_h);         // Write Address to EEPROM         
  while (SSPCON2bits.ACKSTAT);   // test for ACK condition, if received  
  I2C_Done();               // Clear SSPIF flag
  
  WriteI2C(addr_l);         // Write Address to EEPROM         
  while (SSPCON2bits.ACKSTAT);   // test for ACK condition, if received  
  I2C_Done(); 
  
  RestartI2C();             // initiate Restart condition
  I2C_Done();
  
  WriteI2C(Read_com);       // Write Control to EEPROM              
  while (SSPCON2bits.ACKSTAT);    // Test for ACK condition, if received
  I2C_Done();               // Clear SSPIF flag
  
  while (len != 0)
  {
    *data = ReadI2C();      // Enable I2C Receiver & Store EEPROM data to Point buffer 
    I2C_Done(); 
    data++; 
    len--;
      
    if (len == 0) NotAckI2C();
    else AckI2C();          // Continue read next data, send a acknowledge to EEPROM
    
    I2C_Done();
  }
  
  StopI2C();                // send STOP condition
  I2C_Done();               // wait until stop condition is over 
}

