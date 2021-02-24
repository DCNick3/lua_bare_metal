#include <stdio.h>
#include <errno.h>
#include <tty.h>

#ifdef __i386__
#include <arch/i386/peripheral.h>
#endif

int fflush(FILE* stream)
{
	if (stream != stdout)
	{
		errno = ENOSYS;
		return EOF;
	}
	/* No need fo flush, rly */
	return 0;
}

size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream )
{
	if (stream != stdout)
	{
		errno = ENOSYS;
		return EOF;
	}
	
	const uint8_t* txt = (const uint8_t*)ptr;
#ifdef __i386__
	serial_write(txt, (size * count));
#else
	for (size_t p = 0; p < (size * count); p++)
	{
		tty_putchar((char*)txt[p]);
	}
#endif
	
	return (size * count);
}


int fread(void* ptr, size_t size, size_t count, FILE* stream)
{
	if (stream != stdin)
	{
		errno = ENOSYS;
		return EOF;
	}
#ifdef __i386__
	return serial_read((uint8_t*)ptr, (size * count));
#else
	errno = ENOSYS;
	return EOF;
#endif
}
