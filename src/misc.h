#ifndef __MISC_H__
#define __MISC_H__

extern int _argc;
extern char **_argv;

void SavePCX256ToFile(unsigned char *buf, int width, int height, unsigned char *pal, char *name);
void SavePCXRGBToFile(unsigned char *buf, int width, int height, char *name);

void set_TimeCount(unsigned long t);
unsigned long get_TimeCount();

#ifndef DOSISM

long filelength(int handle);

#define stricmp strcasecmp
#define strnicmp strncasecmp
char *strlwr(char *s);

char *itoa(int value, char *string, int radix);
char *ltoa(long value, char *string, int radix);
char *ultoa(unsigned long value, char *string, int radix);

#endif /* DOSISM */

#endif
