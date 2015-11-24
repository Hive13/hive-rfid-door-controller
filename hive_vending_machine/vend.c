#include <Arduino.h>
#include <Adafruit_WS2801.h>

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
		color_at = 0;

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
	digitalWrite(7, LOW);
	digitalWrite(8, HIGH);
	digitalWrite(9, LOW);
	delay(100);
	digitalWrite(8, LOW);
	digitalWrite(9, HIGH);
	delay(900);
	digitalWrite(7, HIGH);
	}

void vend_init(void)
	{
	unsigned char i;

	pinMode(7, OUTPUT);
	pinMode(8, OUTPUT);
	pinMode(9, OUTPUT);
	digitalWrite(7, HIGH);
	digitalWrite(8, LOW);
	digitalWrite(9, HIGH);
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
	unsigned char i;

	// Cycle through all eight buttons, check their values, and do the appropriate event
	for(i = 0; i < soda_count; i++)
		{
		// For Ryan: If buttons 1 and 3 are pressed at the same time then vend the fifth soda.
		// This is the soda that normally would have vended with the button that random currently takes.
		if(!digitalRead(sodaButtons[0][0]) && !digitalRead(sodaButtons[2][0]))
			{
			pressed = 4;
			break;
			}
		/* Display the current temperature */
		if(!digitalRead(sodaButtons[0][0]) && !digitalRead(sodaButtons[1][0]))
			{
			temperature_check();
			return;
			}

		/* Is the current button pressed? */
		if (!digitalRead(sodaButtons[i][0]))
			{
			if(i == RANDOM_SODA_NUMBER)
				{
				pressed = -1;
				do_random_vend();
				}
			else
				pressed = i;
			break;
			}
		}
	set_vend(pressed);
	}

