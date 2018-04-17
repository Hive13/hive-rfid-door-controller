#include "log.h"
#include "schedule.h"
#include "network.h"

#define SAMPLE_COUNT    140
volatile unsigned short samples[SAMPLE_COUNT];

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
	rms = ((unsigned long)sqrt(sum / SAMPLE_COUNT)) * 40;
	log_msg("Measured Current: %lu.%03luA", rms / 1000, rms % 1000);
	*t = m + 1000;
	return SCHEDULE_REDO;
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
	}

void loop(void)
	{
	run_schedule();
	}

/* vim:set filetype=c: */
