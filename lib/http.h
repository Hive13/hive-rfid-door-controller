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

#ifdef __cplusplus
}
#endif

#endif /* __HTTP_H */
