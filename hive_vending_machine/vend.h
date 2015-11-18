#ifndef __VEND_H
#define __VEND_H

void set_vend(char c);
void do_random_vend(void);
void do_vend(void);
void vend_check(void);
void vend_init(void);

#define RANDOM_SODA_NUMBER 4
#define SODA_COUNT (sizeof(sodaButtons) / sizeof(sodaButtons[0]))

#endif /* __VEND_H */
