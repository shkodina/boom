

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lcd.h"

#define LCDPORT PORTC
#define BUTTONPIN PINA

#define OKBUT 2
#define NOBUT 4


#define TEXTLEN 16
#define MENUCOUNT 7

#define ESTOP 0
#define ESTART 1
#define EADMIN 2
#define SADMIN 3
#define SSTART 4
#define SSTOP 5
#define STIMER 6

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

char adminpass   [TEXTLEN] = "333                ";
char startcode   [TEXTLEN] = "353                ";
char stopcode    [TEXTLEN] = "535                ";
char curtext	 [TEXTLEN];

int timer_init_val = 60;
int timer_cur = 0;

//--------------------------------

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
     TIMSK = (1<<TOIE1);

	
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

char PrintToSevenSeg(char value)
{


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
	// TODO

}

//---------------------------------------------------------------

void GamePaused()
{


}

//---------------------------------------------------------------

char CheckKey ()
{
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
						is_timer = 0;
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
					LCDSendCommand(DD_RAM_ADDR2);
					LCDSendTxt(CORRECTPASS);
					break;

				case SSTART:
					StrCp(curtext, startcode, TEXTLEN);
					LCDSendCommand(DD_RAM_ADDR2);
					LCDSendTxt(CODEOK);
					break;

				case SSTOP:
					StrCp(curtext, stopcode, TEXTLEN);
					LCDSendCommand(DD_RAM_ADDR2);
					LCDSendTxt(CODEOK);
					break;

				case STIMER:
					for (char i = 0; i < TEXTLEN; i++)
						if (curtext[i] == ' ')
							curtext[i] = 0;	
					timer_init_val = atoi(curtext);
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
	PORTA = 0b00000000;		DDRA = 0b01000000;
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
    TIMSK = (1<<TOIE1);

	if (!is_key)
		is_key = CheckKey();
	else
		CheckState(is_key);

	key = GetButton();
	if (key)
		MenuSelect(key);

}

//---------------------------------------------------------------

char GetSavedData()
{


	
	return 0;
}

//---------------------------------------------------------------


int main()
{

	Port_Init();
	LCD_Init();
	//LCDSendCommand(DISP_OFF);
	LCDSendCommand(DISP_ON);

	LCDSendCommand(CLR_DISP);

	LCDSendUnsafeCounteredTxt(menu[menu_pos], TEXTLEN);

	SetupTIMER1();

	while (1) 
	{
		
	}
	return 0;
}
