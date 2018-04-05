#include "config.h"
#include "access_temp.h"
#include "log.h"
#include "http.h"
#include "ui.h"
#include "schedule.h"
#include "network.h"
#include "scanner.h"

static void access_handler(unsigned long code)
	{
	check_badge(code, open_door);
	}

void setup(void)
	{
	log_begin(115200);
	ui_init();
	network_init();
	scanner_init(access_handler);
	access_temperature_init();
	beep_it(BEEP_PATTERN_START);
	}

void loop(void)
	{
	run_schedule();
	}

/* vim:set filetype=c: */