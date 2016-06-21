#include <Arduino.h>
#include <Adafruit_WS2801.h>

#include "log.h"
#include "leds.h"
#include "vend.h"

#define LED_DATA_PIN 39 /* green wire */
#define LED_CLOCK_PIN 38 /* blue wire */
#define LED_COUNT 20

static Adafruit_WS2801 leds = Adafruit_WS2801(LED_COUNT, LED_DATA_PIN, LED_CLOCK_PIN);
extern unsigned char soda_count;
extern struct soda sodas[];
extern unsigned char sold_out;

void leds_init(void)
	{
	unsigned char i;
	uint32_t color;

	log_msg("Initializing lights.");
	leds.begin();
	leds_busy();
	}

void leds_busy(void)
	{
	unsigned char i;
	uint32_t color;

	for (i = 0; i < soda_count; i++)
		{
		color = Wheel(random(0, 255));
		leds.setPixelColor(sodas[i].leds[0], color);
		leds.setPixelColor(sodas[i].leds[1], color);
		}
	leds.show();
	}

inline void leds_set_color(struct soda *s, uint32_t color)
	{
	leds.setPixelColor(s->leds[0], color);
	leds.setPixelColor(s->leds[1], color);
	}

void leds_set_color(struct soda *s, uint8_t r, uint8_t g, uint8_t b)
	{
	leds.setPixelColor(s->leds[0], r, g, b);
	leds.setPixelColor(s->leds[1], r, g, b);
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
		leds_set_color(&(sodas[randomLeds]), randomLedsColor);
		leds.show();
		delay(wait);
		}
	}

void leds_off(void)
	{
	unsigned char i;

	for(i = 0; i < leds.numPixels(); i++)
		leds.setPixelColor(i, 0);
	for (i = 0; i < soda_count; i++)
		if (sold_out & (1 << i))
			leds_set_color(&(sodas[i]), 255, 128, 0);
	leds.show();
	}

void leds_one_only(char which, uint32_t color)
	{
	unsigned char i;

	for (i = 0; i < leds.numPixels(); i++)
		{
		if (which >= 0 && which < soda_count && (sodas[which].leds[0] == i || sodas[which].leds[1] == i))
			leds.setPixelColor(i, color);
		else
			leds.setPixelColor(i, 0);
		}
	leds.show();
	}

void leds_one(char which, uint32_t color)
	{
	unsigned char i;

	for (i = 0; i < leds.numPixels(); i++)
		{
		if (which >= 0 && which < soda_count && (sodas[which].leds[0] == i || sodas[which].leds[1] == i))
			leds.setPixelColor(i, color);
		else
			leds.setPixelColor(i, 0);
		}
	for (i = 0; i < soda_count; i++)
		if (sold_out & (1 << i))
			leds_set_color(&(sodas[i]), 255, 128, 0);
	leds.show();
	}

void leds_two(char which, uint32_t color1, uint32_t color2)
	{
	unsigned char i;

	for(i = 0; i < leds.numPixels(); i++)
		{
		if (which >= 0 && which < soda_count && (sodas[which].leds[0] == i || sodas[which].leds[1] == i))
			leds.setPixelColor(i, color1);
		else if (which == soda_count -  1 && (sodas[0].leds[0] == i || sodas[0].leds[1] == i))
			leds.setPixelColor(i, color2);
		else if (which >= 0 && which < soda_count - 1 && (sodas[which + 1].leds[0] == i || sodas[which + 1].leds[1] == i))
			leds.setPixelColor(i, color2);
		else
			leds.setPixelColor(i, 0);
		}
	for (i = 0; i < soda_count; i++)
		if (sold_out & (1 << i))
			leds_set_color(&(sodas[i]), 255, 128, 0);
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
