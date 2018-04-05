#include "config.h"

#include <Arduino.h>
#include <SHA512.h>

#ifdef PLATFORM_ARDUINO
#include <Ethernet.h> 
#include <b64.h>
#include <HttpClient.h>
#else
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#endif

#include "API.h"
#include "http.h"
#include "cJSON.h"
#include "log.h"

#ifdef SODA_MACHINE
#include "leds.h"
#define beep_it(x)
#else
#include "ui.h"
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

static char *hex    = "0123456789ABCDEF";
static char *device = DEVICE;
static char key[]   = KEY;
char nonce[33];

unsigned char val(char *i)
	{
	unsigned char ret = 0;
	char c;

	c = (*i++) | 0x20;
	if (c >= '0' && c <= '9')
		ret = ((c - '0') & 0x0F) << 4;
	else if (c >= 'a' && c <= 'f')
		ret = ((c - 'a' + 10) & 0x0F) << 4;

	c = (*i) | 0x20;
	if (c >= '0' && c <= '9')
		ret |= ((c - '0') & 0x0F);
	else if (c >= 'a' && c <= 'f')
		ret |= ((c - 'a' + 10) & 0x0F);

	return ret;
	}

void print_hex(char *str, char *src, unsigned char sz)
	{
	unsigned char i;

	for (i = 0; i < sz; i++)
		{
		*(str++) = hex[((src[i] & 0xF0) >> 4)];
		*(str++) = hex[(src[i] & 0x0F)];
		}
	*str = 0;
	}

unsigned char parse_response(char *in, struct cJSON **out, char *key, unsigned char key_len, char *rv, unsigned char rv_len)
	{
	unsigned char i, j;
	struct cJSON *result, *data, *response, *cs;
	char *cksum, provided_cksum[SHA512_SZ];
	char code;

	result = cJSON_Parse(in);

	if (!result)
		{
		log_msg("Cannot parse result");
		return RESPONSE_BAD_JSON;
		}
	
	cs = cJSON_GetObjectItem(result, "checksum");
	if (!cs)
		{
		cJSON_Delete(result);
		log_msg("Missing checksum");
		return RESPONSE_BAD_JSON;
		}

	cksum = cs->valuestring;
	data  = cJSON_DetachItemFromObject(result, "data");

	if (!data)
		{
		cJSON_Delete(result);
		log_msg("Missing data");
		return RESPONSE_BAD_JSON;
		}

	get_hash(data, provided_cksum, key, key_len);

	for (i = 0; i < SHA512_SZ; i++)
		{
		j = val(cksum + (2 * i));
		code = (provided_cksum[i] - j);
		if (code)
			break;
		}

	cJSON_Delete(result);

	if (code)
		{
		cJSON_Delete(data);
		log_msg("Invalid checksum");
		return RESPONSE_BAD_CKSUM;
		}
	
	response = cJSON_DetachItemFromObject(data, "nonce_valid");
	if (!response)
		{
		cJSON_Delete(data);
		log_msg("Invalid nonce.");
		update_nonce();
		return RESPONSE_BAD_NONCE;
		}
	if (response->type != cJSON_True)
		{
		cJSON_Delete(data);
		cJSON_Delete(response);
		log_msg("Invalid nonce.");
		update_nonce();
		return RESPONSE_BAD_NONCE;
		}
	cJSON_Delete(response);
	
	response = cJSON_DetachItemFromObject(data, "response");
	if (!response)
		{
		cJSON_Delete(data);
		log_msg("Missing response");
		return RESPONSE_BAD_JSON;
		}

	if (response->type != cJSON_True)
		{
		cJSON_Delete(data);
		cJSON_Delete(response);
		log_msg("Invalid response");
		return RESPONSE_BAD_JSON;
		}
	
	cJSON_Delete(response);

	response = cJSON_DetachItemFromObject(data, "random_response");
	if (!response)
		{
		cJSON_Delete(data);
		log_msg("No random_response");
		return RESPONSE_BAD_CKSUM;
		}
	
	cs = response->child;
	
	for (i = 0; i < rv_len && cs; i++)
		{
		if (cs->type != cJSON_Number)
			break;
		if (cs->valueint != rv[i])
			break;

		cs = cs->next;
		}
	
	cJSON_Delete(response);
	
	if (i < rv_len)
		{
		cJSON_Delete(data);
		log_msg("Invalid random_response");
		return RESPONSE_BAD_CKSUM;
		}

	if (out)
		*out = data;
	else
		cJSON_Delete(data);
		
	response = cJSON_GetObjectItem(data, "new_nonce");
	memmove(nonce, response->valuestring, 32);
	nonce[32] = 0;
	log_msg("Good response!");

	return RESPONSE_GOOD;
	}

void get_hash(struct cJSON *data, char *sha_buf, char *key, unsigned char key_len)
	{
	unsigned char i;
	unsigned long r;
	SHA512 sha;
	char *out;
	
	out = cJSON_Print(data);
	cJSON_Minify(out);

	sha.reset();
	sha.update(key, key_len);
	sha.update(out, strlen(out));
	sha.finalize(sha_buf, sha.hashSize());
	
	free(out);
	}

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
	
	if (i != RESPONSE_GOOD)
		beep_it(&packet_error);
		
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
	
	if (i != RESPONSE_GOOD)
		beep_it(&packet_error);
		
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


