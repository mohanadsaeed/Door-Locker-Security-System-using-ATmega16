/* -----------------------------------------------------------------------------
[FILE NAME]    :	lcd.h

[AUTHOR]       :	MOHANAD K. SAEED

[DATA CREATED] :	12/02/2021

[DESCRIPTION]  :	Header File for LCD Driver
------------------------------------------------------------------------------*/

#ifndef LCD_H
#define LCD_H

#include "../Important Heading Files/std_types.h"
#include "../Important Heading Files/common_macros.h"
#include "../Important Heading Files/micro_config.h"

/* -----------------------------------------------------------------------------
 *                      Preprocessor Macros                                    *
------------------------------------------------------------------------------*/
/* LCD Data bits mode configuration */
#define DATA_BITS_MODE 8

/* Use higher 4 bits in the data port */
#if (DATA_BITS_MODE == 4)
#define UPPER_PORT_PINS
#endif

/* LCD HW Pins */
#define RS PC2
#define RW PC1
#define E  PC0
#define LCD_CTRL_PORT PORTC
#define LCD_CTRL_PORT_DIR DDRC

#define LCD_DATA_PORT PORTA
#define LCD_DATA_PORT_DIR DDRA

/* LCD Commands */
#define CLEAR_COMMAND 0x01
#define FOUR_BITS_DATA_MODE 0x02
#define TWO_LINE_LCD_Four_BIT_MODE 0x28
#define TWO_LINE_LCD_Eight_BIT_MODE 0x38
#define CURSOR_OFF 0x0C
#define CURSOR_ON 0x0E
#define SET_CURSOR_LOCATION 0x80 

/* -----------------------------------------------------------------------------
 *                      Functions Prototypes                                   *
 ------------------------------------------------------------------------------*/
void LCD_sendCommand(uint8 command);
void LCD_displayCharacter(uint8 data);
void LCD_displayString(const char *Str);
void LCD_init(void);
void LCD_clearScreen(void);
void LCD_displayStringRowColumn(uint8 row,uint8 col,const char *Str);
void LCD_goToRowColumn(uint8 row,uint8 col);
void LCD_intgerToString(int data);

#endif
