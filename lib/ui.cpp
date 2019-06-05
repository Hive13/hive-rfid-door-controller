#include <Arduino.h>

#include "ui.h"
#include "log.h"
#include "schedule.h"
#include "output.h"

static struct beep_pattern patterns[] = {
	{ /* BEEP_PATTERN_INIT */
	.beep_ms       = 100,
	.silence_ms    = 100,
	.beep_color    = 0x00000000,
	.silence_color = 0x00000000,
	.cycle_count   = 2,
	.options       = RED_ALWAYS,
	.log_message   = "Initialized UI",
	},
	{ /* BEEP_PATTERN_START */
	.beep_ms       = 400,
	.silence_ms    = 150,
	.beep_color    = 0x0000FF00,
	.silence_color = 0x0000FF00,
	.cycle_count   = 2,
	.options       = RED_WITH_BEEP,
	.log_message   = "Ready to Rumble!",
	},
	{ /* BEEP_PATTERN_NETWORK_ERROR */
	.beep_ms       = 250,
	.silence_ms    = 250,
	.beep_color    = 0x00FF0000,
	.silence_color = 0x00000000,
	.cycle_count   = 4,
	.options       = RED_ALWAYS,
	.log_message   = "Network error",
	},
	{ /* BEEP_PATTERN_PACKET_ERROR */
	.beep_ms       = 100,
	.silence_ms    = 100,
	.beep_color    = 0x00FF8000,
	.silence_color = 0x00000000,
	.cycle_count   = 8,
	.options       = RED_ALWAYS,
	.log_message   = "Packet error",
	},
	{ /* BEEP_PATTERN_INVALID_CARD */
	.beep_ms       = 1000,
	.silence_ms    = 0,
	.beep_color    = 0x00FF0000,
	.silence_color = 0x00000000,
	.cycle_count   = 1,
	.options       = RED_ALWAYS,
	.log_message   = "Invalid card",
	},
	{ /* BEEP_PATTERN_NO_CREDITS */
	.beep_ms       = 100,
	.silence_ms    = 100,
	.beep_color    = 0x00FF0000,
	.silence_color = 0x00FF0000,
	.cycle_count   = 5,
	.options       = RED_ALWAYS,
	.log_message   = "No credits",
	},
	{ /* BEEP_PATTERN_DOORBELL */
	.beep_ms       = 100,
	.silence_ms    = 100,
	.beep_color    = 0x0000FF00,
	.silence_color = 0x00000000,
	.cycle_count   = 5,
	.options       = GREEN_ALWAYS,
	.log_message   = "Doorbell rang",
	},
};

struct door_open
	{
	unsigned int  open_pin;
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
	pinMode(OPEN_PIN,  INPUT_PULLUP);

	leds_init();
	schedule(0, (time_handler *)led_flicker, NULL);
	beep_it(BEEP_PATTERN_INIT);
	}

void beep_it(unsigned char pattern_idx)
	{
	unsigned int i = 0;
	struct beep_pattern *pattern = patterns + pattern_idx;
	register unsigned char light = pattern->options & 0x03;

	if (pattern->log_message)
		log_msg("Beep: %s", pattern->log_message);

	if (light == GREEN_ALWAYS)
		LIGHT_GREEN();
	else if (light == RED_ALWAYS)
		LIGHT_RED();

	while (i < pattern->cycle_count)
		{
		if (i++)
			{
			leds_all(pattern->silence_color);
			delay(pattern->silence_ms);
			}
		leds_all(pattern->beep_color);
		if (light == RED_WITH_BEEP)
			LIGHT_RED();
		else if (light == GREEN_WITH_BEEP)
			LIGHT_GREEN();
		set_output(OUTPUT_SCANNER_BEEPER, BEEP_ON);
		delay(pattern->beep_ms);
		set_output(OUTPUT_SCANNER_BEEPER, BEEP_OFF);
		if (light == RED_WITH_BEEP)
			LIGHT_GREEN();
		else if (light == GREEN_WITH_BEEP)
			LIGHT_RED();
		}

	/* Always leave the light in this state when idle */
	LIGHT_RED();
	leds_off();
	}

static char close_door(struct door_open *d, unsigned long *t, unsigned long m)
	{
	unsigned char c;

	d->beep_state = !d->beep_state;
	set_output(OUTPUT_SCANNER_BEEPER, d->beep_state);
	set_output(OUTPUT_SCANNER_LIGHT,  d->beep_state);

	if (--(d->cycles))
		{
		c = digitalRead(d->open_pin);
		if (!c)
			{
			*t = m + 100;
			return SCHEDULE_REDO;
			}
		}
	set_output(OUTPUT_DOOR_LATCH,     DOOR_LOCKED);
	set_output(OUTPUT_SCANNER_BEEPER, BEEP_OFF);
	LIGHT_RED();
	d->status = OPEN_IDLE;
	return SCHEDULE_DONE;
	}

void open_door(void)
	{
	static struct door_open d =
		{
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
		set_output(OUTPUT_DOOR_LATCH, DOOR_UNLOCKED);
		schedule(millis() + 100, (time_handler *)close_door, &d);
		}
	}

