#include "config.h"

#ifdef PLATFORM_ARDUINO
#include <Ethernet.h>
#endif
#ifdef PLATFORM_ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#endif

#include "network.h"
#include "log.h"
#include "schedule.h"
#include "http.h"

#ifdef PLATFORM_ARDUINO
static byte mac[] = MAC;

char handle_ethernet(void *ptr, unsigned long *t, unsigned long m)
	{
	Ethernet.maintain();
	*t = m + 1000;

	return SCHEDULE_REDO;
	}
#endif

#ifdef PLATFORM_ESP8266
static char *ssid = WIFI_SSID;
static char *pass = WIFI_PASS;

static WiFiUDP   udp;
static IPAddress mc_ip(239, 72, 49, 51);

static struct beep_pattern doorbell_beep =
	{
	.beep_ms     = 100,
	.silence_ms  = 100,
	.cycle_count = 15,
	.options     = GREEN_WITH_BEEP,
	};

char handle_multicast(WiFiUDP *u, unsigned long *t, unsigned long now)
	{
	static unsigned long last_beep = 0;
	int sz = u->parsePacket(), i;
	unsigned char b[8];

	if (sz <= 0)
		return SCHEDULE_REDO;

	i = u->read(b, 8);

	if (i == 8 && !memcmp(b, "doorbell", 8))
		{
		if ((last_beep + 500) <= now)
			beep_it(&doorbell_beep);
		last_beep = now;
		}

	u->flush();

	return SCHEDULE_REDO;
	}

void wifi_error(void)
	{
	WiFi.disconnect(1);
	network_init();
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
	
	/* Do it twice for Arduino because the first request times out. */
	update_nonce();
#endif
#ifdef PLATFORM_ESP8266
	int status;
	unsigned char i = 0;

	log_progress_start("Connecting to SSID %s", ssid);
	status = WiFi.begin(ssid, pass);
	log_progress("[%i]", status);
	while ((status = WiFi.status()) != WL_CONNECTED)
		{
		i = !i;
		digitalWrite(LIGHT_PIN, i);
		log_progress("{%i}", status);
		delay(250);
		}
	log_progress_end("connected!");
	WiFi.setAutoConnect(1);
	WiFi.setAutoReconnect(1);

	udp.beginMulticast(WiFi.localIP(), mc_ip, 12595);

	schedule(0, (time_handler *)handle_multicast, &udp);
#endif
	update_nonce();
	}