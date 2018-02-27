#ifndef __HTTP_H
#define __HTTP_H

#include "config.h"

unsigned char check_badge(unsigned long badge_num, void (*success)(void));
void log_temp(unsigned long temp);
void update_nonce(void);

#endif /* __HTTP_H */
