#include <Arduino.h>
#include "config.h"

#include "lcd.h"

static unsigned char cursor_loc;
static char screen[LCD_SIZE];
static char buffer[LCD_SIZE];
unsigned char lcd_attr;
static unsigned char buf_attr;

void LCD_init(void)
	{
	DDR_LCD_DATA   =   0xFF;
	PORT_LCD_CTRL &= ~(PIN_LCD_RS | PIN_LCD_RW | PIN_LCD_E1 | PIN_LCD_E2);
	DDR_LCD_CTRL  |=  (PIN_LCD_RS | PIN_LCD_RW | PIN_LCD_E1 | PIN_LCD_E2);
	
	cursor_loc = 0;
	lcd_attr = LCD_ATTR_ON | LCD_ATTR_NOASCROLL;
	buf_attr = LCD_ATTR_ON | LCD_ATTR_NOASCROLL;
	memset(screen, ' ', LCD_SIZE);
	memset(buffer, ' ', LCD_SIZE);
  // Wait for more than 15 ms after VCC rises to 4.5 V
  delay(50);
  LCD_putcmd(LCD_INIT, PIN_LCD_E1);  
  LCD_putcmd(LCD_INIT, PIN_LCD_E2);  
  delay(5);
  LCD_putcmd(LCD_INIT, PIN_LCD_E1);  
  LCD_putcmd(LCD_INIT, PIN_LCD_E2);
  delayMicroseconds(100);
  LCD_putcmd(LCD_INIT, PIN_LCD_E1);
  LCD_putcmd(LCD_INIT, PIN_LCD_E2);
	LCD_putcmd(LCD_FUNCTION_SET | LCD_8BIT_MODE | LCD_2LINE_MODE, PIN_LCD_E1);
	LCD_putcmd(LCD_FUNCTION_SET | LCD_8BIT_MODE | LCD_2LINE_MODE, PIN_LCD_E2);
  LCD_putcmd(LCD_DISP_ON_OFF, PIN_LCD_E1);
  LCD_putcmd(LCD_DISP_ON_OFF, PIN_LCD_E2);
  LCD_putcmd(LCD_CLEAR, PIN_LCD_E1);
  LCD_putcmd(LCD_CLEAR, PIN_LCD_E2);
	delay(5);
  LCD_putcmd(LCD_ENTRY_MODE_SET | LCD_ENTRY_INCREMENT, PIN_LCD_E1);
  LCD_putcmd(LCD_ENTRY_MODE_SET | LCD_ENTRY_INCREMENT, PIN_LCD_E2);
  LCD_putcmd(LCD_DISP_ON_OFF | LCD_DISP_ON, PIN_LCD_E1);
  LCD_putcmd(LCD_DISP_ON_OFF | LCD_DISP_ON, PIN_LCD_E2);
	}

void LCD_putcmd(unsigned char data, unsigned char e_pin)
	{
	PORT_LCD_CTRL &= ~(e_pin | PIN_LCD_RS | PIN_LCD_RW);

  PORT_LCD_DATA = data;

	delayMicroseconds(1);
	PORT_LCD_CTRL |= e_pin;
	delayMicroseconds(1);
	PORT_LCD_CTRL &= ~(e_pin);
	delayMicroseconds(100);
	}

void LCD_putch(unsigned char ch, unsigned char e_pin)
	{
	PORT_LCD_CTRL |= PIN_LCD_RS;
	PORT_LCD_CTRL &= ~(e_pin | PIN_LCD_RW);

  PORT_LCD_DATA = ch;

	delayMicroseconds(1);
	PORT_LCD_CTRL |= e_pin;
	delayMicroseconds(1);
	PORT_LCD_CTRL &= ~(e_pin);
	delayMicroseconds(100);
	}
