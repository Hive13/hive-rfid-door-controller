#ifndef __LOG_H
#define __LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#if 1
#define log_begin() _log_begin()
#define log_msg(x, ...) _log_msg(x, ##__VA_ARGS__)
#else
#define log_begin()
#define log_msg(x, ...)
#endif

void _log_begin(void);
void _log_msg(char *format, ...);
#ifdef __cplusplus
};
#endif

#endif /* __LOG_H */
