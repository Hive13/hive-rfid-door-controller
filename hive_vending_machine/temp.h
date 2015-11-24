#ifndef __TEMP_H
#define __TEMP_H

#define COMPRESSOR_RELAY (-1)
#define COMPRESSOR_ON 38.0
#define COMPRESSOR_OFF 34.0
#define COMPRESSOR_ON_DELAY_MILLIS 30000

#define TEMPERATURE_PIN 19
#define TEMPERATURE_POWER_PIN 18
#define TEMPERATURE_UPDATE_INTERVAL 300000
#define TEMPERATURE_READ_TIME 1000

char start_read_temperature(void);
uint32_t get_temperature(void);
void handle_temperature();
void temperature_check(void);
void temperature_init(void);

#endif /* __TEMP_H */
