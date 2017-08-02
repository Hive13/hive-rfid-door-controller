#include <Arduino.h>

#include "schedule.h"

static struct task *task_chain = NULL;

void schedule(unsigned long time, time_handler *func, void *ptr)
	{
#ifdef ARDUINO_AVR_MEGA2560
	unsigned int oldREG;
#endif
	struct task *t = (struct task *)malloc(sizeof(struct task)), *walker = task_chain;

	t->next = NULL;
	t->time = time;
	t->func = func;
	t->data = ptr;

#ifdef ARDUINO_AVR_MEGA2560
	oldREG = SREG;
	cli();
#else
	noInterrupts();
#endif


	if (!task_chain)
		{
		task_chain = t;
		t->prev = NULL;
		}
	else
		{
		while (walker->next)
			walker = walker->next;
		walker->next = t;
		t->prev = walker;
		}
	
#ifdef ARDUINO_AVR_MEGA2560
	SREG = oldREG;
#else
	interrupts();
#endif
	}

void run_schedule(void)
	{
	struct task *t = task_chain, *p;
	unsigned long m = millis();
	char ret;

	while (t)
		{
		if (t->time <= m)
			{
			ret = t->func(t->data, &t->time, m);

			if (ret == SCHEDULE_REDO)
				{
				t = t->next;
				continue;
				}

			if (t->prev)
				t->prev->next = t->next;
			else
				task_chain = t->next;
			if (t->next)
				t->next->prev = t->prev;
			p = t->next;
			free(t);
			t = p;
			}
		else
			t = t->next;
		}
	}
