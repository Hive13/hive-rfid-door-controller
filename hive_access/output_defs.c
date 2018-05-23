#include <Arduino.h>

#include "output.h"

struct output outputs[] =
	{
		{ /* OUTPUT_DOOR_LATCH */
		.type = OUTPUT_TYPE_ARDUINO,
		.pin  = WD4,
		},
		{ /* OUTPUT_SCANNER_BEEPER */
		.type = OUTPUT_TYPE_ARDUINO,
		.pin  = WD8,
		},
		{ /* OUTPUT_SCANNER_LIGHT */
		.type = OUTPUT_TYPE_ARDUINO,
		.pin  = WD0,
		},
	};

unsigned char output_count = (sizeof(outputs) / sizeof(struct output));
