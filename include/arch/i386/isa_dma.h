#ifndef _ISA_DMA_H
#define _ISA_DMA_H

#include <stdint.h>

#define ISA_DMA_MODE_DEMAND  0x00
#define ISA_DMA_MODE_SINGLE  0x01
#define ISA_DMA_MODE_BLOCK   0x02
#define ISA_DMA_MODE_CASCADE 0x03

void isa_dma_init();

/* this OS does not use paging, so all addresses are physical.. We should add some API for virtual-2-physical address conversion, because this may change..
 * This function should return virtual address.. Nah, who cares */
void* isa_dma_alloc_buffer();
void isa_dma_free_buffer(void* buffer);

void isa_dma_mask(int channel);
void isa_dma_unmask(int channel);

void isa_dma_init_transfer(int channel, void* buffer, uint16_t count, int do_write, int is_auto, int down, int mode);

#endif
