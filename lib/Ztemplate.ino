#include "config.h"
#include "temp.h"
#include "log.h"
#include "http.h"
#include "ui.h"
#include "schedule.h"
#include "network.h"
#include "scanner.h"
#include "doorbell.h"
#include "output.h"
#include "eeprom_lib.h"

#ifdef SODA_MACHINE
#include "vend.h"
#endif

void setup(void)
	{
	log_begin(115200);
	eeprom_init();
	output_init();
	ui_init();
	network_init();
#ifdef SODA_MACHINE
	vend_init();
	scanner_init(handle_vend);
#else
	scanner_init(open_door);
#endif
	doorbell_init();
	temperature_init();
	beep_it(BEEP_PATTERN_START);
	}

void loop(void)
	{
	run_schedule();
	}

/* vim:set filetype=c: */
