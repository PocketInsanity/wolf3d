#ifndef __MISC_H__
#define __MISC_H__

extern int _argc;
extern char **_argv;

void set_TimeCount(unsigned long t);
unsigned long get_TimeCount();

long filelength(int handle);

char *itoa(short int value, char *string, int radix);
char *ltoa(long value, char *string, int radix);
char *ultoa(unsigned long value, char *string, int radix);

#else 
#error "fix me TODO"
#endif
