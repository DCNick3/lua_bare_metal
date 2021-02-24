#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *str1, const void *str2, size_t n);
void *memset(void *str, int c, size_t n);
void *memchr(const void *s, int c, size_t n);

size_t strlen(const char *str);
char *strchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
int strcoll(const char *s1, const char *s2);
char *strpbrk(const char *str1, const char *str2);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
size_t strspn(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t n);
char *strerror(int errnum);
char *strstr(const char *haystack, const char *needle);

// Non-standart staff...
void strrev(char *str);
int itoa(uintmax_t num, char* str, int len, int base, char hex_base);

#endif