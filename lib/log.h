#ifndef __LOG_H
#define __LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#if 1
#define log_begin(x) _log_begin(x)
#define log_msg(x, ...) _log_msg(__LINE__, __FILE__, x, ##__VA_ARGS__)
#else
#define log_begin()
#define log_msg(x, ...)
#endif

void _log_begin(unsigned long baud);
void _log_msg(unsigned short line, char *file, char *format, ...);
#ifdef __cplusplus
};
#endif

#endif /* __LOG_H */
