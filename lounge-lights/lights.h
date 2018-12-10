#ifndef __LIGHTS_H
#define __LIGHTS_H

#include "config.h"
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cJSON *get_light_state(void);

#ifdef __cplusplus
}
#endif

#endif /* __LIGHTS_H */
