#include <Wiegand.h>
#include <Crypto.h>
#include <SHA512.h>
#include <CBC.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#include "cJSON.h"
#include "API.h"
#include "temp.h"

#define SHA512_SZ 64
#define RANDOM_REG32  ESP8266_DREG(0x20E44)

#define BEEP_PIN D0
#define D0_PIN D1
#define D1_PIN D2

#define DOOR_PIN D4

#define RESPONSE_BAD_JSON  3
#define RESPONSE_BAD_HTTP  2
#define RESPONSE_BAD_CKSUM 1
#define RESPONSE_GOOD      0

#define DOOR_OPEN_TIME     4000

static WIEGAND wg;
static char *location = "annex";
static char *device   = "annex";
static char *ssid     = "hive13int";
static char *pass     = "hive13int";
static char key[] = {65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65};
static char *hex = "0123456789ABCDEF";
int status = WL_IDLE_STATUS;

typedef void (time_handler)(void *);
struct task
	{
	struct task   *prev, *next;
	unsigned long time;
	time_handler  *func;
	void          *data;
	};

struct task *task_chain = NULL;

void schedule(unsigned long time, time_handler *func, void *ptr)
	{
	struct task *t = (struct task *)malloc(sizeof(struct task)), *walker = task_chain;

	t->next = NULL;
	t->time = time;
	t->func = func;
	t->data = ptr;

	cli();

	if (!task_chain)
		{
		task_chain = t;
		t->prev = NULL;
		}
	else
		{
		while (walker->next)
			walker = walker->next;
		walker->next = t;
		t->prev = walker;
		}
	
	sei();
	}

void close_door(void *junk)
	{
	digitalWrite(DOOR_PIN, HIGH);
	}

void open_door(void)
	{
	unsigned long time = millis() + DOOR_OPEN_TIME;

	digitalWrite(DOOR_PIN, LOW);
	schedule(time, close_door, NULL);
	}

void check_badge(unsigned long badge_num)
	{
	struct cJSON *json, *result;
	unsigned char i;
	unsigned long r;
	char *out;
	char rand[16];
	HTTPClient http;
	int code;
	String body;
	const char host[] = "http://intweb.at.hive13.org/api/access";

	for (i = 0; i < sizeof(rand); i++)
		{
		if (!(i % 4))
			r = RANDOM_REG32;
		rand[i] = ((r >> (3 - i)) & 0xFF);
		}
	
	out = get_request(badge_num, location, device, key, sizeof(key), rand, sizeof(rand));

	Serial.println(out);

	http.begin(host);
	http.addHeader("Content-Type", "application/json");

	code = http.POST((unsigned char *)out, strlen(out));
	free(out);

	if (code != 200)
		return /*(RESPONSE_BAD_HTTP | ((code << 8)) & 0x7F00)*/;

	body = http.getString();
	i = parse_response((char *)body.c_str(), &result, key, sizeof(key), rand, sizeof(rand));

	if (i == RESPONSE_GOOD)
		{
		json = cJSON_GetObjectItem(result, "access");
		if (json && json->type == cJSON_True)
			open_door();
		cJSON_Delete(result);
		}
	else
		{
		Serial.print("Error: ");
		Serial.println(i, DEC);
		}
	}

void setup(void)
	{
	digitalWrite(BEEP_PIN, LOW);
	digitalWrite(DOOR_PIN, HIGH);
	pinMode(BEEP_PIN, OUTPUT);
	pinMode(DOOR_PIN, OUTPUT);

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
	unsigned char type;
	char buf[255];
	struct task *t = task_chain, *p;
	unsigned long m = millis();

	while (t)
		{
		if (t->time <= m)
			{
			t->func(t->data);

			if (t->prev)
				t->prev->next = t->next;
			else
				task_chain = t->next;
			if (t->next)
				t->next->prev = t->prev;
			p = t->next;
			free(t);
			t = p;
			}
		else
			t = t->next;
		}


	handle_temperature();
	if (wg.available())
		{
		code = wg.getCode();
		type = wg.getWiegandType();

		snprintf(buf, 255, "Scanned badge %lu/0x%lX, type W%d", code, code, type);

		Serial.println(buf);
		if (type == 26)
			check_badge(code);
		}
	delay(100);
	}
