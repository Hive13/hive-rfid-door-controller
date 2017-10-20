#ifndef __SODA_TEMP_H
#define __SODA_TEMP_H

#define COMPRESSOR_RELAY (-1)
#define COMPRESSOR_ON 38.0
#define COMPRESSOR_OFF 34.0
#define COMPRESSOR_ON_DELAY_MILLIS 30000

#define TEMPERATURE_PIN 19
#define TEMPERATURE_POWER_PIN 18
#define SODA_TEMP_UPDATE_INTERVAL 1000
#define SODA_TEMP_REPORT_INTERVAL 120000

#ifdef __cplusplus
extern "C" {
#endif

void temperature_check(void);
char log_temp(unsigned long *temp, unsigned long *t, unsigned long m);
void soda_temp_init(void);

#ifdef __cplusplus
};
#endif

#endif /* __SODA_TEMP_H */
