#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>

int printf(const char *format, ...);
//int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);

#include <stdarg.h>

int vprintf(const char *format, va_list ap);
//int vfprintf(FILE *stream, const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list ap); 

typedef void FILE;

#define stdout ((FILE*)1)
#define stderr ((FILE*)1)
#define stdin  ((FILE*)2)
#define EOF -1

size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream );
int fprintf(FILE* stream, const char *format, ...);
int fflush(FILE* stream);
int fread(void* ptr, size_t size, size_t count, FILE* stream);

#endif