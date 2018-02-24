#include <Wiegand.h>

#include "cJSON.h"
#include "pins.h"
#include "API.h"
#include "access_temp.h"
#include "schedule.h"
#include "log.h"
#include "http.h"
#include "ui.h"
#include "wifi.h"

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
struct beep_pattern init_log =
	{
	.beep_ms     = 100,
	.silence_ms  = 100,
	.cycle_count = 3,
	.options     = RED_ALWAYS,
	};
struct beep_pattern init_temp =
	{
	.beep_ms     = 100,
	.silence_ms  = 100,
	.cycle_count = 4,
	.options     = RED_ALWAYS,
	};
struct beep_pattern init_wifi =
	{
	.beep_ms     = 100,
	.silence_ms  = 100,
	.cycle_count = 5,
	.options     = RED_ALWAYS,
	};

void setup(void)
	{
	unsigned char i = LOW;
	ui_init();
	
	beep_it(&init_wg);
	wg.begin(D0_PIN, D0_PIN, D1_PIN, D1_PIN);
	delay(1000);
	beep_it(&init_log);
	log_begin(115200);
	delay(1000);

	beep_it(&init_temp);
	access_temperature_init();
	delay(1000);
	beep_it(&init_wifi);
	wifi_init();
	delay(1000);
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
