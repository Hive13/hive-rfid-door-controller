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

WIEGAND wg;
byte mac[] = {0x90, 0xa2, 0xda, 0x0d, 0x7c, 0x9a};
const char kHostname[] = "door.at.hive13.org";
extern unsigned char soda_count;

void setup()
	{
	EthernetClient c;
	HttpClient http(c);
	int err;
	const char kPath[] = "/vendtest";

	Serial.begin(57600);
	// Let's set up the Ethernet Connections
	Serial.println("Hive13 Vending Arduino Shield v.04");
	Serial.print(soda_count);
	Serial.println(" sodas configured.");
	Serial.println("Initializing lights.");
	leds_init();
	Serial.println("Initializing Ethernet Controller.");
	while (Ethernet.begin(mac) != 1)
		{
		Serial.println("Error obtaining DHCP address.  Let's wait a second and try again");
		delay(1000);
		}
	
	Serial.println("Attempting to make static connection to Door to ensure connectivity.");
	
	err = http.get(kHostname, kPath);
	if (err == 0)
		{
		Serial.println("Vending Machine Test Connection OK");
		err = http.responseStatusCode();
		if (err >= 0)
			{
			Serial.print("Got Status Code: ");
			Serial.println(err);
			// This should be 200 OK.
			}
		else
			Serial.println("Vending Machine Test FAILED to get response headers.");
		}
	else
		Serial.println("Vending Machine Test Connection FAILED.");
	
	wg.begin();
	// wiegand/rfid reader pins
	pinMode(7, OUTPUT);
	pinMode(8, OUTPUT);
	pinMode(9, OUTPUT);
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
	EthernetClient c;
	HttpClient http(c);
	
	digitalWrite(7, HIGH);
	digitalWrite(8, LOW);
	digitalWrite(9, HIGH);
	
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
		err = http.get(kHostname, host_path);
		if (err == 0)
			{
			Serial.println("RFID Badge Connection OK");
			err = http.responseStatusCode();
			if (err >= 0)
				{
				Serial.print("Badge receievd status code: ");
				Serial.println(err);
				if (err == 200)
					do_vend();
				else
					Serial.println("Didn't receive the OK to vend...");
				}
			else
				Serial.println("Err connecting to door controller.");
			}
		else
			Serial.println("Badge Connection FAILED.");
		}
	
	vend_check();
	}

/* vim:set filetype=c: */
