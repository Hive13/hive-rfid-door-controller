#ifndef __SODA_TEMP_H
#define __SODA_TEMP_H

#define COMPRESSOR_RELAY (-1)
#define COMPRESSOR_ON 38.0
#define COMPRESSOR_OFF 34.0
#define COMPRESSOR_ON_DELAY_MILLIS 30000

#ifdef __cplusplus
extern "C" {
#endif

void soda_temp_init(void);

#ifdef __cplusplus
};
#endif

#endif /* __SODA_TEMP_H */
