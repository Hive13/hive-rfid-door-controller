#include <Wiegand.h>
#include "cJSON.h"

#define BEEP_PIN D0
#define D0_PIN D1
#define D1_PIN D2

static WIEGAND wg;

void setup(void)
	{
	digitalWrite(BEEP_PIN, LOW);
	pinMode(BEEP_PIN, OUTPUT);

	wg.begin(D0_PIN, D0_PIN, D1_PIN, D1_PIN);
	Serial.begin(9600);

	delay(500);
	digitalWrite(BEEP_PIN, HIGH);
	delay(100);
	digitalWrite(BEEP_PIN, LOW);
	Serial.println("Ready to rumble!");
	}

void loop(void)
	{
	unsigned long code;
	char buf[255];

	delay(100);
	if (wg.available())
		{
		code = wg.getCode();

		snprintf(buf, 255, "Scanned badge %lu/0x%lX, tpe W%d", code, code, wg.getWiegandType());

		Serial.println(buf);
		}
	}
