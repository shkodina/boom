

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <avr/eeprom.h>

#include "lcd.h"



#define LCDPORT PORTC
#define BUTTONPIN PINA

#define OKBUT 2
#define NOBUT 4

#define KEYIDLEN 8

#define TEXTLEN 16
#define MENUCOUNT 7

#define ESTOP 0
#define ESTART 1
#define EADMIN 2
#define SADMIN 3
#define SSTART 4
#define SSTOP 5
#define STIMER 6

#define EEPROMADR_STARTADDR 		1
#define EEPROMADRORDER_ADMPASS 		0
#define EEPROMADRORDER_STARTCODE	1
#define EEPROMADRORDER_STOPCODE		2
#define EEPROMADRORDER_TIMER		3
#define EEPROMADRORDER_KEY			4

#define WRONGPASS 	"Wrong Password! "
#define CORRECTPASS "Password OK!    "
#define WRONGCODE 	"Wrong Code!     "
#define CODEOK		"Code OK!        "
#define TIMEROK		"Timer set ok!   "
#define GAMEOVER	"   GAME OVER!   "

//------ global ------------------

char menu [MENUCOUNT][TEXTLEN] =  {	"Enter STOP code ",	// 0
									"Enter START code", // 1
									"Enter Admin code", // 2
									"SET Admin code  ", // 3
									"SET START code  ", // 4
									"SET STOP code   ", // 5
									"SET Timer       "};// 6 

char menu_pos = ESTART;

char is_admin = 0;
char is_game = 0;
char is_timer = 0;
char is_key = 0;
char is_reset = 0;

char adminpass   [TEXTLEN] = "333                ";
char startcode   [TEXTLEN] = "353                ";
char stopcode    [TEXTLEN] = "535                ";
char curtext	 [TEXTLEN];
char keyid 		 [KEYIDLEN];
char readedkeyid [KEYIDLEN];


long timer_init_val = 3600;
long timer_cur = 0;

//----------------------------------------------------------------

#define INVBIT(port, bit) port = port ^ (1<<bit);
#define UPBIT(port, bit) port = port | (1<<bit);
#define DOWNBIT(port, bit) port = port & (~(1<<bit));

//----------------------------------------------------------------
//*******************************************************************************
//*******************************************************************************
//*******************************************************************************

unsigned char tcnt, i;
char buffer[8][8];

//Вывод массива
char check_key_id(void)
{
LCDSendCommand(CLR_DISP);

	unsigned char bt = 0;
	for (int n = 0; n < KEYIDLEN; n++)
	    {
			bt = 0;
	        for(int n2 = 0;n2 < 8; n2++)
	        {
	 			if (buffer[n][n2]) {
					bt |= (1 << n2);
				}
	        }
			readedkeyid[n]=bt;
			LCDSendUnsafeCounteredTxt(bt, 1);

	    }    
	if (is_reset){
		// записать считанный код в память
	}else{
		for (char i = 0; i < KEYIDLEN; i++){
			if (keyid[i] != readedkeyid[i])
				return 0;
		}
	}
	return 1;
}

//Функция сброса DS1990
void reset_ds1990(void)
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
   }   
  else
  {
  _delay_us(410); //410 мкс
  sei();
  send_ds1990_command(0x33);
  read_ds1990();
  }
}

//Чтение

void read_ds1990(void)
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
    return check_key_id();
}

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

//*******************************************************************************
//*******************************************************************************
//*******************************************************************************

char StrCmp(char * origin, char * copy, char len)
{

	for (char i = 0; i < len; i++){
		if (origin[i] != copy[i]){
			return 1;
		}
	}
	return 0;
}

//--------------------------------

void StrCp(char * origin, char * copy, char len)
{

	for (char i = 0; i < len; i++){
		copy[i] = origin[i];
	}	
}

//--------------------------------

void SetupTIMER1 (void)
{
     //With 16 MHz clock and 65536 times counting T/C1 overflow interrupt
     // will occur every:
     //   1<<CS10                  4096 mkS  (no prescale Fclk)
     //   1<<CS11                  32.768 mS (Fclk/8)
     //  (1<<CS11)|(1<<CS10)       262.144 mS (Fclk/64)
     //   1<<CS12                  1048.576 mS (Fclk/256)
     TCCR1B = (1<<CS12);
     TCNT1 = 65536-62439;        //примерно 1 секунда
     /* Enable timer 1 overflow interrupt. */
     TIMSK |= (1<<TOIE1);

	
 	 sei();
}

//---------------------------------------------------------------

void SetupTIMER3 (void)
{
     //With 16 MHz clock and 65536 times counting T/C1 overflow interrupt
     // will occur every:
     //   1<<CS10                  4096 mkS  (no prescale Fclk)
     //   1<<CS11                  32.768 mS (Fclk/8)
     //  (1<<CS11)|(1<<CS10)       262.144 mS (Fclk/64)
     //   1<<CS12                  1048.576 mS (Fclk/256)
     TCCR3B = (1<<CS12);
     TCNT3 = 65536-62439;        //примерно 1 секунда
     /* Enable timer 1 overflow interrupt. */
     ETIMSK |= (1<<TOIE3);

 	 sei();
}

//---------------------------------------------------------------

char GetButton()
{
	static char all_released = 0;
/*
	if (!(BUTTONPIN & 0b00000001) && all_released){
		all_released = 0;
		return 1;	
	}
*/	
	if (!(BUTTONPIN & 0b00000010) && all_released){
		all_released = 0;
		return 2;	
	}

	if (!(BUTTONPIN & 0b00000100) && all_released){
		all_released = 0;
		return 3;	
	}

	if (!(BUTTONPIN & 0b00001000) && all_released){
		all_released = 0;
		return 4;	
	}

	if (!(BUTTONPIN & 0b00010000) && all_released){
		all_released = 0;
		return 5;	
	}

	if (    (/*(PINA & 0b00000001)
			|*/(PINA & 0b00000010)
			|(PINA & 0b00000100)
			|(PINA & 0b00001000)
			|(PINA & 0b00010000)) == 30)//31)
	all_released = 1; //all bottons are released

	return 0;
}

//---------------------------------------------------------------

char PrintToSevenSeg(long value)
{
	char stext	 [TEXTLEN];

	sprintf(stext,"%d",value);

	LCDSendCommand(DD_RAM_ADDR);

	LCDSendTxt(stext);

	return 0;
}

//---------------------------------------------------------------

void GameOver()
{
	is_game = 0;
	is_timer = 0;
	LCDSendCommand(CLR_DISP); 
	LCDSendUnsafeCounteredTxt(GAMEOVER, TEXTLEN);

	// reinit timer

	timer_cur = timer_init_val;

	UPBIT(PORTA,6);	
}

//---------------------------------------------------------------

void GamePaused()
{
	is_timer = 0;

	UPBIT(PORTA,6);
}

//---------------------------------------------------------------

void MakeBoom()
{
	is_timer = 0;
	is_game = 0;

	timer_cur = timer_init_val;

	LCDSendCommand(CLR_DISP);
	LCDSendTxt(" BOOM BOOM BOOM "); 
	LCDSendCommand(DD_RAM_ADDR2);
	LCDSendTxt(" BOOM BOOM BOOM ");

	UPBIT(PORTA,6);	
	
}

//---------------------------------------------------------------

char CheckKey ()
{
    reset_ds1990();

	static char was_released = 0;
	if (!(BUTTONPIN & 0b00000001) && was_released){
		was_released = 0;
		return 1;	
	}
	
	if (PINA & 0b00000001)
		was_released = 1;

	return 0;
}

//---------------------------------------------------------------

char CheckState(char is_key_state)
{

	if (is_key_state == 1){ // first key, now just one 
		if (is_game){
			GameOver();
			is_key = 0;
			return 1;
		}else{
			is_key = 0;
			menu_pos = EADMIN;
			LCDSendCommand(CLR_DISP); 
			LCDSendUnsafeCounteredTxt(menu[menu_pos], TEXTLEN);
			return 1;				
		}
	}
	return 0;
}

//---------------------------------------------------------------

char MenuSelect(char key)
{	
	static char pos = 0;
	static char addrr = 0;

	if (key != OKBUT && key != NOBUT){ // some digit

		if (pos == TEXTLEN)
			pos--;

		curtext[pos++] = key + 48; // from digit to asci simbol of digit

		for (char i = pos; i < TEXTLEN; i++){
			curtext[i] = ' ';
		}

		LCDSendCommand(DD_RAM_ADDR2);
		LCDSendUnsafeCounteredTxt(curtext, TEXTLEN);

	}else{ // command buttons
		if (key == NOBUT)
		{
			if (is_timer){ // game already started
				menu_pos = ESTOP;
				LCDSendCommand(CLR_DISP); 
				LCDSendUnsafeCounteredTxt(menu[menu_pos], TEXTLEN);
				for (char i = 0; i < TEXTLEN; i++)
					curtext[i] = ' ';
				pos = 0;
			}else{ // game not started
				if (is_admin){
					if (++menu_pos == MENUCOUNT)
						menu_pos = SADMIN; 
				}else{
					menu_pos = ESTART; 
				}
			
				LCDSendCommand(CLR_DISP); 
				LCDSendUnsafeCounteredTxt(menu[menu_pos], TEXTLEN);
			}
		}else{ // OKBUT try to enter some value
			switch (menu_pos){

				case ESTOP:
					if ( StrCmp(curtext, stopcode, TEXTLEN) ){
						LCDSendCommand(DD_RAM_ADDR2);
						LCDSendTxt(WRONGCODE);
					}else{
						LCDSendCommand(DD_RAM_ADDR2);
						LCDSendTxt(CODEOK);
						GamePaused();
					}
					break;

				case ESTART:
					if ( StrCmp(curtext, startcode, TEXTLEN) ){
						LCDSendCommand(DD_RAM_ADDR2);
						LCDSendTxt(WRONGCODE);
					}else{
						LCDSendCommand(DD_RAM_ADDR2);
						LCDSendTxt(CODEOK);
						is_timer = 1;
						is_game = 1;
					}
					break;

				case EADMIN:
					if ( StrCmp(curtext, adminpass, TEXTLEN) ){
						LCDSendCommand(DD_RAM_ADDR2);
						LCDSendTxt(WRONGPASS);
					}else{
						LCDSendCommand(DD_RAM_ADDR2);
						LCDSendTxt(CORRECTPASS);
						is_admin = 1;
					}
					break;

				case SADMIN:
					StrCp(curtext, adminpass, TEXTLEN);

					addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_ADMPASS;
					eeprom_write_block (adminpass, addrr, TEXTLEN);

					LCDSendCommand(DD_RAM_ADDR2);
					LCDSendTxt(CORRECTPASS);
					break;

				case SSTART:
					StrCp(curtext, startcode, TEXTLEN);

					addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_STARTCODE;
					eeprom_write_block (startcode, addrr, TEXTLEN);

					LCDSendCommand(DD_RAM_ADDR2);
					LCDSendTxt(CODEOK);
					break;

				case SSTOP:
					StrCp(curtext, stopcode, TEXTLEN);

					addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_STOPCODE;
					eeprom_write_block (stopcode, addrr, TEXTLEN);
	
					LCDSendCommand(DD_RAM_ADDR2);
					LCDSendTxt(CODEOK);
					break;

				case STIMER:

					for (char i = 0; i < TEXTLEN; i++)
						if (curtext[i] == ' ')
							curtext[i] = 0;	
					timer_init_val = atoi(curtext);
					timer_cur = timer_init_val;

					addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_TIMER;
					eeprom_write_dword (addrr, timer_init_val);

					LCDSendCommand(DD_RAM_ADDR2);
					LCDSendTxt(TIMEROK);
					is_admin = 0;
					break;

				default:
					break;

			}

		}

		for (char i = 0; i < TEXTLEN; i++)
			curtext[i] = ' ';
		pos = 0;
			
	}

	return 0;
}

//---------------------------------------------------------------

void Port_Init()
{
	PORTA = 0b00000000;		DDRA = 0b11000000;
//	PORTB = 0b00000000;		DDRB = 0b00000000;
	LCDPORT = 0b00000000;	DDRC = 0b11110111;
//	PORTD = 0b11000000;		DDRD = 0b00001000;
//	PORTE = 0b00000000;		DDRE = 0b00110000;
//	PORTF = 0b00000000;		DDRF = 0b00000000;	
//	PORTG = 0b00000000;		DDRG = 0b00000000;
}

//---------------------------------------------------------------

ISR (TIMER1_OVF_vect)
{
	static char key = 0;
	TCNT1 = 65536- 6244; //  31220;
    TCCR1B = (1<<CS12);
    TIMSK |= (1<<TOIE1);

	PrintToSevenSeg(timer_cur);

	if (!is_key)
		is_key = CheckKey();
	else
		CheckState(is_key);

	key = GetButton();
	if (key)
		MenuSelect(key);

}

//---------------------------------------------------------------

ISR (TIMER3_OVF_vect)
{
	TCNT3 = 65536- 62439; 
//    TCCR3B = (1<<CS12);
    ETIMSK |= (1<<TOIE3);


	if (is_timer){
		if (!(--timer_cur))
			MakeBoom();	
		
		INVBIT(PORTA,6);
	}	
}

//---------------------------------------------------------------

char GetSavedData()
{
	char addr = 0;

	//get admin pass
	addr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_ADMPASS;
	eeprom_read_block (adminpass, addr, TEXTLEN);

	//get start code
	addr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_STARTCODE;
	eeprom_read_block (startcode, addr, TEXTLEN);

	//get stop code
	addr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_STOPCODE;
	eeprom_read_block (stopcode, addr, TEXTLEN);
	

	// get timer initial value
	addr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_TIMER;
	timer_init_val = eeprom_read_dword(addr);
	

	//get keyID
	addr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_KEY;
	eeprom_read_block (keyid, addr, KEYIDLEN);

	timer_cur = timer_init_val;
	return 0;
}

//---------------------------------------------------------------

void CheckResset()
{
	if (!(BUTTONPIN & 0b00000001)){
		char addrr;

		addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_ADMPASS;
		eeprom_write_block (adminpass, addrr, TEXTLEN);

		addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_STARTCODE;
		eeprom_write_block (startcode, addrr, TEXTLEN);

		addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_STOPCODE;
		eeprom_write_block (stopcode, addrr, TEXTLEN);

		addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_TIMER;
		eeprom_write_dword (addrr, timer_init_val);
	}
}

//---------------------------------------------------------------


int main()
{
	Port_Init();

	CheckResset();

	GetSavedData();
	LCD_Init();
	//LCDSendCommand(DISP_OFF);
	LCDSendCommand(DISP_ON);

	LCDSendCommand(CLR_DISP);

	while (1)
    {
    reset_ds1990();
    }



	LCDSendUnsafeCounteredTxt(menu[menu_pos], TEXTLEN);

	SetupTIMER1();
	SetupTIMER3();

	while (1) 
	{
		
	}
	return 0;
}
