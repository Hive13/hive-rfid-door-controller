#include "config.h"

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
#include "network.h"
#ifdef SODA_MACHINE
#include "leds.h"
#else
#include "ui.h"
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

char *get_signed_packet(struct cJSON *data)
	{	
	struct cJSON *root = cJSON_CreateObject();
	char sha_buf[SHA512_SZ], sha_buf_out[2 * SHA512_SZ + 1];
	char *out;
	
	get_hash(data, sha_buf, key, sizeof(key));

	print_hex(sha_buf_out, sha_buf, SHA512_SZ);
	
	cJSON_AddItemToObjectCS(root, "data", data);
	cJSON_AddItemToObjectCS(root, "device",  cJSON_CreateString(device));
	cJSON_AddItemToObjectCS(root, "checksum", cJSON_CreateString(sha_buf_out));
	
	out = cJSON_Print(root);
	cJSON_Delete(root);

	return out;
	}

#ifdef PLATFORM_ARDUINO
unsigned char http_request(struct cJSON *data, struct cJSON **result, char *rand)
	{
	char *body, *request;
	EthernetClient ec;
	HttpClient hc(ec);
	int err, body_len;
	struct cJSON *new_nonce;
	unsigned long start, l;
	unsigned char i;
	
#ifdef SODA_MACHINE
	leds_busy();
#endif

	request = get_signed_packet(data);
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
	body = malloc(body_len + 1);

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

	i = parse_response(body, result, key, sizeof(key), rand, RAND_SIZE);
	free(body);
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

unsigned char http_request(struct cJSON *data, struct cJSON **result, char *rand)
	{
	int code;
	HTTPClient http;
	String body;
	unsigned char i;
	struct cJSON *new_nonce;
	char *request;
	
	request = get_signed_packet(data);
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
	i = parse_response((char *)body.c_str(), result, key, sizeof(key), rand, RAND_SIZE);
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

#define RANDOM_REG32  ESP8266_DREG(0x20E44)
void get_rand(char *rand)
	{
	unsigned char i;
	unsigned long r;
	
	for (i = 0; i < RAND_SIZE; i++)
		{
		if (!(i % 4))
			r = RANDOM_REG32;
		rand[i] = ((r >> (3 - i)) & 0xFF);
		}
	}
#endif

void add_random_response(struct cJSON *data, char *rand)
	{
	struct cJSON *json, *prev, *ran = cJSON_CreateArray();
	unsigned char i;
	
	get_rand(rand);
	for (i = 0; i < RAND_SIZE; i++)
		{
		json = cJSON_CreateNumber((long)rand[i]);
		if (!i)
			ran->child = json;
		else
			{
			prev->next = json;
			json->prev = prev;
			}
		prev = json;
		}
	cJSON_AddItemToObjectCS(data, "random_response", ran);
	}

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
			return RESPONSE_GOOD;
		else
			return RESPONSE_ACCESS_DENIED;
		cJSON_Delete(result);
		}
	return i;
	}

unsigned char check_badge(unsigned long badge_num, void (*success)(void))
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
	cJSON_AddItemToObjectCS(data, "soda_status",     json);
	add_random_response(data, rand);
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
