/*
 * 
 *

 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000L
#include <util/delay.h>
#include "i2c_slave.h"
#include "led.h"
//---makra preprocesora----------
#define TWI_CMD_MASTER_WRITE 0x10
#define TWI_CMD_MASTER_READ  0x20
#define TWI_CMD_MASTER_WRITE_ALOT 0x30
#define PORT(x) SPORT(x)
#define SPORT(x) (PORT##x)

#define PIN(x) SPIN(x)
#define SPIN(x) (PIN##x)

#define DDR(x) SDDR(x)
#define SDDR(x) (DDR##x)

unsigned char tab[TWI_RX_BUFFER_MASK];




int main(void)
{
	 unsigned char TWI_slaveAddress, temp;
	AC_init();
//	I2C_init(0x1D);
	//AC();
	 TWI_slaveAddress = 0x66;
	 USI_TWI_Slave_Initialise( TWI_slaveAddress );

	  //__enable_interrupt();
	diodyInit();
	zgasDiode(1);
	zgasDiode(5);
	zgasDiode(4);
	zgasDiode(3);
	zgasDiode(2);
	
	sei();									// Enable interrupts
		

    /* Replace with your application code */
    while (1) 
    {
	
		DiodaNaladowana(AC());
	//	sprintf(tab,"Wynik:  %d ", AC());
	temp =  'M';
	USI_TWI_Transmit_Byte(temp);
		temp =  'm';
	USI_TWI_Transmit_Byte(temp);
		temp =  'm';
	USI_TWI_Transmit_Byte(temp);
		temp =  'i';
	USI_TWI_Transmit_Byte(temp);
		temp =  's';
	USI_TWI_Transmit_Byte(temp);
		temp =  'm';
	USI_TWI_Transmit_Byte(temp);
		temp =  'i';
	USI_TWI_Transmit_Byte(temp);
		temp =  's';
	USI_TWI_Transmit_Byte(temp);

			_delay_ms(1000);
		//}
    }
}

