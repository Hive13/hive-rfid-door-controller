#include "config.h"

#include <Arduino.h>

#include "http.h"
#include "lights.h"
#include "cJSON.h"
#include "API.h"

extern char nonce[33];
static char *location = LOCATION;

struct cJSON *get_light_state(void)
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
