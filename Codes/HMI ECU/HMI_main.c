/* -----------------------------------------------------------------------------
[FILE NAME]    :	HMI ECU

[AUTHOR]       :	MOHANAD K. SAEED

[DATA CREATED] :	11/03/2021

[DESCRIPTION]  :	Door Locker Security System, HMI responsible for
					interfacing with the user
------------------------------------------------------------------------------*/
#define F_CPU 8000000UL
#include "LCD Driver/lcd.h"
#include "Keypad Driver/keypad.h"
#include "UART/uart.h"

/* Random number to check if the control ECU is ready to receive
 * password or not*/
#define CONTROL_ECU_READY 0xF0
/* Global Variable to store the received state of 2 password; matched or not*/
volatile uint8 g_matchingCheck;
/*2 Passwords States*/
enum{UNMATCHED=1,MATCHED=2};
/*Door States*/
enum{OPEN,CHANGE,RESET,OPENED,CLOSED,CLOSING,DONE};
/*Function used to get the password which consists of 6 digits from user*/
void getPassword(uint8 * password);
/*Function used to send the password to Control ECU using UART protocol*/
void sendPassword(uint8 *password);
/*Function used to take the password from the user and send it to Control ECU
 *to set it as password for the door used later to open the door*/
void setPassword(uint8 *password);
/*Function to communicate with Control ECU during opening the door*/
void openDoor(uint8 *password);
/*Function to communicate with Control ECU during changing the password*/
void changePassword(uint8 *password);

int main(void){
	volatile uint8 password[15];
	uint8 key;
	Uart_ConfigType uart;
	/*Setting the UART Configuration*/
	uart.baudRate=9600;
	uart.dataBits=UART_8_BIT;
	uart.stopBits=UART_1_BIT;
	uart.parityType=UART_DISABLE_PARITY;
	/*Initializing UART*/
	UART_init(&uart);
	/*Wait until Control ECU is ready to receive the data from HMI ECU*/
	while(UART_receiveByte() != CONTROL_ECU_READY){}
	/*Initializing LCD*/
	LCD_init();
	setPassword(password);
	while(1){
		LCD_sendCommand(CLEAR_COMMAND);
		/*Display the default message on the LCD*/
		LCD_displayString("- : Open Door");
		LCD_goToRowColumn(1,0);
		LCD_displayString("+ : Change Pass");
		/*Wait the user to choose if he want to open the door or change the password*/
		key=KEYPAD_getPressedKey();
		switch(key){
		case '+':
			UART_sendByte(CHANGE);
			changePassword(password);
			break;
		case '-':
			UART_sendByte(OPEN);
			openDoor(password);
			break;
		}
	}
	return 0;
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : changePassword
[DESCRIPTION]   : Function to communicate with Control ECU during changing
				  the password. This function is responsible for changing
				  the password take the old password from the user then send
				  it to Control ECU to check if it is true or not if the user
				  entered wrong password 3 times. The HMI ECU displays thief !
				  message on LCD for 1 minute the reset to default screen.
				  If the user enter the old password correct the HMI ECU will
				  receive that the password is correct from Control ECU then
				  HMI ECU will display "Successful !" message on LCD.
[Args]		    :
				in  -> point to array:
						This argument is empty array to store the password.
[Return]	   :
				void
------------------------------------------------------------------------------*/
void changePassword(uint8 *password){
	uint8 n=0;
	LCD_sendCommand(CLEAR_COMMAND);
	LCD_displayString("Enter Old Pass:");
	LCD_goToRowColumn(1,0);
	getPassword(password);
	sendPassword(password);
	g_matchingCheck=UART_receiveByte();
	while(((g_matchingCheck==UNMATCHED) & (n<2))){
		n++;
		LCD_sendCommand(CLEAR_COMMAND);
		LCD_displayString("Wrong Password");
		_delay_ms(200);
		LCD_sendCommand(CLEAR_COMMAND);
		LCD_displayString("Enter Old Pass:");
		LCD_goToRowColumn(1,0);
		getPassword(password);
		sendPassword(password);
		g_matchingCheck=UART_receiveByte();
	}
	if(n==2){
		LCD_sendCommand(CLEAR_COMMAND);
		LCD_displayString("Error !!!");
		while(UART_receiveByte()!=RESET){};
	}
	else if (g_matchingCheck==MATCHED){
		setPassword(password);
		while(UART_receiveByte()!=DONE){};
	}
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : openDoor
[DESCRIPTION]   : Function to communicate with Control ECU during opening
				  the door. This function is responsible for taking the password
				  from the user then send it to Control ECU to check if it's
				  correct or not. If the user entered the password wrong 3 times
				  the HMI ECU will display "Thief !" message on LCD. If the user
				  entered the correct password The Control ECU will check and
				  tell the HMI ECU that the password is correct then HMI ECU
				  will display the state of the door sequentially (Opening,
				  Opened, Closing)

[Args]		    :
				in  -> point to array:
						This argument is empty array to store the password.
[Return]	   :
				void
------------------------------------------------------------------------------*/
void openDoor(uint8 *password){
	uint8 n=0;
	LCD_sendCommand(CLEAR_COMMAND);
	LCD_displayString("Enter Pass:");
	LCD_goToRowColumn(1,0);
	getPassword(password);
	sendPassword(password);
	g_matchingCheck=UART_receiveByte();
	while(((g_matchingCheck==UNMATCHED) & (n<2))){
		n++;
		LCD_sendCommand(CLEAR_COMMAND);
		LCD_displayString("Wrong Password");
		_delay_ms(200);
		LCD_sendCommand(CLEAR_COMMAND);
		LCD_displayString("Enter Pass:");
		LCD_goToRowColumn(1,0);
		getPassword(password);
		sendPassword(password);
		g_matchingCheck=UART_receiveByte();
	}
	if(n==2){
		LCD_sendCommand(CLEAR_COMMAND);
		LCD_displayString("Thief !!!");
		while(UART_receiveByte()!=RESET){};
	}
	else if (g_matchingCheck==MATCHED){
		LCD_sendCommand(CLEAR_COMMAND);
		LCD_displayString("Door is opening");
		while(UART_receiveByte()!=OPENED){};
		LCD_sendCommand(CLEAR_COMMAND);
		LCD_displayString("Door is opened");
		while(UART_receiveByte()!=CLOSING){};
		LCD_sendCommand(CLEAR_COMMAND);
		LCD_displayString("Door is closing");
		while(UART_receiveByte()!=CLOSED){};
	}
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : getPassword
[DESCRIPTION]   : This function is responsible for taking the password from
				  the user digit by digit and store them in array and display
				  '*' on LCD instead of each entered digit.

[Args]		    :
				in  -> point to array:
						This argument is empty array to store the password.
[Return]	   :
				void
------------------------------------------------------------------------------*/
void getPassword(uint8 * password){
	uint8 i=0,key;
	_delay_ms(100);
	key=KEYPAD_getPressedKey();
	while(key!=13){
		password[i]=key;
		i++;
		LCD_displayString("*");
		_delay_ms(100);
		key=KEYPAD_getPressedKey();
	}
	password[i]=key;
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : sendPassword
[DESCRIPTION]   : Function is responsible for sending the password (which is
				  entered from the user) to Control ECU

[Args]		    :
				in  -> point to array:
						This argument is array includes the password.
[Return]	   :
				void
------------------------------------------------------------------------------*/
void sendPassword(uint8 *password){
	uint8 i=0;
	while(password[i]!=13){
		UART_sendByte(password[i]);
		i++;
	}
	UART_sendByte(password[i]);
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : setPassword
[DESCRIPTION]   : Function is responsible for taking the password twice from
				  the user and send them to Control ECU to check if the two
				  passwords are matched or not. If the two passwords are not
				  matched the HMI ECU will repeat the process and take the two
				  passwords again from the user.

[Args]		    :
				in  -> point to array:
						This argument is empty array to store the password.
[Return]	   :
				void
------------------------------------------------------------------------------*/
void setPassword(uint8 *password){
	LCD_sendCommand(CLEAR_COMMAND);
	LCD_displayString("Enter New Pass:");
	LCD_goToRowColumn(1,0);
	getPassword(password);
	sendPassword(password);
	LCD_sendCommand(CLEAR_COMMAND);
	LCD_displayString("Reenter New Pass");
	LCD_goToRowColumn(1,0);
	getPassword(password);
	sendPassword(password);
	g_matchingCheck=UART_receiveByte();
	LCD_sendCommand(CLEAR_COMMAND);
	/*Check from the Control ECU if 2 entered password is matched or not*/
	while(g_matchingCheck==UNMATCHED){
		/*as long as the 2 entered password is not matched display error on LCD
		 * and repeat setting password for first time*/
		LCD_displayString("Error Try again");
		_delay_ms(200);  /*Displaying time for error message*/
		LCD_sendCommand(CLEAR_COMMAND);
		LCD_displayString("Enter New Pass:");
		LCD_goToRowColumn(1,0);
		getPassword(password);
		sendPassword(password);
		LCD_sendCommand(CLEAR_COMMAND);
		LCD_displayString("Reenter New Pass");
		LCD_goToRowColumn(1,0);
		getPassword(password);
		sendPassword(password);
		g_matchingCheck=UART_receiveByte();
		LCD_sendCommand(CLEAR_COMMAND);	}
	/*If the 2 entered passwords are matched display successful */
	LCD_displayString("Successful !");
	_delay_ms(200);
}
