#ifndef __AIR_CONTROL_H
#define __AIR_CONTROL_H

#define BLEED_OFF_MS 5000
#define BLEED_PIN    D7
#define COMPRESS_PIN D8
#define PRESSURE_PIN A0
#define OFF_PSI      700
#define ON_PSI       600
#define LOG_INTERVAL 5000

#ifdef __cplusplus
extern "C" {
#endif

void air_compressor_init(void);
char handle_air_compressor(void *ptr, unsigned long *t, unsigned long m);

#ifdef __cplusplus
};
#endif

#endif /* __AIR_CONTROL_H */
