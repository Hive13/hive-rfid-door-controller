#include <Arduino.h>
#include <OneWire.h>

#include "log.h"
#include "schedule.h"
#include "output.h"

extern OneWire *ds;
extern struct output outputs[];
extern unsigned char output_count;

void output_init(void)
	{
	struct output *o = outputs;

	for (; o < outputs + output_count; o++)
		{
		switch (o->type)
			{
			case OUTPUT_TYPE_ARDUINO:
			case OUTPUT_TYPE_ESP:
				pinMode(o->pin, OUTPUT);
				break;
			case OUTPUT_TYPE_AVR:
				*(o->data.avr.port) |= (1 << o->pin);
				break;
			default:
				break;
			}
		}
	log_msg("All outputs set up.");
	}

void set_output(unsigned char output_num, unsigned char state)
	{
	struct output *o = outputs + output_num;
	unsigned char mask;

	log_msg("setting output %u to %u", output_num, state);

	if (o->type == OUTPUT_TYPE_ARDUINO)
		digitalWrite(o->pin, state);
	else if (o->type == OUTPUT_TYPE_AVR)
		{
		mask = (1 << o->pin);
		if (state)
			*(o->data.avr.port) |= mask;
		else
			*(o->data.avr.port) &= ~mask;
		}
	else if (o->type == OUTPUT_TYPE_ESP)
		{
		if (state)
			GPOS |= (1 << o->pin);
		else
			GPOC |= (1 << o->pin);
		}
	else if (o->type == OUTPUT_TYPE_DS2408)
		{
		}
	}

