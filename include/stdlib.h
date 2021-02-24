#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

#include <abort.h>

int abs(int n);
double strtod(const char* str, char** endptr);

#ifndef _MEMORY_H
void *malloc(size_t sz);
void *calloc(size_t number, size_t size);
void *realloc(void *ptr, size_t sz);
void free(void *ptr);
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void exit(int status) __attribute__ ((noreturn));

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) < (b) ? (b) : (a))

#endif