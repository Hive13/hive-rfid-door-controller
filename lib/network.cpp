#include "config.h"

#ifdef PLATFORM_ARDUINO
#include <Ethernet.h>
#endif

#include "network.h"
#include "log.h"
#include "schedule.h"

#ifdef PLATFORM_ARDUINO
static byte mac[] = MAC;

char handle_ethernet(void *ptr, unsigned long *t, unsigned long m)
	{
	Ethernet.maintain();
	*t = m + 1000;

	return SCHEDULE_REDO;
	}
#endif

void network_init(void)
	{
#ifdef PLATFORM_ARDUINO

	log_msg("Initializing Ethernet Controller.");
	while (Ethernet.begin(mac) != 1)
		{
		log_msg("Error obtaining DHCP address.  Let's wait a second and try again.");
		delay(1000);
		}
	schedule(0, handle_ethernet, NULL);

#else
#endif
	}