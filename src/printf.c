#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef __i386__
#include <arch/i386/peripheral.h>
#endif

#include <tty.h>

int printf(const char *format, ...)
{
	va_list arg;
	int done;
	
	va_start(arg, format);
	done = vprintf(format, arg);
	va_end(arg);
	
	return done;
}

int sprintf(char *str, const char *format, ...)
{
	va_list arg;
	int done;
	
	va_start(arg, format);
	done = vsprintf(str, format, arg);
	va_end(arg);
	
	return done;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
	va_list arg;
	int done;
	
	va_start(arg, format);
	done = vsnprintf(str, size, format, arg);
	va_end(arg);
	
	return done;
}


int vprintf(const char *format, va_list ap)
{
	char buf[200];
	char *b = buf;
	int r = vsnprintf(b, sizeof(buf), format, ap);
	if (r >= (int)sizeof(buf))
	{
		b = malloc(r);
		r = vsnprintf(b, sizeof(r), format, ap);
	}
	if (r >= 0)
#ifdef __i386__
		serial_write((uint8_t*)b, r);
#else	
		tty_print(b);
#endif
	return r;
}

int vsprintf(char *str, const char *format, va_list ap)
{
	return vsnprintf(str, SIZE_MAX, format, ap);
}


int vsnprintf(char *str, size_t sz, const char *format, va_list ap)
{
	size_t pos = 0;
#define pc(c) \
{ if (pos < sz) \
{ \
	str[pos] = c; \
} \
pos++; }

#define ps(st) { \
	const char* s = st; \
	while (*s != '\0') \
	{ \
		pc(*s); s++;\
	} \
}
	while (*format != 0)
	{
		char c = *format;
		if (c == '%')
		{
			int flag_minus = 0;
			int flag_plus = 0;
			int flag_space = 0;
			int flag_hash = 0;
			int flag_zero = 0;
			
			(void)flag_hash;
			
			unsigned int width = 0;
			int presicion = 0;
			int size = 0;
			/*
				0 - none
				1 - l
				2 - ll
				3 - h
				4 - hh
				5 - j
				6 - z
				7 - t
				8 - L
			*/
			
			int state = 0;
			/*
				0 - flag,
				1 - width,
				2 - presicion,
				3 - size,
				4 - type,
			*/
			
			char fmt = 0;
			
			char buf1[100], buf2[100]; /* This SHOULD be enough (but might not) */
			
			while (1)
			{
				format++;
				c = *format;
				if (c == '\0')
					return -1;
				
				switch (c)
				{
					
					case '#':
						if (state == 0)
						{
							flag_hash = 1;
						}
						break;
					case '0':
						if (state == 0)
						{
							flag_zero = 1;
						}
						else if (state == 1)
						{
							width *= 10;
						}
						else if (state == 2)
						{
							presicion *= 10;
						}
						break;
					case '-':
						if (state == 0)
						{
							flag_minus = 1;
						}
						break;
					case ' ':
						if (state == 0)
						{
							flag_space = 1;
						}
						break;
					case '+':
						if (state == 0)
						{
							flag_plus = 1;
						}
						break;
					
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						if (state == 0)
							state = 1;
						if (state == 1)
						{
							width = width * 10 + (c - '0');
						}
						else if (state == 2)
						{
							presicion = presicion * 10 + (c - '0');
						}
						break;
						
					case '*':
						if (state == 0 || state == 1)
							width = va_arg(ap, int);
						else if (state == 2)
							presicion = va_arg(ap, int);
						break;
						
					case '.':
						if (state < 2)
						{
							state = 2;
						}
						break;
						
					case 'l':
					case 'h':
					case 'j':
					case 'z':
					case 't':
					case 'L':
						if (state < 3)
							state = 3;
						switch (c)
						{
							case 'l':
								if (size != 1)
									size = 1;
								else
									size = 2;
								break;
								
							case 'h':
								if (size != 3)
									size = 3;
								else
									size = 4;
								break;
								
							case 'j':
								size = 5;
                                break;
							case 'z':
								size = 6;
                                break;
							case 't':
								size = 7;
                                break;
							case 'L':
								size = 8;
                                break;
						}
						break;
						
					case 'd':
					case 'i':
						fmt = 'd';
						goto end;
					case 's':
					case 'S':
						fmt = 's';
						goto end;
						
					case 'o':
					case 'u':
					case 'x':
					case 'X':
					case 'f':
					case 'F':
					case 'e':
					case 'E':
					case 'g':
					case 'G':
					case 'a':
					case 'A':
					case 'c':
					case 'p':
					case 'm':
						fmt = c;
						goto end;
						
					case '%':
						pc('%');
						goto next;
				}
			}
			end:
			
			//if (flag_minus)
			//	width = -width;
			
			switch (fmt)
			{
				case 'd':
				case 'x':
				case 'X':
				case 'u':
				case 'o':
				{
					int sign = 0;
					if (fmt == 'd')
						sign = 1;
					
					char base_chr = 'a';
					
					int base = 10;
					if (fmt == 'x' || fmt == 'X')
						base = 16;
					if (fmt == 'o')
						base = 8;
					
					if (fmt == 'X')
						base_chr = 'A';
					
					
					intmax_t n;
					switch (size)
					{
						case 0:
							n = va_arg(ap, int);
						break;
						case 1:
							n = va_arg(ap, long int);
						break;
						case 2:
							n = va_arg(ap, long long int);
						break;
						case 3:
							n = (short int)va_arg(ap, int);
						break;
						case 4:
							n = (char)va_arg(ap, int);
						break;
						case 5:
							n = va_arg(ap, intmax_t);
						break;
						case 6:
							n = (intmax_t)va_arg(ap, size_t);
						break;
						case 7:
							n = va_arg(ap, ptrdiff_t);
						break;
					}
					int sg = 0;
					if ((sign && n < 0))
					{
						n = -n;
						//pc('-');
						buf1[0] = '-';
						sg = 1;
					}
					else if (flag_plus)
					{
						//pc('+');
						buf1[0] = '+';
						sg = 1;
					}
					else if (flag_space)
					{	
						//pc(' ');
						buf1[0] = ' ';
						sg = 1;
					}
					
					itoa((intmax_t)n, buf1+sg, sizeof(buf1), base, base_chr);
					size_t l = strlen(buf1);
					
					if (width + 1 > sizeof(buf1))
						return -2; /* this may be very bad */
					
					char* b = buf1;
					
					if (l < width)
					{
						if (flag_minus)
						{
							for (unsigned int i = l; i < width; i++)
							{
								buf1[i] = ' ';
							}
							buf1[width] = '\0';
						}
						else 
						{
							char c = flag_zero ? '0' : ' ';
							if (sg && !flag_zero)
								buf2[width-l] = buf1[0];
							if (sg && flag_zero)
								buf2[0] = buf1[0];
							for (unsigned int i = (sg && flag_zero) ? 1 : 0; i < (width - l + ((sg && flag_zero) ? 1 : 0)); i++)
							{
								buf2[i] = c;
							}
							strcpy(&buf2[width-l+sg], buf1+sg);
							b = buf2;
						}
					}
					ps(b);
					goto next; /* Is it a bug?? */
				}
				case 'c':
					pc((char)va_arg(ap, int));
					goto next;
				case 's':
					ps(va_arg(ap, const char*));
					goto next; /* Is it a bug?? (probably yes) */
				default:
					panicf("vsprintf: unsupported format: %c", fmt);
					return -3;
			}
		}
		else
			pc(c);
		
		next:
		
		format++;
	}
	
	pc('\0');
	
	return pos;
}

int fprintf(FILE* stream, const char *format, ...)
{
	va_list arg;
	int done;
	if (stream != stdout)
	{
		errno = ENOSYS;
		return -1;
	}
	
	va_start(arg, format);
	done = vprintf(format, arg);
	va_end(arg);
	
	return done;
}

