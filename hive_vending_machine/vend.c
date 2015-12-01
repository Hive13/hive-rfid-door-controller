#include <Arduino.h>

#include "log.h"
#include "vend.h"
#include "leds.h"
#include "temp.h"

// All eight soda buttons where 0 is the top button and 7 is the bottom button.
// In a format of switch pin number, relay pin number, and then the two led numbers
int sodaButtons[][4] = {
	{22, 37, 18, 19},
	{24, 35, 16, 17},
	{26, 33, 14, 15},
	{28, 31, 12, 13},
	{30, 29, 10, 11},
	{32, 27, 8, 9},
	{34, 25, 6, 7},
	{36, 23, 4, 5},
};

unsigned char soda_count = SODA_COUNT;

void set_vend(char c)
	{
	static unsigned char color_at = 0;
	static char lp = -1;
	char i;
	uint32_t cur_color;

	if (lp != c)
		{
		color_at = 0;
		lp = c;
		}

	for (i = 0; i < SODA_COUNT; i++)
		digitalWrite(sodaButtons[i][1], c != i);
	cur_color = Wheel(color_at++);
	leds_one(c, cur_color);
	}

void do_random_vend(void)
	{
	uint32_t randomSodaColor;
	int randomSoda;

	// Pick the color that the chosen soda will be
	randomSodaColor = Wheel(random() & 0xFF);
	// Display the light show
	randomColors(20, 5);
	log_msg("Vending random soda!");
	// Choose the random soda to vend
	randomSoda = random() % SODA_COUNT;
	leds_one(randomSoda, randomSodaColor);
	digitalWrite(sodaButtons[randomSoda][1], 0);
	log_msg("Random soda is %d!\n", randomSoda);
	// Let the chosen soda stay lit for one second
	delay(1000);
	// Turn off the LED
	leds_off();
	}

void do_vend(void)
	{
	log_msg("Vending.");
	digitalWrite(VEND_PIN, LOW);
	digitalWrite(WIEGAND_LIGHT_PIN, HIGH);
	digitalWrite(BEEP_PIN, LOW);
	delay(100);
	digitalWrite(WIEGAND_LIGHT_PIN, LOW);
	digitalWrite(BEEP_PIN, HIGH);
	delay(900);
	digitalWrite(VEND_PIN, HIGH);
	}

void vend_init(void)
	{
	unsigned char i;

	pinMode(VEND_PIN, OUTPUT);
	pinMode(WIEGAND_LIGHT_PIN, OUTPUT);
	pinMode(BEEP_PIN, OUTPUT);
	digitalWrite(VEND_PIN, HIGH);
	digitalWrite(WIEGAND_LIGHT_PIN, LOW);
	digitalWrite(BEEP_PIN, HIGH);
	/*
		Set soda button switch pins to input and pull them high
		Set soda relay pins to output
	*/
	for(i = 0; i < soda_count; i++)
		{
		pinMode(sodaButtons[i][0], INPUT);
		digitalWrite(sodaButtons[i][0], HIGH);
		pinMode(sodaButtons[i][1], OUTPUT);
		}
	}

void vend_check(void)
	{
	char pressed = -1;
	unsigned char i, mask = 0;
	static unsigned char prev_mask = 0, debounce_count;

	// Cycle through all eight buttons, check their values, and do the appropriate event
	for (i = soda_count - 1; i < soda_count; i--)
		{
		mask <<= 1;
		if (digitalRead(sodaButtons[i][0]))
			mask |= 1;
		}
	
	if (prev_mask != mask)
		{
		prev_mask = mask;
		debounce_count = 0;
		mask = 0;
		}
	else if (debounce_count <= MIN_DEBOUNCE_COUNT)
		{
		debounce_count++;
		mask = 0;
		}
	
	if (!mask)
		;
	else if (mask == 0x05)
		pressed = 4;
	else if (mask == 0x03)
		{
		temperature_check();
		return;
		}
	else if (mask == (1 << RANDOM_SODA_NUMBER))
		do_random_vend();
	else if (!(mask & (mask - 1))) /* If only one bit is set in the mask */
		{
		pressed = 0;
		while (mask >>= 1)
			pressed++;
		}
	set_vend(pressed);
	}

