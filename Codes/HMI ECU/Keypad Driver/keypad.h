/* -----------------------------------------------------------------------------
[FILE NAME]    :	keypad.h

[AUTHOR]       :	MOHANAD K. SAEED

[DATA CREATED] :	12/02/2021

[DESCRIPTION]  :	Header File for Keypad Driver
------------------------------------------------------------------------------*/

#ifndef KEYPAD_H
#define KEYPAD_H

#include "../Important Heading Files/std_types.h"
#include "../Important Heading Files/micro_config.h"
#include "../Important Heading Files/common_macros.h"

/* -----------------------------------------------------------------------------
 *                      Preprocessor Macros                                    *
 ------------------------------------------------------------------------------*/

/* Keypad configurations for number of rows and columns */
#define N_col 4
#define N_row 4

/* Keypad Port Configurations */
#define KEYPAD_PORT_OUT PORTB
#define KEYPAD_PORT_IN  PINB
#define KEYPAD_PORT_DIR DDRB

/* -----------------------------------------------------------------------------
 *                      Functions Prototypes                                   *
/* ---------------------------------------------------------------------------*/

/*
 * Function responsible for getting the pressed keypad key
 */
uint8 KEYPAD_getPressedKey(void);

#endif
