#ifndef __LOG_H
#define __LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#if 1
#define log_begin(x)               _log_begin(x)
#define log_msg(x, ...)            _log_msg(__LINE__, __FILE__, x, ##__VA_ARGS__)
#define log_progress_start(x, ...) _log_progress_start(__LINE__, __FILE__, x, ##__VA_ARGS__)
#define log_progress_end(x, ...)   _log_progress_end(__LINE__, __FILE__, x, ##__VA_ARGS__)
#define log_progress(x, ...)       _log_progress(__LINE__, __FILE__, x, ##__VA_ARGS__)
#else
#define log_begin()
#define log_msg(x, ...)
#define log_progress_start(x, ...)
#define log_progress_end(x, ...)
#define log_progress(x, ...)
#endif

void _log_begin(unsigned long baud);
void _log_msg(unsigned short line, char *file, char *format, ...);
void _log_progress_start(unsigned short line, char *file, char *format, ...);
void _log_progress_end(unsigned short line, char *file, char *format, ...);
void _log_progress(unsigned short line, char *file, char *format, ...);
#ifdef __cplusplus
};
#endif

#endif /* __LOG_H */
