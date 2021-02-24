#include <locale.h>
#include <limits.h>

/* Set and/or return the current locale.  */
char *setlocale (int __category, const char *__locale)
{
	(void)__category; (void)__locale;
	return "C"; /* We are ignorong that */
}

struct lconv clocale = 
{
	".",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
};

/* Return the numeric/monetary information for the current locale.  */
struct lconv *localeconv (void)
{
	return &clocale;
}