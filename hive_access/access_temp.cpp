#include <Arduino.h>
#include <OneWire.h>

#include "temp.h"
#include "access_temp.h"
#include "log.h"
#include "schedule.h"
#include "http.h"

char main_temperature_sensor(struct temp_sensor *me, unsigned long temp);

struct temp_sensor sensors[] =
	{
		{
		.addr = {0x28, 0xFF, 0xF6, 0x10, 0x82, 0x16, 0x03, 0xC3},
		.func = main_temperature_sensor,
		.desc = "Main Sensor",
		},
		{
		.addr = {0x28, 0xFF, 0x4D, 0xC8, 0x82, 0x16, 0x04, 0x62},
		.func = NULL,
		.desc = "Backup Sensor",
		},
	};

char main_temperature_sensor(struct temp_sensor *me, unsigned long temp)
	{
	log_msg("Temperature: %lu.%lu", temp / 10, temp % 10);
	log_temp(temp);
	/*if (temp > 750)
		digitalWrite(TEMPERATURE_POKE_PIN, LOW);
	else
		digitalWrite(TEMPERATURE_POKE_PIN, HIGH);*/
	return 0;
	}

void access_temperature_init(void)
	{
	digitalWrite(TEMPERATURE_POKE_PIN, HIGH);
	pinMode(TEMPERATURE_POKE_PIN, OUTPUT);

	temperature_init(TEMPERATURE_PIN, TEMPERATURE_UPDATE_INTERVAL, sensors, (sizeof(sensors) / sizeof(sensors[0])));
	}
