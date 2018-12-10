#include "config.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "cJSON.h"
#include "log.h"
#include "lights.h"
#include "schedule.h"
#include "network.h"

static IPAddress mc_ip(239, 72, 49, 51);
static WiFiUDP udp;

unsigned char bulbs[4];
#define BULB_COUNT (sizeof(bulbs) / sizeof(bulbs[0]))

unsigned char get_state(char *data, unsigned long *time, unsigned long now)
	{
	struct cJSON *item, *result = get_light_state();
	unsigned char i;
	
	if (!result)
		return SCHEDULE_DONE;
	item = result->child;
	for (i = 0; i < BULB_COUNT && item; i++)
		{
		bulbs[i] = (item->type == cJSON_True || (item->type== cJSON_Number && item->valueint));
		item = item->next;
		}

	cJSON_Delete(result);
	return SCHEDULE_DONE;
	}

char handle_mc(WiFiUDP *u, unsigned long *time, unsigned long now)
	{
	char buffer[256];
	int sz;
	
	*time = now + 1000;
	sz = u->parsePacket();
	if (!sz)
		return SCHEDULE_REDO;
	
	if (sz >= 5 && u->destinationIP() == mc_ip)
		{
		udp.read(buffer, sizeof(buffer));
		
		if (sz == 5 && !memcmp(buffer, "light", 5))
			schedule(0, (time_handler *)get_state, NULL);
		}
	
	u->flush();
	return SCHEDULE_REDO;
	}

void setup(void)
	{
	pinMode(D1, OUTPUT);
	pinMode(D5, OUTPUT);
	pinMode(D6, OUTPUT);
	pinMode(D7, OUTPUT);
	bulbs[0] = 1;
	bulbs[1] = 1;
	bulbs[2] = 1;
	bulbs[3] = 1;
	digitalWrite(D1, bulbs[0]);
	digitalWrite(D5, bulbs[1]);
	digitalWrite(D6, bulbs[2]);
	digitalWrite(D7, bulbs[3]);

	log_begin(115200);
	network_init();
	udp.beginMulticast(WiFi.localIP(), mc_ip, MULTICAST_PORT);
	schedule(0, (time_handler *)handle_mc, &udp);
	schedule(0, (time_handler *)get_state, NULL);
	}

void loop(void)
	{
	run_schedule();
	digitalWrite(D1, bulbs[0]);
	digitalWrite(D5, bulbs[1]);
	digitalWrite(D6, bulbs[2]);
	digitalWrite(D7, bulbs[3]);
	}
