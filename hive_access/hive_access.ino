#include "cJSON.h"
#include "config.h"
#include "API.h"
#include "access_temp.h"
#include "schedule.h"
#include "log.h"
#include "http.h"
#include "ui.h"
#include "network.h"
#include "scanner.h"

struct beep_pattern start_of_day =
	{
	.beep_ms     = 400,
	.silence_ms  = 150,
	.cycle_count = 2,
	.options     = RED_WITH_BEEP,
	};
struct beep_pattern init =
	{
	.beep_ms     = 100,
	.silence_ms  = 100,
	.cycle_count = 2,
	.options     = RED_ALWAYS,
	};

static void access_handler(unsigned long code)
	{
	check_badge(code, open_door);
	}

void setup(void)
	{
	ui_init();
	
	beep_it(&init);
	log_begin(115200);
	network_init();
	scanner_init(access_handler);
	access_temperature_init();

	beep_it(&start_of_day);
	log_msg("Ready to rumble!");
	}

void loop(void)
	{
	run_schedule();
	}
