#ifndef __UI_H
#define __UI_H

#include "config.h"
#include "leds.h"
#include "output.h"

#ifdef LIGHT_INV
#define LIGHT_INIT 0
#define LIGHT_RED()   set_output(OUTPUT_SCANNER_LIGHT, LOW)
#define LIGHT_GREEN() set_output(OUTPUT_SCANNER_LIGHT, HIGH)
#else
#define LIGHT_INIT 1
#define LIGHT_RED()   set_output(OUTPUT_SCANNER_LIGHT, HIGH)
#define LIGHT_GREEN() set_output(OUTPUT_SCANNER_LIGHT, LOW)
#endif

#ifdef BEEP_INV
#define BEEP_ON  1
#define BEEP_OFF 0
#else
#define BEEP_ON  0
#define BEEP_OFF 1
#endif

#ifdef DOOR_LOCK_INV
#define DOOR_LOCKED   1
#define DOOR_UNLOCKED 0
#else
#define DOOR_LOCKED   0
#define DOOR_UNLOCKED 1
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
#define BEEP_PATTERN_DOORBELL 6

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
