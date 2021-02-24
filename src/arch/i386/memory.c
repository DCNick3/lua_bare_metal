#include <memory.h>
#include <abort.h>
#include <string.h>
#include <stdint.h>
#include <multiboot.h>

static size_t block_size = 128; // This will allow us to use 111.875 MiB

#define MEM_NULL (SIZE_MAX)

struct __attribute__((packed)) memory_block_entry {
	void *ptr;
};
typedef struct memory_block_entry memory_block_entry_t;

static memory_block_entry_t *memmap;
static size_t memory_sz;
static char *memory_min;

size_t memory_init(multiboot_info_t* mboot)
{
	memory_min = (void*)(0x01000000 + 4 * 1024 * 1024);
	size_t sz = (mboot->mem_upper - 1024 * 15) * 1024;
	sz = sz - (sz % block_size);
	//char *memory_max = memory_min + sz;
	sz /= block_size;
	
	memory_block_entry_t* mem_info = (memory_block_entry_t*)0x01000000;
	memory_block_entry_t* mem_info_end = mem_info + sz;
	
	if ((char*)mem_info_end > (char*)memory_min)
	{
		sz = memory_min - (char*)mem_info;
		sz = sz / sizeof(memory_block_entry_t);
		// Reduce memory size for where we can have enough meminfo
	}
	
	memmap = mem_info;
	memory_sz = sz;
	
	for (size_t i = 0; i < sz; i++)
	{
		//memory_block_entry_t* nxt = (i < sz-1) ? &memmap[i + 1] : NULL;
		memmap[i] = (memory_block_entry_t){
			NULL,
		};
	}
	
	return sz * block_size;
}

void *malloc(size_t sz)
{
	if (sz == 0)
		return NULL;
	
	size_t blk_sz = sz / block_size;
	if (sz % block_size != 0)
		blk_sz++;
	
	size_t sp = 0;
	size_t ssz = 0;
	size_t p = 0;
	while (p < memory_sz)
	{
		if (memmap[p].ptr == NULL)
			ssz++;
		else
		{
			sp = p + 1;
			ssz = 0;
		}
		if (ssz == blk_sz)
		{
			void *ptr = memory_min + sp * block_size;
			for (size_t i = sp; i < sp + blk_sz; i++)
			{
				memmap[i].ptr = ptr;
			}
			return ptr;
		}
		p++;
	}
	return NULL;
}

void *calloc(size_t number, size_t size)
{
	return malloc(number*size);
}

void *realloc(void *ptr, size_t sz)
{
	if (ptr == NULL)
		return malloc(sz);
	
	if (sz == 0)
	{
		free(ptr);
		return NULL;
	}
	
	char* p = ptr;
	if (p < memory_min)
		panic("Attempt to realloc invalid pointer");
	size_t off = (size_t)(p - memory_min);
	if (off % block_size != 0)
		panic("Attempt to realloc invalid pointer");
	
	off /= block_size;
	
	
	size_t ns = sz / block_size;
	if (sz % block_size != 0)
		ns++;
	
	size_t os = 0;
	{
		size_t o = off;
		while (o < memory_sz && memmap[o].ptr == ptr)
		{
			o++;
			os++;
		}
	}
	
	if (os == ns)
		return ptr;
	if (os > ns)
	{
		size_t o = off + os - ns;
		while (o < memory_sz && memmap[o].ptr == ptr)
		{
			o++;
			memmap[o].ptr = NULL;
		}
		return ptr;
	}
	else
	{
		size_t as = ns - os;
		size_t o = off + os;
		size_t hs = 0;
		while (o < memory_sz && memmap[o].ptr == NULL)
		{
			o++;
			hs++;
			if (hs == as)
				break;
		}
		if (hs != as)
		{
			void* np = malloc(ns * block_size);
			memcpy(np, ptr, ns * block_size);
			free(ptr);
			return np;
		}
		else
		{
			for (size_t i = off + os; i < off + ns; i++)
			{
				memmap[i].ptr = ptr;
			}
			return ptr;
		}
	}
}

void free(void *ptr)
{
	if (ptr == NULL)
		return;
	
	char* p = ptr;
	if (p < memory_min)
		panic("Attempt to free invalid pointer");
	size_t off = (size_t)(p - memory_min);
	if (off % block_size != 0)
		panic("Attempt to free invalid pointer");
	
	off /= block_size;
	while (off < memory_sz && memmap[off].ptr == ptr)
	{
		memmap[off].ptr = NULL;
		off++;
	}
}

size_t memory_used(void)
{
	size_t r = 0;
	for (size_t o = 0; o < memory_sz; o++)
	{
		if (memmap[o].ptr != NULL)
			r++;
	}
	return r * block_size;
}

size_t memory_free(void)
{
	return memory_sz * block_size - memory_used();
}
