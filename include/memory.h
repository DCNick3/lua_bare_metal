#ifndef _MEMORY_H
#define _MEMORY_H

#include <stddef.h>
#include <multiboot.h>

#ifndef _STDLIB_H
void *malloc(size_t sz);
void *calloc(size_t number, size_t size);
void *realloc(void *ptr, size_t sz);
void free(void *ptr);
#endif

size_t memory_init(multiboot_info_t* mboot);

size_t memory_used(void);
size_t memory_free(void);

#endif