#ifndef __MISC_H__
#define __MISC_H__

extern int _argc;
extern char **_argv;

long filelength(int handle);

char *itoa(short int value, char *string, int radix);
char *ltoa(long value, char *string, int radix);
char *ultoa(unsigned long value, char *string, int radix);

#elif 
#error "fix me TODO"
#endif
