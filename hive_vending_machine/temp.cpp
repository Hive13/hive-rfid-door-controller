#include <Arduino.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <OneWire.h>

#include "temp.h"

#define COMPRESSOR_RELAY (-1)
#define COMPRESSOR_ON 38.0
#define COMPRESSOR_OFF 34.0
#define COMPRESSOR_ON_DELAY_MILLIS 30000

#define TEMPERATURE_PIN 19
#define TEMPERATURE_POWER_PIN 18
#define TEMPERATURE_UPDATE_INTERVAL 300000
#define TEMPERATURE_READ_TIME 1000

static OneWire ds(TEMPERATURE_PIN);
static byte addr[8];
static const char temp_host[] = "portal.hive13.org";

char start_read_temperature(void)
	{
	if (!ds.search(addr))
		{
		//no more sensors on chain, reset search
		ds.reset_search();
		return -1;
		}
	
	if (OneWire::crc8(addr, 7) != addr[7])
		{
		Serial.println("CRC is not valid!");
		return -1;
		}
	
	if (addr[0] != 0x10 && addr[0] != 0x28)
		{
		Serial.print("Device is not recognized");
		return -1;
		}
	
	ds.reset();
	ds.select(addr);
	ds.write(0x44);
	return 0;
	}

float get_temperature(void)
	{
	//returns the temperature from one DS18S20 in Fahrenheit
	byte data[12], present;
	float tempRead;
	int i;
	
	present = ds.reset();
	ds.select(addr);  
	ds.write(0xBE); // Read Scratchpad
	
	for (i = 0; i < 9; i++) // we need 9 bytes
		data[i] = ds.read();
	
	ds.reset_search();
	
	tempRead = ((data[1] << 8) | data[0]); //using two's compliment
	tempRead /= 16;
	return (tempRead * 1.8 + 32); /* De-suckigrade the temp */
	}

void handle_temperature()
	{
	float temp = get_temperature();
	static unsigned long start_at = 0, update_temperature_at = 0;
	char webstr[255], float_str[16];
	unsigned long m = millis();
	int err;
	EthernetClient c;
	HttpClient http(c);

	dtostrf(temp, 4, 2, float_str);

	snprintf(webstr, sizeof(webstr), "T=%s\n", float_str);
	Serial.print(webstr);
	
	if (temp <= COMPRESSOR_OFF)
		{
		digitalWrite(COMPRESSOR_RELAY, LOW);
		start_at = m + COMPRESSOR_ON_DELAY_MILLIS;
		}
	else if (temp >= COMPRESSOR_ON && start_at <= m)
		digitalWrite(COMPRESSOR_RELAY, HIGH);

	if (update_temperature_at <= m)
		{
		Ethernet.maintain();
		Serial.print("Logging temperature: ");
		update_temperature_at = m + TEMPERATURE_UPDATE_INTERVAL;
		snprintf(webstr, sizeof(webstr), "/isOpen/logger.php?sodatemp=%s", float_str);
		err = http.get(temp_host, webstr);
		Serial.println(err);
		}
	}

