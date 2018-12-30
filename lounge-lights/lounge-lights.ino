#include "config.h"

#include "log.h"
#include "lights.h"
#include "schedule.h"
#include "network.h"
#include "eeprom_lib.h"

void setup(void)
	{
	log_begin(115200);
	eeprom_init();
	network_init();
	lights_init();
	}

void loop(void)
	{
	run_schedule();
	}
