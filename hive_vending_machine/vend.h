#ifndef __VEND_H
#define __VEND_H

#ifdef __cplusplus
extern "C" {
#endif

void set_vend(char c);
void do_random_vend(void);
void do_vend(void);
void vend_check(void);
void vend_init(void);

#ifdef __cplusplus
};
#endif

#define RANDOM_SODA_NUMBER 4
#define SODA_COUNT (sizeof(sodaButtons) / sizeof(sodaButtons[0]))

#endif /* __VEND_H */
