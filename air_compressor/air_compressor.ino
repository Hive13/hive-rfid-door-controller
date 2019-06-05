#include "config.h"
#include "http.h"
#include "schedule.h"
#include "network.h"
#include "output.h"
#include "air_control.h"
#include "eeprom_lib.h"
#include "log.h"

void setup(void)
	{
	log_begin(115200);
	eeprom_init();
	network_init();
	air_compressor_init();
	}

void loop(void)
	{
	run_schedule();
	}

/* vim:set filetype=c: */
