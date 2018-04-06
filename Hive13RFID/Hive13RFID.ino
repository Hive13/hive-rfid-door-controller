#include <Ethernet.h>
#include <EthernetUdp.h>

#include "config.h"
#include "log.h"
#include "http.h"
#include "ui.h"
#include "schedule.h"
#include "network.h"
#include "scanner.h"

static EthernetClient         client;
static EthernetUDP            udp;
static IPAddress              mc_ip(239, 72, 49, 51);
static unsigned long          usersCache[100];
static volatile unsigned char doorbell_data = 5;

/*
	Buzz for 1000ms, sending out a packet every 250ms.
	Wait an additional 4000ms before accepting the next
	button press.
*/
static char doorbell(char *data, unsigned long *time, unsigned long now)
	{
	if (++(*data) < 4)
		{
		udp.beginPacket(mc_ip, MULTICAST_PORT);
		udp.write("doorbell");
		udp.endPacket();
		*time = now + 250;
		return SCHEDULE_REDO;
		}
	digitalWrite(BUZZER_PIN, LOW);
	*time = now + 4000;
	if (*data == 4)
		return SCHEDULE_REDO;
	return SCHEDULE_DONE;
	}

static char doorbell_holdoff(char *data, unsigned long *time, unsigned long now)
	{
	log_msg("holdoff()");
	if (doorbell_data == 5)
		ring_doorbell(1);
	return SCHEDULE_DONE;
	}

/*
	This function must work inside or outside of an ISR.
*/
void ring_doorbell(char send_packet)
	{
	digitalWrite(BUZZER_PIN, HIGH);
	if (send_packet)
		{
		doorbell_data = 0;
		schedule(0, doorbell, &doorbell_data);
		}
	else
		{
		doorbell_data = 3;
		schedule(millis() + 1000, doorbell, &doorbell_data);
		}
	}

void doorbell_isr(void)
	{
	static volatile void *doorbell_t = NULL;
	char pressed = !digitalRead(DOORBELL_PIN);
	unsigned long m = millis();

	if (pressed)
		doorbell_t = schedule(m + 50, doorbell_holdoff, NULL);
	else if (doorbell_t)
		{
		schedule_cancel(doorbell_t);
		doorbell_t = NULL;
		}
	}

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

void setup(void)
	{
	log_begin(115200);
	ui_init();

	digitalWrite(BUZZER_PIN, LOW);
	pinMode(DOORBELL_PIN,    INPUT_PULLUP);
	pinMode(BUZZER_PIN,      OUTPUT);
	attachInterrupt(digitalPinToInterrupt(DOORBELL_PIN), doorbell_isr, CHANGE);

	network_init();
	udp.beginMulticast(mc_ip, MULTICAST_PORT);

	memset(usersCache, 0, sizeof(usersCache));
	scanner_init(cache_access_handler);
	beep_it(BEEP_PATTERN_START);
	}

void loop(void)
	{
	char buffer[UDP_TX_PACKET_MAX_SIZE];
	int sz;

	run_schedule();

	sz = udp.parsePacket();
	if (sz)
		{
		log_msg("Packet!");
		udp.read(buffer, UDP_TX_PACKET_MAX_SIZE);
		}
	}

/* vim:set filetype=c: */