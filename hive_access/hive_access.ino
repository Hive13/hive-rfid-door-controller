#include <Wiegand.h>

#include "cJSON.h"
#include "config.h"
#include "API.h"
#include "access_temp.h"
#include "schedule.h"
#include "log.h"
#include "http.h"
#include "ui.h"
#include "network.h"

static WIEGAND wg;

struct beep_pattern start_of_day =
	{
	.beep_ms     = 400,
	.silence_ms  = 150,
	.cycle_count = 2,
	.options     = RED_WITH_BEEP,
	};
struct beep_pattern init_wg =
	{
	.beep_ms     = 100,
	.silence_ms  = 100,
	.cycle_count = 2,
	.options     = RED_ALWAYS,
	};

void setup(void)
	{
	ui_init();
	
	beep_it(&init_wg);
	wg.begin(D0_PIN, D0_PIN, D1_PIN, D1_PIN);
	log_begin(115200);

	access_temperature_init();
	network_init();
	update_nonce();

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
