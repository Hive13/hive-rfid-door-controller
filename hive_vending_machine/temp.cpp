#include <Arduino.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <OneWire.h>

#include "log.h"
#include "API.h"
#include "temp.h"
#include "leds.h"
#include "schedule.h"
#include "http.h"
#include "cJSON.h"

extern unsigned char key[16];
extern char *device;

unsigned char main_temperature_addr[] = {0x28, 0xB7, 0xC2, 0x28, 0x07, 0x00, 0x00, 0x2B};
struct temp_sensor sensors[MAX_NUM_SENSORS];
unsigned char sensor_count;

static OneWire ds(TEMPERATURE_PIN);
static byte addr[8];
static char temp_host[] = "portal.hive13.org";
static uint32_t cur_temp = 0;

void log_temp(unsigned long temp);

char main_temperature_sensor(struct temp_sensor *me, unsigned long temp)
	{
	char buf[256];

	log_msg("Temperature: %hu.%hu", temp / 10, temp % 10);
	log_temp(temp);
	cur_temp = temp;

	/*if (temp <= COMPRESSOR_OFF)
		{
		digitalWrite(COMPRESSOR_RELAY, LOW);
		start_at = m + COMPRESSOR_ON_DELAY_MILLIS;
		}
	else if (temp >= COMPRESSOR_ON && start_at <= m)
		digitalWrite(COMPRESSOR_RELAY, HIGH);*/

	return 0;
	}

void temperature_init(void)
	{
	static unsigned long allocate_a_long;

	log_msg("Initializing temperature controller.");
	memset(sensors, 0, MAX_NUM_SENSORS * sizeof(struct temp_sensor));

	memmove(sensors[0].addr, main_temperature_addr, ONEWIRE_ADDR_SZ);
	sensors[0].desc = "Main Sensor";
	sensors[0].func = main_temperature_sensor;
	sensors[0].data = &allocate_a_long;

	sensor_count = 1;

	pinMode(TEMPERATURE_POWER_PIN, OUTPUT);
	pinMode(COMPRESSOR_RELAY, OUTPUT);
	digitalWrite(COMPRESSOR_RELAY, LOW);
	digitalWrite(TEMPERATURE_POWER_PIN, HIGH);
	schedule(0, handle_temperature, sensors);
	}

char start_read_temperature(void)
	{
	unsigned char i;
	unsigned char look_addr[8];
	
	while (ds.search(look_addr))
		{
		/*log_msg("Found %02X %02X %02X %02X %02X %02X %02X %02X",
			look_addr[0], look_addr[1], look_addr[2], look_addr[3], look_addr[4], look_addr[5], look_addr[6], look_addr[7]);*/
		if (OneWire::crc8(look_addr, 7) != look_addr[7])
			{
			log_msg("CRC is not valid!");
			continue;
			}
		if (look_addr[0] != 0x10 && look_addr[0] != 0x28)
			{
			log_msg("Device not a DS.");
			continue;
			}
		ds.reset();
		ds.select(look_addr);
		ds.write(0x44);
		}
	ds.reset_search();
	return 0;
	}

void temperature_check(void)
	{
	unsigned char light, p;
	uint32_t color;

	if (cur_temp < 320)
		{
		light = 0;
		color = Color(0, 255, 0);
		}
	else if (cur_temp >= 480)
		{
		light = 7;
		color = Color(0, 255, 0);
		}
	else
		{
		light = (unsigned char)((cur_temp - 320) / 20);
		p = (cur_temp % 20) * 12;
		color = Color(0 + p, 0, 255 - p);
		}
	leds_one(light, color, 0);
	}

uint32_t get_temperature(unsigned char *addr)
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

char handle_temperature(void *ptr, unsigned long *t, unsigned long m)
	{
	static unsigned char flag = 0;
	unsigned char i;
	unsigned long temp;

	if (!flag)
		{
		if (start_read_temperature())
			*t = 0;
		else
			{
			*t = m + TEMPERATURE_READ_TIME;
			flag = 1;
			}
		return SCHEDULE_REDO;
		}
	
	flag = 0;
	for (i = 0; i < sensor_count; i++)
		{
		temp = get_temperature(sensors[i].addr);
		if (sensors[i].func)
			sensors[i].func(sensors + i, temp);
		}
	*t = m + TEMPERATURE_UPDATE_INTERVAL - TEMPERATURE_READ_TIME;
	return SCHEDULE_REDO;
	}

void log_temp(unsigned long temp)
	{
	static unsigned long log_count = 0;
	char *body, *out;
	struct cJSON *json, *resp, *cs;
	unsigned char rv[2 * sizeof(unsigned long)];
	unsigned long rc;
	unsigned long start = millis();

	memcpy(rv, &start, sizeof(unsigned long));
	memcpy(rv + sizeof(unsigned long), &log_count, sizeof(unsigned long));
	log_count++;

	json = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(json, "temperature", cJSON_CreateNumber(temp));

	out = log_data(json, device, key, sizeof(key), rv, sizeof(rv));
	log_msg("Sending %s", out);

	rc = http_get_json("intweb.at.hive13.org", "/api/access", out, &body);
	free(out);

	if (rc != RESPONSE_GOOD)
		{
		log_msg("GET failed: %hhd", rc);
		return rc;
		}

	log_msg("Body: %s", body);

	rc = parse_response(body, &resp, key, sizeof(key), rv, sizeof(rv));
	free(body);

	log_msg("rc: %hhd", rc);
	if (rc != RESPONSE_GOOD)
		return rc;

	/*if (!(cs = cJSON_GetObjectItem(resp, "vend")) || cs->type != cJSON_True)
		{
		log_msg("Didn't get a vend response back.");
		cJSON_Delete(resp);
		return RESPONSE_ACCESS_DENIED;
		}*/

	cJSON_Delete(resp);
	return RESPONSE_GOOD;
	}
