#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
                     
#include "misc.h"

long filelength(int handle)
{
	struct stat buf;
	
	if (fstat(handle, &buf) == -1) {
		perror("filelength");
		exit(EXIT_FAILURE);
	}
	
	return buf.st_size;
}

char *ltoa(long value, char *string, int radix)
{
	sprintf(string, "%d", value);
	return string;
}

char *ultoa(unsigned long value, char *string, int radix)
{
	sprintf(string, "%u", value);
	return string;
}
	