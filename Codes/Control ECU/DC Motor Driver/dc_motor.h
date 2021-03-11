/* -----------------------------------------------------------------------------
[FILE NAME]    :	dc_motor.h

[AUTHOR]       :	MOHANAD K. SAEED

[DATA CREATED] :	20/02/2021

[DESCRIPTION]  :	Header File for DC Motor Driver
------------------------------------------------------------------------------*/

#ifndef DC_MOTOR_H
#define DC_MOTOR_H

#include "../Important Heading Files/std_types.h"
#include "../Important Heading Files/common_macros.h"
#include "../Important Heading Files/micro_config.h"

/* ----------------------------------------------------------------------------
 *                         Timer Stucture Definition                          *
 -----------------------------------------------------------------------------*/

typedef enum{
	CW,CCW
}Dcmotor_rotDir;

typedef struct{
	uint8 speedPercentage;
	Dcmotor_rotDir rotationDirection;
}Dcmotor_ConfigType;

/* ----------------------------------------------------------------------------
 *                      Preprocessor Macros                                   *
  ----------------------------------------------------------------------------*/
#define TIMER2
#define TOP 255
/* DC Motor HW Pins */
#define DCMOTOR_PORT PORTD
#define DCMOTOR_PORT_DIR DDRD
#define IN1 PD5
#define IN2 PD6


/* -----------------------------------------------------------------------------
 *                      Functions Prototypes                                   *
 ------------------------------------------------------------------------------*/
void DCMOTOR_init(const Dcmotor_ConfigType * Motor_Ptr);
void DCMOTOR_revertRotationDirection(void);
void DCMOTOR_changeRotationDirection(Dcmotor_rotDir direction);
void DCMOTOR_stop(void);
void DCMOTOR_changeSpeed(uint8 percentage);

#endif
