#ifndef __TEMP_H
#define __TEMP_H

#define TEMPERATURE_UPDATE_INTERVAL 60000
#define TEMPERATURE_READ_TIME 1000
#define ONEWIRE_ADDR_SZ 8
#define MAX_NUM_SENSORS 4

#ifdef __cplusplus
extern "C" {
#endif

typedef char (sensor_func)(struct temp_sensor *, unsigned long);

char handle_temperature(void *ptr, unsigned long *t, unsigned long m);
void temperature_init(void);

struct temp_sensor
	{
	unsigned char addr[ONEWIRE_ADDR_SZ];
	sensor_func *func;
	char *desc;
	char *log_name;
	};

#ifdef __cplusplus
};
#endif

#endif /* __TEMP_H */
