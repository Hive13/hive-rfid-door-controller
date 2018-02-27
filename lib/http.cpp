#include <Wiegand.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#include "http.h"
#include "cJSON.h"
#include "API.h"
#include "temp.h"
#include "schedule.h"
#include "log.h"
#include "ui.h"
#include "wifi.h"

#define RANDOM_REG32  ESP8266_DREG(0x20E44)

struct beep_pattern invalid_card =
	{
	.beep_ms     = 1000,
	.silence_ms  = 0,
	.cycle_count = 1,
	.options     = RED_ALWAYS,
	};
struct beep_pattern network_error =
	{
	.beep_ms     = 250,
	.silence_ms  = 250,
	.cycle_count = 4,
	.options     = RED_ALWAYS,
	};
struct beep_pattern packet_error =
	{
	.beep_ms     = 250,
	.silence_ms  = 250,
	.cycle_count = 8,
	.options     = RED_ALWAYS,
	};

static char *location    = LOCATION;
static char *device      = DEVICE;
static char key[]        = KEY;
static const char host[] = HTTP_HOST;
static char nonce[33];

unsigned char http_request(unsigned char *request, struct cJSON **result, char *rand, unsigned char rand_len)
	{
	int code;
	HTTPClient http;
	String body;
	unsigned char i;
	struct cJSON *new_nonce;
	
	log_msg("Request: %s", request);

	http.begin(host);
	http.addHeader("Content-Type", "application/json");

	code = http.POST(request, strlen((char *)request));
	free(request);

	if (code != 200)
		{
		log_msg("Got response back: %i", code);
		beep_it(&network_error);
		wifi_error();
		return RESPONSE_BAD_HTTP;
		}

	body = http.getString();
	log_msg("Response: %s", body.c_str());
	i = parse_response((char *)body.c_str(), result, key, sizeof(key), rand, rand_len);
	if (i == RESPONSE_GOOD)
		{
		new_nonce = cJSON_GetObjectItem(*result, "new_nonce");
		memmove(nonce, new_nonce->valuestring, 32);
		nonce[32] = 0;
		log_msg("Good response!");
		}
	else if (i == RESPONSE_BAD_NONCE)
		{
		log_msg("Invalid nonce.");
		update_nonce();
		}
	else
		{
		log_msg("Error: %i", i);
		beep_it(&packet_error);
		}
	return i;
	}

unsigned char check_badge(unsigned long badge_num, void (*success)(void))
	{
	struct cJSON *json, *result;
	unsigned char i;
	unsigned long r;
	unsigned char *out;
	char rand[16];

	for (i = 0; i < sizeof(rand); i++)
		{
		if (!(i % 4))
			r = RANDOM_REG32;
		rand[i] = ((r >> (3 - i)) & 0xFF);
		}
	
	out = (unsigned char *)get_request(badge_num, "access", location, device, key, sizeof(key), rand, sizeof(rand), nonce);
	
	i = http_request(out, &result, rand, sizeof(rand));

	if (i == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "access");
		if (json && json->type == cJSON_True)
			{
			if (success)
				success();
			i = 1;
			}
		else
			{
			log_msg("Access denied.");
			beep_it(&invalid_card);
			i = 0;
			}
		cJSON_Delete(result);
		return i;
		}
	return 0;
	}

void log_temp(unsigned long temp)
	{
	struct cJSON *json, *result;
	unsigned char i;
	unsigned long r;
	unsigned char *out;
	char rand[16];

	for (i = 0; i < sizeof(rand); i++)
		{
		if (!(i % 4))
			r = RANDOM_REG32;
		rand[i] = ((r >> (3 - i)) & 0xFF);
		}
	
	json = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(json, "item", cJSON_CreateString("annex"));
	cJSON_AddItemToObjectCS(json, "temperature", cJSON_CreateNumber(temp));

	out = (unsigned char *)log_data(json, device, key, sizeof(key), rand, sizeof(rand), nonce);
	i = http_request(out, &result, rand, sizeof(rand));

	if (i == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "error");
		out = json ? (unsigned char *)json->valuestring : NULL;
		log_msg("Temperature recorded: %s", out);
		cJSON_Delete(result);
		}
	}

void update_nonce(void)
	{
	struct cJSON *result;
	unsigned char i;
	unsigned long r;
	unsigned char *out;
	char rand[16];
	
	for (i = 0; i < sizeof(rand); i++)
		{
		if (!(i % 4))
			r = RANDOM_REG32;
		rand[i] = ((r >> (3 - i)) & 0xFF);
		}
	
	out = (unsigned char *)get_nonce(device, key, sizeof(key), rand, sizeof(rand));
	i = http_request(out, &result, rand, sizeof(rand));

	if (i == RESPONSE_GOOD)
		cJSON_Delete(result);
	}

