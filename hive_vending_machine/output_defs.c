#include <Arduino.h>

#include "output.h"
#include "ui.h"

struct output outputs[] =
	{
		{ /* OUTPUT_DOOR_LATCH */
		.type = OUTPUT_TYPE_ARDUINO,
		.init = DOOR_LOCKED,
		.pin  = -1,
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
		{ /* OUTPUT_VEND_RELAY */
		.type = OUTPUT_TYPE_ARDUINO,
		.init = 1,
		.pin  = 7,
		},
		{ /* OUTPUT_COMPRESSOR_RELAY */
		.type = OUTPUT_TYPE_ARDUINO,
		.init = 1,
		.pin  = -1,
		},
	};

unsigned char output_count = (sizeof(outputs) / sizeof(struct output));
