#ifndef __UI_H
#define __UI_H

#define BEEP_PIN  D8
#define D0_PIN    D1
#define D1_PIN    D2
#define LIGHT_PIN D0
#define OPEN_PIN  D3
#define DOOR_PIN  D4

#define LIGHT_RED()   digitalWrite(LIGHT_PIN, LOW)
#define LIGHT_GREEN() digitalWrite(LIGHT_PIN, HIGH)

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

struct door_open
	{
	unsigned int  beep_pin, light_pin, door_pin, open_pin;
	unsigned char beep_state, cycles, status;
	};

void beep_it(struct beep_pattern *pattern);
char led_flicker(void *data, unsigned long *time, unsigned long now);
char close_door(struct door_open *d, unsigned long *t, unsigned long m);
void open_door(void);
void ui_init(void);

#ifdef __cplusplus
}
#endif
#endif /* __UI_H */
