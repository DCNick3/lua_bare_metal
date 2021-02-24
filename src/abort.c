#include <abort.h>
#include <stdio.h>

void _abort(char* file, int line)
{
	printf("! ABORT at '%s:%d' !\n", file, line);
	while (1) {}
}

void _panic(char* file, int line, char* msg)
{
	printf("! PANIC: %s at '%s:%d' !\n", msg, file, line);
	while (1) {}
}

void _panicf(char* msg, ...)
{
	va_list ap;
	
	va_start(ap, msg);
	vprintf(msg, ap);
	va_end(ap);
	
	while (1) {}
}