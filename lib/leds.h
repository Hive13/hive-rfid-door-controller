#ifndef __LEDS_H
#define __LEDS_H

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SODA_MACHINE
void leds_one(char which, uint32_t color, unsigned char do_sold_out);
void leds_two(char which, uint32_t color1, uint32_t color_2);
void leds_init(void);
void leds_off(void);
void leds_busy(void);
void leds_random(char which);
void leds_all(uint32_t color);
#else
#define leds_one(which, color, do_sold_out)
#define leds_two(which, color1, color_2)
#define leds_init()
#define leds_off()
#define leds_busy()
#define leds_random(which)
#define leds_all(color)
#endif
uint32_t Color(byte r, byte g, byte b);
uint32_t Wheel(byte WheelPos);

#ifdef __cplusplus
};
#endif

#endif /* __LEDS_H */
