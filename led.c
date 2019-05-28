/*
 * led.c
 *
 * Created: 13.12.2018 01:16:49
 *  Author: Misiek
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include "led.h"
void AC_init()
{
	ADMUX = (1<<REFS1) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0) ; //to jest ok (chyba)
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);// | (1<<ADSC);
	//ADCSRB = (1<<ADLAR);
	DIDR0 = (1<<ADC7D); //(1<<ADC6D) | (1<<ADC5D) | (1<<ADC4D) | (1<<ADC3D) | (1<<ADC2D) | (1<<ADC1D) | (1<<ADC0D); //ok moze zostac
	
//	ADMUX = 0b10000111;
	//ADCSRA = 0b10000000;
	//ADCSRB = 0b10010000;
	
}
int AC()
{
	ADCSRA |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC));
	//ADCSRA = (1<<ADEN);
	return ADC;
}

void DiodaNaladowana(int ac)
{
	if (ac>=855)
	{
		zapalDiode(NaladowanaBateriaWPelni);
		zgasDiode(NaladowanaBateria);
		zgasDiode(SrednioNaladowana);
		zgasDiode(BliskaRozladowania);
	} 
	else if(ac>= 500 && ac<855)
	{
		zgasDiode(NaladowanaBateriaWPelni);
		zapalDiode(NaladowanaBateria);
		zgasDiode(SrednioNaladowana);
		zgasDiode(BliskaRozladowania);		
	}
	else if(ac>= 1 && ac<500)
	{
		zgasDiode(NaladowanaBateriaWPelni);
		zgasDiode(NaladowanaBateria);
		zapalDiode(SrednioNaladowana);
		zgasDiode(BliskaRozladowania);
	}	
	else if(ac>= 0 && ac<1)
	{
		zgasDiode(NaladowanaBateriaWPelni);
		zgasDiode(NaladowanaBateria);
		zgasDiode(SrednioNaladowana);
		zapalDiode(BliskaRozladowania);
	}	
	else
	{
		zapalDiode(5);
		_delay_ms(500);
		zgasDiode(5);
		_delay_ms(1000);
		//tu jak bedzie tranzystor bedzie trzeba odcianac zasilanie reszty ukladu
	}
}
/*
void I2C_init(uint8_t address)
{
	USICR = (1<<USISIE) 
	| (0<<USIOIE) 
	| (0<<USIWM1) | (1<<USIWM0) 
	| (1<<USICS1) | (0<<USICLK) 
	| (1<<USITC);
	
	USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) | (1<<USIDC);
	DDR(PORT_SDA) |= (1<<PIN_SDA); //sda jako wyjscie
	DDR(PORT_SCL) |= (1<<PIN_SCL); //sda jako wyjscie
	PORT(PORT_SDA) |= (1<<PIN_SDA); //stan wysoki na sda
 	PORT(PORT_SCL) |= (1<<PIN_SCL); //stan wysoki na scl
	DDR(PORT_SDA) &= ~(1<<PIN_SDA);	//sda jako wejscie
	
	USICR |= (1<<USISIE);
}*/

//obs³uga diod
void diodyInit()
{
	DDR(LED_75) |= (1<<L_75);
	DDR(LED_50) |= (1<<L_50);
	DDR(LED_25) |= (1<<L_25);
	DDR(LED_0)  |= (1<<L_0);
	DDR(ERROR)  |= (1<<ErrD);
}
void zapalDiode(int dioda)
{
	switch(dioda)
	{
		case 1: PORT(LED_75) &= ~(1<<L_75); break;
		case 2: PORT(LED_50) &= ~(1<<L_50); break;
		case 3: PORT(LED_25) &= ~(1<<L_25); break;
		case 4: PORT(LED_0)  &= ~(1<<L_0); break;
		case 5: PORT(ERROR)  &= ~(1<<ErrD); break;
		
	}
}
void zgasDiode(int dioda)
{
	switch(dioda)
	{
		case 1: PORT(LED_75) |= (1<<L_75); break;
		case 2: PORT(LED_50) |= (1<<L_50); break;
		case 3: PORT(LED_25) |= (1<<L_25); break;
		case 4: PORT(LED_0) |= (1<<L_0); break;
		case 5: PORT(ERROR) |= (1<<ErrD); break;
	}
}