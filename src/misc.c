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
