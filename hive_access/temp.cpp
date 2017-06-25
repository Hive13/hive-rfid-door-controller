#include <Arduino.h>
#include <OneWire.h>

#include "temp.h"
#include "log.h"
#include "schedule.h"
#include "http.h"

unsigned char main_temperature_addr[] = {0x28, 0xFF, 0xF6, 0x10, 0x82, 0x16, 0x03, 0xC3};
unsigned char bkup_temperature_addr[] = {0x28, 0xFF, 0x4D, 0xC8, 0x82, 0x16, 0x04, 0x62};
struct temp_sensor sensors[MAX_NUM_SENSORS];
unsigned char sensor_count;

static OneWire ds(TEMPERATURE_PIN);

char main_temperature_sensor(struct temp_sensor *me, unsigned long temp)
	{
	char buf[256];

	log_msg("Temperature: %hu.%hu", temp / 10, temp % 10);
	log_temp(temp);
	/*if (temp > 750)
		digitalWrite(TEMPERATURE_POKE_PIN, LOW);
	else
		digitalWrite(TEMPERATURE_POKE_PIN, HIGH);*/
	return 0;
	}

void temperature_init(void)
	{
	static unsigned long allocate_a_long;	
		
	memset(sensors, 0, MAX_NUM_SENSORS * sizeof(struct temp_sensor));
	
	memmove(sensors[0].addr, main_temperature_addr, ONEWIRE_ADDR_SZ);
	sensors[0].desc = "Main Sensor";
	sensors[0].func = main_temperature_sensor;
	sensors[0].data = &allocate_a_long;
	
	memmove(sensors[1].addr, bkup_temperature_addr, ONEWIRE_ADDR_SZ);
	sensor_count = 2;
	
	digitalWrite(TEMPERATURE_POKE_PIN, HIGH);
	pinMode(TEMPERATURE_POKE_PIN, OUTPUT);
	schedule(0, handle_temperature, sensors);
	}

char start_read_temperature(void)
	{
	unsigned char i;
	unsigned char look_addr[8];
	
	while (ds.search(look_addr))
		{
		/*log_msg("Found %02X %02X %02X %02X %02X %02X %02X %02X",
			look_addr[0], look_addr[1], look_addr[2], look_addr[3], look_addr[4], look_addr[5], look_addr[6], look_addr[7]);*/
		if (OneWire::crc8(look_addr, 7) != look_addr[7])
			{
			Serial.println("CRC is not valid!");
			continue;
			}
		if (look_addr[0] != 0x10 && look_addr[0] != 0x28)
			{
			Serial.println("Device not a DS.");
			continue;
			}
		ds.reset();
		ds.select(look_addr);
		ds.write(0x44);
		}	
	ds.reset_search();
	return 0;
	}

uint32_t get_temperature(unsigned char *addr)
	{
	//returns the temperature from one DS18S20 in Fahrenheit
	byte data[12], present;
	uint32_t tempRead;
	unsigned char i;
	
	present = ds.reset();
	ds.select(addr);  
	ds.write(0xBE); // Read Scratchpad
	
	for (i = 0; i < 9; i++) // we need 9 bytes
		data[i] = ds.read();
	
	ds.reset_search();
	
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

char handle_temperature(void *ptr, unsigned long *t, unsigned long m)
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
		}
	*t = m + TEMPERATURE_UPDATE_INTERVAL - TEMPERATURE_READ_TIME;
	return SCHEDULE_REDO;
	}
