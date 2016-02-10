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

//������� ������ DS1990

char reset_ds1990(char* keyid, char keylen)
{
	DDRB|=_BV(0);        // ����� ����� ����������� ��� �����
	PORTB&=~_BV(0);

	cli();
	_delay_us(580); //������������� 0 � ���. ����� 480 ���

	DDRB&=~_BV(0);       //����� ����� ����������� ��� ����
	_delay_us(70);  //�������� ������� ����������� 70 ���
   	if(PINB & (1<<PINB0))  //���� 1 - ��� ������� �����������
   	{
   		_delay_us(410); //410 ���
   		sei();
		return 0;
   	}   
  	else
  	{
  		_delay_us(410); //410 ���
  		sei();
  		send_ds1990_command(0x33);
  		read_ds1990(keyid, keylen);
		return 1;
  	}
}

//-------------------------------------------------------------------

//������

void read_ds1990(char* keyid, char keylen)
{ 
	cli(); 
	  for(i = 0; i < 8; i++) // �������� ����
	  {

	    for(tcnt = 0; tcnt < 8; tcnt++)          // ������� ����
	    {

	    DDRB|=_BV(0);
	    PORTB&=~_BV(0);
	    _delay_us(3);// ���� 6 ���
	    PORTB&=~_BV(0);
	    DDRB&=~_BV(0);
	    _delay_us(6); // ���� 9 ���

	 
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

//������� ��������� ������

void send_ds1990_command(unsigned char command)
{
cli();
unsigned char data=command;
   for(i=0;i<8;i++)
   {
   data=data<<7;        //����� �� i �������� �����
   command=command>>1;  //����� ������������� �����
     if(data==0x00)       //�������� ���.0
     {
     DDRB&=~_BV(0);
     PORTB&=~_BV(0);          
     _delay_us(15);  //10 ���
     DDRB|=_BV(0);
     PORTB&=~_BV(0);
     _delay_us(100);  //60 ���
     DDRB|=_BV(0);
     PORTB|=_BV(0);     
     _delay_us(2);   //�������� �������� �� 6 ���
      }
     else                 //�������� ���.1
     {
/*     DDRB|=_BV(0);
     PORTB|=_BV(0);     
     _delay_us(6);   //�������� �������� �� 6 ���
     DDRB&=~_BV(0);  
     PORTB&=~_BV(0);    
     _delay_us(10);   //�������� �������� �� 64 ��� */
	 
     DDRB&=~_BV(0);
     PORTB&=~_BV(0);          
     _delay_us(15);  //10 ���
     DDRB|=_BV(0);
     PORTB|=_BV(0);     
     _delay_us(100);   //�������� �������� �� 6 ���
 	  
     }
   data=command;        //����� �������� ��� ������
   };
sei();
}

//-------------------------------------------------------------------
