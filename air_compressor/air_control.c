#include <Arduino.h>

#include "air_control.h"
#include "schedule.h"
#include "http.h"

void air_compressor_init(void)
	{
	digitalWrite(COMPRESS_PIN, LOW);
	digitalWrite(BLEED_PIN,    LOW);
	pinMode(COMPRESS_PIN, OUTPUT);
	pinMode(BLEED_PIN,    OUTPUT);

	schedule(0, (time_handler *)handle_air_compressor, NULL);
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
	
	sched = schedule(m + BLEED_OFF_MS, end_bleed_off, NULL);
	}

char handle_air_compressor(void *ptr, unsigned long *t, unsigned long m)
	{
	static unsigned int log_time = 0;
	static char state = LOW;

	int val = analogRead(PRESSURE_PIN);
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

	if (val < 0 || val > OFF_PSI)
		{
		digitalWrite(COMPRESS_PIN, LOW);
		if (state == HIGH)
			bleed_off(m);
		state = LOW;
		}
	else if (val < ON_PSI)
		{
		digitalWrite(COMPRESS_PIN, HIGH);
		state = HIGH;
		}
	
	if (log_time < m)
		{
		log_temp(val, DEVICE);
		log_time = m + LOG_INTERVAL;
		}

	*t = m + 1000;
	return SCHEDULE_REDO;
	}
