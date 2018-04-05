#include "config.h"
#include "soda_temp.h"
#include "vend.h"
#include "log.h"
#include "http.h"
#include "ui.h"
#include "schedule.h"
#include "network.h"
#include "scanner.h"

void setup(void)
	{
	log_begin(115200);
	ui_init();
	network_init();
	vend_init();
	scanner_init(handle_vend);
	soda_temp_init();
	beep_it(BEEP_PATTERN_START);
	}

void loop(void)
	{
	run_schedule();
	}

/* vim:set filetype=c: */