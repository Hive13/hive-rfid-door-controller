#include <Arduino.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <OneWire.h>

#include "log.h"
#include "temp.h"
#include "leds.h"
#include "http.h"

static OneWire ds(TEMPERATURE_PIN);
static byte addr[8];
static char temp_host[] = "portal.hive13.org";
static uint32_t temp = 0;

void temperature_init(void)
	{
	pinMode(TEMPERATURE_POWER_PIN, OUTPUT);
	pinMode(COMPRESSOR_RELAY, OUTPUT);
	digitalWrite(COMPRESSOR_RELAY, LOW);
	digitalWrite(TEMPERATURE_POWER_PIN, HIGH);
	}

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
		log_msg("CRC is not valid!");
		return -1;
		}
	
	if (addr[0] != 0x10 && addr[0] != 0x28)
		{
		log_msg("Device is not recognized");
		return -1;
		}
	
	ds.reset();
	ds.select(addr);
	ds.write(0x44);
	return 0;
	}

void temperature_check(void)
	{
	unsigned char light, p;
	uint32_t color;

	if (temp < 320)
		{
		light = 0;
		color = Color(0, 255, 0);
		}
	else if (temp >= 480)
		{
		light = 7;
		color = Color(0, 255, 0);
		}
	else
		{
		light = (unsigned char)((temp - 320) / 20);
		p = (temp % 20) * 12;
		color = Color(0 + p, 0, 255 - p);
		}
	leds_one_only(light, color);
	}

uint32_t get_temperature(void)
	{
	//returns the temperature from one DS18S20 in Fahrenheit
	byte data[12], present;
	uint32_t tempRead;
	unsigned char i;
	
	present = ds.reset();
	ds.select(addr);  
	ds.write(0xBE); // Read Scratchpad
	
	for (i = 0; i < 9; i++) // we need 9 bytes
		data[i] = ds.read();
	
	ds.reset_search();
	
	tempRead = ((data[1] << 8) | data[0]); //using two's compliment
	/*
		Optimized math for fixed point.
		1. Multiply by 10 for fixed point.
		2. Divide by 16 because the result is 16 times the actual temperature.
		3. Multiply by 1.8 (9 / 5) to convert from suckigrade.
		4. Add 320 to finish de-suckigrading the temperature.
		(10 * 9) / (16 * 5) = (9 / 8)
	*/
	tempRead *= 9; /* Convert to fixed point */
	tempRead /= 8;
	return tempRead + 320;
	}

void handle_temperature()
	{
	static unsigned long start_at = 0, update_temperature_at = 0, temp_ready_time = 0;
	char webstr[255];
	unsigned long m = millis();

	if (!temp_ready_time)
		{
		if (!start_read_temperature())
			temp_ready_time = m + TEMPERATURE_READ_TIME;
		else
			temp_ready_time = 0;
		return;
		}
	else if (temp_ready_time <= m)
		temp_ready_time = 0;
	else
		return;
	
	temp = get_temperature();

	
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
		log_msg("Logging temperature %lu.%lu.", temp / 10, temp % 10);
		update_temperature_at = m + TEMPERATURE_UPDATE_INTERVAL;
		snprintf(webstr, sizeof(webstr), "/isOpen/logger.php?sodatemp=%lu.%lu", temp / 10, temp % 10);
		http_get("temp", temp_host, webstr);
		}
	}

