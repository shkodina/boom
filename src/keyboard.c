#include <avr/io.h>
#include "keyboard.h"

#define F_CPU 1000000UL  // 16 MHz
#include <util/delay.h>

void MatrixKeyInit()
{
	MPORT = SCANMASK;
	//MDDRPORT = 0b11110000;
	PORTF = 0b00001111;
	DDRF  = 0b11110000;
}

unsigned char GetKey()
{
	unsigned char pv = 0;

/*
	for (char i = 0; i < SCANLINE; i++){
		MPORT &= ~(SCANMASK);
		MPORT |= _BV(i);
		_delay_us(1);
		pv = MPIN & SCANMAS;
		if (pv){
			switch (pv) {
				case 1:
					return 4*i+1;
					break;
				case 2:
					return 4*i+2;
					break;
				case 4:
					return 4*i+3;
					break;
				case 8:
					return 4*i+4;
					break;
				default:
					return 0;
					break;
			}
		}
	}

	PORTF = 0b00000000;
	_delay_us(1);
	
	if ( (MPIN & 0b00001111) < 0b00001111 )
	{
		PORTB = PORTB ^ (1<<6);
	}
*/
	return 0; 
}

