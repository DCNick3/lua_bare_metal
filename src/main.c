#include <multiboot.h>
#include <tty.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <setjmp.h>
#include <time.h>
#include <abort.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <lua_files.h>

#include <arch/i386/peripheral.h>

void dummy(jmp_buf env)
{
	longjmp(env, 100);
}

void init_peripheral();

void kernel_main(unsigned long magic, unsigned long addr)
{
	multiboot_info_t *mbi;
	
	tty_init();
	init_peripheral();

	
	
	printf("Hello world!\n");
	
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		tty_print("Invalid bootloader magic!");
		return;
	}
	
	mbi = (multiboot_info_t*)addr;
	printf("flags: %08X; lo-mem: %dKiB; hi-mem: %dKiB\n", mbi->flags, mbi->mem_lower, mbi->mem_upper);
	/*tty_print_num(mbi->flags, 16);
	tty_print("; lo-mem: ");
	tty_print_num(mbi->mem_lower, 10);
	tty_print("KiB; hi-mem: ");
	tty_print_num(mbi->mem_upper, 10);
	tty_print("KiB\n");*/
	
	printf("Init_mem\n");
	size_t sz = memory_init(mbi);
	printf("Heap is %d KiB\n", sz / 1024);
	
	printf("================\n");
	
	printf("Test setjmp\n");
	
	jmp_buf env;
	
	int v;
	if ((v = setjmp(env)) != 0)
	{
		printf("Dummy passed: %d\n", v);
	}
	else
		dummy(env);
	
	printf("I'm ok!\n");
	
	printf("Creating lua state\n");
	
	lua_State* L = luaL_newstate();
	
	printf("Am I still alive?\nOpening libs\n");
	
	luaL_openlibs(L);
	
	printf("Am I REALLY still alive?\nTrying to load main chunk\n");
	
	luaL_loadbuffer(L, lua_main, lua_main_size, "@main.lua");
	
	printf("Oh god, i'm calling it\n");
	
	lua_call(L, 0, 0);
	
	printf("Lua-goodbye\n");
	
	/*
	tty_print("Try realloc 128, 129, 1 B\n");
	void *d1 = realloc(NULL, 128), *d2 = realloc(NULL, 129), *d3 = realloc(NULL, 1);
	tty_print("Values: ");
	tty_print_num((int)d1, 16);
	tty_print(", ");
	tty_print_num((int)d2, 16);
	tty_print(", ");
	tty_print_num((int)d3, 16);
	tty_print("\n");
	
	tty_print("change size of d1 to 129: ");
	d1 = realloc(d1, 129);
	tty_print_num((int)d1, 16);
	tty_print("\n");
	
	tty_print("memset it!");
	memset(d1, 'A', 128);
	((char*)d1)[128] = '\0';
	tty_print(d1);
	tty_print("\n");
	
	tty_print("Used/Free: ");
	tty_print_num(memory_used(), 10);
	tty_print("/");
	tty_print_num(memory_free(), 10);
	tty_print("\n");
	*/
	
	//__asm__ __volatile__ ("sti");
	
	printf("ticks: %llu\n", timer_ticks());
	
	while (1) {}
}
