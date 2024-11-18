#ifndef __E2PROM_H__
#define __E2PROM_H__

#define  Write_com   0xa0
#define  Read_com    0xa1

extern void I2C_Done (void);

extern void EE_Page_Write (uint32_t addr, uint8_t len, uint8_t *data);
extern void EE_Page_Read (uint32_t addr, uint8_t len, uint8_t *data);
extern void EEPROM_Write (uint32_t addr, uint8_t data);
extern void EEPROM_ACK (uint8_t ctrl);

extern uint8_t EEPROM_Read (uint32_t addr);

extern void i2c_init (void);


#endif


