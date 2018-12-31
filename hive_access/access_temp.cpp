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
		.addr     = {0x28, 0xFF, 0x4D, 0xC8, 0x82, 0x16, 0x04, 0x62},
		.func     = main_temperature_sensor,
		.desc     = "Main Sensor",
		.log_name = "annex",
		},
	};

char main_temperature_sensor(struct temp_sensor *me, unsigned long temp)
	{
	log_msg("Temperature: %lu.%lu", temp / 10, temp % 10);
	return 0;
	}

void access_temperature_init(void)
	{
	temperature_init(TEMPERATURE_UPDATE_INTERVAL, sensors, (sizeof(sensors) / sizeof(sensors[0])));
	}
