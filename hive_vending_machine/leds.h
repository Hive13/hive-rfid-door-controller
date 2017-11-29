#ifndef __LEDS_H
#define __LEDS_H

#ifdef __cplusplus
extern "C" {
#endif

void leds_one(char which, uint32_t color, unsigned char do_sold_out);
void leds_two(char which, uint32_t color1, uint32_t color_2);
void leds_init(void);
void leds_off(void);
void leds_busy(void);
void leds_random(char which);
void leds_all(uint32_t color);
uint32_t Color(byte r, byte g, byte b);
uint32_t Wheel(byte WheelPos);

#ifdef __cplusplus
};
#endif

#endif /* __LEDS_H */
