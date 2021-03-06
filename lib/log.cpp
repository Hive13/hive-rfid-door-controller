#include <Arduino.h>

#include "log.h"

void _log_begin(unsigned long baud)
	{
	Serial.begin(baud);
	}

void _log_msg(unsigned short line, char *file, char *format, ...)
	{
	char message[1024];
	va_list vargs;
	int i;

	i = snprintf(message, sizeof(message), "%s:%hu - ", file, line);

	if (i < 0)
		return;

	va_start(vargs, format);
	vsnprintf(message + i, sizeof(message) - i, format, vargs);
	va_end(vargs);

	Serial.println(message);
	}

void _log_progress_start(unsigned short line, char *file, char *format, ...)
	{
	char message[1024];
	va_list vargs;
	int i;

	i = snprintf(message, sizeof(message), "%s:%hu - ", file, line);

	if (i < 0)
		return;

	va_start(vargs, format);
	vsnprintf(message + i, sizeof(message) - i, format, vargs);
	va_end(vargs);

	Serial.print(message);
	}

void _log_progress_end(unsigned short line, char *file, char *format, ...)
	{
	char message[1024];
	va_list vargs;

	va_start(vargs, format);
	vsnprintf(message, sizeof(message), format, vargs);
	va_end(vargs);

	Serial.println(message);
	}

void _log_progress(unsigned short line, char *file, char *format, ...)
	{
	char message[1024];
	va_list vargs;

	va_start(vargs, format);
	vsnprintf(message, sizeof(message), format, vargs);
	va_end(vargs);

	Serial.print(message);
	}
