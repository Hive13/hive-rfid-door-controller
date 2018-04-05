#include "leds.h"
#include "soda_temp.h"
#include "vend.h"
#include "log.h"
#include "API.h"
#include "schedule.h"
#include "network.h"
#include "scanner.h"

void setup()
	{
	log_begin(115200);
	leds_init();
	vend_init();
	network_init();	
	scanner_init(handle_vend);
	soda_temp_init();
	}

void loop()
	{
	run_schedule();
	}

/* vim:set filetype=c: */