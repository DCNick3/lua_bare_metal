#include <arch/i386/peripheral.h>
#include <procio.h>
#include <stdint.h>

static volatile uint64_t counter;

#define PORT 0x40
#define PORT_C0 (PORT+0)
#define PORT_C1 (PORT+1)
#define PORT_C2 (PORT+2)
#define PORT_MC (PORT+3)

#define DIVIDER 1193
#define NSPT 999847

void floppy_timer();

void timer_init()
{
	outb(PORT_MC, 0x36);
	io_wait();
	outb(PORT_C0, DIVIDER & 0xFF);
	io_wait();
	outb(PORT_C0, (DIVIDER >> 8) & 0xFF);
	io_wait();
}

void timer_isr()
{
	counter++;
	floppy_timer();
}

uint64_t timer_ticks()
{
	cli();
	uint64_t c = counter;
	sti();
	return c;
}

