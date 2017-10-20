#include <Arduino.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <OneWire.h>

#include "log.h"
#include "API.h"
#include "soda_temp.h"
#include "temp.h"
#include "leds.h"
#include "schedule.h"
#include "http.h"
#include "cJSON.h"
#include "schedule.h"

extern unsigned char key[16];
extern char *device;

static uint32_t cur_temp = 0;

char soda_temperature_sensor(struct temp_sensor *me, unsigned long temp)
	{
	//log_msg("Temperature: %lu.%lu", temp / 10, temp % 10);
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

struct temp_sensor sensors[] = 
	{
		{
		.addr = {0x28, 0xB7, 0xC2, 0x28, 0x07, 0x00, 0x00, 0x2B},
		.func = soda_temperature_sensor,
		.desc = "Main Sensor",
		},
	};

void soda_temp_init(void)
	{
	pinMode(TEMPERATURE_POWER_PIN, OUTPUT);
	pinMode(COMPRESSOR_RELAY, OUTPUT);
	digitalWrite(COMPRESSOR_RELAY, LOW);
	digitalWrite(TEMPERATURE_POWER_PIN, HIGH);

	temperature_init(TEMPERATURE_PIN, SODA_TEMP_UPDATE_INTERVAL, sensors, (sizeof(sensors) / sizeof(sensors[0])));
	schedule(millis() + 2 * (SODA_TEMP_UPDATE_INTERVAL + TEMPERATURE_READ_TIME), log_temp, &cur_temp);
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

char log_temp(unsigned long *temp, unsigned long *t, unsigned long m)
	{
	static unsigned long log_count = 0;
	char *body, *out;
	struct cJSON *json, *resp, *cs;
	unsigned char rv[2 * sizeof(unsigned long)];
	unsigned long rc;

	*t = m + SODA_TEMP_REPORT_INTERVAL;

	memcpy(rv, &m, sizeof(unsigned long));
	memcpy(rv + sizeof(unsigned long), &log_count, sizeof(unsigned long));
	log_count++;

	json = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(json, "item",        cJSON_CreateString("soda_machine"));
	cJSON_AddItemToObjectCS(json, "temperature", cJSON_CreateNumber(*temp));

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
		return SCHEDULE_REDO;

	/*if (!(cs = cJSON_GetObjectItem(resp, "vend")) || cs->type != cJSON_True)
		{
		log_msg("Didn't get a vend response back.");
		cJSON_Delete(resp);
		return RESPONSE_ACCESS_DENIED;
		}*/

	cJSON_Delete(resp);
	return SCHEDULE_REDO;
	}
