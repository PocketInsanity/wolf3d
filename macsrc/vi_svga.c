#include <stdio.h>
#include <stdlib.h>

#include <vga.h>
#include <vgakeyboard.h>

#include "wolfdef.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s <mac wolf3d resource fork>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	 
	if (InitResources(argv[1])) {
		fprintf(stderr, "could not load %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	
	return WolfMain(argc, argv);
}
