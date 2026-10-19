/*
 * LEDExpOne.cpp
 *
 * Created: 2026/02/28 
 * Author : L Mbatha 224473710
 */ 
#define F_CPU 16000000UL   // 16 MHz
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

int main(void)
{
	// PORTD as output
	DDRD = 0xFF;

	// PC0 as input with pull-up
	DDRC &= ~(1 << 0);
	PORTC |= (1 << 0);

	uint8_t i;

	while (1)
	{
		if ((PINC & (1 << 0)) == 0)  // Button pressed
		{
			for (i = 0; i < 8; i++)
			{
				PORTD = (1 << i);
				_delay_ms(250);
				PORTD = 0;
				_delay_ms(250);
			}
		}
		else  // Button not pressed
		{
			for (i = 0; i < 8; i++)
			{
				PORTD = (1 << i);
				_delay_ms(500);
				PORTD = 0;
				_delay_ms(500);
			}
		}
	}

	
	/*DDRD = 0xFF;

	while(1)
	{

		PORTD = 255;

		_delay_ms(1000);

		PORTD = 0;

		_delay_ms(1000);

	}*/
	
	/*DDRD = 0xFF;

	DDRC &= ~(1<<0);

	PORTC |= (1<<0);

	while(1)
	{

		if((PINC & (1<<0))==0)
		PORTD = 255;
		else
		PORTD = 0;

	}*/
}

