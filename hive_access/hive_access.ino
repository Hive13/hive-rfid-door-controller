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

#define BEEP_PIN  D8
#define D0_PIN    D1
#define D1_PIN    D2
#define LIGHT_PIN D0
#define OPEN_PIN  D3
#define DOOR_PIN  D4

/* Number of ms */
#define DOOR_OPEN_TIME     5000

#define OPEN_IDLE        0
#define OPEN_IN_PROGRESS 1

static WIEGAND wg;
static char *ssid = "hive13int";
static char *pass = "hive13int";
int status        = WL_IDLE_STATUS;

struct door_open
	{
	unsigned int  beep_pin, light_pin, door_pin, open_pin;
	unsigned char beep_state, cycles, status;
	};

char close_door(struct door_open *d, unsigned long *t, unsigned long m)
	{
	unsigned char c;
	
	d->beep_state = !d->beep_state;
	digitalWrite(d->beep_pin,  d->beep_state);
	digitalWrite(d->light_pin, d->beep_state);

	if (--(d->cycles))
		{
		c = digitalRead(d->open_pin);
		if (!c)
			{
			*t = m + 100;
			return SCHEDULE_REDO;
			}
		}
	digitalWrite(d->door_pin,  HIGH);
	digitalWrite(d->beep_pin,  LOW);
	digitalWrite(d->light_pin, LOW);
	d->status = OPEN_IDLE;
	return SCHEDULE_DONE;
	}

void open_door(void)
	{
	static struct door_open d =
		{
		beep_pin:   BEEP_PIN,
		light_pin:  LIGHT_PIN,
		door_pin:   DOOR_PIN,
		open_pin:   OPEN_PIN,
		beep_state: 0,
		cycles:     0,
		status:     OPEN_IDLE,
		};
	
	d.cycles     = (DOOR_OPEN_TIME / 100);
	d.beep_state = 0;
	
	if (d.status == OPEN_IDLE)
		{
		d.status = OPEN_IN_PROGRESS;
		digitalWrite(d.door_pin, LOW);
		schedule(millis() + 100, (time_handler *)close_door, &d);
		}
	}

void setup(void)
	{
	unsigned char i = LOW;
	digitalWrite(BEEP_PIN,  LOW);
	digitalWrite(DOOR_PIN,  HIGH);
	digitalWrite(LIGHT_PIN, i);
	
	pinMode(BEEP_PIN,  OUTPUT);
	pinMode(DOOR_PIN,  OUTPUT);
	pinMode(LIGHT_PIN, OUTPUT);
	pinMode(OPEN_PIN,  INPUT);

	wg.begin(D0_PIN, D0_PIN, D1_PIN, D1_PIN);
	log_begin(115200);

	Serial.print("Connecting to SSID ");
	Serial.println(ssid);

	status = WiFi.begin(ssid, pass);
	while (WiFi.status() != WL_CONNECTED)
		{
		i = !i;
		digitalWrite(LIGHT_PIN, i);
		Serial.print(".");
		delay(250);
		}
	Serial.print("connected!");
	
	temperature_init();

	digitalWrite(BEEP_PIN, HIGH);
	delay(200);
	digitalWrite(BEEP_PIN, LOW);
	delay(100);
	digitalWrite(BEEP_PIN, HIGH);
	delay(200);
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
			check_badge(code, open_door);
		}
	}
