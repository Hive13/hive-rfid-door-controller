#include <Wiegand.h>
#include <Crypto.h>
#include <SHA512.h>
#include <CBC.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#include "cJSON.h"
#include "temp.h"

#define SHA512_SZ 64
#define RANDOM_REG32  ESP8266_DREG(0x20E44)

#define BEEP_PIN D0
#define D0_PIN D1
#define D1_PIN D2

#define RESPONSE_BAD_JSON  3
#define RESPONSE_BAD_HTTP  2
#define RESPONSE_BAD_CKSUM 1
#define RESPONSE_GOOD      0

static WIEGAND wg;
static char *location = "annex";
static char *ssid = "hive13int";
static char *pass = "hive13int";
static char key[] = {65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65};
static char *hex = "0123456789ABCDEF";
int status = WL_IDLE_STATUS;

void open_door(void)
	{
	}

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

unsigned short get_response(char *in, struct cJSON **out)
	{
	HTTPClient http;
	const char host[] = "http://172.16.3.78/access.pl";
	int code;
	unsigned char i, j;
	String body;
	struct cJSON *result, *data, *response;
	char *cksum, provided_cksum[SHA512_SZ];

	http.begin(host);

	code = http.POST((unsigned char *)in, strlen(in));
	if (code != 200)
		return RESPONSE_BAD_HTTP;

	body   = http.getString();
	result = cJSON_Parse(body.c_str());
	cksum  = cJSON_GetObjectItem(result, "checksum")->valuestring;
	data   = cJSON_DetachItemFromObject(result, "data");

	get_hash(data, provided_cksum);

	for (i = 0; i < SHA512_SZ; i++)
		{
		j = val(cksum + (2 * i));
		code = (provided_cksum[i] - j);
		if (code)
			break;
		}
	
	if (code)
		{
		cJSON_Delete(data);
		return RESPONSE_BAD_CKSUM;
		}
	
	response = cJSON_DetachItemFromObject(data, "response");
	if (response->type != cJSON_True)
		{
		cJSON_Delete(data);
		return RESPONSE_BAD_JSON;
		}

	if (out)
		*out = data;
	else
		cJSON_Delete(data);

	return RESPONSE_GOOD;
	}

void get_hash(struct cJSON *data, char *sha_buf)
	{
	unsigned char i;
	unsigned long r;
	SHA512 sha;
	char *out;
	
	out = cJSON_Print(data);
	cJSON_Minify(out);

	sha.reset();
	sha.update(key, sizeof(key));
	sha.update(out, strlen(out));
	sha.finalize(sha_buf, sha.hashSize());
	
	free(out);
	}

void check_badge(unsigned long badge_num)
	{
	struct cJSON
		*data = cJSON_CreateObject(),
		*root = cJSON_CreateObject(),
		*ran  = cJSON_CreateArray(),
		*json, *prev, *result;
	unsigned char i;
	unsigned long r;
	SHA512 sha;
	char sha_buf[SHA512_SZ], sha_buf_out[2 * SHA512_SZ + 1], *ptr;
	char *out;

	for (i = 0; i < 16; i++)
		{
		if (!(i % 4))
			r = RANDOM_REG32;
		json = cJSON_CreateNumber((r >> (3 - i)) & 0xFF);
		if (!i)
			ran->child = json;
		else
			{
			prev->next = json;
			json->prev = prev;
			}
		prev = json;
		}
	
	cJSON_AddItemToObjectCS(data, "badge", cJSON_CreateNumber(badge_num));
	cJSON_AddItemToObjectCS(data, "location", cJSON_CreateString(location));
	cJSON_AddItemToObjectCS(data, "random", ran);
	cJSON_AddItemToObjectCS(data, "version", cJSON_CreateNumber(1));
	get_hash(data, sha_buf);

	for (i = 0, ptr = sha_buf_out; i < sha.hashSize(); i++)
		{
		*(ptr++) = hex[((sha_buf[i] & 0xF0) >> 4)];
		*(ptr++) = hex[(sha_buf[i] & 0x0F)];
		}
	*ptr = 0;
	
	cJSON_AddItemToObject(root, "data", data);
	cJSON_AddItemToObject(root, "checksum", cJSON_CreateString(sha_buf_out));
	out = cJSON_Print(root);
	cJSON_Delete(root);
	Serial.println(out);

	if (get_response(out, &result) == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "access");
		if (json->type == cJSON_True)
			open_door();
		cJSON_Delete(result);
		}
	free(out);
	}

void setup(void)
	{
	digitalWrite(BEEP_PIN, LOW);
	pinMode(BEEP_PIN, OUTPUT);

	wg.begin(D0_PIN, D0_PIN, D1_PIN, D1_PIN);
	Serial.begin(115200);

	Serial.print("Connecting to SSID ");
	Serial.println(ssid);

	status = WiFi.begin(ssid, pass);
	while (WiFi.status() != WL_CONNECTED)
		{
		Serial.print(".");
		delay(500);
		}
	Serial.print("connected!");
	
	temperature_init();

	delay(500);
	digitalWrite(BEEP_PIN, HIGH);
	delay(100);
	digitalWrite(BEEP_PIN, LOW);
	Serial.println("Ready to rumble!");
	}

void loop(void)
	{
	unsigned long code;
	char buf[255];

	delay(100);
	handle_temperature();
	if (wg.available())
		{
		code = wg.getCode();

		snprintf(buf, 255, "Scanned badge %lu/0x%lX, tpe W%d", code, code, wg.getWiegandType());

		Serial.println(buf);
		check_badge(code);
		}
	}
