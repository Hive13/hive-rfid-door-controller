#ifndef __LCD_H
#define __LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#define PIN_LCD_DATA PINL
#define DDR_LCD_DATA DDRL
#define PORT_LCD_DATA PORTL

#define PIN_LCD_CTRL PINH
#define DDR_LCD_CTRL DDRH
#define PORT_LCD_CTRL PORTH

#define PIN_LCD_RS (1 << 3)
#define PIN_LCD_RW (1 << 4)
#define PIN_LCD_E1 (1 << 5)
#define PIN_LCD_E2 (1 << 6)

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

void LCD_init(void);
void LCD_putch(unsigned char ch, unsigned char e_pin);
void LCD_putcmd(unsigned char data, unsigned char e_pin);

#ifdef __cplusplus
};
#endif

#endif /* __LCD_H */
