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
	//double d;
	
	gettimeofday(&t1, NULL);
	secs = t1.tv_sec - t0.tv_sec;
	usecs = t1.tv_usec - t0.tv_usec;
	if (usecs < 0) {
		usecs += 1000000;
		secs--;
	}
	//d = (double)tc0 + (double)secs * 70.0 + (double)usecs * 70.0 / 1000000.0;
	//d = (double)tc0 + ((double)secs * 1000000.0 + (double)usecs) / (1000000.0/70.0);
	//tc = (long)d;
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

/* from Dan Olson */
static void put_dos2ansi(byte attrib)
{
	byte fore,back,blink=0,intens=0;
	
	fore = attrib&15;	// bits 0-3
	back = attrib&112; // bits 4-6
       	blink = attrib&128; // bit 7
	
	// Fix background, blink is either on or off.
	back = back>>4;

	// Fix foreground
	if (fore > 7) {
		intens = 1;
		fore-=8;
	}

	// Convert fore/back
	switch (fore) {
		case 0: // BLACK
			fore=30;
			break;
		case 1: // BLUE
			fore=34;
			break;
		case 2: // GREEN
			fore=32;
			break;
		case 3: // CYAN
			fore=36;
			break;
		case 4: // RED
			fore=31;
			break;
		case 5: // Magenta
			fore=35;
			break;
		case 6: // BROWN(yellow)
			fore=33;
			break;
		case 7: //GRAy
			fore=37;
			break;
	}
			
	switch (back) {
		case 0: // BLACK
			back=40;
			break;
		case 1: // BLUE
			back=44;
			break;
		case 2: // GREEN
			back=42;
			break;
		case 3: // CYAN
			back=46;
			break;
		case 4: // RED
			back=41;
			break;
		case 5: // Magenta
			back=45;
			break;
		case 6: // BROWN(yellow)
			back=43;
			break;
		case 7: //GRAy
			back=47;
			break;
	}
	if (blink)
		printf ("%c[%d;5;%dm%c[%dm", 27, intens, fore, 27, back);
	else
		printf ("%c[%d;25;%dm%c[%dm", 27, intens, fore, 27, back);
}

void DisplayTextSplash(byte *text)
{
	int i;
	
	//printf("%02X %02X %02X %02X\n", text[0], text[1], text[2], text[3]);
	text += 4;
	
	for (i = 0; i < 7*160; i += 2) {
		put_dos2ansi(text[i+0]);
		if (text[i+1])
			printf("%c", text[i+1]);
		else
			printf(" ");
	}
	printf("%c[m", 27);
	printf("\n");
}
