#ifndef __VEND_H
#define __VEND_H

#ifdef __cplusplus
extern "C" {
#endif

struct soda
	{
	unsigned char switch_pin;
	unsigned char relay_pin;
	unsigned char diet;
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
#define SODA_COUNT (sizeof(sodas) / sizeof(sodas[0]))
#define VEND_PIN 7
#define BEEP_PIN 9
#define WIEGAND_LIGHT_PIN 8
#define MIN_DEBOUNCE_COUNT 100

#define KIND_ANY 0
#define KIND_REGULAR 1
#define KIND_DIET 2

#endif /* __VEND_H */
