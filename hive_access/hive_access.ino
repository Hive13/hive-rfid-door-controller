#include <Wiegand.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#include "cJSON.h"
#include "API.h"
#include "temp.h"
#include "schedule.h"
#include "log.h"
#include "http.h"
#include "ui.h"

static WIEGAND wg;
static char *ssid = "hive13int";
static char *pass = "hive13int";
int status        = WL_IDLE_STATUS;

struct beep_pattern start_of_day =
	{
	.beep_ms     = 200,
	.silence_ms  = 100,
	.cycle_count = 2,
	.options     = RED_WITH_BEEP,
	};

void setup(void)
	{
	unsigned char i = LOW;
	ui_init();

	wg.begin(D0_PIN, D0_PIN, D1_PIN, D1_PIN);
	log_begin(115200);

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
	
	temperature_init();

	beep_it(&start_of_day);
	log_msg("Ready to rumble!");
	}

void loop(void)
	{
	unsigned long code;
	unsigned char type;

	run_schedule();
	
	if (wg.available())
		{
		code = wg.getCode();
		type = wg.getWiegandType();

		log_msg("Scanned badge %lu/0x%lX, type W%d", code, code, type);

		if (type == 26)
			check_badge(code, open_door);
		}
	}
