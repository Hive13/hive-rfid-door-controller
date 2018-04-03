// https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino
#include <Wiegand.h>

#include "leds.h"
#include "soda_temp.h"
#include "vend.h"
#include "log.h"
#include "API.h"
#include "schedule.h"
#include "network.h"

static WIEGAND wg;

void setup()
	{
	log_begin(115200);
	log_msg("Hive13 Vending Arduino Shield v.04");
	leds_init();
	vend_init();
	network_init();	
	wg.begin();
	soda_temp_init();
	}

void loop()
	{
	unsigned long code;

	run_schedule();
	
	if (wg.available())
		{
		code = wg.getCode();
		log_msg("Scanned badge %lu/0x%lX, type W%d", code, code, wg.getWiegandType());
		handle_vend(code);
		}
	}

/* vim:set filetype=c: */
