#include <arch/i386/peripheral.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <procio.h>

#define IBUF_SZ 1024
#define OBUF_SZ 1024

#define BAUD_RATE 1

uint16_t port = 0x3F8;

uint8_t ibuf[IBUF_SZ], obuf[OBUF_SZ];
uint16_t ibufp1 = 0, ibufp2 = 0, obufp1 = 0, obufp2 = 0;

void set_baud_rate(uint16_t divisor)
{
	uint8_t  d = inb(port+3);
	io_wait()	;
	outb(port+3, d | 0x80);
	io_wait();
	outb(port, (uint8_t)(divisor & 0xFF));
	io_wait();
	outb(port+1, (uint8_t)((divisor >> 8) & 0xFF));
	io_wait();
	outb(port+3, d & 0x7F);
	io_wait();
}

void serial_init()
{
	outb(port+1, 0x00);
	io_wait();
	set_baud_rate(BAUD_RATE);
	io_wait();
	outb(port+3, 0x03); //8N1	
	io_wait();
	outb(port+1, 0x03); // Enable DR, THRE
	io_wait();
}

void serial_disable()
{
	outb(port+1, 0x00);
	io_wait();
}

static int transmit = 0;

void serial_ib(uint8_t d)
{
	ibuf[ibufp2] = d;
	//printf("ib %x", d);

	ibufp2 = (ibufp2 + 1) % IBUF_SZ;
}

int serial_ob(uint8_t *d)
{
	if (obufp1 == obufp2)
		return 1;
	*d = obuf[obufp1];
	obufp1 = (obufp1 + 1) % OBUF_SZ;
	return 0;
}

size_t serial_write(const uint8_t *ptr, size_t n)
{
	if (n == 0)
		return 0;
	size_t bsz = OBUF_SZ / 2;
	size_t pos = 0;

	while (pos < n)
	{
		if (!transmit || (size_t)abs(obufp1 - obufp2) < bsz)
		{
			cli();

			size_t ad = min(bsz, n - pos);

			for (size_t i = 0; i < ad; i++)
			{
				obuf[obufp2] = ptr[pos];
				pos++;
			
				obufp2 = (obufp2 + 1) % OBUF_SZ;
			}

			transmit = 1;
			outb(port, obuf[obufp1]);
			obufp1 = (obufp1 + 1) % OBUF_SZ;
			sti();
		}
	}

	while (transmit) {}
	return n;
}

size_t serial_read(uint8_t *ptr, size_t n)
{
	size_t pos = 0;
	while (pos < n)
	{
		if (abs(ibufp1 - ibufp2) >= 1)
		{
			cli();

			size_t bs = min(n-pos, (size_t)abs(ibufp1 - ibufp2));
			
			for (size_t i = 0; i < bs; i++)
			{
				ptr[pos] = ibuf[ibufp1];
				pos++;
			
				ibufp1 = (ibufp1 + 1) % IBUF_SZ;
			}

			sti();
		}
	}
	return n;
}

void serial_isr()
{
	uint8_t iid;
	while (!((iid = inb(port+2)) & 1))
	{
		uint8_t id = iid & 0x6;
		if (id == 4)
		{
			serial_ib(inb(port));
		}
		if (id == 2)
		{
			if (transmit)
			{
				uint8_t d;
				if (!serial_ob(&d))
				{
					outb(port, d);
				}
				else
					transmit = 0;
			}
		}
	}
}

