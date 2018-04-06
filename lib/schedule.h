#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#define SCHEDULE_DONE 0
#define SCHEDULE_REDO 1

#define STATUS_DEAD 0
#define STATUS_SCHEDULED 1
#define STATUS_RUNNING 2
#define STATUS_DELETED 3

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
	unsigned char status;
	};

void schedule_cancel(void *ptr);
void *schedule(unsigned long time, time_handler *func, void *ptr);
void run_schedule(void);

#ifdef __cplusplus
}
#endif

#endif /* __SCHEDULE_H */
