#include <arch/i386/peripheral.h>
#include <procio.h>

uint16_t pic_get_mask()
{
	return inb(PIC1_DATA) | (inb(PIC2_DATA) << 8);
}

void pic_set_mask(uint16_t mask)
{
	outb(PIC1_DATA, mask & 0xFF);
	outb(PIC2_DATA, (mask & 0xFF00 ) >> 8);
}

void pic_set_bit(uint8_t id)
{
	pic_set_mask(pic_get_mask() | (1 << id));
}

void pic_reset_bit(uint8_t id)
{
	pic_set_mask(pic_get_mask() & ~(1 << id));
}

void pic_init()
{
	uint8_t a1, a2;
 
	a1 = inb(PIC1_DATA);                        // save masks
	a2 = inb(PIC2_DATA);
 
	outb(PIC1_CMD, ICW1_INIT+ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_CMD, ICW1_INIT+ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, PIC1_OFF);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(PIC2_DATA, PIC2_OFF);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
 
	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();
 
	outb(PIC1_DATA, a1);   // restore saved masks.
	io_wait();
	outb(PIC2_DATA, a2);
	io_wait();

	pic_set_mask(0xFFE0);
	sti();
}
