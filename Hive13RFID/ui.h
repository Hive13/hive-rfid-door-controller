#ifndef __UI_H
#define __UI_H

#define LIGHT_RED()   digitalWrite(LIGHT_PIN, HIGH)
#define LIGHT_GREEN() digitalWrite(LIGHT_PIN, LOW)

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
