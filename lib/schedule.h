#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#define SCHEDULE_DONE 0
#define SCHEDULE_REDO 1

#ifdef __cplusplus
extern "C" {
#endif

typedef char (time_handler)(void *, unsigned long *, unsigned long);
struct task
	{
	struct task   *prev, *next;
	unsigned long time;
	time_handler  *func;
	void          *data;
	};

void schedule_cancel(void *ptr);
void *schedule(unsigned long time, time_handler *func, void *ptr);
void run_schedule(void);

#ifdef __cplusplus
}
#endif

#endif /* __SCHEDULE_H */
