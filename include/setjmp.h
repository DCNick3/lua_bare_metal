#ifndef _SETJMP_H
#define _SETJMP_H

#include <stdint.h>

typedef struct 
#ifdef __i386__
__attribute__((packed))
#endif
{
#ifdef __i386__
	uint32_t d[6];
#endif
} jmp_buf[1];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int value) __attribute__ ((noreturn));

#endif