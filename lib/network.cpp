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
#ifndef NO_SCANNER
#include "leds.h"
#include "output.h"
#endif

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
static WiFiUDP udp;
static IPAddress mc_ip(239, 72, 49, 51);

struct mc_operation
	{
	unsigned char header[8];
	void *ptr;
	} mc_ops[8];

void wifi_error(void)
	{
	WiFi.disconnect(1);
	network_init();
	}

void register_mc(char header[], void *ptr)
	{
	unsigned char i;

	for (i = 0; i < (sizeof(mc_ops) / sizeof(mc_ops[0])); i++)
		if (!mc_ops[i].ptr)
			{
			memmove(mc_ops[i].header, header, 8);
			mc_ops[i].ptr = ptr;
			break;
			}
	}

static char handle_mc(WiFiUDP *u, unsigned long *time, unsigned long now)
	{
	char buffer[256];
	int sz;
	unsigned char i;

	*time = now + 100;
	sz = u->parsePacket();
	if (!sz)
		return SCHEDULE_REDO;

	if (sz >= 8
#ifdef PLATFORM_ESP8266
		&& u->destinationIP() == mc_ip
#endif
		&& (sz = u->read(buffer, sizeof(buffer))) >= 8)
		{
		for (i = 0; i < (sizeof(mc_ops) / sizeof(mc_ops[0])); i++)
			{
			if (!mc_ops[i].ptr)
				continue;
			if (!memcmp(buffer, mc_ops[i].header, 8))
				schedule(0, (time_handler *)mc_ops[i].ptr, NULL);
			}
		}

	u->flush();
	return SCHEDULE_REDO;
	}
#endif

void network_init(void)
	{
#ifdef PLATFORM_ARDUINO
	log_msg("Initializing Ethernet Controller.");
	leds_busy();
	while (Ethernet.begin(mac) != 1)
		{
		log_msg("Error obtaining DHCP address.  Let's wait a second and try again.");
		delay(1000);
		}
	schedule(0, handle_ethernet, NULL);
	leds_off();

	/* Do it twice for Arduino because the first request times out. */
	update_nonce();
	udp.beginMulticast(mc_ip, MULTICAST_PORT);
#endif
#ifdef PLATFORM_ESP8266
	int status;
	unsigned char i = 0;

	memset(mc_ops, 0, sizeof(mc_ops));
	log_progress_start("Connecting to SSID %s", ssid);
	status = WiFi.begin(ssid, pass);
	log_progress("[%i]", status);
	while ((status = WiFi.status()) != WL_CONNECTED)
		{
#ifndef NO_SCANNER
		i = !i;
		set_output(OUTPUT_SCANNER_LIGHT, i);
#endif
		log_progress("{%i}", status);
		delay(250);
		}
	log_progress_end("connected!");
	WiFi.setAutoConnect(1);
	WiFi.setAutoReconnect(1);
	udp.beginMulticast(WiFi.localIP(), mc_ip, MULTICAST_PORT);
#endif
	schedule(0, (time_handler *)handle_mc, &udp);
	update_nonce();
	}
