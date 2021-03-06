#include <Arduino.h>

#include "output.h"
#include "ui.h"

struct output outputs[] =
	{
		{ /* OUTPUT_DOOR_LATCH */
		.type = OUTPUT_TYPE_ARDUINO,
		.init = DOOR_LOCKED,
		.pin  = A0,
		},
		{ /* OUTPUT_SCANNER_BEEPER */
		.type = OUTPUT_TYPE_ARDUINO,
		.init = BEEP_OFF,
		.pin  = 12,
		},
		{ /* OUTPUT_SCANNER_LIGHT */
		.type = OUTPUT_TYPE_ARDUINO,
		.init = LIGHT_INIT,
		.pin  = 6,
		},
	};

unsigned char output_count = (sizeof(outputs) / sizeof(struct output));
