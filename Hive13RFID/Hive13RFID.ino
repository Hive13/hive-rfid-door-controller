#include <b64.h>
#include <HttpClient.h>

#include <SHA512.h>

#include <Wiegand.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include "cJSON.h"
#include "API.h"
#include "ui.h"
#include "schedule.h"
#include "log.h"

#define BODY_SZ 1024
#define DOORBELL_PIN 18

#define NETWORK_TIMEOUT 5000
#define NETWORK_DELAY   25

char *location   = "main_door";
char key[]       = {'Y', 'o', 'u', ' ', 'l', 'o', 's', 't', ' ', 't', 'h', 'e', 'G', 'a', 'm', 'e'};

// I'm using the Wiegand library to make reading the files easier. 
// https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino.git
// Pins:  6  = Relay for the door
//        13 = Red/Green light on RFID Reader.  High == Red, Low == Green.
//        12 = Buzzer attached to RFID Reader.  High == Off, Low == On.
//        A0 = Magnetic Switch attached to top of door.  

char check_badge(unsigned long badge)
	{
	static unsigned long scan_count = 0;
	char body[BODY_SZ], *out;
	EthernetClient ec;
	HttpClient hc(ec);
	int err, body_len;
	struct cJSON *resp, *cs;
	unsigned long start = millis(), l;
	unsigned char rv[2 * sizeof(unsigned long)];
	unsigned char rc;

	memcpy(rv, &start, sizeof(unsigned long));
	memcpy(rv + sizeof(unsigned long), &scan_count, sizeof(unsigned long));
	scan_count++;

	out = get_request(badge, "access", location, location, key, sizeof(key), rv, sizeof(rv));
	l = strlen(out);
	Serial.print("Sending ");
	Serial.println(out);

	hc.beginRequest();
  hc.post("intweb.at.hive13.org", "/api/access");
	hc.flush();
	hc.sendHeader("Content-Type", "application/json");
	hc.flush();
	hc.sendHeader("Content-Length", l);
	hc.flush();
	hc.write(out, l);
	hc.flush();
	free(out);

	err = hc.responseStatusCode();
	if (err != 200)
		return RESPONSE_BAD_HTTP;
	body_len = hc.contentLength();
	if (body_len + 1 > BODY_SZ)
		return RESPONSE_BAD_HTTP;
	
	if (hc.skipResponseHeaders() > 0)
		{
		Serial.println("Header error.");
		return RESPONSE_BAD_HTTP;
		}
	
	start = millis();
	l = 0;
	while ((hc.connected() || hc.available()) && ((millis() - start) < NETWORK_TIMEOUT))
		{
		if (hc.available())
			{
			body[l++] = hc.read();
			body_len--;
			start = millis();
			}
		else
			delay(NETWORK_DELAY);
		}
	body[l++] = 0;
	hc.stop();
	Serial.print("Body: ");
	Serial.println(body);

	rc = parse_response(body, &resp, key, sizeof(key), rv, sizeof(rv));

	if (rc != RESPONSE_GOOD)
		return rc;

	if (!(cs = cJSON_GetObjectItem(resp, "access")) || cs->type != cJSON_True)
		{
		cJSON_Delete(resp);
		return RESPONSE_ACCESS_DENIED;
		}

	cJSON_Delete(resp);
	return RESPONSE_GOOD;
	}

WIEGAND wg;
byte mac[]     = {0x48, 0x49, 0x56, 0x45, 0x31, 0x33};
byte ip[]      = {172, 16, 3, 245};
byte gateway[] = {172, 16, 2, 1};
byte subnet[]  = {255, 255, 254, 0};
byte dns_d[]   = {172, 16, 2, 1};
EthernetClient client;
EthernetUDP    udp;

// Cache of badge numbers that have succesfully opened the door.
unsigned long usersCache[100];

struct beep_pattern start_of_day =
	{
	.beep_ms     = 200,
	.silence_ms  = 100,
	.cycle_count = 2,
	.options     = RED_WITH_BEEP,
	};

static char doorbell(void *data, unsigned long *time, unsigned long now)
	{
	//static IPAddress mc_ip(172, 16, 3, 255);
	static IPAddress mc_ip(239, 72, 49, 51);
	udp.beginPacket(mc_ip, 12595);
	udp.write("doorbell");
	udp.endPacket();
	return SCHEDULE_DONE;
	}

void doorbell_isr(void)
	{
	schedule(0, doorbell, NULL);
	}

void setup()
	{
	ui_init();
	log_begin(115200);

	pinMode(DOORBELL_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(DOORBELL_PIN), doorbell_isr, FALLING);

  Serial.println("Initializing Ethernet Controller.");
	Ethernet.begin(mac, ip, dns_d, gateway, subnet);
	udp.begin(12595);

  // Initialize Wiegand Interface
  wg.begin();
  
  // initialize the usersCache array as all 0.
	memset(usersCache, 0, sizeof(usersCache));
	
	beep_it(&start_of_day);
	}

void loop()
	{
	char badge_code;
	unsigned long badgeID = 0;
	unsigned long time = 0;
	boolean cached = false;
	unsigned char i;

	run_schedule();

	if (!wg.available())
		return;

	badgeID = wg.getCode();

	// Sending Debug info to USB...
	Serial.print("Wiegand HEX = ");
	Serial.print(badgeID, HEX);
	Serial.print(", DECIMAL = ");
	Serial.print(badgeID);
	Serial.print(", Type W");
	Serial.println(wg.getWiegandType());

	// Check to see if the user has been cached
	for (i = 0; i < (sizeof(usersCache) / sizeof(usersCache[0])); i++)
		{
		if(usersCache[i] == badgeID && usersCache[i] > 0)
			{
			// If it has been cached open the door.
			Serial.println("Verified via cache ok to open door...");
			cached = true;
			open_door();
			}
		}

	badge_code = check_badge(badgeID);
	Serial.print("Badge code: ");
	Serial.println(badge_code, DEC);

	if (badge_code == RESPONSE_GOOD)
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
	else if (cached == true)
		{
		// remove them from the cached list as they are not allowed in
		for (i = 0; i < (sizeof(usersCache) / sizeof(usersCache[0])); i++)
			if (usersCache[i] == badgeID)
				usersCache[i] = 0;
		}
	}
