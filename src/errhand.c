#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "errhand.h"

void fatal_error(char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	printf("Fatal error: ");
	vprintf(fmt, argptr);
	va_end(argptr);
	exit(EXIT_FAILURE);
}
