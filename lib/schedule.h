#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#define SCHEDULE_DONE 0
#define SCHEDULE_REDO 1

typedef char (time_handler)(void *, unsigned long *, unsigned long);
struct task
	{
	struct task   *prev, *next;
	unsigned long time;
	time_handler  *func;
	void          *data;
	};

void schedule(unsigned long time, time_handler *func, void *ptr);
void run_schedule(void);

#endif /* __SCHEDULE_H */
