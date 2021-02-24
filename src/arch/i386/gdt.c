#include <stdint.h>
#include <string.h>

#include <arch/i386/gdt.h>

/* Do lgdt */
extern void gdt_flush(uint32_t);
extern void idt_flush(uint32_t);

static void init_gdt();
static void gdt_set_gate(uint32_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

static void init_idt();
static void idt_set_gate(uint8_t i, uint32_t offset, uint16_t sel, uint8_t flags);

#define GDT_SIZE 5
#define IDT_SIZE 256

gdt_entry_t gdt_entries[GDT_SIZE];
gdt_ptr_t gdt_ptr;
idt_entry_t idt_entries[IDT_SIZE];
idt_ptr_t idt_ptr;

extern uint32_t isrns;

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

void init_descriptor_tables()
{
	init_gdt();
	init_idt();
}

static void init_gdt()
{
	gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_SIZE) - 1;
	gdt_ptr.base = (uint32_t)&gdt_entries;

	memset(gdt_entries, 0, sizeof(gdt_entries));

	gdt_set_gate(0, 0, 0, 0, 0); // null segment, not used
	gdt_set_gate(1, 0x00, 0xFFFFFFFF, 0x9A, 0xCF); // code
	gdt_set_gate(2, 0x00, 0xFFFFFFFF, 0x92, 0xCF); //data
	

	gdt_flush((uint32_t)&gdt_ptr);
}

static void gdt_set_gate(uint32_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
	gdt_entry_t *e = &gdt_entries[i];

	e->limit_low = limit & 0xFFFF;
	e->base_low = base & 0xFFFF;

	e->base_middle = (base & 0x00FF0000) >> 16;
	e->granularity = (gran & 0xF0) | ((limit & 0xF0000) >> 16);
	e->base_high = (base & 0xFF000000) >> 24;

	e->access = access;
}

static void init_idt()
{
	idt_ptr.limit = (sizeof(idt_entry_t) * IDT_SIZE) - 1;
	idt_ptr.base = (uint32_t)&idt_entries;

	memset(idt_entries, 0, sizeof(idt_entries));

	for (int i = 0; i < 256; i++)
	{
		idt_set_gate(i, *((&isrns) + i), 0x08, 0x8E);
	}

	idt_flush((uint32_t)&idt_ptr);
}

static void idt_set_gate(uint8_t i, uint32_t offset, uint16_t sel, uint8_t flags)
{
	idt_entry_t *e = &idt_entries[i];
	
	e->offset_low = offset & 0xFFFF;
	e->selector = sel;
	e->zero = 0;
	e->flags = flags;
	e->offset_high = (offset & 0xFFFF0000) >> 16;
}

