#include <LiquidCrystal.h>
#include "log.h"
#include "schedule.h"
#include "lcd.h"

#define SAMPLE_COUNT    140
volatile unsigned short samples[SAMPLE_COUNT];
unsigned short current;

LiquidCrystal lcd(8, 9, 10, 4, 5, 6, 7);

ISR(ADC_vect)
	{
	static unsigned char at = 0;

	samples[at++] = ADC;
	if (at >= SAMPLE_COUNT)
		at = 0;
	}

ISR(TIMER1_COMPA_vect)
	{
	ADCSRA |= (1 << ADSC);
	}

char display_current(void *ptr, unsigned long *t, unsigned long m)
	{
	unsigned long rms, sum = 0;
	unsigned char i;
	signed long scaled;

	for (i = 0; i < SAMPLE_COUNT; i++)
		{
		scaled = ((signed long)samples[i]) - 512;
		sum += (scaled * scaled);
		}
	rms = (((unsigned long)sqrt(sum / SAMPLE_COUNT)) * 4) / 10;
	current = (unsigned short)(rms & 0xffff);
	*t = m + 1000;
	return SCHEDULE_REDO;
	}

static volatile signed char pos = 0;

static volatile unsigned long pin_a = 0;
static volatile unsigned long pin_b = 0;
#define SEPARATION 35
#define SPACING    50

ISR(INT0_vect)
	{
	unsigned long m = millis();

	if (m - pin_a < SPACING)
		return;

	pin_a = m;

	if (pin_a - pin_b < SEPARATION)
		pos--;
	}

ISR(INT1_vect)
	{
	unsigned long m = millis();

	if (m - pin_b < SPACING)
		return;

	pin_b = m;

	if (pin_b - pin_a < SEPARATION)
		pos++;
	}

void setup(void)
	{
	unsigned char i, oldREG;
	log_begin(115200);

	oldREG = SREG;
	cli();
	for (i = 0; i < SAMPLE_COUNT; i++)
		samples[i] = 512;
	ADMUX  = 0;
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	OCR1A  = (F_CPU / (SAMPLE_COUNT * 8)) - 1;
	TCCR1A = 0;
	TCCR1B = (1 < WGM12) | (1 < CS11);
	TIMSK1 = (1 << OCIE1A);
	SREG = oldREG;

	schedule(0, display_current, NULL);
	log_msg("starting");

	lcd.begin(40, 2);
	lcd.print("Moo??");
	DDRD  &= 0xF3;
	PORTD |= 0x0C;
	EICRA = (1 << ISC11) | (1 << ISC01);
	EIMSK = (1 << INT0) | (1 << INT1);
	}

void loop(void)
	{
	char buf[30];
	static signed char shown_pos = 1;
	static unsigned char shown_current = 1;
	run_schedule();
	if (pos != shown_pos || current != shown_current)
		{
		sprintf(buf, "Sw: %hhi", pos);
		lcd.clear();
		lcd.print(buf);
		sprintf(buf, "Current: %hu.%huA", current / 10, current % 10);
		lcd.setCursor(0, 1);
		lcd.print(buf);
		shown_pos = pos;
		shown_current = current;
		}
	}

/* vim:set filetype=c: */
