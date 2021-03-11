/* -----------------------------------------------------------------------------
[FILE NAME]    :	Control ECU

[AUTHOR]       :	MOHANAD K. SAEED

[DATA CREATED] :	11/03/2021

[DESCRIPTION]  :	Door Locker Security System, Control ECU which is
					responsible for the system operations and control.
------------------------------------------------------------------------------*/

#define F_CPU 8000000UL
#include "DC Motor Driver/dc_motor.h"
#include "External EEPROM/external_eeprom.h"
#include "Timer 0/timer0.h"
#include "Timer 1/timer1.h"
#include "Timer 2/timer2.h"
#include "UART/uart.h"

/* Random number to check if the control ECU is ready to receive
 * password or not*/
#define CONTROL_ECU_READY 0xF0
/* Global Variable to store the state of 2 passwords; matched or not*/
volatile uint8 g_matchingCheck;
/* Global Variable to store the number of seconds counted by timer 1*/
volatile uint8 g_seconds=0;
/*2 Passwords States*/
enum{UNMATCHED=1,MATCHED=2};
/*Door States*/
enum{OPEN,CHANGE,RESET,OPENED,CLOSED,CLOSING,DONE};
/*Function to check if 2 passwords are matched or not*/
uint8 matchingCheck(uint8 * password , uint8 * password_2);
/*Function to receive password from HMI ECU using UART protocol*/
void receivePassword(uint8 *password);
/*Function to write password to EEPROM*/
void writePasswordToEeprom(uint8 *password);
/*Function to read password from EEPROM*/
void readPasswordFromEeprom(uint8 *password);
/*Function to activate the buzzer*/
void BUZZER_on(void);
/*Call back function for timer 1*/
void periodCallBack(void);
/*Function to make the process of opening the door*/
void openDoor(uint8 *password,uint8 *password_2);
/*Function to make the process of changing the password*/
void changePassword(uint8 *password,uint8 *password_2);
/*Function to set the password and save it to EEPROM*/
void setPassword(uint8 *password,uint8 *password_2);

int main(void){
	volatile uint8 password[7];
	volatile uint8 password_2[7];
	uint8 state;
	Timer1_ConfigType period;
	Dcmotor_ConfigType motor;
	Uart_ConfigType uart;
	/*Setting the UART Configurations*/
	uart.baudRate=9600;
	uart.dataBits=UART_8_BIT;
	uart.stopBits=UART_1_BIT;
	uart.parityType=UART_DISABLE_PARITY;
	/*Initializing UART*/
	UART_init(&uart);
	/*Enable I-Bit*/
	SET_BIT(SREG,7);
	/*Setting the Timer 1 Configurations to count 1 second every Interrupt*/
	period.mode=TIMER1_CTC;
	period.clock=TIMER1_F_CPU_64;
	period.initialValue=0;
	period.oc1AMode=OC1_A_DISCONNECT;
	period.oc1BMode=OC1_B_DISCONNECT;
	period.tick=15625;
	TIMER1_init(&period);
	TIMER1_setCallBack(periodCallBack,TIMER1_CTC);
	TIMER1_stopCount();
	/*Initializing EEPROM*/
	EEPROM_init();
	/*Setting DC Motor Configurations*/
	motor.speedPercentage=0;
	motor.rotationDirection=CW;
	/*Initializing DC Motor*/
	DCMOTOR_init(&motor);
	/*Tell HMI ECU the I am ready to receive the data*/
	UART_sendByte(CONTROL_ECU_READY);
	setPassword(password,password_2);
	while(1){
		/*Waiting to receive the state from HMI ECU; Open the door or Change
		 * the password*/
		state=UART_receiveByte();
		switch(state){
		case OPEN:
			openDoor(password,password_2);
			break;
		case CHANGE:
			changePassword(password,password_2);
			break;
		}
	}
	return 0;
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : setPassword
[DESCRIPTION]   : Function is responsible for receiving two passwords from
				  the HMI ECU and and check if they are matched or not then send
				  the check to HMI ECU. Repeating the process until the HMI ECU
				  send two matched passwords.

[Args]		    :
				in  -> point to array:
						This argument is empty array to store the first password.
				in  -> point to array:
						This argument is empty array to store the second password.
[Return]	   :
				void
------------------------------------------------------------------------------*/
void setPassword(uint8 *password,uint8 *password_2){
	receivePassword(password);
	receivePassword(password_2);
	g_matchingCheck=matchingCheck(password,password_2);
	/*While the 2 passwords are not matched repeat receiving 2 password and
	 * repeat checking*/
	while(g_matchingCheck==UNMATCHED){
		UART_sendByte(UNMATCHED);
		receivePassword(password);
		receivePassword(password_2);
		g_matchingCheck=matchingCheck(password,password_2);
	}
	/*Once they are matched save the password into the EEPROM*/
		UART_sendByte(MATCHED);
		writePasswordToEeprom(password);
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : changePassword
[DESCRIPTION]   : This function is responsible for receiving password from
				  HMI ECU and checking if this password and the password saved in
				  EEPROM are matched or not. If the received password is not
				  matched for 3 times the buzzer will be activated for 1 min.
				  If the password is matched then received the new password from
				  HMI ECU to replace with the saved password in EEPROM
[Args]		    :
				in  -> point to array:
						This argument is empty array to store the received
						password.
				in  -> point to array:
						This argument is empty array to store the saved password
						in EEPROM.
[Return]	   :
				void
------------------------------------------------------------------------------*/
void changePassword(uint8 *password,uint8 *password_2){
	uint8 n=0;
	receivePassword(password);
	readPasswordFromEeprom(password_2);
	g_matchingCheck=matchingCheck(password,password_2);
	UART_sendByte(g_matchingCheck);
	while((g_matchingCheck==UNMATCHED) & (n<2)){
		n++;
		receivePassword(password);
		g_matchingCheck=matchingCheck(password,password_2);
		UART_sendByte(g_matchingCheck);
	}
	if(n==2){
		BUZZER_on();
	}
	else if(g_matchingCheck==MATCHED){
		setPassword(password,password_2);
		UART_sendByte(DONE);
	}
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : openDoor
[DESCRIPTION]   : This function is responsible for receiving the password
				  from the HMI ECU to check if it's correct or not by comparing
				  it with the saved password in EEPROM. If the received
				  password is wrong 3 times the buzzer will be activated for
				  1 minute. If the password is correct, The DC Motor will rotate
				  to open the door from 15 seconds and send OPENING to HMI ECU
				  then stop for 3 seconds and send OPENED to HMI ECU then
				  revert it direction and rotate for 15 seconds and send CLOSED
				  to HMI ECU

[Args]		    :
				in  -> point to array:
						This argument is empty array to store the received
						password.
				in  -> point to array:
						This argument is empty array to store the saved password
						in EEPROM.
[Return]	   :
				void
------------------------------------------------------------------------------*/
void openDoor(uint8 *password,uint8 *password_2){
	uint8 n=0,i;
	receivePassword(password);
	readPasswordFromEeprom(password_2);
	g_matchingCheck=matchingCheck(password,password_2);
	UART_sendByte(g_matchingCheck);
	while((g_matchingCheck==UNMATCHED) & (n<2)){
		n++;
		receivePassword(password);
		g_matchingCheck=matchingCheck(password,password_2);
		UART_sendByte(g_matchingCheck);
	}
	if(n==2){
		BUZZER_on();
	}
	else if(g_matchingCheck==MATCHED){
		DCMOTOR_changeRotationDirection(CW);
		DCMOTOR_changeSpeed(100);
		TIMER1_startCount(TIMER1_F_CPU_64);
		while(g_seconds<15){};
		TIMER1_stopCount();
		g_seconds=0;
		DCMOTOR_stop();
		UART_sendByte(OPENED);
		TIMER1_startCount(TIMER1_F_CPU_64);
		while(g_seconds<3){};
		TIMER1_stopCount();
		g_seconds=0;
		UART_sendByte(CLOSING);
		DCMOTOR_changeRotationDirection(CCW);
		DCMOTOR_changeSpeed(100);
		TIMER1_startCount(TIMER1_F_CPU_64);
		while(g_seconds<15){};
		g_seconds=0;
		TIMER1_stopCount();
		DCMOTOR_stop();
		UART_sendByte(CLOSED);
	}
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : Buzzer_on
[DESCRIPTION]   : This function is responsible for activating the buzzer for
				  1 minute.

[Args]		    :
				void
[Return]	   :
				void
------------------------------------------------------------------------------*/
void BUZZER_on(void){
	SET_BIT(DDRD,PD2);
	SET_BIT(PORTD,PD2);
	TIMER1_startCount(TIMER1_F_CPU_64);
	while(g_seconds<60){};
	g_seconds=0;
	TIMER1_stopCount();
	CLEAR_BIT(PORTD,PD2);
	UART_sendByte(RESET);
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : periodCallBack
[DESCRIPTION]   : Function is responsible for incrementing the seconds global
			      variable each timer interrupt.

[Args]		    :
				void
[Return]	   :
				void
------------------------------------------------------------------------------*/
void periodCallBack(void){
	g_seconds++;
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : readPasswordFromEeprom
[DESCRIPTION]   : Function is responsible for reading password from the
				  external memory EEPROM

[Args]		    :
				in  -> point to array:
						This argument is an empty array to store the password
						which will be read from the EEPROM.
[Return]	   :
				void
------------------------------------------------------------------------------*/
void readPasswordFromEeprom(uint8 *password){
	uint8 i;
	for(i=0;i<6;i++){
		EEPROM_readByte((0x0311+8*i),password);
	}
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : writePasswordToEeprom
[DESCRIPTION]   : Function is responsible for writing the password in the
				  external memory EEPROM

[Args]		    :
				in  -> point to array:
						This argument is array include the password which will
						be written to EEPROM.
[Return]	   :
				void
------------------------------------------------------------------------------*/
void writePasswordToEeprom(uint8 *password){
	uint8 i;
	for(i=0;i<6;i++){
		EEPROM_writeByte((0x0311+8*i),password[i]);
	}
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : receivePassword
[DESCRIPTION]   : Function is responsible for receiving password from HMI ECU

[Args]		    :
				in  -> point to array:
						This argument is an empty array to store the password.
[Return]	   :
				void
------------------------------------------------------------------------------*/
void receivePassword(uint8 *password){
	uint8 i;
	for(i=0;i<6;i++){
		password[i]=UART_receiveByte();
	}
}

/* ---------------------------------------------------------------------------
[FUNCTION NAME] : matchingCheck
[DESCRIPTION]   : Function is responsible for comparing two password.

[Args]		    :
				in  -> point to array:
						This argument is array includes the first password.
				in  -> point to array:
						This argument is array includes the second password.

[Return]	   :
				out -> MATCHED OR UNMATCHED
------------------------------------------------------------------------------*/
uint8 matchingCheck(uint8 * password , uint8 * password_2){
	uint8 i,j=0;
	for (i=0;i<6;i++){
		if(password[i]==password_2[i]){
			j++;
		}
	}
	if(j==6)
		return MATCHED;
	else
		return UNMATCHED;
}

