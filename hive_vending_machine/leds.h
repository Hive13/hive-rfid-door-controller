#ifndef __LEDS_H
#define __LEDS_H

void randomColors(uint8_t wait, uint8_t numberCycles);
void turnOffLeds(char except);
void leds_init(void);
uint32_t Color(byte r, byte g, byte b);
uint32_t Wheel(byte WheelPos);

#endif /* __LEDS_H */
