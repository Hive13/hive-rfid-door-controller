#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#include "schedule.h"
#include "log.h"
#include "ui.h"
#include "wifi.h"

static char *ssid = "hive13int";
static char *pass = "hive13int";

char log_wifi_stuff(void *ptr, unsigned long *t, unsigned long now)
	{
	int s = WiFi.status();

	log_msg("Wifi status: %i", s);

	*t = now + 5000;
	return SCHEDULE_REDO;
	}

void wifi_error(void)
	{
	unsigned char i = 0;
	WiFi.disconnect();
	log_progress_start("Wifi Disconnect %i - reconnecting", WiFi.status());

	while (WiFi.status() != WL_CONNECTED)
		{
		i = !i;
		digitalWrite(LIGHT_PIN, i);
		log_progress(".");
		delay(250);
		}
	log_progress_end("reconnected.");
	}

void wifi_init(void)
	{
	int status;
	unsigned char i = 0;

	log_progress_start("Connecting to SSID %s", ssid);
	status = WiFi.begin(ssid, pass);
	while (WiFi.status() != WL_CONNECTED)
		{
		i = !i;
		digitalWrite(LIGHT_PIN, i);
		log_progress(".");
		delay(250);
		}
	log_progress_end("connected!");
	WiFi.setAutoConnect(1);
	WiFi.setAutoReconnect(1);

	schedule(0, log_wifi_stuff, NULL);
	}

