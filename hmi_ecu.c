/******************************************************************************
 *
 * Module: HMI_ECU
 *
 * File Name: hmi_ecu.c
 *
 * Description: source file for the Human Machine Interface ECU
 *
 * Author: Esmael Ehab
 *
 *******************************************************************************/

#include "micro_config.h"
#include "hmi_ecu.h"
#include <avr/io.h>
#include <avr/delay.h>
#include "uart.h"
#include "lcd.h"
#include "keypad.h"
#include "timer.h"

char g_inputPassword[PASSWORD_SIZE];  /*global variable that takes the password from the human*/

uint8 g_matchStatus = MISMATCHED;     /*global variable that hold the matching status*/

uint8 g_tick=0;                       /*global variable that count the seconds by timer*/

uint8 g_passwordMistake=0;            /*global variable that hold the number of input password mistakes*/

char UART_command;                    /*global variable that hold the command from control_ecu*/

int main(void)
{
	/* Enable global interrupts by setting I-bit */
	SREG  |= (1<<7);

    /* initialize the UART with configuration :
     * (8 data bits, one stop bit, no parity bit, 9600 baud rate)
     */
	UART_ConfigTypes UART_Config = {DATA_BITS_8, ONE_STOP_BIT, NO_PARITY, 9600};
    UART_init(&UART_Config);

    /* initialize the lcd */
	LCD_init();

	/* initialize the password for the first time */
	APP_newPassword();

	uint8 mainOptions_key;

	while(1)
	{
		APP_mainOptions(); /*display main options*/

		mainOptions_key = KEYPAD_getPressedKey();  /*get the pressed option*/

		switch (mainOptions_key)
		{
		/*in case user chosen open door*/
		case OPEN_DOOR :

			/* 1-Ask the user for the password
			 * 2-Send command to till control_ecu to check the password
			 * 3-Send the input password
			 * 4-send command to till control_ecu what user chosen
			 */
			APP_askForPassword();
			APP_sendToCONTROL_ECU(SEND_CHECK_PASSWORD);
			APP_sendPassword(g_inputPassword, PASSWORD_SIZE);
			APP_sendToCONTROL_ECU(OPEN_DOOR);

			/*receive the command order from control_ecu*/
			g_matchStatus = APP_receiveFromCONTROL_ECU();

			/*in case the order was opening door*/
			if(g_matchStatus ==  OPENING_DOOR)
			{
				/*begin opening/closing door task*/
				APP_openingDoor();
			}
			/*in case wrong password task*/
			else if(g_matchStatus == WRONG_PASSWORD)
			{
				/*begin wrong password task*/
				APP_wrongPassword();
			}
			break;
		/*in case user chosen change password*/
		case CHANGE_PASSWORD :

			/* 1-Ask the user for the password
			 * 2-Send command to till control_ecu to check the password
			 * 3-Send the input password
			 * 4-send command to till control_ecu what user chosen
			 */
			APP_askForPassword();
			APP_sendToCONTROL_ECU(SEND_CHECK_PASSWORD);
			APP_sendPassword(g_inputPassword, PASSWORD_SIZE);
			APP_sendToCONTROL_ECU(CHANGE_PASSWORD);

			/*receive the command order from control_ecu*/
			g_matchStatus = APP_receiveFromCONTROL_ECU();

			/*in case the order was changing password*/
			if(g_matchStatus == CHANGING_PASSWORD)
			{
				/*begin changing password task*/
				APP_newPassword();
			}
			/*in case wrong password task*/
			else if(g_matchStatus == WRONG_PASSWORD)
			{
				/*begin wrong password task*/
				APP_wrongPassword();
			}
			break;
		}
	}

}

void APP_newPassword(void)
{
	g_matchStatus = MISMATCHED; /* set it to mismatched for new passwords check */

	/* doesn't terminate until the the hmi_ecu gets the same two password*/
	while(g_matchStatus == MISMATCHED)
	{
		/* 1- Clear screen and display "New Password"
		 * 2- Clear screen and Display "Enter A Password"
		 * 3- Move the cursor and get the password from keypad
		 * 4- Send a command to till control_ecu that you got the first password
		 * 5- Then send it to control_ecu
		 */
		LCD_clearScreen();
		LCD_displayString("  New Password");
		_delay_ms(DISPLAY_TIME);              /*delay time to show the message*/
		LCD_clearScreen();
		LCD_displayString("Enter A Password:");
		LCD_moveCursor(1,0);
		APP_getPassword(g_inputPassword,PASSWORD_SIZE);

	    APP_sendToCONTROL_ECU(SEND_FIRST_PASSWORD);
		APP_sendPassword(g_inputPassword,PASSWORD_SIZE);

		/* 1- Clear screen and Display "ReEnter Password"
		 * 2- Move the cursor and get the password from keypad
		 * 3- Send a command to till control_ecu that you got the confirm password
		 * 4- Then send it to control_ecu
		 */
		LCD_clearScreen();
		LCD_displayString("ReEnter Password:");
		LCD_moveCursor(1,0);
		APP_getPassword(g_inputPassword,PASSWORD_SIZE);

		APP_sendToCONTROL_ECU(SEND_CONFIRM_PASSWORD);
		APP_sendPassword(g_inputPassword,PASSWORD_SIZE);

		/* wait until the control_ecu is ready to send the password confirm status */
		g_matchStatus = APP_receiveFromCONTROL_ECU();
		/*in case the two passwords are not the same*/
		if (g_matchStatus == MISMATCHED)
		{
			LCD_clearScreen();
			LCD_displayString("Mismatched Pass");
			_delay_ms(DISPLAY_TIME);
		}
	}
}

void APP_getPassword(char a_inputPassword[], char SIZE)
{
	LCD_moveCursor(1,0);
	uint8 i = 0, password_key=0;
	/* doesn't terminate until a 5 numbers password is stored */
	while ( i != PASSWORD_SIZE )
	{
		/* 1- get the pressed key
		 * 2- Store the key value i=only if it's a number
		 * 3- don't show the password by displaying '*'
		 * 4- store the number in the array
		 */
		password_key = KEYPAD_getPressedKey();
		if ( (password_key >= 0) && (password_key <= 9) )
		{
			LCD_displayCharacter('*');
			a_inputPassword[i] = password_key;
			i++;
		}
		_delay_ms(KEYPAD_TIME);    /*delay time for the keypad buttons*/
	}
	while (KEYPAD_getPressedKey() != 13){} /*stay until user press enter*/

}

void APP_sendPassword(char a_inputPassword[], char SIZE)
{
	uint8 i;
	/*send each number of the password the the control_ecu by UART*/
	for (i=0; i<SIZE; i++)
	{
		UART_sendByte(a_inputPassword[i]);
		_delay_ms(SEND_RECEIVE_TIME);      /*delay for the sending receiving time between the ECUS*/
	}
}

void APP_mainOptions(void)
{
	/* display the main options */
	LCD_clearScreen();
	LCD_displayString("+ : Open Door");
	LCD_moveCursor(1,0);
	LCD_displayString("- : Change Pass");
}

void APP_askForPassword(void)
{
	/*display "enter password" and get the input password from the user*/
	LCD_clearScreen();
	LCD_displayString("Enter Password:");
	APP_getPassword(g_inputPassword, PASSWORD_SIZE);
}

void APP_openingDoor(void)
{
	/*start the timer to count the seconds*/
	APP_startTimer();

	/*display "door is opening" for 15 seconds*/
	LCD_clearScreen();
	LCD_displayString("Door Is Opening");
	while(g_tick < OPENING_DOOR_TIME);
    g_tick = 0;

	/*display "door is holding" for 3 seconds*/
	LCD_clearScreen();
	LCD_displayString("Door Is Holding");
	while(g_tick < HOLDING_DOOR_TIME);
	g_tick = 0;

	/*display "door is closing for 15 seconds*/
	LCD_clearScreen();
	LCD_displayString("Door Is Closing");
	while(g_tick < CLOSING_DOOR_TIME);
	g_tick = 0;

	TIMER_DeInit(); /*stop the timer*/
	LCD_clearScreen();
}

void APP_wrongPassword(void)
{
	g_passwordMistake++;
	LCD_clearScreen();
	LCD_displayString("Wrong Password");
	_delay_ms(DISPLAY_TIME);
	/*in case input password was wrong 3 times start warning for 1 minute*/
	if (g_passwordMistake == MAX_NUM_OF_MISTAKES)
	{
		/*start the timer to count the seconds*/
		APP_startTimer();

		LCD_clearScreen();
		LCD_displayString("Warning Alert!");
		while(g_tick != WARNING_TIME);
		g_passwordMistake = 0;
		g_tick = 0;
		/*stop the timer*/
		TIMER_DeInit();
	}
	LCD_clearScreen();
}

void TimerCallBackProcessing(void)
{
	g_tick++;
}

void APP_startTimer(void)
{
	/* initialize the TIMER with configuration :
	 * (compare mode (CTC), initial value=0, prescaler 1024)
	 * F(timer)=8Mhz/1024=7812.5hz so the cycle = 128us so to make an interrupt
	 * every 1 second we set the compare value to 1s/128us = 7813
	 */
	TIMER_ConfigTypes TIMER_Config = {TIMER_1, CHANNEL_A, COMPARE, F_CPU_1024, 0, 7813};
	/*start the timer*/
	TIMER_init(&TIMER_Config);
	/* set the call back function called by the timer */
	TIMER1_setCallBack(TimerCallBackProcessing);
}

void APP_sendToCONTROL_ECU(char UART_command)
{
	/* 1-inform control_ecu that you are ready to send
	 * 2-wait until control_ecu ready to receive
	 * 3-send the required command to control_ecu
	 * 4-wait until control_ecu receive it
	 */
	UART_sendByte(READY_TO_SEND);
	while(UART_recieveByte() != READY_TO_RECEIVE);
	UART_sendByte(UART_command);
	while(UART_recieveByte() != RECEIVE_DONE){};
}

char APP_receiveFromCONTROL_ECU(void)
{
	/* 1-wait until control_ecu ready to send
	 * 2-inform control_ecu that you are ready to receive
	 * 3-receive the command from control_ecu
	 * 4-inform control_ecu that you received the command
	 */
	while(UART_recieveByte() != READY_TO_SEND);
	UART_sendByte(READY_TO_RECEIVE);
	UART_command = UART_recieveByte();
	UART_sendByte(RECEIVE_DONE);
	return UART_command;
}








