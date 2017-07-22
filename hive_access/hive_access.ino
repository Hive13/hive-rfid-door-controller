#include <Wiegand.h>

#include "cJSON.h"
#include "API.h"
#include "temp.h"
#include "schedule.h"
#include "log.h"
#include "http.h"
#include "ui.h"
#include "wifi.h"

static WIEGAND wg;

struct beep_pattern start_of_day =
	{
	.beep_ms     = 200,
	.silence_ms  = 100,
	.cycle_count = 2,
	.options     = RED_WITH_BEEP,
	};

void setup(void)
	{
	unsigned char i = LOW;
	ui_init();

	wg.begin(D0_PIN, D0_PIN, D1_PIN, D1_PIN);
	log_begin(115200);

	temperature_init();
	wifi_init();

	beep_it(&start_of_day);
	log_msg("Ready to rumble!");
	}

void loop(void)
	{
	unsigned long code;
	unsigned char type;

	run_schedule();
	
	if (wg.available())
		{
		code = wg.getCode();
		type = wg.getWiegandType();

		log_msg("Scanned badge %lu/0x%lX, type W%d", code, code, type);

		if (type == 26)
			check_badge(code, open_door);
		}
	}
