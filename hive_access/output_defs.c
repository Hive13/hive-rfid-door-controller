#include <Arduino.h>

#include "output.h"
#include "ui.h"

struct output outputs[] =
	{
		{ /* OUTPUT_DOOR_LATCH */
		.type = OUTPUT_TYPE_DS2408,
		.init = DOOR_LOCKED,
		.data = { .ds2408_addr = {0x29, 0x29, 0x9F, 0x27, 0x00, 0x00, 0x00, 0x2D} },
		.pin  = 0,
		},
		{ /* OUTPUT_SCANNER_BEEPER */
		.type = OUTPUT_TYPE_DS2408,
		.init = BEEP_OFF,
		.data = { .ds2408_addr = {0x29, 0x29, 0x9F, 0x27, 0x00, 0x00, 0x00, 0x2D} },
		.pin  = 1,
		},
		{ /* OUTPUT_SCANNER_LIGHT */
		.type = OUTPUT_TYPE_DS2408,
		.init = LIGHT_INIT,
		.data = { .ds2408_addr = {0x29, 0x29, 0x9F, 0x27, 0x00, 0x00, 0x00, 0x2D} },
		.pin  = 2,
		},
	};

unsigned char output_count = (sizeof(outputs) / sizeof(struct output));
