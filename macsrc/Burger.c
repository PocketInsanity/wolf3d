/*
Copyright (C) 1992-1994 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdlib.h>
#include <string.h>

#include "wolfdef.h"

#define BRGR 0x42524752

/**********************************

	Load and set a palette resource

**********************************/

void SetAPalette(Word PalNum)
{
	SetAPalettePtr(LoadAResource(PalNum));		/* Set the current palette */
	ReleaseAResource(PalNum);					/* Release the resource */
}

/**********************************

	Fade the screen to black

**********************************/

void FadeToBlack(void)
{
	unsigned char MyPal[768];

	memset(MyPal, 0, sizeof(MyPal));        /* Fill with black */
	MyPal[0] = MyPal[1] = MyPal[2] = 255;
	FadeToPtr(MyPal);
}

/**********************************

	Fade the screen to a palette
	
**********************************/

void FadeTo(Word RezNum)
{
	FadeToPtr(LoadAResource(RezNum));
        ReleaseAResource(RezNum);
}
                        
/**********************************

	Resource manager subsystem

**********************************/

/**********************************

	Load a personal resource

**********************************/

void *LoadAResource(Word RezNum) 
{
	return(LoadAResource2(RezNum, BRGR));
}

/**********************************

	Allow a resource to be purged

**********************************/

void ReleaseAResource(Word RezNum)
{
	ReleaseAResource2(RezNum, BRGR);
}

/**********************************

	Force a resource to be destroyed

**********************************/

void KillAResource(Word RezNum)
{
	KillAResource2(RezNum, BRGR);
}

unsigned short SwapUShort(unsigned short Val)
{
	return ((Val<<8) | (Val>>8));
}

/**********************************

	Decompress using LZSS

**********************************/

void DLZSS(Byte *Dest,Byte *Src,LongWord Length)
{
	Word BitBucket;
	Word RunCount;
	Word Fun;
	Byte *BackPtr;
	
	if (!Length) {
		return;
	}
	BitBucket = (Word) Src[0] | 0x100;
	++Src;
	do {
		if (BitBucket&1) {
			Dest[0] = Src[0];
			++Src;
			++Dest;
			--Length;
		} else {
			RunCount = (Word) Src[0] | ((Word) Src[1]<<8);
			Fun = 0x1000-(RunCount&0xfff);
			BackPtr = Dest-Fun;
			RunCount = ((RunCount>>12) & 0x0f) + 3;
			if (Length >= RunCount) {
				Length -= RunCount;
			} else {
				RunCount = Length;
				Length = 0;
			}
			while (RunCount--) {
				*Dest++ = *BackPtr++;
			}
			Src+=2;
		}
		BitBucket>>=1;
		if (BitBucket==1) {
			BitBucket = (Word)Src[0] | 0x100;
			++Src;
		}
	} while (Length);
}

/**********************************

	Allocate some memory

**********************************/

void *AllocSomeMem(LongWord Size)
{
	return (void *)malloc(Size);
}

/**********************************

	Release some memory

**********************************/

void FreeSomeMem(void *MemPtr)
{
	free(MemPtr);
}
