#include <Wiegand.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#include "cJSON.h"
#include "API.h"
#include "temp.h"
#include "schedule.h"
#include "log.h"
#include "http.h"
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

static char *location    = "annex";
static char *device      = "annex";
static char key[]        = {'F', 'u', 'c', 'k', 'F', 'u', 'c', 'k', 'F', 'u', 'c', 'k', '!', '!', '!', '!'};
static const char host[] = "http://intweb.at.hive13.org/api/access";
static char nonce[33];

void check_badge(unsigned long badge_num, void (*success)(void))
	{
	struct cJSON *json, *result;
	unsigned char i;
	unsigned long r;
	char *out;
	char rand[16];
	HTTPClient http;
	int code;
	String body;

	for (i = 0; i < sizeof(rand); i++)
		{
		if (!(i % 4))
			r = RANDOM_REG32;
		rand[i] = ((r >> (3 - i)) & 0xFF);
		}
	
	out = get_request(badge_num, "access", location, device, key, sizeof(key), rand, sizeof(rand), nonce);

	log_msg("Request: %s", out);

	http.begin(host);
	http.addHeader("Content-Type", "application/json");

	code = http.POST((unsigned char *)out, strlen(out));
	free(out);

	if (code != 200)
		{
		log_msg("Got response back: %i", code);
		beep_it(&network_error);
		wifi_error();
		return /*(RESPONSE_BAD_HTTP | ((code << 8)) & 0x7F00)*/;
		}

	body = http.getString();
	log_msg("Response: %s", body.c_str());
	i = parse_response((char *)body.c_str(), &result, key, sizeof(key), rand, sizeof(rand));

	if (i == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "new_nonce");
		memmove(nonce, json->valuestring, 32);
		nonce[32] = 0;
		log_msg("Good response!");
		json = cJSON_GetObjectItem(result, "access");
		if (json && json->type == cJSON_True)
			success();
		else
			{
			log_msg("Access denied.");
			beep_it(&invalid_card);
			}
		cJSON_Delete(result);
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
	}

void log_temp(unsigned long temp)
	{
	struct cJSON *json, *result;
	unsigned char i;
	unsigned long r;
	char *out;
	char rand[16];
	HTTPClient http;
	int code;
	String body;

	for (i = 0; i < sizeof(rand); i++)
		{
		if (!(i % 4))
			r = RANDOM_REG32;
		rand[i] = ((r >> (3 - i)) & 0xFF);
		}
	
	json = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(json, "item", cJSON_CreateString("annex"));
	cJSON_AddItemToObjectCS(json, "temperature", cJSON_CreateNumber(temp));

	out = log_data(json, device, key, sizeof(key), rand, sizeof(rand), nonce);
	log_msg(out);

	http.begin(host);
	http.addHeader("Content-Type", "application/json");

	code = http.POST((unsigned char *)out, strlen(out));
	free(out);

	if (code != 200)
		{
		log_msg("Got response back: %i", code);
		wifi_error();
		return;
		}

	body = http.getString();
	log_msg("Response: %s", body.c_str());
	i = parse_response((char *)body.c_str(), &result, key, sizeof(key), rand, sizeof(rand));

	if (i == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "new_nonce");
		memmove(nonce, json->valuestring, 32);
		nonce[32] = 0;
		json = cJSON_GetObjectItem(result, "error");
		out = json ? json->valuestring : NULL;
		log_msg("Temperature recorded: %s", out);
		cJSON_Delete(result);
		}
	else if (i == RESPONSE_BAD_NONCE)
		{
		log_msg("Invalid nonce.");
		update_nonce();
		}
	else
		log_msg("Error: %i", i);
	}

void update_nonce(void)
	{
	struct cJSON *json, *result;
	unsigned char i;
	unsigned long r;
	char *out;
	char rand[16];
	HTTPClient http;
	int code;
	String body;
	
	for (i = 0; i < sizeof(rand); i++)
		{
		if (!(i % 4))
			r = RANDOM_REG32;
		rand[i] = ((r >> (3 - i)) & 0xFF);
		}
	
	out = get_nonce(device, key, sizeof(key), rand, sizeof(rand));
	log_msg(out);

	http.begin(host);
	http.addHeader("Content-Type", "application/json");

	code = http.POST((unsigned char *)out, strlen(out));
	free(out);

	if (code != 200)
		{
		log_msg("Got response back: %i", code);
		wifi_error();
		return;
		}

	body = http.getString();
	log_msg("Response: %s", body.c_str());
	i = parse_response((char *)body.c_str(), &result, key, sizeof(key), rand, sizeof(rand));

	if (i == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "new_nonce");
		memmove(nonce, json->valuestring, 32);
		nonce[32] = 0;
		cJSON_Delete(result);
		}
	else
		log_msg("Error: %i", i);
	}

