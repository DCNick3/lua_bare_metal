#include <arch/i386/peripheral.h>
#include <stdio.h>

void process_irq(uint8_t id)
{
	if (id >= 8)
		outb(PIC2_CMD, PIC_EOI);
	outb(PIC1_CMD, PIC_EOI);

	if (id == 0)
	{
		timer_isr();
	}

	if (id == 1)
	{
		kb_byte(inb(0x60));
	}

	if (id == 3 || id == 4)
	{
		serial_isr();
	}
	
	//printf("irq%d\n", id);
}
