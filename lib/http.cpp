#include "config.h"

#include <Wiegand.h>
#ifdef PLATFORM_ARDUINO
#include <Ethernet.h> 
#include <b64.h>
#include <HttpClient.h>
#else
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#endif

#include "http.h"
#include "cJSON.h"
#include "API.h"
#include "schedule.h"
#include "log.h"
#ifdef SODA_MACHINE
#include "leds.h"
#else
#include "ui.h"
#endif

#ifndef PLATFORM_ARDUINO
#include "wifi.h"

#define RANDOM_REG32  ESP8266_DREG(0x20E44)
#endif

#ifdef SODA_MACHINE
#define beep_it(x)
#else
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
#endif

static char *location    = LOCATION;
static char *device      = DEVICE;
static char key[]        = KEY;
static char nonce[33];

#ifdef PLATFORM_ARDUINO
#define RAND_SIZE (2 * sizeof(unsigned long))
#define BODY_SZ 1024

unsigned char http_request(unsigned char *request, struct cJSON **result, char *rand, unsigned char rand_len)
	{
	char body[BODY_SZ];
	EthernetClient ec;
	HttpClient hc(ec);
	int err, body_len;
	struct cJSON *new_nonce;
	unsigned long start, l;
	unsigned char i;
	
#ifdef SODA_MACHINE
	leds_busy();
#endif
	l = strlen(request);
	log_msg("Request: %s", request);

	hc.beginRequest();
	hc.post("intweb.at.hive13.org", "/api/access");
	hc.flush();
	hc.sendHeader("Content-Type", "application/json");
	hc.flush();
	hc.sendHeader("Content-Length", l);
	hc.flush();
	hc.write(request, l);
	hc.flush();
	free(request);

	err = hc.responseStatusCode();
	if (err != 200)
		return RESPONSE_BAD_HTTP;
	body_len = hc.contentLength();
	if (body_len + 1 > BODY_SZ)
		return RESPONSE_BAD_HTTP;

	if (hc.skipResponseHeaders() > 0)
		{
		Serial.println("Header error.");
		return RESPONSE_BAD_HTTP;
		}

	start = millis();
	l = 0;
	while ((hc.connected() || hc.available()) && ((millis() - start) < NETWORK_TIMEOUT))
		{
		if (hc.available())
			{
			body[l++] = hc.read();
			body_len--;
			start = millis();
			}
		else
			delay(NETWORK_DELAY);
		}
	body[l++] = 0;
	hc.stop();
#ifdef SODA_MACHINE
	leds_off();
#endif
	log_msg("Response: %s", body);

	i = parse_response(body, result, key, sizeof(key), rand, rand_len);
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

static unsigned long scan_count = 0;
void get_rand(char *rand)
	{
	unsigned long m = millis();
	memcpy(rand, &m, sizeof(unsigned long));
	memcpy(rand + sizeof(unsigned long), &scan_count, sizeof(unsigned long));
	scan_count++;
	}

#endif
#ifdef PLATFORM_ESP
static const char host[] = HTTP_HOST;

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

#define RAND_SIZE 16

void get_rand(char *rand)
	{
	unsigned char i;
	unsigned long r;
	
	for (i = 0; i < sizeof(rand); i++)
		{
		if (!(i % 4))
			r = RANDOM_REG32;
		rand[i] = ((r >> (3 - i)) & 0xFF);
		}
	}
#endif

unsigned char can_vend(unsigned long badge)
	{
	unsigned char *out;
	struct cJSON *result, *json;
	unsigned char i;
	char rand[RAND_SIZE];
	
	get_rand(rand);
	out = (unsigned char *)get_request(badge, "vend", NULL, device, key, sizeof(key), rand, sizeof(rand), nonce);
	i = http_request(out, &result, rand, sizeof(rand));

	if (i == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "vend");
		if (json && json->type == cJSON_True)
			return RESPONSE_GOOD;
		else
			return RESPONSE_ACCESS_DENIED;
		cJSON_Delete(result);
		}
	return i;
	}

unsigned char check_badge(unsigned long badge_num, void (*success)(void))
	{
	struct cJSON *json, *result;
	unsigned char i;
	unsigned char *out;
	char rand[RAND_SIZE];
	
	get_rand(rand);
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

void log_temp(unsigned long temp, char *name)
	{
	struct cJSON *json, *result;
	unsigned char i;
	unsigned char *out;
	char rand[RAND_SIZE];

	json = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(json, "item", cJSON_CreateString(name));
	cJSON_AddItemToObjectCS(json, "temperature", cJSON_CreateNumber(temp));
	
	get_rand(rand);
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

void update_soda_status(unsigned char sold_out_mask)
	{
	struct cJSON *json, *item, *prev, *result;
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
	
	get_rand(rand);
	out = (unsigned char *)soda_status(json, device, key, sizeof(key), rand, sizeof(rand), nonce);
	i = http_request(out, &result, rand, sizeof(rand));

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
	struct cJSON *result;
	unsigned char i;
	unsigned char *out;
	char rand[RAND_SIZE];
	
	get_rand(rand);
	out = (unsigned char *)get_nonce(device, key, sizeof(key), rand, sizeof(rand));
	i = http_request(out, &result, rand, sizeof(rand));

	if (i == RESPONSE_GOOD)
		cJSON_Delete(result);
	}

