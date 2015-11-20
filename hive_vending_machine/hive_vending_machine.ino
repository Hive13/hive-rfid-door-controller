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

static WIEGAND wg;
static byte mac[] = {0x90, 0xa2, 0xda, 0x0d, 0x7c, 0x9a};
static char kHostname[] = "door.at.hive13.org";

void setup()
	{
	char kPath[] = "/vendtest";

	Serial.begin(57600);
	// Let's set up the Ethernet Connections
	Serial.println("Hive13 Vending Arduino Shield v.04");
	Serial.println("Initializing lights.");
	leds_init();
	Serial.println("Initializing Ethernet Controller.");
	while (Ethernet.begin(mac) != 1)
		{
		Serial.println("Error obtaining DHCP address.  Let's wait a second and try again");
		delay(1000);
		}
	
	http_get("door test", kHostname, kPath);
	wg.begin();
	// wiegand/rfid reader pins
	pinMode(TEMPERATURE_POWER_PIN, OUTPUT);
	pinMode(COMPRESSOR_RELAY, OUTPUT);
	digitalWrite(COMPRESSOR_RELAY, LOW);
	digitalWrite(TEMPERATURE_POWER_PIN, HIGH);
	vend_init();
	}

void loop()
	{
	char host_path[255], i;
	unsigned long code, m = millis();
	int err;
	static unsigned long temp_ready_time = 0;
	
	
	if (!temp_ready_time)
		{
		i = start_read_temperature();
		if (!i)
			temp_ready_time = m + TEMPERATURE_READ_TIME;
		else
			temp_ready_time = 0;
		}
	else if (temp_ready_time <= m)
		{
		temp_ready_time = 0;
		handle_temperature();
		}
	
	if(wg.available())
		{
		code = wg.getCode();
		snprintf(host_path, sizeof(host_path), "Scanned badge %lu/0x%lX, type W%d\n", code, code, wg.getWiegandType());
		Serial.print(host_path);
		// This is what we do when we actually get the OK to vend...
		snprintf(host_path, sizeof(host_path), "/vendcheck/%lu/go", code);
		err = http_get("vend", kHostname, host_path);
		if (err == 200)
			do_vend();
		else
			Serial.println("Didn't receive the OK to vend...");
		}
	
	vend_check();
	}

/* vim:set filetype=c: */
