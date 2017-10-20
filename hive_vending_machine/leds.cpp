#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "log.h"
#include "leds.h"
#include "vend.h"

#define LED_PIN   6
#define LED_COUNT 8

extern unsigned char soda_count;
extern struct soda sodas[];
extern unsigned char sold_out;

static Adafruit_NeoPixel leds = Adafruit_NeoPixel(soda_count, LED_PIN, NEO_GRB | NEO_KHZ800);

void leds_show(unsigned char do_sold_out)
	{
	unsigned char i;

	if (do_sold_out)
		for (i = 0; i < soda_count; i++)
			if (sold_out & (1 << i))
				leds.setPixelColor(i, 255, 128, 0);
	leds.show();
	}

void leds_init(void)
	{
	unsigned char i;
	uint32_t color;

	log_msg("Initializing lights.");
	leds.begin();
	for (i = 0; i < soda_count; i++)
		leds.setPixelColor(i, 255, 0, 0);
	leds_show(0);
	delay(250);
	for (i = 0; i < soda_count; i++)
		leds.setPixelColor(i, 0, 255, 0);
	leds_show(0);
	delay(250);
	for (i = 0; i < soda_count; i++)
		leds.setPixelColor(i, 0, 0, 255);
	leds_show(0);
	delay(250);
	for (i = 0; i < soda_count; i++)
		leds.setPixelColor(i, 255, 255, 255);
	leds_show(0);
	delay(250);
	leds_busy();
	}

void leds_busy(void)
	{
	unsigned char i;
	uint32_t color;

	for (i = 0; i < soda_count; i++)
		{
		color = Wheel(random(0, 255));
		leds.setPixelColor(i, color);
		}
	leds_show(0);
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
		leds.setPixelColor(randomLeds, randomLedsColor);
		leds_show(0);
		delay(wait);
		}
	}

void leds_off(void)
	{
	unsigned char i;

	for(i = 0; i < leds.numPixels(); i++)
		leds.setPixelColor(i, 0);
	leds_show(1);
	}

void leds_one(char which, uint32_t color, unsigned char do_sold_out)
	{
	unsigned char i;

	for (i = 0; i < leds.numPixels(); i++)
		{
		if (which == i)
			leds.setPixelColor(i, color);
		else
			leds.setPixelColor(i, 0UL);
		}
	leds_show(do_sold_out);
	}

void leds_two(char which, uint32_t color1, uint32_t color2)
	{
	unsigned char i;

	for(i = 0; i < leds.numPixels(); i++)
		{
		if (which == i)
			leds.setPixelColor(i, color1);
		else if (which == soda_count - 1 && !i)
			leds.setPixelColor(i, color2);
		else if ((which + 1) == i)
			leds.setPixelColor(i, color2);
		else
			leds.setPixelColor(i, 0);
		}
	
	leds_show(1);
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
