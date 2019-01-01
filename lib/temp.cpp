#include <Arduino.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <OneWire.h>

#include "log.h"
#include "API.h"
#include "temp.h"
#include "schedule.h"
#include "http.h"
#include "cJSON.h"

extern unsigned char key[16];
extern char *device;
extern OneWire *ds = NULL;

unsigned char sensor_count;

void temperature_init(struct temp_sensor *sensors, unsigned char count)
	{
	log_msg("Initializing temperature controller.");

	sensor_count = count;

	schedule(0, (time_handler *)handle_temperature, sensors);
	}

char start_read_temperature(void)
	{
	unsigned char i;
	unsigned char look_addr[8];

	while (ds->search(look_addr))
		{
#ifdef LOG_FOUND_1WIRE
		log_msg("Found %02X %02X %02X %02X %02X %02X %02X %02X",
			look_addr[0], look_addr[1], look_addr[2], look_addr[3], look_addr[4], look_addr[5], look_addr[6], look_addr[7]);
#endif
		if (OneWire::crc8(look_addr, 7) != look_addr[7])
			{
			log_msg("CRC is not valid!");
			continue;
			}
		if (look_addr[0] == 0x29)
			{
			log_msg("Found 8-IO latch.");
			continue;
			}
		if (look_addr[0] != 0x10 && look_addr[0] != 0x28)
			{
			log_msg("Device not a DS.");
			continue;
			}
		ds->reset();
		ds->select(look_addr);
		ds->write(0x44);
		}
	ds->reset_search();
	return 0;
	}

uint32_t get_temperature(unsigned char *addr)
	{
	//returns the temperature from one DS18S20 in Fahrenheit
	byte data[12], present;
	uint32_t tempRead;
	unsigned char i;

	present = ds->reset();
	ds->select(addr);
	ds->write(0xBE); // Read Scratchpad

	for (i = 0; i < 9; i++) // we need 9 bytes
		data[i] = ds->read();

	ds->reset_search();

	tempRead = ((data[1] << 8) | data[0]); //using two's compliment
	/*
		Optimized math for fixed point.
		1. Multiply by 10 for fixed point.
		2. Divide by 16 because the result is 16 times the actual temperature.
		3. Multiply by 1.8 (9 / 5) to convert from suckigrade.
		4. Add 320 to finish de-suckigrading the temperature.
		(10 * 9) / (16 * 5) = (9 / 8)
	*/
	tempRead *= 9; /* Convert to fixed point */
	tempRead /= 8;
	return tempRead + 320;
	}

char handle_temperature(struct temp_sensor *sensors, unsigned long *t, unsigned long m)
	{
	static unsigned char flag = 0;
	unsigned char i;
	unsigned long temp;

	if (!flag)
		{
		if (start_read_temperature())
			*t = 0;
		else
			{
			*t = m + TEMPERATURE_READ_TIME;
			flag = 1;
			}
		return SCHEDULE_REDO;
		}

	flag = 0;
	for (i = 0; i < sensor_count; i++)
		{
		temp = get_temperature(sensors[i].addr);
		if (sensors[i].func)
			sensors[i].func(sensors + i, temp);
		if (sensors[i].log_name)
			log_temp(temp, sensors[i].log_name);
		}
	*t = m + TEMPERATURE_UPDATE_INTERVAL;
	return SCHEDULE_REDO;
	}
