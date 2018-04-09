#include <Arduino.h>

#include "schedule.h"

static struct task *task_chain = NULL;

#ifdef ARDUINO_AVR_MEGA2560
#define STOP_ISRS() oldREG = SREG; cli()
#define START_ISRS() SREG = oldREG
#else
#define STOP_ISRS() noInterrupts()
#define START_ISRS() interrupts()
#endif

void schedule_delete(struct task *t)
	{
	if (t->prev)
		t->prev->next = t->next;
	else
		task_chain = t->next;
	if (t->next)
		t->next->prev = t->prev;
	free(t);
	}

void schedule_cancel(void *ptr)
	{
	struct task *t = task_chain;
#ifdef ARDUINO_AVR_MEGA2560
	unsigned int oldREG;
#endif

	STOP_ISRS();
	while (t)
		{
		if (t == ptr)
			{
			if (t->status == STATUS_RUNNING)
				t->status = STATUS_DELETED;
			else
				{
				schedule_delete(t);
				break;
				}
			}
		t = t->next;
		}
	START_ISRS();
	}

void *schedule(unsigned long time, time_handler *func, void *ptr)
	{
#ifdef ARDUINO_AVR_MEGA2560
	unsigned int oldREG;
#endif
	struct task *t = (struct task *)malloc(sizeof(struct task)), *walker = task_chain;

	t->next   = NULL;
	t->time   = time;
	t->func   = func;
	t->data   = ptr;
	t->status = STATUS_SCHEDULED;
	
	STOP_ISRS();
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
	
	START_ISRS();
	return t;
	}

void run_schedule(void)
	{
#ifdef ARDUINO_AVR_MEGA2560
	unsigned int oldREG;
#endif
	struct task *t = task_chain, *p;
	unsigned long m = millis();
	char ret;
	
	STOP_ISRS();
	while (t)
		{
		if (t->status == STATUS_DEAD || t->status == STATUS_DELETED)
			{
			p = t->next;
			schedule_delete(t);
			t = p;
			}
		else if (t->time <= m && t->status == STATUS_SCHEDULED)
			{
			t->status = STATUS_RUNNING;

			START_ISRS();
			ret = t->func(t->data, &t->time, m);
			STOP_ISRS();

			if (ret == SCHEDULE_REDO && t->status != STATUS_DELETED)
				{
				t->status = STATUS_SCHEDULED;
				t = t->next;
				continue;
				}

			p = t->next;
			schedule_delete(t);
			t = p;
			}
		else
			t = t->next;
		}
	START_ISRS();
	}
