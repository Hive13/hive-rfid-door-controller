#include "config.h"

#include <Arduino.h>

#include "http.h"
#include "lights.h"
#include "cJSON.h"
#include "API.h"
#include "network.h"
#include "schedule.h"
#include "eeprom_lib.h"

extern char nonce[33];

#define BULB_COUNT (sizeof(config->bulbs) / sizeof(config->bulbs[0]))

static struct cJSON *get_light_state(void)
	{
	struct cJSON *result, *data = cJSON_CreateObject();
	char rand[RAND_SIZE];
	unsigned char i;

	cJSON_AddItemToObjectCS(data, "nonce",           cJSON_CreateString(nonce));
	cJSON_AddItemToObjectCS(data, "operation",       cJSON_CreateString("get_light_state"));
	add_random_response(data, rand);
	cJSON_AddItemToObjectCS(data, "version",         cJSON_CreateNumber(2));
	i = http_request(data, &result, rand);

	data = NULL;

	if (i == RESPONSE_GOOD)
		{
		data = cJSON_DetachItemFromObject(result, "states");
		cJSON_Delete(result);
		}
	return data;
	}

static unsigned char get_state(void *data, unsigned long *time, unsigned long now)
	{
	struct cJSON *item, *result = get_light_state();
	unsigned char i;

	if (!result)
		return SCHEDULE_DONE;
	item = result->child;
	for (i = 0; i < BULB_COUNT && item; i++)
		{
		config->bulbs[i] = (item->type == cJSON_True || (item->type== cJSON_Number && item->valueint));
		item = item->next;
		}

	digitalWrite(D1, config->bulbs[0]);
	digitalWrite(D5, config->bulbs[1]);
	digitalWrite(D6, config->bulbs[2]);
	digitalWrite(D7, config->bulbs[3]);

	eeprom_save();

	cJSON_Delete(result);
	return SCHEDULE_DONE;
	}

void lights_init(void)
	{
	pinMode(D1, OUTPUT);
	pinMode(D5, OUTPUT);
	pinMode(D6, OUTPUT);
	pinMode(D7, OUTPUT);
	digitalWrite(D1, config->bulbs[0]);
	digitalWrite(D5, config->bulbs[1]);
	digitalWrite(D6, config->bulbs[2]);
	digitalWrite(D7, config->bulbs[3]);

	register_mc("lights!!!", (void *)get_state);
	}
