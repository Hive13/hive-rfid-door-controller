#include <Ethernet.h>
// https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino
#include <Wiegand.h>
#include <SPI.h>
// The b64 and HttpClient libraries are both in this repository:
// https://github.com/amcewen/HttpClient
#include <b64.h>
#include <HttpClient.h>
// https://github.com/adafruit/Adafruit-WS2801-Library
#include <Adafruit_WS2801.h>
// https://github.com/PaulStoffregen/OneWire
#include <OneWire.h>

#include "leds.h"
#include "temp.h"
#include "vend.h"
#include "http.h"
#include "log.h"

static WIEGAND wg;
static byte mac[] = {0x90, 0xa2, 0xda, 0x0d, 0x7c, 0x9a};
static char kHostname[] = "door.at.hive13.org";

void setup()
	{
	char kPath[] = "/vendtest";

	log_begin();
	
	log_msg("Hive13 Vending Arduino Shield v.04");
	leds_init();
	log_msg("Initializing Ethernet Controller.");
	while (Ethernet.begin(mac) != 1)
		{
		log_msg("Error obtaining DHCP address.  Let's wait a second and try again.");
		delay(1000);
		}
	
	http_get("door test", kHostname, kPath);
	wg.begin();
	temperature_init();
	vend_init();
	}

void loop()
	{
	char host_path[255];
	unsigned long code;
	int err;

	handle_temperature();
	
	if(wg.available())
		{
		code = wg.getCode();
		log_msg("Scanned badge %lu/0x%lX, type W%d\n", code, code, wg.getWiegandType());

		snprintf(host_path, sizeof(host_path), "/vendcheck/%lu/go", code);
		err = http_get("vend", kHostname, host_path);
		if (err == 200)
			do_vend();
		else
			log_msg("Didn't receive the OK to vend...\n");
		}
	
	vend_check();
	}

/* vim:set filetype=c: */
