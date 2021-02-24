#include <ctype.h>

int isdigit(int c)
{
	return c >= '0' && c <= '9';
}

int isxdigit(int c)
{
	return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int isalpha(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isalnum(int c)
{
	return isalpha(c) || isdigit(c);
}

int isspace(int c)
{
	return c == ' ' || c == '\t' || c == '\v' || c == '\n' || c == '\r' || c == '\f';
}

int islower(int c)
{
	return (c >= 'a' && c <= 'z');
}

int isupper(int c)
{
	return (c >= 'A' && c <= 'Z');
}

int ispunct(int c)
{
	return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~');
}

int iscntrl(int c)
{
	return (c >= '\x00' && c <= '\x1F') || c == '\x7F';
}

int isgraph(int c)
{
	return isalnum(c) || ispunct(c);
}


int toupper(int c)
{
	if (c >= 'a' && c <= 'z')
	{
		return c - 'a' + 'A';
	}
	return c;
}

int tolower(int c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return c - 'A' + 'a';
	}
	return c;
}

