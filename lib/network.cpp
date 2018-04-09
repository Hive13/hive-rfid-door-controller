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
#include "leds.h"

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
#endif
	update_nonce();
	}