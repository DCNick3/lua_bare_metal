#include <arch/i386/peripheral.h>
#include <arch/i386/gdt.h>
#include <string.h>
#include <stdio.h>
#include <tty.h>

typedef struct {
	int bcd;
	int twentyFourHours;
} cmos_time_format_t;

static void cmos_get_time_format(cmos_time_format_t* f)
{
	uint8_t v = cmos_get(0x0B);
	f->bcd = !((v & 0x04) != 0);
	f->twentyFourHours = (v & 0x02) != 0;
}

static uint8_t cmos_time_tobin(cmos_time_format_t f, uint8_t v)
{
	if (f.bcd)
	{
		return (v & 0x0F) + ((v & 0xF0) >> 4) * 10;
	}
	else
	{
		return v;
	}
}

static void cmos_get_time_raw(cmos_time_t *t)
{
	while ((cmos_get(0x0A) & (1 << 7))) {} /* wait for 'Update in progress' flag being clear */
	
	cmos_time_format_t f;
	cmos_get_time_format(&f);
	
	t->secounds = cmos_time_tobin(f, cmos_get(0x00));
	t->minutes = cmos_time_tobin(f, cmos_get(0x02));
	
	t->hours = cmos_get(0x04);
	int isPm = t->hours & 0x80;
	if (isPm)
	{
		t->hours = t->hours & 0x7F;
	}
	t->hours = cmos_time_tobin(f, t->hours);
	if (!f.twentyFourHours && isPm)
	{
		t->hours = (t->hours + 12) % 24; /* TODO: possibly fix it... */
	}
	
	t->weekday = cmos_time_tobin(f, cmos_get(0x06));
	t->day = cmos_time_tobin(f, cmos_get(0x07));
	t->month = cmos_time_tobin(f, cmos_get(0x08));
	t->year = cmos_time_tobin(f, cmos_get(0x09));
	t->century = cmos_time_tobin(f, cmos_get(0x32));
}

void cmos_get_time(cmos_time_t *t)
{
	cmos_time_t t1 = {0}, t2 = {0};
	do 
	{
		cmos_get_time_raw(&t1);
		cmos_get_time_raw(&t2);
	} while (memcmp(&t1, &t2, sizeof(cmos_time_t)) != 0);
	*t = t1;
}


void init_peripheral()
{
	init_descriptor_tables();
	pic_init();
	kb_init();
	serial_init();
	timer_init();
    isa_dma_init();
    floppy_init();
}

void isr_handler(uint8_t err, uint8_t isr)
{
	if (isr < 0x20)
	{
		sti();
		tty_print("FAULT");
		printf("fault %02x:%02x", isr, err);
		while (1) {}
	} 
	else if (isr < 0x30)
	{
		process_irq(isr-0x20);
	}
	else
	{
		//printf("isr-%02x:%02x\n", isr, err);
	}
}

