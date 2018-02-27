#include <Wiegand.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include "config.h"
#include "cJSON.h"
#include "API.h"
#include "ui.h"
#include "schedule.h"
#include "log.h"
#include "http.h"

char *location = "main_door";
char key[]     = {'Y', 'o', 'u', ' ', 'l', 'o', 's', 't', ' ', 't', 'h', 'e', 'G', 'a', 'm', 'e'};

WIEGAND wg;
byte mac[]     = {0x48, 0x49, 0x56, 0x45, 0x31, 0x33};
byte ip[]      = {172, 16, 3, 245};
byte gateway[] = {172, 16, 2, 1};
byte subnet[]  = {255, 255, 254, 0};
byte dns_d[]   = {172, 16, 2, 1};

EthernetClient   client;
EthernetUDP      udp;
static IPAddress mc_ip(239, 72, 49, 51);
unsigned long    usersCache[100];

static volatile unsigned char doorbell_data = 5;

struct beep_pattern start_of_day =
	{
	.beep_ms     = 200,
	.silence_ms  = 100,
	.cycle_count = 2,
	.options     = RED_WITH_BEEP,
	};

/*
	Buzz for 1000ms, sending out a packet every 250ms.
	Wait an additional 4000ms before accepting the next
	button press.
*/
static char doorbell(char *data, unsigned long *time, unsigned long now)
	{
	if (((++(*data)) & 0x0F) < 4)
		{
		udp.beginPacket(mc_ip, 12595);
		udp.write("doorbell");
		udp.endPacket();
		*time = now + 250;
		return SCHEDULE_REDO;
		}
	digitalWrite(BUZZER_PIN, LOW);
	*time = now + 4000;
	if (((*data) & 0x0F) == 4)
		return SCHEDULE_REDO;
	return SCHEDULE_DONE;
	}

static char doorbell_holdoff(char *data, unsigned long *time, unsigned long now)
	{
	log_msg("holdoff()");
	if ((doorbell_data & 0x0F) == 5)
		ring_doorbell(1);
	return SCHEDULE_DONE;
	}

/*
	This function must work inside or outside of an ISR.
*/
void ring_doorbell(char send_packet)
	{
	doorbell_data = send_packet ? 0x80 : 0;
	digitalWrite(BUZZER_PIN, HIGH);
	schedule(0, doorbell, &doorbell_data);
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

void setup()
	{
	ui_init();
	log_begin(115200);

	digitalWrite(BUZZER_PIN, LOW);
	pinMode(DOORBELL_PIN,    INPUT_PULLUP);
	pinMode(BUZZER_PIN,      OUTPUT);
	attachInterrupt(digitalPinToInterrupt(DOORBELL_PIN), doorbell_isr, CHANGE);

	log_msg("Initializing Ethernet Controller.");
	Ethernet.begin(mac, ip, dns_d, gateway, subnet);
	udp.beginMulticast(mc_ip, 12595);

	// Initialize Wiegand Interface
	wg.begin();

	// initialize the usersCache array as all 0.
	memset(usersCache, 0, sizeof(usersCache));

	beep_it(&start_of_day);
	}

void loop()
	{
	unsigned char badge_ok, cached;
	char buffer[UDP_TX_PACKET_MAX_SIZE];
	unsigned long badgeID = 0;
	unsigned char type;
	unsigned long time = 0;
	unsigned char i;
	int sz;

	run_schedule();

	sz = udp.parsePacket();
	if (sz)
		{
		log_msg("Packet!");
		udp.read(buffer, UDP_TX_PACKET_MAX_SIZE);
		}

	if (!wg.available())
		return;

	badgeID = wg.getCode();
	type    = wg.getWiegandType();

	log_msg("Badge scan: %lu/0x%lX, type W%hd", badgeID, badgeID, type);

	if (type != 26)
		return;

	// Check to see if the user has been cached
	for (i = 0; i < (sizeof(usersCache) / sizeof(usersCache[0])); i++)
		{
		if(usersCache[i] == badgeID && usersCache[i] > 0)
			{
			log_msg("Verified via cache ok to open door...");
			cached = 1;
			open_door();
			break;
			}
		}

	badge_ok = check_badge(badgeID, NULL);

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
					usersCache[i] = badgeID;
					break;
					}
				}
			}
		}
	else if (cached)
		{
		// remove them from the cached list as they are not allowed in
		for (i = 0; i < (sizeof(usersCache) / sizeof(usersCache[0])); i++)
			if (usersCache[i] == badgeID)
				usersCache[i] = 0;
		}
	}
