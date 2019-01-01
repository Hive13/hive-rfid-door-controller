#include "config.h"
#include <Arduino.h>

#include "soda_temp.h"
#include "temp.h"
#include "leds.h"
#include "cJSON.h"
#include "schedule.h"

uint32_t cur_temp = 0;

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
	pinMode(COMPRESSOR_RELAY, OUTPUT);
	digitalWrite(COMPRESSOR_RELAY, LOW);

	temperature_init(sensors, (sizeof(sensors) / sizeof(sensors[0])));
	}

