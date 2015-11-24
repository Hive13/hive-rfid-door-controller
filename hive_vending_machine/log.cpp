#include <Arduino.h>

#include "log.h"

#define BAUD 57600

void _log_begin(void)
	{
	Serial.begin(BAUD);
	}

void _log_msg(char *format, ...)
	{
	char message[256];
	va_list vargs;

	va_start(vargs, format);
	vsnprintf(message, sizeof(message), format, vargs);
	va_end(vargs);

	Serial.println(message);
	}
