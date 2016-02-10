#include "onewire.h"

#include <avr/interrupt.h>

#define F_CPU 16000000UL  // 16 MHz
#include <util/delay.h>

//-------------------------------------------------------------------

unsigned char tcnt, i;
char buffer[8][8];


//-------------------------------------------------------------------

char check_key_id(char* keyid, char keylen)
{
//LCDSendCommand(CLR_DISP);

	unsigned char bt = 0;
	for (int n = 0; n < keylen; n++)
	{
		bt = 0;
	    for(int n2 = 0;n2 < 8; n2++)
	    {
	 		if (buffer[n][n2]) {
				bt |= (1 << n2);
			}
	    }
		keyid[n]=bt;
		//LCDSendUnsafeCounteredTxt(bt, 1);
	}    
	return 1;
}

//-------------------------------------------------------------------

//Функция сброса DS1990

char reset_ds1990(char* keyid, char keylen)
{
	DDRB|=_BV(0);        // Вывод порта настраиваем как выход
	PORTB&=~_BV(0);

	cli();
	_delay_us(580); //Устанавливаем 0 в теч. около 480 мкс

	DDRB&=~_BV(0);       //Вывод порта настраиваем как вход
	_delay_us(70);  //Ожидание сигнала присутствия 70 мкс
   	if(PINB & (1<<PINB0))  //Если 1 - нет сигнала присутствия
   	{
   		_delay_us(410); //410 мкс
   		sei();
		return 0;
   	}   
  	else
  	{
  		_delay_us(410); //410 мкс
  		sei();
  		send_ds1990_command(0x33);
  		read_ds1990(keyid, keylen);
		return 1;
  	}
}

//-------------------------------------------------------------------

//Чтение

void read_ds1990(char* keyid, char keylen)
{ 
	cli(); 
	  for(i = 0; i < 8; i++) // байтовый цикл
	  {

	    for(tcnt = 0; tcnt < 8; tcnt++)          // битовый цикл
	    {

	    DDRB|=_BV(0);
	    PORTB&=~_BV(0);
	    _delay_us(3);// ждем 6 мкс
	    PORTB&=~_BV(0);
	    DDRB&=~_BV(0);
	    _delay_us(6); // ждем 9 мкс

	 
	            if(PINB & (1 << PINB0))
	            {
	            buffer[i][tcnt] = 1;
	            }
	            else
	            {
	            buffer[i][tcnt] = 0;
	            }
		_delay_us(120);
	    }
	    //_delay_ms(0.044);
	 }

	sei();
    return check_key_id(keyid, keylen);
}

//-------------------------------------------------------------------

//Функция пересылки команд

void send_ds1990_command(unsigned char command)
{
cli();
unsigned char data=command;
   for(i=0;i<8;i++)
   {
   data=data<<7;        //Сдвиг на i разрядов влево
   command=command>>1;  //Сдвиг передаваемого байта
     if(data==0x00)       //Передача лог.0
     {
     DDRB&=~_BV(0);
     PORTB&=~_BV(0);          
     _delay_us(15);  //10 мкс
     DDRB|=_BV(0);
     PORTB&=~_BV(0);
     _delay_us(100);  //60 мкс
     DDRB|=_BV(0);
     PORTB|=_BV(0);     
     _delay_us(2);   //Задержка примерно на 6 мкс
      }
     else                 //Передача лог.1
     {
/*     DDRB|=_BV(0);
     PORTB|=_BV(0);     
     _delay_us(6);   //Задержка примерно на 6 мкс
     DDRB&=~_BV(0);  
     PORTB&=~_BV(0);    
     _delay_us(10);   //Задержка примерно на 64 мкс */
	 
     DDRB&=~_BV(0);
     PORTB&=~_BV(0);          
     _delay_us(15);  //10 мкс
     DDRB|=_BV(0);
     PORTB|=_BV(0);     
     _delay_us(100);   //Задержка примерно на 6 мкс
 	  
     }
   data=command;        //Новое значение для сдвига
   };
sei();
}

//-------------------------------------------------------------------
