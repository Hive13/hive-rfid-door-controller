#ifndef __UI_H
#define __UI_H

#include "config.h"
#include "leds.h"

#ifdef LIGHT_INV
#define LIGHT_RED(pin)   digitalWrite((pin), LOW)
#define LIGHT_GREEN(pin) digitalWrite((pin), HIGH)
#else
#define LIGHT_RED(pin)   digitalWrite((pin), HIGH)
#define LIGHT_GREEN(pin) digitalWrite((pin), LOW)
#endif

#ifdef BEEP_INV
#define BEEP_ON  HIGH
#define BEEP_OFF LOW
#else
#define BEEP_ON  LOW
#define BEEP_OFF HIGH
#endif

#define OPEN_IDLE        0
#define OPEN_IN_PROGRESS 1

#define GREEN_ALWAYS    0
#define RED_ALWAYS      1
#define RED_WITH_BEEP   2
#define GREEN_WITH_BEEP 3


#define BEEP_PATTERN_INIT  0
#define BEEP_PATTERN_START 1
#define BEEP_PATTERN_NETWORK_ERROR 2
#define BEEP_PATTERN_PACKET_ERROR 3
#define BEEP_PATTERN_INVALID_CARD 4
#define BEEP_PATTERN_NO_CREDITS 5

#ifdef __cplusplus
extern "C" {
#endif

struct beep_pattern
	{
	unsigned long beep_ms;
	unsigned long silence_ms;
	unsigned long beep_color;
	unsigned long silence_color;
	unsigned long cycle_count;
	unsigned char options;
	char *log_message;
	};

void beep_it(unsigned char pattern_idx);
void open_door(void);
void ui_init(void);

#ifdef __cplusplus
}
#endif
#endif /* __UI_H */
