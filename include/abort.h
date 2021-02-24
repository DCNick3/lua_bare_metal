#ifndef _ABORT_H
#define _ABORT_H

void _abort(char* file, int line) __attribute__ ((noreturn));
void _panic(char* file, int line, char* msg) __attribute__ ((noreturn));
void _panicf(char* msg, ...) __attribute__ ((noreturn));

#define abort() _abort(__FILE__, __LINE__)
#define panic(msg) _panic(__FILE__, __LINE__, msg)
#define panicf(msg, ...) _panicf("! PANIC: " msg " at %s:%d !", __VA_ARGS__, __FILE__, __LINE__)

#endif