/******************************************************************************
 *
 * Module: HMI_ECU
 *
 * File Name: hmi_ecu.h
 *
 * Description: header file for the Human Machine Interface ECU
 *
 * Author: Esmael Ehab
 *
 *******************************************************************************/

#ifndef HMI_ECU_H_
#define HMI_ECU_H_

/*******************************************************************************
 *                             Definitions                                     *
 *******************************************************************************/
/*   definition for hmi_ecu   */
#define PASSWORD_SIZE           5
#define SEND_RECEIVE_TIME      10
#define DISPLAY_TIME         1500
#define KEYPAD_TIME           500
#define MAX_NUM_OF_MISTAKES     3
#define OPENING_DOOR_TIME      15
#define HOLDING_DOOR_TIME       3
#define CLOSING_DOOR_TIME      15
#define WARNING_TIME           60

/*   definitions for uart commands with control_ecu   */
#define MISMATCHED              0
#define MATCHED                 1
#define OPENING_DOOR          ')'
#define WRONG_PASSWORD        '_'
#define CHANGING_PASSWORD     '('
#define OPEN_DOOR             '+'
#define CHANGE_PASSWORD       '-'
#define READY_TO_SEND         '$'
#define READY_TO_RECEIVE      '&'
#define RECEIVE_DONE          '@'
#define SEND_FIRST_PASSWORD   '^'
#define SEND_CONFIRM_PASSWORD '?'
#define SEND_CHECK_PASSWORD   ','

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
/* Description :
 * function that initiate the timer with the required configuration
 * to calculate the seconds
 */
void APP_startTimer(void);

/* Description :
 * function that set the call back with timer
 */
void TimerCallBackProcessing(void);

/* Description :
 * function send command to the UART from hmi_ecu
 */
void APP_sendToCONTROL_ECU(char UART_command);

/* Description :
 * function receive commands by the UART from control_ecu
 * and return that commands
 */
char APP_receiveFromCONTROL_ECU(void);

/* Description :
 * function that set a new password
 * if : 1- application start for the first time
 *      2- user choose to change the password
 */
void APP_newPassword(void);

/* Description :
 * function takes the password from a_inputPassword
 * and send it to control_ecu by uart
 */
void APP_sendPassword(char a_inputPassword[], char SIZE);

/* Description :
 * function that gets the password from keypad
 * and display '*' on the screen
 */
void APP_getPassword(char a_inputPassword[], char SIZE);

/* Description :
 * function show the main option
 */
void APP_mainOptions(void);

void APP_askForPassword(void);

/* Description :
 * function that start opening door task
 */
void APP_openingDoor(void);

/* Description :
 * function that start the wrong password task
 * shows warin alert if the password mistakes was 3
 */
void APP_wrongPassword(void);

#endif /* HMI_ECU_H_ */
