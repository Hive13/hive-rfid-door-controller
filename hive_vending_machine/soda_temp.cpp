#include <Arduino.h>

#include "soda_temp.h"
#include "temp.h"
#include "leds.h"
#include "cJSON.h"
#include "schedule.h"

extern unsigned char key[16];
extern char *device;

static uint32_t cur_temp = 0;

char soda_temperature_sensor(struct temp_sensor *me, unsigned long temp)
	{
	cur_temp = temp;

	/*if (temp <= COMPRESSOR_OFF)
		{
		digitalWrite(COMPRESSOR_RELAY, LOW);
		start_at = m + COMPRESSOR_ON_DELAY_MILLIS;
		}
	else if (temp >= COMPRESSOR_ON && start_at <= m)
		digitalWrite(COMPRESSOR_RELAY, HIGH);*/

	return 0;
	}

struct temp_sensor sensors[] = 
	{
		{
		.addr     = {0x28, 0xB7, 0xC2, 0x28, 0x07, 0x00, 0x00, 0x2B},
		.func     = soda_temperature_sensor,
		.desc     = "Main Sensor",
		.log_name = "soda_machine",
		},
	};

void soda_temp_init(void)
	{
	pinMode(TEMPERATURE_POWER_PIN, OUTPUT);
	pinMode(COMPRESSOR_RELAY, OUTPUT);
	digitalWrite(COMPRESSOR_RELAY, LOW);
	digitalWrite(TEMPERATURE_POWER_PIN, HIGH);

	temperature_init(TEMPERATURE_PIN, TEMPERATURE_UPDATE_INTERVAL, sensors, (sizeof(sensors) / sizeof(sensors[0])));
	}

void temperature_check(void)
	{
	unsigned char light, p;
	uint32_t color;

	if (cur_temp < 320)
		{
		light = 0;
		color = Color(0, 255, 0);
		}
	else if (cur_temp >= 480)
		{
		light = 7;
		color = Color(0, 255, 0);
		}
	else
		{
		light = (unsigned char)((cur_temp - 320) / 20);
		p = (cur_temp % 20) * 12;
		color = Color(0 + p, 0, 255 - p);
		}
	leds_one(light, color, 0);
	}
