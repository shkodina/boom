

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <avr/eeprom.h>

#include "onewire.h"
#include "lcd.h"
#include "keyboard.h"


#define F_CPU 1000000UL
#include <util/delay.h>





#define LCDPORT PORTC
#define BUTTONPIN PINA

#define OKBUT 88	// *
#define NOBUT 89	// #
#define RESETBUT 9 // 9

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
char keyid 		 [KEYIDLEN] = "++++++++";
char readedkeyid [KEYIDLEN] = "--------";


long timer_init_val = 60*60*1;
long timer_cur = 60*60*1;
char time[6];


//----------------------------------------------------------------

#define INVBIT(port, bit) port = port ^ (1<<bit);
#define UPBIT(port, bit) port = port | (1<<bit);
#define DOWNBIT(port, bit) port = port & (~(1<<bit));

//----------------------------------------------------------------

char StrCmp(char * origin, char * copy, char len)
{

	for (char i = 0; i < len; i++){
		if (origin[i] != copy[i]){
			return 1;
		}
	}
	return 0;
}

//-------------------------------------------------------------------

void StrCp(char * origin, char * copy, char len)
{

	for (char i = 0; i < len; i++){
		copy[i] = origin[i];
	}	
}

//-------------------------------------------------------------------

void clear_key()
{
	for (char i = 0; i < KEYIDLEN; i++){
		readedkeyid[i] = 255;
	}
}

//---------------------------------------------------------------

char GetButton()
{
	static char was_released = 1;

	char key = ReadFromKeyboard();

	if (key){
		if (was_released){
			was_released = 0;
			return key;
		}
	}else{
		was_released = 1;
	}

	return 0;
}

//---------------------------------------------------------------

char GetSevenCode (char val, char need_point)
{
	char res = 0;
	switch (val){
		case 1:
			res = 0b00000110;
			break;
		case 2:
			res = 0b01011011;
			break;
		case 3:
			res = 0b01001111;
			break;
		case 4:
			res = 0b01100110;
			break;
		case 5:
			res = 0b01101101;
			break;
		case 6:
			res = 0b01111101;
			break;
		case 7:
			res = 0b00000111;
			break;
		case 8:
			res = 0b01111111;
			break;
		case 9:
			res = 0b01101111;
			break;
		case 0:
			res = 0b00111111;
			break;

		default:
			res = 0b00000000;
			break;
	}

	if (need_point)
		res |= 0b10000000;

	return res;
	
}

//--------------------------------------------------------------------

char PrintToSevenSeg(long value)
{
	for (char i = 0; i < 7; i++){
		PORTA = (1 << i);
		PORTE = GetSevenCode(time[i], i%2);
		_delay_us(200);	
		PORTA = 0x00;
		PORTE = 0x00;
		_delay_us(20);
	}
	return 0;
}

//---------------------------------------------------------------

void GameOver()
{
	is_game = 0;
	is_timer = 0;

	LCDSendCommand(CLR_DISP); 
	//LCDSendUnsafeCounteredTxt(GAMEOVER, TEXTLEN);
	LCDSendTxt("   GAME OVER!");

	// reinit timer

	timer_cur = timer_init_val;
	DOWNBIT(PORTB,6);
	_delay_ms(3000);
}

//---------------------------------------------------------------

void GamePaused()
{
	is_timer = 0;

	UPBIT(PORTB,6);
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

	DOWNBIT(PORTB,6);

	// подорвём заряд
	DDRD |= 0b01000000;
	PORTD |= 0b01000000;

	_delay_ms(5000);
	
}

//---------------------------------------------------------------

char CheckKey ()
{
	if (!reset_ds1990(readedkeyid, KEYIDLEN))
		return 0;
//	return !StrCmp(readedkeyid, keyid, KEYIDLEN);

	if ( StrCmp(readedkeyid, keyid, KEYIDLEN) ){ // считываются разные ключи
		return 0;
	}else{
		clear_key();
		return 1;
	}
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
#define MAX_OK_COUNTER 3
char MenuSelect(char key)
{	
	static char pos = 0;
	static char addrr = 0;
	static char OK_COUNTER = 0;

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
			OK_COUNTER = 0;
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
					if (++OK_COUNTER != MAX_OK_COUNTER)
						break;
					OK_COUNTER = 0;

					if ( StrCmp(curtext, stopcode, TEXTLEN) ){
						LCDSendCommand(DD_RAM_ADDR2);
						LCDSendTxt(WRONGCODE);
					}else{
						LCDSendCommand(DD_RAM_ADDR2);
						LCDSendTxt(CODEOK);
						GamePaused();
						MenuSelect(NOBUT);
					}
					break;

				case ESTART:
					if (++OK_COUNTER != MAX_OK_COUNTER)
						break;
					OK_COUNTER = 0;

					if ( StrCmp(curtext, startcode, TEXTLEN) ){
						LCDSendCommand(DD_RAM_ADDR2);
						LCDSendTxt(WRONGCODE);
					}else{
						LCDSendCommand(DD_RAM_ADDR2);
						LCDSendTxt(CODEOK);
						is_timer = 1;
						is_game = 1;
						MenuSelect(NOBUT);
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

					char timer_val[3];
					char timer_d_val[3];
					for (char i = 0; i < 3; i++){
						timer_val[0] = curtext [i*2]; 	
						timer_val[1] = curtext [i*2 + 1]; 	
						timer_val[2] = 0;
						timer_d_val[i] = atoi(timer_val);

					}

					timer_init_val = 	(long)(timer_d_val[0]) * 60 * 60 + 
										(long)(timer_d_val[1]) * 60 +
										(long)(timer_d_val[2])	;

					timer_cur = timer_init_val;

					addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_TIMER;
					eeprom_write_dword (addrr, timer_init_val);

					LCDSendCommand(DD_RAM_ADDR2);
					LCDSendTxt(TIMEROK);
					is_admin = 0;
					MenuSelect(NOBUT);
					break;

				default:
					break;

			}

		}

		if (!OK_COUNTER){
			for (char i = 0; i < TEXTLEN; i++)
				curtext[i] = ' ';
			pos = 0;
		}	
	}

	return 0;
}

//---------------------------------------------------------------

void Port_Init()
{
	PORTA = 0b00000000;		DDRA = 0b00111111;
	PORTB = 0b00000000;		DDRB = 0b11000000;
	LCDPORT = 0b00000000;	DDRC = 0b11110111;
	PORTD = 0b11000000;		DDRD = 0b01000000;
	PORTE = 0b00000000;		DDRE = 0b11111111;
	PORTF = 0b00000000;		DDRF = 0b00001111;	
//	PORTG = 0b00000000;		DDRG = 0b00000000;
}

//-------------------------------------------------------------------

void SetupTIMER1 (void)
{
     TCCR1B = (1<<CS12);
     TCNT1 = 65536-50;        
     TIMSK |= (1<<TOIE1); // разрешим прерывание по таймеру
 	 sei();
}

//---------------------------------------------------------------

void SetupTIMER0 (void)
{
	TIMSK &=~(1<<OCIE0 | 1<< TOIE0);	// Запрещаем прерывания таймера 2
	ASSR  = 1<<AS0;				// Включаем асинхронный режим
	TCNT0 = 0;
	TCCR0 = 5<<CS20; 
	TIMSK |= 1<< TOIE0;
}
//---------------------------------------------------------------

ISR (TIMER1_OVF_vect)
{
	static char key = 0;


	PrintToSevenSeg(timer_cur);

	if (!is_key)
		is_key = CheckKey();
	else
		CheckState(is_key);

	key = GetButton();
	if (key)
		MenuSelect(key-1);

	// run timer
	TCNT1 = 65536 - 100; //  31220;
    TCCR1B = (1<<CS12);
    TIMSK |= (1<<TOIE1);


}

//---------------------------------------------------------------

ISR (TIMER0_OVF_vect)
{
cli();

	if (is_timer){
		INVBIT(PORTB,6);
		if (!(--timer_cur))
			MakeBoom();	
	}	
		
	time[0] = ((timer_cur / 3600) / 10); // hours
	time[1] = ((timer_cur / 3600) % 10); // hours
	time[2] = ((timer_cur % 3600) / 60) / 10; // minutes
	time[3] = ((timer_cur % 3600) / 60) % 10; // minutes
	time[4] = (timer_cur % 60) / 10; // seconds
	time[5] = (timer_cur % 60) % 10; // seconds

 sei();
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
	char kkey = ReadFromKeyboard() - 1;

	for (char i = 0; i < 10; i++)
		kkey = ReadFromKeyboard() - 1;

	if ( kkey == RESETBUT ){
		char addrr;

		addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_ADMPASS;
		eeprom_write_block (adminpass, addrr, TEXTLEN);

		addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_STARTCODE;
		eeprom_write_block (startcode, addrr, TEXTLEN);

		addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_STOPCODE;
		eeprom_write_block (stopcode, addrr, TEXTLEN);

		addrr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_TIMER;
		eeprom_write_dword (addrr, timer_init_val);

		LCDSendCommand(CLR_DISP);
		LCDSendTxt("PUT THE KEY!");

		char is_exit = 0;
		while (!is_exit) {
			if (!reset_ds1990(readedkeyid, KEYIDLEN))
				continue;

			if ( StrCmp(readedkeyid, keyid, KEYIDLEN) ){ // считываются разные ключи
				StrCp(readedkeyid, keyid, KEYIDLEN);
				LCDSendCommand(CLR_DISP);
				LCDSendTxt("WRONG KEY READ!");
			}else{										// считанные ключи совпали
				char addr = EEPROMADR_STARTADDR + TEXTLEN * EEPROMADRORDER_KEY;
				eeprom_write_block (keyid, addr, KEYIDLEN);
				is_exit = 1;

				LCDSendCommand(CLR_DISP);
				LCDSendTxt("KEY READED!");

				_delay_ms(5000);

				LCDSendCommand(CLR_DISP);

				clear_key();
			}
			_delay_ms(10); // может кнопка недоприжалась
		}

	}
}

//---------------------------------------------------------------


int main()
{

	Port_Init();
	InitializeKeyboard();

	LCD_Init();
	LCDSendCommand(DISP_ON);
	LCDSendCommand(CLR_DISP);



	CheckResset();

	GetSavedData();


	LCDSendUnsafeCounteredTxt(menu[menu_pos], TEXTLEN);

	SetupTIMER1();
	SetupTIMER0();


	//set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	while (1) 
	{
/*
		PORTB |= _BV(6);
		_delay_ms(500);
		PORTB &= ~_BV(6); 
		_delay_ms(500);
*/
		sleep_cpu();	
	}
	return 0;
}
