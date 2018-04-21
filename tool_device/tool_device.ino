#include <LiquidCrystal.h>
#include "log.h"
#include "schedule.h"
#include "lcd.h"

#define SAMPLE_COUNT    140
volatile unsigned short samples[SAMPLE_COUNT];

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
	rms = ((unsigned long)sqrt(sum / SAMPLE_COUNT)) * 40;
	log_msg("Measured Current: %lu.%03luA", rms / 1000, rms % 1000);
	*t = m + 1000;
	return SCHEDULE_REDO;
	}

static signed char pos = 0;

/*                      _______         _______        
            Pin1 ______|       |_______|       |______ Pin1 
  negative <---     _______         _______         __      --> positive
            Pin2 __|       |_______|       |_______|   Pin2


  new  new  old  old 
  pin2 pin1 pin2 pin1 Result 
  ---- ---- ---- ---- ------ 
  0    0    0    0    no movement
  0    0    0    1    +1
  0    0    1    0    -1
  0    0    1    1    +2 (assume pin1 edges only)
  0    1    0    0    -1
  0    1    0    1    no movement 
  0    1    1    0    -2 (assume pin1 edges only)
  0    1    1    1    +1
  1    0    0    0    +1
  1    0    0    1    -2 (assume pin1 edges only)
  1    0    1    0    no movement
  1    0    1    1    -1
  1    1    0    0    +2 (assume pin1 edges only)
  1    1    0    1    -1
  1    1    1    0    +1
  1    1    1    1    no movement
*/


#define encoder0PinA  2
#define encoder0PinB  3

void doEncoderA()
	{
	if (digitalRead(encoder0PinA) == HIGH)
		{
		if (digitalRead(encoder0PinB) == LOW)
			pos++;
    else
			pos--;
		}
  else
		{
		if (digitalRead(encoder0PinB) == HIGH)
			pos++;
		else
			pos--;	
		}
	}

void doEncoderB()
	{
	if (digitalRead(encoder0PinB) == HIGH)
		{
		if (digitalRead(encoder0PinA) == HIGH)
			pos++;
		else
			pos--;
		}
	else
		{
		if (digitalRead(encoder0PinA) == LOW)
			pos++;
		else
			pos--;
		}
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
	lcd.print("Moo?");
	attachInterrupt(0, doEncoderA, CHANGE);
	attachInterrupt(1, doEncoderB, CHANGE);

	}

void loop(void)
	{
	static signed char shown_pos = 1;
	run_schedule();
	if (pos != shown_pos)
		{
		lcd.clear();
		lcd.print("Sw: ");
		lcd.print(pos, DEC);
		shown_pos = pos;
		}
	}

/* vim:set filetype=c: */
