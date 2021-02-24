#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __i386__
#include <arch/i386/peripheral.h>

#endif

/* 2001-01-01 (mod 400 year */
#define LEAPOCH (978307200LL /*+ 86400*(31+29)*/)

#define DAYS_PER_400Y (365*400 + 97)
#define DAYS_PER_100Y (365*100 + 24)
#define DAYS_PER_4Y   (365*4   + 1)
static const char days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};

struct tm resbuf;

#ifdef __i386__
static void cmos_time_to_tm(cmos_time_t *ct, struct tm *tm)
{
	tm->tm_sec = ct->secounds;
	tm->tm_min = ct->minutes;
	tm->tm_hour = ct->hours;
	tm->tm_mday = ct->day;
	tm->tm_mon = ct->month-1;
	tm->tm_year = (ct->year + ct->century * 100) - 1900;
	tm->tm_wday = ct->weekday-1;
	tm->tm_yday = tm->tm_mday;
	for (int i = 0; i < tm->tm_mon; i++)
		tm->tm_yday += days_in_month[i];
	tm->tm_isdst = 0;
}
#endif

time_t time(time_t *timer)
{
	time_t r = (time_t)-1;
	
#ifdef __i386__
	
	cmos_time_t t;
	struct tm tm;
	cmos_get_time(&t);
	
	cmos_time_to_tm(&t, &tm);
	r = mktime(&tm);
#endif
	
	if (timer != NULL)
		*timer = r;
	
	return r;
}

struct tm* gmtime(const time_t *timer)
{
	time_t t = *timer;
	struct tm* r = &resbuf;
	
	r->tm_sec = t % 60;
	t /= 60;
	
	r->tm_min = t % 60;
	t /= 60;
	
	r->tm_hour = t % 24;
	t /= 24;
	
	/* Now we have days since 1970-01-01 */
	int y = 2001;
	long long int ld = t - LEAPOCH / 86400;
	
	/* ld - days since 2001-01-01 */
	
	r->tm_wday = (ld + 1) % 7;
	
	y += (ld / DAYS_PER_400Y) * 400;
	ld %= DAYS_PER_400Y;
	
	y += (ld / DAYS_PER_100Y) * 100;
	ld %= DAYS_PER_100Y;
	
	y += (ld / DAYS_PER_4Y) * 4;
	ld %= DAYS_PER_4Y;
	
	y += ld / 365;
	ld %= 365;
	
	r->tm_year = y - 1900;
	r->tm_yday = ld;
	
	int vy = (y % 400 == 0) || (y % 100 != 0 && y % 4 == 0);
	for (r->tm_mon = 0; r->tm_mon < 12; r->tm_mon++)
	{
		int n = days_in_month[r->tm_mon];
		if (n == 28 && vy)
			n++;
		
		if (ld > days_in_month[r->tm_mon])
		{
			ld -= days_in_month[r->tm_mon];
		}
		else
			break;
	}
	
	r->tm_mday = ld+1;
	r->tm_isdst = 0;
	
	return r;
}

struct tm* localtime(const time_t *timer)
{
	return gmtime(timer);
}

time_t mktime(struct tm *tm)
{
	time_t r = tm->tm_sec + tm->tm_min * 60 + tm->tm_hour * 3600 + (tm->tm_mday-1) * 86400;
	int y = tm->tm_year + 1900;
	int vy = (y % 400 == 0) || (y % 100 != 0 && y % 4 == 0);
	for (int i = 0; i < tm->tm_mon; i++)
	{
		r += days_in_month[i] * 86400;
		if (days_in_month[i] == 28 && vy)
			r += 86400;
	}
	int dy = abs(y - 2001);
	int dd = (dy / 400) * DAYS_PER_400Y;
	dy = dy % 400;
	dd += (dy / 100) * DAYS_PER_100Y;
	dy = dy % 100;
	dd += (dy / 4) * DAYS_PER_4Y;
	dy = dy % 4;
	dd += dy * 365;
	dy = 0;
	
	/* dd is days since 2000-01-01 (or before) */
	
	r += LEAPOCH;
	
	if (y > 2000)
		r += dd * 86400;
	else
		r -= dd * 86400;
	
	return r;
}

double difftime(time_t time1, time_t time0)
{
	return(time1 - time0);
}

clock_t clock(void)
{
#ifdef __i386__
	return timer_ticks() * (CLOCKS_PER_SEC / 1000);
#endif
}

int usleep(useconds_t usec)
{
	clock_t t = clock();
	while (t + usec > clock())
	{}
	return 0;
}

static const char* weekdays_short[7] = {
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
};

static const char* weekdays_full[7] = {
	"Sunday",
	"Mondat",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
};

static const char* month_short[12] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};

static const char* month_full[12] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
};

size_t strftime(char *s, size_t max, const char *format, const struct tm *tm)
{
	size_t pos = 0;
	char buf[100];
#define pc(v) \
{ if (pos < max) \
	s[pos] = v; pos++; }

#define ps(v) \
{ const char* vv = v; while (*vv != '\0') { pc(*vv); vv++; } }

#define pf(f, ...) \
snprintf(buf, sizeof(buf), f, __VA_ARGS__); ps(buf);

#define pr(f) \
strftime(buf, sizeof(buf), f, tm); ps(buf);

	while (*format != '\0')
	{
		char c = *format;
		if (c == '%')
		{
		next:
			format++;
			c = *format;
			switch(c)
			{
				case 'a':
					ps(weekdays_short[tm->tm_wday]);
					break;
				case 'A':
					ps(weekdays_full[tm->tm_wday]);
					break;
				case 'h':
				case 'b':
					ps(month_short[tm->tm_mon]);
					break;
				case 'B':
					ps(month_full[tm->tm_mon]);
					break;
				case 'c':
					pr("%d.%m.%Y %H:%M:%S");
					//pf("%02d.%02d.%04d %02d:%02d:%02d", tm->tm_mday, tm->tm_mon+1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
					break;
				case 'C':
					pf("%02d", (1900 + tm->tm_year) / 100);
					break;
				case 'd':
					pf("%02d", tm->tm_mday);
					break;
				case 'D':
					pr("%m/%d/%y");
					break;
				case 'e':
					pf("%2d", tm->tm_mday);
					break;
				case 'E':
					goto next;
				case 'F':
					pr("%Y-%m-%d");
					break;
				
				case 'W':
				case 'U':
				case 'G':
				case 'g':
				case 'V':
					panic("AAaaAaAAAaA (2 hard) ((Not implemented))");
					break;
					
				case 'H':
					pf("%02d", tm->tm_hour);
					break;
				case 'I':
					pf("%02d", ((tm->tm_hour - 1) % 12) + 1);
					break;
				case 'j':
					pf("%03d", tm->tm_yday);
					break;
				case 'k':
					pf("%2d", tm->tm_hour);
					break;
				case 'l':
					pf("%2d", ((tm->tm_hour - 1) % 12) + 1);
					break;
				case 'm':
					pf("%02d", tm->tm_mon + 1);
					break;
				case 'M':
					pf("%02d", tm->tm_min);
					break;
				case 'n':
					pc('\n');
					break;
				case 'p':
					if (tm->tm_hour < 12)
					{
						ps("AM");
					}
					else
					{					
						ps("PM");
					}
					break;
				case 'P':
					if (tm->tm_hour < 12)
					{
						ps("am");
					}
					else
					{
						ps("pm");
					}
					break;
				case 'r':
					pr("%I:%M:%S %p");
					break;
				case 'R':
					pr("%H:%M");
					break;
				case 's':
					{
						time_t t = mktime((struct tm *)tm);
						pf("%lld", t);
					}
					break;
				case 'S':
					pf("%02d", tm->tm_sec);
					break;
				case 't':
					pc('\t');
					break;
				case 'T':
					pr("%H:%M:%S");
					break;
				case 'u':
					pf("%d", ((tm->tm_wday - 1) % 7) + 1);
					break;
				case 'w':
					pf("%d", tm->tm_wday);
					break;
				case 'x':
					pr("%d.%m.%Y");
					break;
				case 'X':
					pr("%H:%M:%S");
					break;
				case 'y':
					pf("%02d", tm->tm_year);
					break;
				case 'Y':
					pf("%04d", tm->tm_year + 1900);
					break;
				case 'z':
					ps("+0000");
					break;
				case 'Z':
					ps("UTC");
					break;
				
				case '%':
					pc('%');
					break;
				default:
					panicf("Unsupported format: %c", c);
					break;
			}
		}
		else
		{
			pc(c);
		}
		format++;
	}
	return pos;
}

