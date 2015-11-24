#include <Arduino.h>
#include <Adafruit_WS2801.h>

#include "leds.h"
#include "vend.h"

#define LED_DATA_PIN 39 /* green wire */
#define LED_CLOCK_PIN 38 /* blue wire */
#define LED_COUNT 20

static Adafruit_WS2801 leds = Adafruit_WS2801(LED_COUNT, LED_DATA_PIN, LED_CLOCK_PIN);
extern unsigned char soda_count;
extern int sodaButtons[][4];

void leds_init(void)
	{
	unsigned char i;
	uint32_t color;

	leds.begin();
	for (i = 0; i < soda_count; i++)
		{
		color = Wheel(random(0, 255));
		leds.setPixelColor(sodaButtons[i][2], color);
		leds.setPixelColor(sodaButtons[i][3], color);
		}
	leds.show();
	}

/* LED Helper functions */
void randomColors(uint8_t wait, uint8_t numberCycles)
	{
	unsigned int i, randomLeds;
	uint32_t randomLedsColor;

	for(i = 0; i < numberCycles * leds.numPixels(); i++)
		{
		randomLeds = random(0, 8);
		randomLedsColor = Wheel(random(0, 255));
		// Set groups of two to the same color. The +4 is to make the 16 out of 20 that turn on the end ones.
		leds.setPixelColor(randomLeds * 2 + 4, randomLedsColor);
		leds.setPixelColor(randomLeds * 2 + 1 + 4, randomLedsColor);
		leds.show();
		delay(wait);
		}
	}

void leds_off(void)
	{
	unsigned char i;

	for(i = 0; i < leds.numPixels(); i++)
		leds.setPixelColor(i, 0);
	leds.show();
	}

void leds_one(char which, uint32_t color)
	{
	unsigned char i;

	for(i = 0; i < leds.numPixels(); i++)
		{
		if (which >= 0 && which < soda_count && (sodaButtons[which][2] == i || sodaButtons[which][3] == i))
			leds.setPixelColor(i, color);
		else
			leds.setPixelColor(i, 0);
		}
	leds.show();
	}

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
	{
	uint32_t c;
	c = r;
	c <<= 8;
	c |= g;
	c <<= 8;
	c |= b;
	return c;
	}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
	{
	if (WheelPos < 85)
		return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	else if (WheelPos < 170)
		{
		WheelPos -= 85;
		return Color(255 - WheelPos * 3, 0, WheelPos * 3);
		}
	else
		{
		WheelPos -= 170;
		return Color(0, WheelPos * 3, 255 - WheelPos * 3);
		}
	}
