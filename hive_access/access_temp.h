#ifndef __ACCESS_TEMP_H
#define __ACCESS_TEMP_H

#define TEMPERATURE_PIN D5
#define TEMPERATURE_UPDATE_INTERVAL 60000

#define TEMPERATURE_POKE_PIN D6

#ifdef __cplusplus
extern "C" {
#endif

void access_temperature_init(void);

#ifdef __cplusplus
};
#endif

#endif /* __ACCESS_TEMP_H */
