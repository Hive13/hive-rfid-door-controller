#include <Ethernet.h>
// https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino
#include <Wiegand.h>

#include "leds.h"
#include "soda_temp.h"
#include "vend.h"
#include "log.h"
#include "API.h"
#include "schedule.h"

static WIEGAND wg;
static byte mac[] = {0x90, 0xa2, 0xda, 0x0d, 0x7c, 0x9a};
static volatile unsigned char sold_out_t = 0;
static volatile char sold_out_changed = 0;
static volatile unsigned long sold_out_changed_at = 0;
unsigned char sold_out = 0;

ISR(PCINT2_vect)
	{
	sold_out_t = PINK;
	sold_out_changed = 1;
	sold_out_changed_at = millis();
	}

char handle_ethernet(void *ptr, unsigned long *t, unsigned long m)
	{
	Ethernet.maintain();

	return SCHEDULE_REDO;
	}

void setup()
	{
	char kPath[] = "/vendtest";

	cli();
	PCICR |= 1 << PCIE2;
	PCMSK2 = 0xFF;
	sold_out = PINK;
	sei();

	log_begin(115200);
	log_msg("Hive13 Vending Arduino Shield v.04");
	leds_init();
	vend_init();
	log_msg("Initializing Ethernet Controller.");
	while (Ethernet.begin(mac) != 1)
		{
		log_msg("Error obtaining DHCP address.  Let's wait a second and try again.");
		delay(1000);
		}
	schedule(0, handle_ethernet, NULL);
	
	wg.begin();
	soda_temp_init();
	}

void loop()
	{
	unsigned long code;

	run_schedule();
	if (sold_out_changed && sold_out_changed_at < millis() - 250)
		{
		sold_out = sold_out_t;
		log_msg("Sold out: %02hhX", sold_out);
		sold_out_changed = 0;
		}
	
	if (wg.available())
		{
		code = wg.getCode();
		log_msg("Scanned badge %lu/0x%lX, type W%d", code, code, wg.getWiegandType());
		handle_vend(code);
		}
	}

/* vim:set filetype=c: */
