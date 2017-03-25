#include <Ethernet.h>
// https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino
#include <Wiegand.h>
#include <SPI.h>
// The b64 and HttpClient libraries are both in this repository:
// https://github.com/amcewen/HttpClient
#include <b64.h>
#include <HttpClient.h>
// https://github.com/adafruit/Adafruit-WS2801-Library
#include <Adafruit_WS2801.h>
// https://github.com/PaulStoffregen/OneWire
#include <OneWire.h>

#include "leds.h"
#include "temp.h"
#include "vend.h"
#include "log.h"
#include "sha256.h"

static WIEGAND wg;
static byte mac[] = {0x90, 0xa2, 0xda, 0x0d, 0x7c, 0x9a};
static volatile unsigned char sold_out_t = 0;
static volatile char sold_out_changed = 0;
static volatile unsigned long sold_out_changed_at = 0;
unsigned char sold_out = 0;
void hmac_test(void);

ISR(PCINT2_vect)
	{
	sold_out_t = PINK;
	sold_out_changed = 1;
	sold_out_changed_at = millis();
	}

void setup()
	{
	char kPath[] = "/vendtest";

	cli();
	PCICR |= 1 << PCIE2;
	PCMSK2 = 0xFF;
	sold_out = PINK;
	sei();

	log_begin();
	log_msg("Hive13 Vending Arduino Shield v.04");
	leds_init();
	vend_init();
	log_msg("Initializing Ethernet Controller.");
	while (Ethernet.begin(mac) != 1)
		{
		log_msg("Error obtaining DHCP address.  Let's wait a second and try again.");
		delay(1000);
		}
	
	wg.begin();
	temperature_init();
	}

void loop()
	{
	char host_path[255];
	unsigned long code;
	int err;

	handle_temperature();
	if (sold_out_changed && sold_out_changed_at < millis() - 250)
		{
		sold_out = sold_out_t;
		log_msg("Sold out: %02hhX", sold_out);
		sold_out_changed = 0;
		}
	
	if(wg.available())
		{
		code = wg.getCode();
		log_msg("Scanned badge %lu/0x%lX, type W%d", code, code, wg.getWiegandType());

		if (can_vend(code))
			do_vend();
		else
			log_msg("Didn't receive the OK to vend...");
		}
	
	vend_check();
	}

/* vim:set filetype=c: */
