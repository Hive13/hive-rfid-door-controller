#ifndef __LCD_H
#define __LCD_H

#define LCD_MODE_4BIT


#define PIN_LCD_D7 7
#define PIN_LCD_D6 6
#define PIN_LCD_D5 5
#define PIN_LCD_D4 4
#define PIN_LCD_RW 9
#define PIN_LCD_E1 10
#define PIN_LCD_E2 11
#define PIN_LCD_RS 8





#define LCD_CHARSTR(x) LCD_CHARSTR1(x)
#define LCD_CHARSTR2(x) #x
#define LCD_CHARSTR1(x) LCD_CHARSTR2(\00 ## x)

#define LCD_COLS 40
#define LCD_ROWS 4
#define LCD_SIZE (LCD_ROWS * LCD_COLS)

#define LCD_HOME 0x02
#define LCD_NEXT_LINE 0xC0
#define LCD_CLEAR 0x01

#define LCD_SET_CGRAM_ADDR 0x40
#define LCD_SET_DDRAM_ADDR 0x80

#define LCD_INIT 0x30

#define LCD_FUNCTION_SET 0x20
#define LCD_8BIT_MODE 0x10
#define LCD_4BIT_MODE 0x00
#define LCD_2LINE_MODE 0x08
#define LCD_5X8_FONT 0x04

#define LCD_DISP_ON_OFF 0x08
#define LCD_DISP_ON 0x04
#define LCD_DISP_CURSOR 0x02
#define LCD_DISP_BLINK 0x01

#define LCD_ENTRY_MODE_SET 0x04
#define LCD_ENTRY_INCREMENT 0x02
#define LCD_ENTRY_SHIFT 0x01

#define LCD_ATTR_CURSOR 0x80
#define LCD_ATTR_BLINK 0x40
#define LCD_ATTR_ON 0x20
/* Don't scroll the screen without an explicit '\n' */
#define LCD_ATTR_NOASCROLL 0x10


#endif /* __LCD_H */