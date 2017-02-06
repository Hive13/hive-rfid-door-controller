#ifndef __TEMP_H
#define __TEMP_H

#define TEMPERATURE_PIN D3
#define TEMPERATURE_POWER_PIN 18
#define TEMPERATURE_UPDATE_INTERVAL 3000
#define TEMPERATURE_READ_TIME 1000

#define ONEWIRE_ADDR_SZ 8
#define MAX_NUM_SENSORS 4
#define TEMPERATURE_POKE_PIN D4

#ifdef __cplusplus
extern "C" {
#endif

typedef char (sensor_func)(struct temp_sensor *, unsigned long);

char start_read_temperature(void);
uint32_t get_temperature(void);
void handle_temperature();
void temperature_check(void);
void temperature_init(void);

struct temp_sensor
	{
	unsigned char addr[ONEWIRE_ADDR_SZ];
	sensor_func *func;
	char *desc;
	void *data;
	};
	

#ifdef __cplusplus
};
#endif

#endif /* __TEMP_H */
