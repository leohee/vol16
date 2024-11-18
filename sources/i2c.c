#include "main.h"
#include "i2c.h"

//***********************************************
//*   Check I2C action that is completed        *
//***********************************************

void I2C_Done (void)
{
	while (!PIR1bits.SSPIF);  // Completed the action when the SSPIF is Hi.

	PIR1bits.SSPIF = 0;       // Clear SSPIF
}

//************************************************
//* Initial I2C Master Mode with 7 bits Address  *
//* Clock Speed : 100KHz @4MHz      *
//************************************************
void i2c_init (void)
{
	OpenI2C(MASTER,SLEW_ON);
	SSPADD = 15;
}


