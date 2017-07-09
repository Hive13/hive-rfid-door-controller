#include <Arduino.h>

#include "ui.h"
#include "log.h"
#include "schedule.h"

struct door_open
	{
	unsigned int  beep_pin, light_pin, door_pin, open_pin;
	unsigned char beep_state, cycles, status;
	};

static char led_flicker(void *data, unsigned long *time, unsigned long now)
	{
	static unsigned char c = 0;

	if (!c)
		{
		*time = now + 50;
		LIGHT_GREEN();
		}
	else
		{
		*time = now + 2500;
		LIGHT_RED();
		}
	c = !c;
	return SCHEDULE_REDO;
	}

void ui_init(void)
	{
	digitalWrite(BEEP_PIN,  LOW);
	digitalWrite(DOOR_PIN,  HIGH);
	digitalWrite(LIGHT_PIN, LOW);
	
	pinMode(BEEP_PIN,  OUTPUT);
	pinMode(DOOR_PIN,  OUTPUT);
	pinMode(LIGHT_PIN, OUTPUT);
	pinMode(OPEN_PIN,  INPUT);

	schedule(0, (time_handler *)led_flicker, NULL);
	}

void beep_it(struct beep_pattern *pattern)
	{
	unsigned int i = 0;
	register unsigned char light = pattern->options & 0x03;

	if (light == GREEN_ALWAYS)
		LIGHT_GREEN();
	else if (light == RED_ALWAYS)
		LIGHT_RED();

	while (i < pattern->cycle_count)
		{
		if (i++)
			delay(pattern->silence_ms);
		if (light == RED_WITH_BEEP)
			LIGHT_RED();
		else if (light == GREEN_WITH_BEEP)
			LIGHT_GREEN();
		digitalWrite(BEEP_PIN, HIGH);
		delay(pattern->beep_ms);
		digitalWrite(BEEP_PIN, LOW);
		if (light == RED_WITH_BEEP)
			LIGHT_GREEN();
		else if (light == GREEN_WITH_BEEP)
			LIGHT_RED();
		}
	
	/* Always lease the light in this state when idle */
	LIGHT_RED();
	}

static char close_door(struct door_open *d, unsigned long *t, unsigned long m)
	{
	unsigned char c;
	
	d->beep_state = !d->beep_state;
	digitalWrite(d->beep_pin,  d->beep_state);
	digitalWrite(d->light_pin, d->beep_state);

	if (--(d->cycles))
		{
		c = digitalRead(d->open_pin);
		if (!c)
			{
			*t = m + 100;
			return SCHEDULE_REDO;
			}
		}
	digitalWrite(d->door_pin,  HIGH);
	digitalWrite(d->beep_pin,  LOW);
	digitalWrite(d->light_pin, LOW);
	d->status = OPEN_IDLE;
	return SCHEDULE_DONE;
	}

void open_door(void)
	{
	static struct door_open d =
		{
		beep_pin:   BEEP_PIN,
		light_pin:  LIGHT_PIN,
		door_pin:   DOOR_PIN,
		open_pin:   OPEN_PIN,
		beep_state: 0,
		cycles:     0,
		status:     OPEN_IDLE,
		};
	
	d.cycles     = (DOOR_OPEN_TIME / 100);
	d.beep_state = 0;
	
	if (d.status == OPEN_IDLE)
		{
		d.status = OPEN_IN_PROGRESS;
		digitalWrite(d.door_pin, LOW);
		schedule(millis() + 100, (time_handler *)close_door, &d);
		}
	}

