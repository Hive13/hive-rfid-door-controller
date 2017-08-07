#include <b64.h>
#include <HttpClient.h>

#include <Wiegand.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include "cJSON.h"
#include "API.h"
#include "ui.h"
#include "schedule.h"
#include "log.h"
#include "pins.h"

#define BODY_SZ 1024

#define NETWORK_TIMEOUT 5000
#define NETWORK_DELAY   25

char *location   = "main_door";
char key[]       = {'Y', 'o', 'u', ' ', 'l', 'o', 's', 't', ' ', 't', 'h', 'e', 'G', 'a', 'm', 'e'};

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
	pinMode(DOORBELL_PIN, INPUT_PULLUP);
	pinMode(BUZZER_PIN,   OUTPUT);
	attachInterrupt(digitalPinToInterrupt(DOORBELL_PIN), doorbell_isr, CHANGE);

  Serial.println("Initializing Ethernet Controller.");
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
	char badge_code;
	char buffer[UDP_TX_PACKET_MAX_SIZE];
	unsigned long badgeID = 0;
	unsigned char type;
	unsigned long time = 0;
	boolean cached = false;
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
			cached = true;
			open_door();
			break;
			}
		}

	badge_code = check_badge(badgeID);
	log_msg("Response from check_badge(): %d", badge_code);

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
