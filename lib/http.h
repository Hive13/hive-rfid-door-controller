#ifndef __HTTP_H
#define __HTTP_H

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned char check_badge(unsigned long badge_num, void (*success)(void));
void log_temp(unsigned long temp, char *name);
void update_soda_status(unsigned char sold_out_mask);
void update_nonce(void);
unsigned char can_vend(unsigned long badge);
char *get_signed_packet(struct cJSON *data);
unsigned char http_request(struct cJSON *data, struct cJSON **result, char *rand);
void add_random_response(struct cJSON *data, char *rand);

#ifdef PLATFORM_ARDUINO
#define RAND_SIZE (2 * sizeof(unsigned long))
#endif
#ifdef PLATFORM_ESP8266
#define RAND_SIZE 16
#endif

#ifdef __cplusplus
}
#endif

#endif /* __HTTP_H */
