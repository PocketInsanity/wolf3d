#include "id_heads.h"

#ifdef DOSISM /* TODO: bad name.. since djgpp code would use interrupt (maybe uclock?) most likely */

#define TICKCOUNT 70

long TimeCount;
double AdjClock = CLOCKS_PER_SEC / TICKCOUNT;
double LastClock;

void set_TimeCount(unsigned long count)
{
	/* TODO: would using queryperformancecounter be better? */
	TimeCount = count;
	LastClock = clock();
}

unsigned long get_TimeCount()
{
	return (unsigned long)(TimeCount + ((clock() - LastClock) / AdjClock));
}

#else /* DOSISM */

/* TimeCount from David Haslam -- dch@sirius.demon.co.uk */

static struct timeval t0;
static long tc0;

void set_TimeCount(unsigned long t)
{
	tc0 = t;
	gettimeofday(&t0, NULL);
}

unsigned long get_TimeCount()
{
	struct timeval t1;
	long secs, usecs;
	long tc;
	
	gettimeofday(&t1, NULL);
	secs = t1.tv_sec - t0.tv_sec;
	usecs = t1.tv_usec - t0.tv_usec;
	if (usecs < 0) {
		usecs += 1000000;
		secs--;
	}
	tc = tc0 + secs * 70 + usecs * 70 / 1000000;
	return tc;
}

long filelength(int handle)
{
	struct stat buf;
	
	if (fstat(handle, &buf) == -1) {
		perror("filelength");
		exit(EXIT_FAILURE);
	}
	
	return buf.st_size;
}

char *strlwr(char *s)
{
	char *p = s;
	
	while (*p) {
		*p = tolower(*p);
		p++;
	}
	
	return s;
}
	
char *itoa(short int value, char *string, int radix)
{
	/* wolf3d only uses radix 10 */
	sprintf(string, "%d", value);
	return string;
}

char *ltoa(long value, char *string, int radix)
{
	sprintf(string, "%ld", value);
	return string;
}

char *ultoa(unsigned long value, char *string, int radix)
{
	sprintf(string, "%lu", value);
	return string;
}

#endif /* DOSISM */
