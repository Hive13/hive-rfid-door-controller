#include "config.h"
#include "soda_temp.h"
#include "log.h"
#include "http.h"
#include "ui.h"
#include "schedule.h"
#include "network.h"
#include "scanner.h"
#include "doorbell.h"

#ifdef SODA_MACHINE
#include "vend.h"
#else
#ifdef CACHE_BADGES
static unsigned long usersCache[100];

static void cache_access_handler(unsigned long code)
	{
	unsigned char badge_ok, cached = 0;
	unsigned char i;
	
	// Check to see if the user has been cached
	for (i = 0; i < (sizeof(usersCache) / sizeof(usersCache[0])); i++)
		{
		if(usersCache[i] == code && usersCache[i] > 0)
			{
			log_msg("Verified via cache ok to open door...");
			cached = 1;
			open_door();
			break;
			}
		}

	badge_ok = check_badge(code, NULL);

	if (badge_ok)
		{
		// We only need to open the door here if it wasn't opened from checking the cache.
		if (!cached)
			{
			open_door();
			// user is allowed in but hasn't been cached, add them to the cache.
			Serial.println("Adding user to cache.");
			for (i = 0; i < (sizeof(usersCache) / sizeof(usersCache[0])); i++)
				{
				if (usersCache[i] == 0)
					{
					usersCache[i] = code;
					break;
					}
				}
			}
		}
	else if (cached)
		{
		// remove them from the cached list as they are not allowed in
		for (i = 0; i < (sizeof(usersCache) / sizeof(usersCache[0])); i++)
			if (usersCache[i] == code)
				usersCache[i] = 0;
		}
	}
#else
static void access_handler(unsigned long code)
	{
	check_badge(code, open_door);
	}
#endif
#endif

void setup(void)
	{
	log_begin(115200);
	ui_init();
	network_init();
#ifdef SODA_MACHINE
	vend_init();
	scanner_init(handle_vend);
#else
#ifdef CACHE_BADGES
	memset(usersCache, 0, sizeof(usersCache));
	scanner_init(cache_access_handler);
#else
	scanner_init(access_handler);
#endif
#endif
	doorbell_init();
	soda_temp_init();
	beep_it(BEEP_PATTERN_START);
	}

void loop(void)
	{
	run_schedule();
	}

/* vim:set filetype=c: */