#include "config.h"

#include <Arduino.h>
#include <Wiegand.h>

#include "scanner.h"
#include "schedule.h"
#include "log.h"

static WIEGAND wg;
#ifdef CACHE_BADGES
#define CACHE_SIZE 100
static unsigned long usersCache[CACHE_SIZE];
#endif

static char handle_scan(scan_handler *h, unsigned long *t, unsigned long m)
	{
	unsigned long code;
	unsigned char badge_ok, i;

	if (!wg.available())
		return SCHEDULE_REDO;

	code = wg.getCode();
	log_msg("Scanned badge %lu/0x%lX, type W%d", code, code, wg.getWiegandType());

#ifdef CACHE_BADGES
	if (check_badge(code))
		h();
	return SCHEDULE_REDO;
#endif

	/* Check to see if the user has been cached */
	for (i = 0; i < CACHE_SIZE; i++)
		if(usersCache[i] == code && usersCache[i] > 0)
			{
			log_msg("Verified via cache ok to open door...");
			h();
			break;
			}

	badge_ok = check_badge(code);
	if (badge_ok)
		{
		/* We only need to open the door here if it wasn't opened from checking the cache. */
		if (i == CACHE_SIZE)
			{
			h();
			/* user is allowed in but hasn't been cached, add them to the cache. */
			log_msg("Adding user to cache.");
			for (i = 0; i < CACHE_SIZE; i++)
				if (usersCache[i] == 0)
					{
					usersCache[i] = code;
					break;
					}
			}
		}
	else if (i < CACHE_SIZE)
		{
		/* Remove them from the cache. */
		for (i = 0; i < CACHE_SIZE; i++)
			if (usersCache[i] == code)
				usersCache[i] = 0;
		}
	return SCHEDULE_REDO;
	}

void scanner_init(scan_handler *handler)
	{
#ifdef CACHE_BADGES
	memset(usersCache, 0, sizeof(usersCache));
#endif
#ifdef PLATFORM_ARDUINO
	wg.begin(WIEGAND_D0_PIN, digitalPinToInterrupt(WIEGAND_D0_PIN), WIEGAND_D1_PIN, digitalPinToInterrupt(WIEGAND_D1_PIN));
#else
	wg.begin(WIEGAND_D0_PIN, WIEGAND_D0_PIN, WIEGAND_D1_PIN, WIEGAND_D1_PIN);
#endif
	schedule(0, (time_handler *)handle_scan, (void *)handler);
	}
