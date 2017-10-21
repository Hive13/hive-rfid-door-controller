#include <Arduino.h>

#include "cJSON.h"
#include "API.h"
#include "log.h"
#include "vend.h"
#include "leds.h"
#include "temp.h"
#include "http.h"
#include "schedule.h"

unsigned char key[] = {'S', 'o', 'd', 'a', ' ', 'M', 'a', 'c', 'h', 'i', 'n', 'e', '!', '!', '!', '!'};
char *device = "soda_machine";

// All eight soda buttons where 0 is the top button and 7 is the bottom button.
// In a format of switch pin number, relay pin number, and a diet flag.
struct soda sodas[] = {
	{22, 37, 0},
	{24, 35, 0},
	{26, 33, 0},
	{28, 31, 0},
	{30, 29, 0},
	{32, 27, 1},
	{34, 25, 1},
	{36, 23, 1},
};

unsigned char soda_count = SODA_COUNT;
static unsigned char larsen_on = 0;
extern unsigned char sold_out;

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
		leds_one(c, cur_color, 1);
		}
	}

void do_random_vend(unsigned char kind)
	{
	unsigned long stop_at, randomSodaColor;
	unsigned char randomSoda, tries = 0, color_at = 20 * SODA_COUNT, i;

	// Display the light show
	log_msg("Vending random soda!");

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
			else if (kind == KIND_DIET && sodas[randomSoda].diet)
				break;
			else if (kind == KIND_REGULAR && !sodas[randomSoda].diet)
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
		leds_one(randomSoda, randomSodaColor, 1);
		}

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

	DDRK = 0;
	PORTK = 0xFF;

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
		pinMode(sodas[i].switch_pin, INPUT);
		digitalWrite(sodas[i].switch_pin, HIGH);
		pinMode(sodas[i].relay_pin, OUTPUT);
		digitalWrite(sodas[i].relay_pin, HIGH);
		}

	schedule(0, vend_check, NULL);
	}

char vend_check(void *ptr, unsigned long *t, unsigned long m)
	{
	char pressed = -1;
	unsigned char i, mask = 0;
	static unsigned char prev_mask = 0, debounce_count;

	// Cycle through all eight buttons, check their values, and do the appropriate event
	digitalWrite(BEEP_PIN, HIGH);
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
			digitalWrite(BEEP_PIN, LOW);
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
			digitalWrite(BEEP_PIN, LOW);
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

signed char can_vend(unsigned long badge)
	{
	static unsigned long scan_count = 0;
	char *body, *out;
	struct cJSON *resp, *cs;
	unsigned char rv[2 * sizeof(unsigned long)];
	unsigned char rc;
	unsigned long start = millis();

	memcpy(rv, &start, sizeof(unsigned long));
	memcpy(rv + sizeof(unsigned long), &scan_count, sizeof(unsigned long));
	scan_count++;

	out = get_request(badge, "vend", NULL, device, key, sizeof(key), rv, sizeof(rv));
	log_msg("Sending %s", out);

	rc = http_get_json("intweb.at.hive13.org", "/api/access", out, &body);
	free(out);

	if (rc != RESPONSE_GOOD)
		{
		log_msg("GET failed: %hhd", rc);
		return rc;
		}

	log_msg("Body: %s", body);

	rc = parse_response(body, &resp, key, sizeof(key), rv, sizeof(rv));
	free(body);

	log_msg("rc: %hhd", rc);
	if (rc != RESPONSE_GOOD)
		return rc;

	if (!(cs = cJSON_GetObjectItem(resp, "vend")) || cs->type != cJSON_True)
		{
		log_msg("Didn't get a vend response back.");
		cJSON_Delete(resp);
		return RESPONSE_ACCESS_DENIED;
		}

	cJSON_Delete(resp);
	return RESPONSE_GOOD;
	}

