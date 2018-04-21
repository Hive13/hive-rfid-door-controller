#include <Arduino.h>
#include "config.h"

#include "lcd.h"

static unsigned char cursor_loc;
static char screen[LCD_SIZE];
static char buffer[LCD_SIZE];
unsigned char lcd_attr;
static unsigned char buf_attr;

void LCD_init()
	{
	pinMode(PIN_LCD_D7, OUTPUT);
	pinMode(PIN_LCD_D6, OUTPUT);
	pinMode(PIN_LCD_D5, OUTPUT);
	pinMode(PIN_LCD_D4, OUTPUT);
	pinMode(PIN_LCD_E1, OUTPUT);
	pinMode(PIN_LCD_E2, OUTPUT);
	pinMode(PIN_LCD_RW, OUTPUT);
	pinMode(PIN_LCD_RS, OUTPUT);
	
	cursor_loc = 0;
	lcd_attr = LCD_ATTR_ON | LCD_ATTR_NOASCROLL;
	buf_attr = LCD_ATTR_ON | LCD_ATTR_NOASCROLL;
	memset(screen, ' ', LCD_SIZE);
	memset(buffer, ' ', LCD_SIZE);
  // Wait for more than 15 ms after VCC rises to 4.5 V
  _delay_ms(50);
  LCD_putcmd(LCD_INIT, PIN_LCD_E1);  
  LCD_putcmd(LCD_INIT, PIN_LCD_E2);  
  _delay_ms(5);
  LCD_putcmd(LCD_INIT, PIN_LCD_E1);  
  LCD_putcmd(LCD_INIT, PIN_LCD_E2);
  _delay_us(100);
  LCD_putcmd(LCD_INIT, PIN_LCD_E1);
  LCD_putcmd(LCD_INIT, PIN_LCD_E2);
	LCD_putcmd(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE_MODE, PIN_LCD_E1);
	LCD_putcmd(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE_MODE, PIN_LCD_E2);
  LCD_putcmd(LCD_DISP_ON_OFF, PIN_LCD_E1);
  LCD_putcmd(LCD_DISP_ON_OFF, PIN_LCD_E2);
  LCD_putcmd(LCD_CLEAR, PIN_LCD_E1);
  LCD_putcmd(LCD_CLEAR, PIN_LCD_E2);
  LCD_putcmd(LCD_ENTRY_MODE_SET | LCD_ENTRY_INCREMENT, PIN_LCD_E1);
  LCD_putcmd(LCD_ENTRY_MODE_SET | LCD_ENTRY_INCREMENT, PIN_LCD_E2);
  LCD_putcmd(LCD_DISP_ON_OFF | LCD_DISP_ON, PIN_LCD_E1);
	LCD_putcmd(LCD_DISP_ON_OFF | LCD_DISP_ON, PIN_LCD_E2);
	}

void LCD_putcmd(unsigned char data, unsigned char e_pin)
	{
	digitalWrite(PIN_LCD_RS, LOW);
	digitalWrite(e_pin, LOW);
	digitalWrite(PIN_LCD_RW, LOW);

  PORTD = data;

	_delay_us(1);
	digitalWrite(e_pin, HIGH);
	_delay_us(1);
	digitalWrite(e_pin, LOW);
	_delay_us(100);
	}