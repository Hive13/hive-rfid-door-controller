#include <Wiegand.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#include "cJSON.h"
#include "API.h"
#include "temp.h"
#include "schedule.h"

#define RANDOM_REG32  ESP8266_DREG(0x20E44)

#define BEEP_PIN  D0
#define D0_PIN    D1
#define D1_PIN    D2
#define LIGHT_PIN D0
#define OPEN_PIN  D0
#define DOOR_PIN  D4

/* Number of ms */
#define DOOR_OPEN_TIME     5000

static WIEGAND wg;
static char *location = "annex";
static char *device   = "annex";
static char *ssid     = "hive13int";
static char *pass     = "hive13int";
static char key[] = {65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65};
int status = WL_IDLE_STATUS;

struct door_open
	{
	unsigned int  beep_pin, light_pin, door_pin, open_pin;
	unsigned char beep_state, cycles;
	};

char close_door(struct door_open *d, unsigned long *t, unsigned long m)
	{
	unsigned char c;
	
	d->beep_state = !d->beep_state;
	digitalWrite(BEEP_PIN,  d->beep_state);
	digitalWrite(LIGHT_PIN, d->beep_state);

	if (--(d->cycles))
		{
		c = digitalRead(d->open_pin);
		if (c)
			{
			*t = m + 100;
			return SCHEDULE_REDO;
			}
		}
	digitalWrite(d->door_pin, HIGH);
	return SCHEDULE_DONE;
	}

void open_door(void)
	{
	static struct door_open d;
	
	d.cycles     = (DOOR_OPEN_TIME / 200);
	d.beep_state = 0;
	d.beep_pin   = BEEP_PIN;
	d.light_pin  = LIGHT_PIN;
	d.door_pin   = DOOR_PIN;
	d.open_pin   = OPEN_PIN;
	
	digitalWrite(d.door_pin, LOW);
	schedule(0, (time_handler *)close_door, &d);
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
	
	out = get_request(badge_num, "access", location, device, key, sizeof(key), rand, sizeof(rand));

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
	digitalWrite(BEEP_PIN,  LOW);
	digitalWrite(DOOR_PIN,  HIGH);
	digitalWrite(LIGHT_PIN, LOW);
	
	pinMode(BEEP_PIN,  OUTPUT);
	pinMode(DOOR_PIN,  OUTPUT);
	pinMode(LIGHT_PIN, OUTPUT);
	pinMode(OPEN_PIN,  INPUT);

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

	run_schedule();
	
	if (wg.available())
		{
		code = wg.getCode();
		type = wg.getWiegandType();

		snprintf(buf, 255, "Scanned badge %lu/0x%lX, type W%d", code, code, type);

		Serial.println(buf);
		if (type == 26)
			check_badge(code);
		}
	}
