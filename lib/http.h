#ifndef __HTTP_H
#define __HTTP_H

#include "config.h"

#define RESPONSE_BAD_NONCE     5
#define RESPONSE_ACCESS_DENIED 4
#define RESPONSE_BAD_JSON      3
#define RESPONSE_BAD_HTTP      2
#define RESPONSE_BAD_CKSUM     1
#define RESPONSE_GOOD          0

#ifdef __cplusplus
extern "C" {
#endif

unsigned char check_badge(unsigned long badge_num);
void log_temp(unsigned long temp, char *name);
void update_soda_status(unsigned char sold_out_mask);
void update_nonce(void);
unsigned char can_vend(unsigned long badge);

#ifdef __cplusplus
}
#endif

#endif /* __HTTP_H */
