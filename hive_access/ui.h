#ifndef __UI_H
#define __UI_H

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

/* Number of ms */
#define DOOR_OPEN_TIME     5000

#define OPEN_IDLE        0
#define OPEN_IN_PROGRESS 1

#define GREEN_ALWAYS    0
#define RED_ALWAYS      1
#define RED_WITH_BEEP   2
#define GREEN_WITH_BEEP 3

#ifdef __cplusplus
extern "C" {
#endif

struct beep_pattern
	{
	unsigned long beep_ms;
	unsigned long silence_ms;
	unsigned long cycle_count;
	unsigned char options;
	};

void beep_it(struct beep_pattern *pattern);
void open_door(void);
void ui_init(void);

#ifdef __cplusplus
}
#endif
#endif /* __UI_H */
