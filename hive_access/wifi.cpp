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

	*t = now + 10000;
	return SCHEDULE_REDO;
	}

void wifi_error(void)
	{
	WiFi.disconnect(1);
	wifi_init();
	}

void wifi_init(void)
	{
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

	schedule(0, log_wifi_stuff, NULL);
	}

