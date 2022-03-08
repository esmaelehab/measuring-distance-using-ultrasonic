/******************************************************************************
 *
 * Module: main
 *
 * File Name: Mini_Project_4.c
 *
 * Description: source file for the main function
 *
 * Author: Esmael Ehab
 *
 *******************************************************************************/


#include "lcd.h"
#include "ultrasonic_sensor.h"
#include <avr/io.h>              /* to enable global interrupt */
#include <util/delay.h>          /* For the delay functions */

int main(void)
{
	uint16 distance;

	SREG = (1<<7);         /* enable global interrupt */
	LCD_init();            /* initialize the lcd */

	/* Display this string "Distance=    cm" only once on LCD at the first row */
	LCD_displayString("Distance=     cm");

	while(1)
	{
		/*get the distance from the ultrasonic*/
		distance = ULTRASONIC_readDistance();

		/*move the cursor and display the distance*/
		LCD_moveCursor(0,10);
		if (distance >= 100)
		{
		LCD_intgerToString(distance);
		}
		else
		{
			LCD_intgerToString(distance);
			LCD_displayCharacter(' ');
		}
		_delay_ms(750);
	}
}


