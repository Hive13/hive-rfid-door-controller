#include <Wiegand.h>
#include <Crypto.h>
#include <SHA512.h>
#include <CBC.h>
#include "cJSON.h"

#define SHA512_SZ 64
#define RANDOM_REG32  ESP8266_DREG(0x20E44)

#define BEEP_PIN D0
#define D0_PIN D1
#define D1_PIN D2

static WIEGAND wg;
static char *location = "annex";
static char key[] = {65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65};
static char *hex = "0123456789ABCDEF";

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
		*json, *prev;
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
	free(out);
	}

void setup(void)
	{
	digitalWrite(BEEP_PIN, LOW);
	pinMode(BEEP_PIN, OUTPUT);

	wg.begin(D0_PIN, D0_PIN, D1_PIN, D1_PIN);
	Serial.begin(9600);

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
	if (wg.available())
		{
		code = wg.getCode();

		snprintf(buf, 255, "Scanned badge %lu/0x%lX, tpe W%d", code, code, wg.getWiegandType());

		Serial.println(buf);
		check_badge(code);
		}
	}
