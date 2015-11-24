#ifndef __LEDS_H
#define __LEDS_H

#ifdef __cplusplus
extern "C" {
#endif

void randomColors(uint8_t wait, uint8_t numberCycles);
void leds_one(char which, uint32_t color);
void leds_init(void);
void leds_off(void);
uint32_t Color(byte r, byte g, byte b);
uint32_t Wheel(byte WheelPos);

#ifdef __cplusplus
};
#endif

#endif /* __LEDS_H */
