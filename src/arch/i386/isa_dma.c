#include <arch/i386/peripheral.h>
#include <arch/i386/isa_dma.h>
#include <procio.h>
#include <abort.h>
#include <stdio.h>

/*
 *    Ch0-3 Ch4-7            
 * ID Port  Port  Size  R/W  Function
 * 0  0x00  0xC0  Word  W    Start Address Register channel 0/4 (unusable)
 * 1  0x01  0xC2  Word  W    Count Register channel 0/4 (unusable)
 * 2  0x02  0xC4  Word  W    Start Address Register channel 1/5
 * 3  0x03  0xC6  Word  W    Count Register channel 1/5
 * 4  0x04  0xC8  Word  W    Start Address Register channel 2/6
 * 5  0x05  0xCA  Word  W    Count Register channel 2/6
 * 6  0x06  0xCC  Word  W    Start Address Register channel 3/7
 * 7  0x07  0xCE  Word  W    Count Register channel 3/7
 *-8  0x08  0xD0  Byte  R    Status Register
 *-8  0x08  0xD0  Byte  W    Command Register
 *-9  0x09  0xD2  Byte  W    Request Register
 * 10 0x0A  0xD4  Byte  W    Single Channel Mask Register
 * 11 0x0B  0xD6  Byte  W    Mode Register
 * 12 0x0C  0xD8  Byte  W    Flip-Flop Reset Register
 *-13 0x0D  0xDA  Byte  R    Intermediate Register
 * 13 0x0D  0xDA  Byte  W    Master Reset Register
 * 14 0x0E  0xDC  Byte  W    Mask Reset Register
 * 15 0x0F  0xDE  Byte  RW   MultiChannel Mask Register (reading is undocumented, but it works!) 
 *
 */

#define ADDR0  0
#define CNT0   1

#define STATUS 8
#define CMD    8
#define RQST   9
#define SMSK   10
#define MODE   11
#define FFRST  12
#define IRREG  13
#define MSTRST 13
#define MSKRST 14
#define MMSK   15

/* 1st is controller id, then register id */
/* port = c * 0xC0 + id * (c + 1) */
#define PORT(c, id) (((c)*0xC0) + (id) * ((c) + 1))

static uint8_t page_addr_reg[] = {
    0x87, 0x83, 0x81, 0x82, 0x8F, 0x8B, 0x89, 0x8A,
};

#define dbg_log(...) printf(__VA_ARGS__);

static uint8_t mask[2];

/* should be enough, 4 * 64 KiB */
#define DMA_POOL_SIZE 4
#define DMA_BUFFER_SIZE (64 * 1024)

static uint32_t used_dma_buffers = 0;
static uint8_t dma_pool[DMA_POOL_SIZE * DMA_BUFFER_SIZE] __attribute__((aligned(DMA_BUFFER_SIZE))); 

void isa_dma_init()
{
    dbg_log("isa_dma_init\n");
    /* reset controllers, set mask */
    outb(PORT(0, MSTRST), 0x00);
    mask[0] = 0x0F;
    outb(PORT(1, MSTRST), 0x00);
    mask[1] = 0x0F;
}

void write_word(uint8_t reg, uint16_t word)
{
    cli();
    outb(reg, (uint8_t)(word >> 0));
    outb(reg, (uint8_t)(word >> 8));
    sti();
}

void isa_dma_mask(int channel)
{
    int c = channel >= 4;
    mask[c] |= 1 << (channel % 4);
    outb(PORT(c, MMSK), mask[c]);
}

void isa_dma_unmask(int channel)
{
    int c = channel >= 4;
    mask[c] &= ~(1 << (channel % 4));
    outb(PORT(c, MMSK), mask[c]);
}

/* this OS does not use paging, so all addresses are physical.. We should add some API for virtual-2-physical address conversion, because this may change..
 * This function should return virtual address.. Nah, who cares */
void* isa_dma_alloc_buffer()
{
    int id = 0;
    while (id < DMA_POOL_SIZE && used_dma_buffers >> id)
        id++;
    if (id >= DMA_POOL_SIZE)
        panic("isa_dma_alloc_buffer is out of pools\n");
    used_dma_buffers |= 1 << id;
    return dma_pool + id * DMA_BUFFER_SIZE;
}
void isa_dma_free_buffer(void* buffer)
{
    int id = ((uint8_t*)buffer - dma_pool) / DMA_BUFFER_SIZE;
    int rem = ((uint8_t*)buffer - dma_pool) % DMA_BUFFER_SIZE;
    if (id < 0 || id >= DMA_POOL_SIZE || rem != 0)
        panicf("isa_dma_free_buffer received invalid pointer: %x\n", (uint32_t)buffer);
    used_dma_buffers &= ~(1 << id);
}

void isa_dma_init_transfer(int channel, void* buffer, uint16_t count, int do_write, int is_auto, int down, int mode)
{
    /* TODO: we should add some DEFINE to disable this checks */
    if (channel <= 0 || channel == 4 || channel > 7)
        panicf("invalid isa_dma channel used: %d\n", channel);
    if (((uint32_t)buffer >> 24) != 0)
        panicf("isa_dma buffer exceeded 16 MiB boundary: 0x%x\n", (unsigned)buffer);
    if (((uint32_t)buffer >> 16) != (((uint32_t)buffer + count) >> 16))
        panicf("isa_dma buffer and count is aligned wrongly: 0x%x 0x%x\n", (unsigned)buffer, (unsigned)count);
    
    int controller = channel >= 4;
    
    write_word(PORT(controller, ADDR0 + channel % 4), (uint16_t)(uint32_t)buffer);
    write_word(PORT(controller, CNT0  + channel % 4), count);
    outb(page_addr_reg[channel], (uint8_t)((uint32_t)buffer >> 16));

    outb(PORT(controller, MODE), (channel % 4) | (do_write ? 0x04 : 0x08) | ((is_auto != 0) << 4) | ((down != 0) << 5) | ((mode % 4) << 6));
}



