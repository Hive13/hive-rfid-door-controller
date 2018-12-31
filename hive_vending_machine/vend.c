#include "config.h"

#include <Arduino.h>
#include <avr/eeprom.h>

#include "cJSON.h"
#include "log.h"
#include "vend.h"
#include "ui.h"
#include "http.h"
#include "schedule.h"
#include "output.h"

#define EEPROM_VER1_SIGNATURE 0xD384

struct vend_eeprom
	{
	unsigned short size;
	unsigned short signature;
	unsigned char  soda_count;
	unsigned char  soda_type[];
	};

// All eight soda buttons where 0 is the top button and 7 is the bottom button.
// In a format of switch pin number, relay pin number, and a type.
struct soda sodas[] = {
	{22, 37, 2},
	{24, 35, 1},
	{26, 33, 1},
	{28, 31, 1},
	{30, 29, 1},
	{32, 27, 2},
	{34, 25, 2},
	{36, 23, 3},
};

unsigned char soda_count = SODA_COUNT;
static unsigned char larsen_on = 0;
unsigned char sold_out = 0;
static volatile unsigned char sold_out_t = 0;
static void *sold_out_schedule = NULL;

char update_sold_out(volatile unsigned char *ptr, unsigned long *t, unsigned long m)
	{
	if (sold_out != sold_out_t)
		{
		sold_out = sold_out_t;
		log_msg("Sold out: %02hhX", sold_out);
		update_soda_status(sold_out);
		}

	return SCHEDULE_DONE;
	}

ISR(PCINT2_vect)
	{
	schedule_cancel(sold_out_schedule);
	sold_out_schedule = schedule(millis() + 250, update_sold_out, &sold_out_t);
	sold_out_t = PINK;
	}

void type_display(void)
	{
	unsigned char i;
	unsigned long color;

	leds_out(0);
	for (i = 0; i < soda_count; i++)
		{
		switch (sodas[i].type)
			{
			case KIND_DIET:
				color = Color(255, 0, 0);
				break;
			case KIND_REGULAR:
				color = Color(0, 255, 0);
				break;
			case KIND_BEER:
				color = Color(255, 255, 0);
				break;
			case KIND_WATER:
				color = Color(255, 255, 255);
				break;
			default:
				color = 0;
			}
		leds_one(i, color, 0);
		}
	leds_one(-1, 0, LEDS_SHOW);
	}

void set_vend(char c)
	{
	static unsigned char color_at = 0;
	static char lp = -1;
	static unsigned short larsen_at = 0;
	char i;
	uint32_t cur_color;

	if (lp != c)
		{
		color_at = 0;
		lp = c;
		larsen_at = 0;
		}

	for (i = 0; i < SODA_COUNT; i++)
		digitalWrite(sodas[i].relay_pin, c != i);
	if (c == -1 && larsen_on)
		{
		c = larsen_at / 256;
		cur_color = Color((larsen_at % 256), (larsen_at % 256), (larsen_at % 256));
		larsen_at++;
		if (larsen_at >= (256 * 8))
			larsen_at = 0;
		leds_two(c, ~cur_color, cur_color);
		}
	else
		{
		cur_color = Wheel(color_at++);
		leds_one(c, cur_color, LEDS_OTHERS_OFF | LEDS_SHOW | LEDS_SOLD_OUT);
		}
	}

void do_random_vend(unsigned char kind)
	{
	unsigned long stop_at, randomSodaColor;
	unsigned char randomSoda, tries = 0, color_at = 20 * SODA_COUNT, i;

	// Display the light show
	log_msg("Vending random soda!");
	leds_out(0);

	/* Light show while picking the soda */
	for (i = 0; i < color_at; i++)
		{
		tries = 0;
		/*
			Choose the random soda to vend
			Eventually break the loop if shit hits the fan
		*/
		while (--tries)
			{
			randomSoda = random() % SODA_COUNT;
			if (sold_out & (1 << randomSoda))
				continue;
			if (kind == KIND_ANY)
				break;
			else if (kind == sodas[randomSoda].type)
				break;
			}
		leds_random(randomSoda);
		delay(20);
		}

	digitalWrite(sodas[randomSoda].relay_pin, 0);
	log_msg("Random soda is %d!\n", randomSoda);

	/* Cycle the chosen soda for one second */
	color_at = 0;
	stop_at = millis() + 1000;
	while (millis() < stop_at)
		{
		randomSodaColor = Wheel(color_at++);
		leds_one(randomSoda, randomSodaColor, LEDS_SHOW | LEDS_OTHERS_OFF | LEDS_SOLD_OUT);
		}

	leds_off();
	}

void handle_vend(unsigned long code)
	{
	unsigned char rv = can_vend(code);

	if (rv == RESPONSE_ACCESS_DENIED)
		{
		beep_it(BEEP_PATTERN_NO_CREDITS);
		return;
		}

	log_msg("Vending.");
	leds_all(Color(0, 255, 0));
	set_output(OUTPUT_VEND_RELAY, 0);
	LIGHT_GREEN();
	set_output(OUTPUT_SCANNER_BEEPER, BEEP_ON);
	delay(100);
	set_output(OUTPUT_SCANNER_BEEPER, BEEP_OFF);
	LIGHT_RED();
	delay(900);
	set_output(OUTPUT_VEND_RELAY, 1);
	leds_off();
	}

void vend_init(void)
	{
	unsigned char i;
	unsigned char buf[128];
	struct vend_eeprom *prom = buf;

	DDRK = 0;
	PORTK = 0xFF;

	/*
		Set soda button switch pins to input and pull them high
		Set soda relay pins to output
	*/
	for(i = 0; i < soda_count; i++)
		{
		pinMode(sodas[i].switch_pin, INPUT);
		digitalWrite(sodas[i].switch_pin, HIGH);
		pinMode(sodas[i].relay_pin, OUTPUT);
		digitalWrite(sodas[i].relay_pin, HIGH);
		}

	log_msg("Loading vend data from EEPROM");
	eeprom_read_block(buf, 0, 4);
	switch (prom->signature)
		{
		case EEPROM_VER1_SIGNATURE:
			eeprom_read_block(buf + 4, 4, prom->size - 4);
			for (i = 0; i < soda_count; i++)
				sodas[i].type = prom->soda_type[i];
			break;
		default: /* Initialize */
			log_msg("No data stored, initializing.");
			prom->signature  = EEPROM_VER1_SIGNATURE;
			prom->size       = 5 + soda_count;
			prom->soda_count = soda_count;
			memset(prom->soda_type, 0, soda_count);
			for (i = 0; i < soda_count; i++)
				prom->soda_type[i] = sodas[i].type;
			eeprom_write_block(buf, 0, prom->size);
			break;
		}

	/* Set up the sold out pins */
	cli();
	PCICR |= 1 << PCIE2;
	PCMSK2 = 0xFF;
	sold_out = PINK;
	sei();

	schedule(0, vend_check, NULL);
	}

char vend_check(void *ptr, unsigned long *t, unsigned long m)
	{
	char pressed = -1;
	unsigned char i, mask = 0;
	static unsigned char prev_mask = 0, debounce_count;

	// Cycle through all eight buttons, check their values, and do the appropriate event
	set_output(OUTPUT_SCANNER_BEEPER, BEEP_OFF);
	for (i = soda_count - 1; i < soda_count; i--)
		{
		mask <<= 1;
		if (!digitalRead(sodas[i].switch_pin))
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
		{
		if (sold_out & 0x10)
			{
			set_output(OUTPUT_SCANNER_BEEPER, BEEP_ON);
			pressed = -1;
			}
		else
			pressed = 4;
		}
	else if (mask == 0x03)
		{
		temperature_check();
		return SCHEDULE_REDO;
		}
	else if (mask == 0x06)
		larsen_on = 1;
	else if (mask == 0x0A)
		larsen_on = 0;
	else if (mask == 0x18)
		{
		type_display();
		return SCHEDULE_REDO;
		}
	else if (mask == (1 << RANDOM_SODA_NUMBER))
		do_random_vend(KIND_ANY);
	else if (mask == 0x30)
		do_random_vend(KIND_DIET);
	else if (mask == 0xC0)
		do_random_vend(KIND_REGULAR);
	else if (mask == 0xA0)
		digitalWrite(BEEP_PIN, LOW);
	else if (!(mask & (mask - 1))) /* If only one bit is set in the mask */
		{
		if (mask & sold_out)
			{
			set_output(OUTPUT_SCANNER_BEEPER, BEEP_ON);
			pressed = -1;
			}
		else
			{
			pressed = 0;
			while (mask >>= 1)
				pressed++;
			}
		}
	set_vend(pressed);
	return SCHEDULE_REDO;
	}
