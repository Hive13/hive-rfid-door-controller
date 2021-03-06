#include "config.h"

#include <Arduino.h>

#include "http.h"
#include "cJSON.h"
#include "API.h"
#include "log.h"
#ifndef NO_SCANNER
#include "ui.h"
#endif

extern char nonce[33];
static char *location = LOCATION;

unsigned char can_vend(unsigned long badge)
	{
	unsigned char *out;
	struct cJSON *result, *json, *data = cJSON_CreateObject();
	unsigned char i;
	char rand[RAND_SIZE];

	cJSON_AddItemToObjectCS(data, "badge",           cJSON_CreateNumber(badge));
	cJSON_AddItemToObjectCS(data, "nonce",           cJSON_CreateString(nonce));
	cJSON_AddItemToObjectCS(data, "operation",       cJSON_CreateString("vend"));
	add_random_response(data, rand);
	cJSON_AddItemToObjectCS(data, "version",         cJSON_CreateNumber(2));
	i = http_request(data, &result, rand);

	if (i == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "vend");
		if (json && json->type == cJSON_True)
			i = RESPONSE_GOOD;
		else
			i = RESPONSE_ACCESS_DENIED;
		cJSON_Delete(result);
		}
	return i;
	}

unsigned char check_badge(unsigned long badge_num)
	{
	struct cJSON *json, *result, *data = cJSON_CreateObject();
	unsigned char i;
	char rand[RAND_SIZE];

	cJSON_AddItemToObjectCS(data, "badge",           cJSON_CreateNumber(badge_num));
	cJSON_AddItemToObjectCS(data, "item",            cJSON_CreateString(location));
	cJSON_AddItemToObjectCS(data, "nonce",           cJSON_CreateString(nonce));
	cJSON_AddItemToObjectCS(data, "operation",       cJSON_CreateString("access"));
	add_random_response(data, rand);
	cJSON_AddItemToObjectCS(data, "version",         cJSON_CreateNumber(2));
	i = http_request(data, &result, rand);

	if (i == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "access");
		if (json && json->type == cJSON_True)
			i = 1;
		else
			{
			log_msg("Access denied.");
#ifndef NO_SCANNER
			beep_it(BEEP_PATTERN_INVALID_CARD);
#endif
			i = 0;
			}
		cJSON_Delete(result);
		return i;
		}
	return 0;
	}

void log_temp(unsigned long temp, char *name)
	{
	struct cJSON *json, *result, *data = cJSON_CreateObject();
	unsigned char i;
	unsigned char *out;
	char rand[RAND_SIZE];

	json = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(json, "item", cJSON_CreateString(name));
	cJSON_AddItemToObjectCS(json, "temperature", cJSON_CreateNumber(temp));

	cJSON_AddItemToObjectCS(data, "log_data",        json);
	cJSON_AddItemToObjectCS(data, "nonce",           cJSON_CreateString(nonce));
	cJSON_AddItemToObjectCS(data, "operation",       cJSON_CreateString("log"));
	add_random_response(data, rand);
	cJSON_AddItemToObjectCS(data, "version",         cJSON_CreateNumber(2));
	i = http_request(data, &result, rand);

	if (i == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "error");
		out = json ? (unsigned char *)json->valuestring : NULL;
		log_msg("Temperature recorded: %s", out);
		cJSON_Delete(result);
		}
	}

void update_soda_status(unsigned char sold_out_mask)
	{
	struct cJSON *json, *item, *prev, *result, *data = cJSON_CreateObject();
	unsigned char i;
	unsigned char *out;
	char rand[RAND_SIZE];

	json = cJSON_CreateArray();
	for (i = 0; i < 8; i++)
		{
		item = cJSON_CreateObject();
		cJSON_AddItemToObjectCS(item, "slot", cJSON_CreateNumber(i + 1));
		cJSON_AddItemToObjectCS(item, "sold_out", cJSON_CreateBool(sold_out_mask & (1 << i)));
		if (!i)
			json->child = item;
		else
			{
			prev->next = item;
			item->prev = prev;
			}
		prev = item;
		}

	cJSON_AddItemToObjectCS(data, "nonce",           cJSON_CreateString(nonce));
	cJSON_AddItemToObjectCS(data, "operation",       cJSON_CreateString("soda_status"));
	add_random_response(data, rand);
	cJSON_AddItemToObjectCS(data, "soda_status",     json);
	cJSON_AddItemToObjectCS(data, "version",         cJSON_CreateNumber(2));

	i = http_request(data, &result, rand);

	if (i == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "error");
		out = json ? (unsigned char *)json->valuestring : NULL;
		log_msg("Sold out updated: %s", out);
		cJSON_Delete(result);
		}
	}

void update_nonce(void)
	{
	struct cJSON *result, *data = cJSON_CreateObject();
	unsigned char i;
	char rand[RAND_SIZE];

	cJSON_AddItemToObjectCS(data, "operation",       cJSON_CreateString("get_nonce"));
	add_random_response(data, rand);
	cJSON_AddItemToObjectCS(data, "version",         cJSON_CreateNumber(2));
	i = http_request(data, &result, rand);

	if (i == RESPONSE_GOOD)
		cJSON_Delete(result);
	}

unsigned char quiet_hours(unsigned int *start_ms, unsigned int *end_ms, unsigned char *in_quiet)
	{
	struct cJSON *result, *data = cJSON_CreateObject(), *item;
	unsigned char i;
	char rand[RAND_SIZE];

	cJSON_AddItemToObjectCS(data, "nonce",     cJSON_CreateString(nonce));
	cJSON_AddItemToObjectCS(data, "operation", cJSON_CreateString("quiet_hours"));
	add_random_response(data, rand);
	cJSON_AddItemToObjectCS(data, "version",   cJSON_CreateNumber(2));
	i = http_request(data, &result, rand);

	if (i == RESPONSE_GOOD)
		{
		if (start_ms && (item = cJSON_GetObjectItem(result, "start_ms")))
			*start_ms = item->valueint;
		if (end_ms && (item = cJSON_GetObjectItem(result, "end_ms")))
			*end_ms = item->valueint;
		if (in_quiet && (item = cJSON_GetObjectItem(result, "in_quiet")))
			*in_quiet = cJSON_IsTrue(item);
		cJSON_Delete(result);
		}

	return i;
	}
