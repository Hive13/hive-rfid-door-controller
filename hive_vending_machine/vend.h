#ifndef __VEND_H
#define __VEND_H

#ifdef __cplusplus
extern "C" {
#endif

struct soda
	{
	unsigned char switch_pin;
	unsigned char relay_pin;
	unsigned char type;
	};

void set_vend(char c);
void do_random_vend(unsigned char kind);
char vend_check(void *ptr, unsigned long *t, unsigned long m);
void vend_init(void);
void handle_vend(unsigned long code);

#ifdef __cplusplus
};
#endif

#define RANDOM_SODA_NUMBER 4
#define MIN_DEBOUNCE_COUNT 100

#define KIND_ANY     0
#define KIND_REGULAR 1
#define KIND_DIET    2
#define KIND_WATER   3
#define KIND_BEER    4

#endif /* __VEND_H */
