#include <stdint.h>
#include <string.h>

#include <arch/i386/vga.h>

static uint16_t* vga_buffer = (uint16_t*)VGA_BUFFER_ADDRESS;

int vga_init(void)
{
	vga_reset();
	return 0;
}

int vga_set_cursor(int row, int col)
{
	(void)row;
	(void)col;
	//TODO: implement it somehow...
	return 0;
}

int vga_put_entry(int row, int col, uint16_t ent)
{
	vga_buffer[row * VGA_WIDTH + col] = ent;
	return 0;
}

int vga_reset(void)
{
	for (int i = 0; i < VGA_HEIGHT; i++)
		for (int j = 0; j < VGA_WIDTH; j++)
			vga_put_entry(i, j, vga_entry(' ', vga_attrib(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK)));
	vga_set_cursor(0, 0);
	return 0;
}

int vga_scroll(int n) 
{
	memcpy(vga_buffer, vga_buffer + VGA_WIDTH * n, (VGA_HEIGHT - n) * VGA_WIDTH * sizeof(*vga_buffer));
	for (int i = 0; i < VGA_WIDTH; i++)
		vga_put_entry(VGA_HEIGHT-1, i, vga_entry(' ', vga_attrib(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK)));
	return 0;
}


