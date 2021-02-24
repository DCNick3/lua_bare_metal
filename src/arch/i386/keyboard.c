#include <arch/i386/peripheral.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define IN_BUF_SZ 512

static uint8_t  ibuffer[IN_BUF_SZ];
static uint16_t ibufp1 = 0, ibufp2 = 0;

#define BUFCL (abs(ibuf1 - ibuf1))

void kb_init()
{
	memset(ibuffer, 0, sizeof(ibuffer));
}

void kb_byte(uint8_t byte)
{
	printf("kbr %02x  ", byte);

	ibuffer[ibufp2] = byte;
	ibufp2 = (ibufp2 + 1) % IN_BUF_SZ;
}
