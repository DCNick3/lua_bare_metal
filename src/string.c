#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>


void *memcpy(void *dest, const void *src, size_t n)
{
	char *d = (char*)dest, *s = (char*)src;
	for (size_t i = 0; i < n; i++)
		d[i] = s[i];
	return d;
}

int memcmp(const void *str1, const void *str2, size_t n)
{
	char *a = (char*)str1, *b = (char*)str2;
	for (size_t i = 0; i < n; i++)
	{
		if (a[i] < b[i])
			return -1;
		else if (a[i] > b[i])
			return 1;
	}
	return 0;
}

void *memset(void *str, int c, size_t n)
{
	char *a = (char*)str;
	for (size_t i = 0; i < n; i++)
	{
		a[i] = (char)c;
	}
	return a;
}

size_t strlen(const char *str)
{
	size_t sz = 0;
	while (*str != '\0')
	{
		sz++;
		str++;
	}
	return sz;
}

char* strcpy(char* d, const char* s)
{
	do 
	{
		*d = *s;
		d++; s++;
	} while (*s != '\0');
	return d;
}

int strcmp(const char* str1, const char* str2)
{
	while (*str1 != '\0' && *str2 != '\0')
	{
		if (*str1 > *str2)
			return 1;
		if (*str1 < *str2)
			return -1;
		str1++; str2++;
	}
	if (*str1 == '\0' && *str2 != '\0')
		return 1;
	if (*str1 != '\0' && *str2 == '\0')
		return -1;
	return 0;
}

int strcoll(const char* str1, const char* str2)
{
	return strcmp(str1, str2); /* We use only c locale. */
}

char* strchr(const char *s, int c)
{
	while (*s != '\0')
	{
		if (*s == (char)c)
			return (char*)s;
		s++;
	}
	return NULL;
}

static int maxExponent = 511;	/* Largest possible base 10 exponent.  Any
				 * exponent larger than this will already
				 * produce underflow or overflow, so there's
				 * no need to worry about additional digits.
				 */
static double powersOf10[] = {	/* Table giving binary powers of 10.  Entry */
    10.,			/* is 10^2^i.  Used to convert decimal */
    100.,			/* exponents into floating-point numbers. */
    1.0e4,
    1.0e8,
    1.0e16,
    1.0e32,
    1.0e64,
    1.0e128,
    1.0e256
};

/*
 *----------------------------------------------------------------------
 *
 * strtod --
 *
 *	This procedure converts a floating-point number from an ASCII
 *	decimal representation to internal double-precision format.
 *
 * Results:
 *	The return value is the double-precision floating-point
 *	representation of the characters in string.  If endPtr isn't
 *	NULL, then *endPtr is filled in with the address of the
 *	next character after the last one that was part of the
 *	floating-point number.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

double strtod( const char *string, char **endPtr)
{
    int sign, expSign = 0;
    double fraction, dblExp, *d;
    const char *p;
    int c;
    int exp = 0;		/* Exponent read from "EX" field. */
    int fracExp = 0;		/* Exponent that derives from the fractional
				 * part.  Under normal circumstatnces, it is
				 * the negative of the number of digits in F.
				 * However, if I is very long, the last digits
				 * of I get dropped (otherwise a long I with a
				 * large negative exponent could cause an
				 * unnecessary overflow on I alone).  In this
				 * case, fracExp is incremented one for each
				 * dropped digit. */
    int mantSize;		/* Number of digits in mantissa. */
    int decPt;			/* Number of mantissa digits BEFORE decimal
				 * point. */
    const char *pExp;		/* Temporarily holds location of exponent
				 * in string. */

    /*
     * Strip off leading blanks and check for a sign.
     */

    p = string;
    while (isspace((char)*p)) {
	p += 1;
    }
    if (*p == '-') {
	sign = 1;
	p += 1;
    } else {
	if (*p == '+') {
	    p += 1;
	}
	sign = 0;
    }

    /*
     * Count the number of digits in the mantissa (including the decimal
     * point), and also locate the decimal point.
     */

    decPt = -1;
    for (mantSize = 0; ; mantSize += 1)
    {
	c = *p;
	if (!isdigit(c)) {
	    if ((c != '.') || (decPt >= 0)) {
		break;
	    }
	    decPt = mantSize;
	}
	p += 1;
    }

    /*
     * Now suck up the digits in the mantissa.  Use two integers to
     * collect 9 digits each (this is faster than using floating-point).
     * If the mantissa has more than 18 digits, ignore the extras, since
     * they can't affect the value anyway.
     */
    
    pExp  = p;
    p -= mantSize;
    if (decPt < 0) {
	decPt = mantSize;
    } else {
	mantSize -= 1;			/* One of the digits was the point. */
    }
    if (mantSize > 18) {
	fracExp = decPt - 18;
	mantSize = 18;
    } else {
	fracExp = decPt - mantSize;
    }
    if (mantSize == 0) {
	fraction = 0.0;
	p = string;
	goto done;
    } else {
	int frac1, frac2;
	frac1 = 0;
	for ( ; mantSize > 9; mantSize -= 1)
	{
	    c = *p;
	    p += 1;
	    if (c == '.') {
		c = *p;
		p += 1;
	    }
	    frac1 = 10*frac1 + (c - '0');
	}
	frac2 = 0;
	for (; mantSize > 0; mantSize -= 1)
	{
	    c = *p;
	    p += 1;
	    if (c == '.') {
		c = *p;
		p += 1;
	    }
	    frac2 = 10*frac2 + (c - '0');
	}
	fraction = (1.0e9 * frac1) + frac2;
    }

    /*
     * Skim off the exponent.
     */

    p = pExp;
    if ((*p == 'E') || (*p == 'e')) {
	p += 1;
	if (*p == '-') {
	    expSign = 1;
	    p += 1;
	} else {
	    if (*p == '+') {
		p += 1;
	    }
	    expSign = 0;
	}
	if (!isdigit((char)*p)) {
	    p = pExp;
	    goto done;
	}
	while (isdigit((char)*p)) {
	    exp = exp * 10 + (*p - '0');
	    p += 1;
	}
    }
    if (expSign) {
	exp = fracExp - exp;
    } else {
	exp = fracExp + exp;
    }

    /*
     * Generate a floating-point number that represents the exponent.
     * Do this by processing the exponent one bit at a time to combine
     * many powers of 2 of 10. Then combine the exponent with the
     * fraction.
     */
    
    if (exp < 0) {
		expSign = 1;
		exp = -exp;
    } else {
		expSign = 0;
    }
    if (exp > maxExponent) {
		exp = maxExponent;
		errno = ERANGE;
    }
    dblExp = 1.0;
    for (d = powersOf10; exp != 0; exp >>= 1, d += 1) {
	if (exp & 01) {
	    dblExp *= *d;
	}
    }
    if (expSign) {
	fraction /= dblExp;
    } else {
	fraction *= dblExp;
    }

done:
    if (endPtr != NULL) {
	*endPtr = (char *) p;
    }

    if (sign) {
	return -fraction;
    }
    return fraction;
}

char *strpbrk(const char *str1, const char *str2)
{
	while (*str1 != '\0')
	{
		const char* c = str2;
		while (*c != '\0')
		{
			if (*str1 == *c)
				return (char*)str1;
			c++;
		}
		str1++;
	}
	return NULL;
}

size_t strspn(const char *str1, const char *str2)
{
	size_t s = 0;
	while (*str1 != '\0')
	{
		const char* c = str2;
		while (*c != '\0')
		{
			if (*str1 == *c)
				goto nxt;
			c++;
		}
		return s;
		
		nxt:
		str1++; s++;
	}
	return s;
}

int strncmp(const char *str1, const char *str2, size_t n)
{
	size_t p = 0;
	while (*str1 != '\0' && *str2 != '\0' && p < n)
	{
		if (*str1 > *str2)
			return 1;
		if (*str1 < *str2)
			return -1;
		str1++; str2++; p++;
	}
	if (*str1 == '\0' && *str2 != '\0')
		return 1;
	if (*str1 != '\0' && *str2 == '\0')
		return -1;
	return 0;
}

char *sys_errlist[] = {
    "no error (operation succeeded",		/* 0 */
    "not owner",				/* EPERM */
    "no such file or directory",		/* ENOENT */
    "no such process",				/* ESRCH */
    "interrupted system call",			/* EINTR */
    "I/O error",				/* EIO */
    "no such device or address",		/* ENXIO */
    "argument list too long",			/* E2BIG */
    "exec format error",			/* ENOEXEC */
    "bad file number",				/* EBADF */
    "no children",				/* ECHILD */
    "no more processes",			/* EAGAIN */
    "not enough memory",			/* ENOMEM */
    "permission denied",			/* EACCESS */
    "bad address in system call argument",	/* EFAULT */
    "block device required",			/* ENOTBLK */
    "mount device busy",			/* EBUSY */
    "file already exists",			/* EEXIST */
    "cross-domain link",			/* EXDEV */
    "no such device",				/* ENODEV */
    "not a directory",				/* ENOTDIR */
    "illegal operation on a directory",		/* EISDIR */
    "invalid argument",				/* EINVAL */
    "file table overflow",			/* ENFILE */
    "too many open files",			/* EMFILE */
    "inappropriate device for ioctl",		/* ENOTTY */
    "text file or pseudo-device busy",		/* ETXTBSY */
    "file too large",				/* EFBIG */
    "no space left in file system domain",	/* ENOSPC */
    "illegal seek",				/* ESPIPE */
    "read-only file system",			/* EROFS */
    "too many links",				/* EMLINK */
    "broken pipe",				/* EPIPE */
    "math argument out of range",		/* EDOM */
    "math result unrepresentable",		/* ERANGE */
    "operation would block",			/* EWOULDBLOCK */
    "operation now in progress",		/* EINPROGRESS */
    "operation already in progress",		/* EALREADY */
    "socket operation on non-socket",		/* ENOTSOCK */
    "destination address required",		/* EDESTADDRREQ */
    "message too long",				/* EMSGSIZE */
    "protocol wrong type for socket",		/* EPROTOTYPE */
    "bad proocol option",			/* ENOPROTOOPT */
    "protocol not suppored",			/* EPROTONOSUPPORT */
    "socket type not supported",		/* ESOCKTNOSUPPORT */
    "operation not supported on socket",	/* EOPNOTSUPP */
    "protocol family not supported",		/* EPFNOSUPPORT */
    "address family not supported by protocol family",	/* EAFNOSUPPORT */
    "address already in use",			/* EADDRINUSE */
    "can't assign requested address",		/* EADDRNOTAVAIL */
    "network is down",				/* ENETDOWN */
    "network is unreachable",			/* ENETUNREACH */
    "network dropped connection on reset",	/* ENETRESET */
    "software caused connection abort",		/* ECONNABORTED */
    "connection reset by peer",			/* ECONNRESET */
    "no buffer space available",		/* ENOBUFS */
    "socket is already connected",		/* EISCONN */
    "socket is not connected",			/* ENOTCONN */
    "can't send afer socket shutdown",		/* ESHUTDOWN */
    "undefined error (59)",			/* not used */
    "connection timed out",			/* ETIMEDOUT */
    "connection refused",			/* ECONNREFUSED */
    "too many levels of symbolic links",	/* ELOOP */
    "file name too long",			/* ENAMETOOLONG */
    "host is down",				/* EHOSTDOWN */
    "host is unreachable",			/* EHOSTUNREACH */
    "directory not empty",			/* ENOTEMPTY */
    "too many processes",			/* EPROCLIM */
    "too many users",				/* EUSERS */
    "disk quota exceeded",			/* EDQUOT */
    "stale remote file handle",			/* ESTALE */
    "pathname hit remote file system",		/* EREMOTE */
    "undefined error (72)",			/* not used */
    "undefined error (73)",			/* not used */
    "undefined error (74)",			/* not used */
    "undefined error (75)",			/* not used */
    "undefined error (76)",			/* not used */
    "identifier removed",			/* EIDRM */
};
int sys_nerr = sizeof(sys_errlist)/sizeof(char *);

/*
 *----------------------------------------------------------------------
 *
 * strerror --
 *
 *	Map an integer error number into a printable string.
 *
 * Results:
 *	The return value is a pointer to a string describing
 *	error.  The first character of string isn't capitalized.
 *
 * Side effects:
 *	Each call to this procedure may overwrite the value returned
 *	by the previous call.
 *
 *----------------------------------------------------------------------
 */

char * strerror(error)
    int error;			/* Integer identifying error (must be
				 * one of the officially-defined Sprite
				 * errors, as defined in errno.h). */
{
    static char defaultMsg[50];

    if ((error <= sys_nerr) && (error > 0)) {
	return sys_errlist[error];
    }
    (void) sprintf(defaultMsg, "unknown error (%d)", error);
    return defaultMsg;
}

char *strstr(const char *haystack, const char *needle)
{
	size_t nl = strlen(needle);
	while (*haystack != '\0')
	{
		if (strncmp(haystack, needle, nl) == 0)
			return (char*)haystack;
		haystack++;
	}
	return NULL;
}

void strrev(char *str)
{
	int i;
	int j;
	unsigned char a;
	unsigned len = strlen((const char *)str);
	for (i = 0, j = len - 1; i < j; i++, j--)
	{
		a = str[i];
		str[i] = str[j];
		str[j] = a;
	}
}

int itoa(uintmax_t num, char* str, int len, int base, char hex_base)
{
	uintmax_t sum = num;
	int i = 0;
	int digit;
	if (len == 0)
		return -1;
	do
	{
		digit = sum % base;
		if (digit < 0xA)
			str[i++] = '0' + digit;
		else
			str[i++] = hex_base + digit - 0xA;
		sum /= base;
	}while (sum && (i < (len - 1)));
	if (i == (len - 1) && sum)
		return -1;
	str[i] = '\0';
	strrev(str);
	return 0;
}

void *memchr(const void *s, int c, size_t n)
{
	const char* ptr = (const char*)s;
	for (size_t p = 0; p < n; p++)
	{
		if (ptr[p] == (char)c)
			return (void*)&ptr[p];
	}
	return NULL;
}
