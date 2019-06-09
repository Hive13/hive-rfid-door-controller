#include <Arduino.h>

#include "air_control.h"
#include "schedule.h"
#include "http.h"
#include "log.h"

static unsigned char in_quiet;


char handle_quiet_hours(void *ptr, unsigned long *t, unsigned long m)
	{
	unsigned int start_ms, end_ms;

	quiet_hours(&start_ms, &end_ms, &in_quiet);
	*t = m + (start_ms < end_ms ? start_ms : end_ms);
	return SCHEDULE_REDO;
	}

void air_compressor_init(void)
	{
	digitalWrite(COMPRESS_PIN, LOW);
	digitalWrite(BLEED_PIN,    LOW);
	pinMode(COMPRESS_PIN, OUTPUT);
	pinMode(BLEED_PIN,    OUTPUT);

	schedule(0, (time_handler *)handle_air_compressor, NULL);
	schedule(0, (time_handler *)handle_quiet_hours, NULL);
	}

char end_bleed_off(void *ptr, unsigned long *t, unsigned long m)
	{
	digitalWrite(BLEED_PIN, LOW);
	return SCHEDULE_DONE;
	}

void bleed_off(unsigned long m)
	{
	static void *sched = NULL;
	digitalWrite(BLEED_PIN, HIGH);

	if (sched)
		schedule_cancel(sched);

	sched = schedule(m, end_bleed_off, NULL);
	}

char handle_air_compressor(void *ptr, unsigned long *t, unsigned long m)
	{
	static unsigned int log_time = 0;
	static signed char debounce = 0;
	static unsigned char state = 0;

	signed int val = analogRead(PRESSURE_PIN);
	/*
		Sensor is 500 mV at 0 P.S.I. and 4500 mV at 200 P.S.I.
		There is a resistor network dividing this by 2.
		ADC is 0-1024, with 1024 being 3200 mV, or .32/mV
		0 P.S.I. is 80.
		200 P.S.I. is 720 (or 640 more)
		We want a value in tenths of P.S.I.

		1. Subtract 80.
		2. Multiply by 2000 / 640 => 25 / 8.
	*/
	val = ((val - 80) * 25) / 8;
	if (val < 0 || val > 2000 || in_quiet) /* These vals should not happen; shut off NOW (also quiet hours) */
		debounce = 127;
	else if (val > OFF_PSI && debounce < BOUNCE_COUNT && state)
		debounce++;
	else if (val < ON_PSI && debounce > -BOUNCE_COUNT && !state)
		debounce--;
	else
		debounce = 0;

	log_msg("PSI: %d.%d", val / 10, val % 10);
	log_msg("Debounce: %d", debounce);

	if (debounce >= BOUNCE_COUNT)
		{
		debounce = 0;
		digitalWrite(COMPRESS_PIN, LOW);
		if (state)
			bleed_off(m + BLEED_OFF_MS);
		state = 0;
		}
	else if (debounce <= -BOUNCE_COUNT)
		{
		debounce = 0;
		digitalWrite(COMPRESS_PIN, HIGH);
		state = 1;
		}

	if (log_time < m)
		{
		log_temp(val, DEVICE);
		log_time = m + LOG_INTERVAL;
		}

	*t = m + 1000;
	return SCHEDULE_REDO;
	}
