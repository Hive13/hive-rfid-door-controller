#include "config.h"

#include <Arduino.h>
#include <Wiegand.h>

#include "scanner.h"
#include "schedule.h"
#include "log.h"

static WIEGAND wg;

static char handle_scan(scan_handler *h, unsigned long *t, unsigned long m)
	{
	unsigned long code;
	if (wg.available())
		{
		code = wg.getCode();
		log_msg("Scanned badge %lu/0x%lX, type W%d", code, code, wg.getWiegandType());
		h(code);
		}
	return SCHEDULE_REDO;
	}

void scanner_init(scan_handler *handler)
	{
	wg.begin(WIEGAND_D0_PIN, WIEGAND_D0_PIN, WIEGAND_D1_PIN, WIEGAND_D1_PIN);
	schedule(0, handle_scan, handler);
	}