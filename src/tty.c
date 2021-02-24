#include <string.h>
#include <tty.h>

#ifdef __i386__ // Here we need to use vga

#include <arch/i386/vga.h>

#endif

static int tty_row, tty_col;

#ifdef __i386__
static uint8_t tty_attrib;

#define TTY_WIDTH VGA_WIDTH
#define TTY_HEIGHT VGA_HEIGHT

#endif

static int tty_was_init = 0;

int tty_init(void)
{
	tty_row = 0;
	tty_col = 0;
	
#ifdef __i386__
	vga_init();
	tty_attrib = vga_attrib(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
#endif

	tty_was_init = 1;
	return 0;
}



static void tty_nextline()
{
	tty_row++;
	tty_col = 0;
	if (tty_row >= TTY_HEIGHT)
	{
		tty_row = TTY_HEIGHT-1;
#ifdef __i386__
		vga_scroll(1);
#endif
	}
}

int tty_putchar(char c)
{
	if (!tty_was_init)
		return 1;
	
	switch (c)
	{
		case '\r':
		{
			tty_col = 0;
			break;
		}
		case '\n':
		{
			tty_nextline();
			break;
		}
		default:
		{
#ifdef __i386__
			vga_put_entry(tty_row, tty_col, vga_entry(c, tty_attrib));
#endif
			tty_col++;
			if (tty_col >= TTY_WIDTH)
			{
				tty_nextline();
			}
			break;
		}
	}
	return 0;
}

int tty_print(char* str)
{
	if (!tty_was_init)
		return 1;
	
	while ((*str) != '\0')
	{
		tty_putchar(*str);
		str++;
	}
	return 0;
}

