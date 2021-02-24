#ifndef _PERIPHERAL_H
#define _PERIPHERAL_H

#include <procio.h>
#include <stdint.h>
#include <stdlib.h>

static inline void cli(void)
{
	asm volatile ("cli");
}

static inline void sti(void)
{
	asm volatile ("sti");
}

typedef struct {
	uint8_t secounds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t weekday;
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t century;
} cmos_time_t;

void cmos_get_time(cmos_time_t *t);

#define cmos_select(p) outb(0x70, p)

static inline uint8_t cmos_get(uint8_t reg)
{
	cmos_select(reg);
	//io_wait();
	return inb(0x71);
}

static inline void cmos_set(uint8_t reg, uint8_t value)
{
	cmos_select(reg);
	//io_wait();
	outb(0x71, value);
}

void pic_init();
void process_irq(uint8_t id);

void kb_init();
void kb_byte(uint8_t byte);

void serial_init();
size_t serial_write(const uint8_t *ptr, size_t n);
size_t serial_read(uint8_t *ptr, size_t n);
void serial_isr();

void timer_init();
void timer_isr();
uint64_t timer_ticks();

void isa_dma_init();
void floppy_init();

#define PIC1 0x20
#define PIC2 0xA0

#define PIC1_CMD PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_CMD PIC2
#define PIC2_DATA (PIC2+1)


 
#define ICW1_ICW4		0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE		0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL		0x08		/* Level triggered (edge) mode */
#define ICW1_INIT		0x10		/* Initialization - required! */
 
#define ICW4_8086		0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO		0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM		0x10		/* Special fully nested (not) */

#define PIC_EOI 0x20

#define PIC1_OFF 0x20
#define PIC2_OFF 0x28



#endif
