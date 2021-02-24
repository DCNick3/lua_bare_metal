#include <stdlib.h>
#include <abort.h>

int abs(int i)
{
	if (i < 0)
		return -i;
	else
		return i;
}

void exit(int status)
{
	panicf("Exitted with code %d", status);
}