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

#define RANDOM_REG32  ESP8266_DREG(0x20E44)

static char *location    = "annex";
static char *device      = "annex";
static char key[]        = {'F', 'u', 'c', 'k', 'F', 'u', 'c', 'k', 'F', 'u', 'c', 'k', '!', '!', '!', '!'};
static const char host[] = "http://intweb.at.hive13.org/api/access";

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
	
	out = get_request(badge_num, "access", location, device, key, sizeof(key), rand, sizeof(rand));

	Serial.println(out);

	http.begin(host);
	http.addHeader("Content-Type", "application/json");

	code = http.POST((unsigned char *)out, strlen(out));
	free(out);

	if (code != 200)
		{
		log_msg("Got response back: %i", code);
		return /*(RESPONSE_BAD_HTTP | ((code << 8)) & 0x7F00)*/;
		}

	body = http.getString();
	i = parse_response((char *)body.c_str(), &result, key, sizeof(key), rand, sizeof(rand));

	if (i == RESPONSE_GOOD)
		{
		Serial.println("Good response!");
		json = cJSON_GetObjectItem(result, "access");
		if (json && json->type == cJSON_True)
			success();
		else
			Serial.println("Access denied.");
		cJSON_Delete(result);
		}
	else
		log_msg("Error: %i", i);
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
	cJSON_AddItemToObjectCS(json, "temperature", cJSON_CreateNumber(temp));

	out = log_data(json, device, key, sizeof(key), rand, sizeof(rand));
	log_msg(out);

	http.begin(host);
	http.addHeader("Content-Type", "application/json");

	code = http.POST((unsigned char *)out, strlen(out));
	free(out);

	if (code != 200)
		{
		log_msg("Got response back: %i", code);
		return;
		}

	body = http.getString();
	i = parse_response((char *)body.c_str(), &result, key, sizeof(key), rand, sizeof(rand));

	if (i == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "error");
		out = json ? json->valuestring : NULL;
		log_msg("Temperature recorded: %s", out);
		cJSON_Delete(result);
		}
	else
		log_msg("Error: %i", i);
	}

