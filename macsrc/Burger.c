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

#include "wolfdef.h"		/* Get the prototypes */
#include <string.h>
#include <stdio.h>

unsigned char *VideoPointer;
Word VideoWidth;
LongWord YTable[480]; 

Word FontX;
Word FontY;
unsigned char *FontPtr; 
unsigned char *FontWidths;
Word FontHeight;
Word FontFirst; 
Word FontLast;
Word FontLoaded; 
Word FontInvisible;
unsigned char FontOrMask[16];

Word SystemState=3;

#define BRGR 0x42524752

/**********************************

	Graphics subsystem

**********************************/

/**********************************

	Draw a masked shape

**********************************/

void InitYTable(void)
{
	Word i;
	LongWord Offset;

	i = 0;
	Offset = 0;
	do {
		YTable[i] = Offset;
		Offset+=VideoWidth;
	} while (++i<480);
}

/**********************************

	Draw a shape

**********************************/

void DrawShape(Word x,Word y,void *ShapePtr)
{
	unsigned char *ScreenPtr;
	unsigned char *Screenad;
	unsigned char *ShapePtr2;
	unsigned short *ShapePtr3;
	Word Width;
	Word Height;
	Word Width2;

	ShapePtr3 = ShapePtr;
	Width = sMSB(ShapePtr3[0]);		/* 16 bit width */
	Height = sMSB(ShapePtr3[1]);		/* 16 bit height */
	ShapePtr2 = (unsigned char *) &ShapePtr3[2];
	ScreenPtr = (unsigned char *) &VideoPointer[YTable[y]+x];
	
	
	do {
		Width2 = Width;
		Screenad = ScreenPtr;
		do {
			*Screenad++ = *ShapePtr2++;
		} while (--Width2);
		ScreenPtr +=VideoWidth;
	} while (--Height);
}

/**********************************

	Draw a masked shape

**********************************/

void DrawMShape(Word x,Word y,void *ShapePtr)
{
	unsigned char *ScreenPtr;
	unsigned char *Screenad;
	unsigned char *MaskPtr;
	unsigned char *ShapePtr2;
	Word Width;
	Word Height;
	Word Width2;

	ShapePtr2 = ShapePtr;
	Width = ShapePtr2[1]; 
	Height = ShapePtr2[3];

	ShapePtr2 +=4;
	MaskPtr = &ShapePtr2[Width*Height];
	ScreenPtr = (unsigned char *) &VideoPointer[YTable[y]+x];
	do {
		Width2 = Width;
		Screenad = ScreenPtr;
		do {
			*Screenad = (*Screenad & *MaskPtr++) | *ShapePtr2++;
			++Screenad;
		} while (--Width2);
		ScreenPtr +=VideoWidth;
	} while (--Height);
}

/**********************************

	Draw a masked shape with an offset

**********************************/

void DrawXMShape(Word x,Word y,void *ShapePtr)
{
	unsigned short *ShapePtr2;
	ShapePtr2 = ShapePtr;
	x += sMSB(ShapePtr2[0]);
	y += sMSB(ShapePtr2[1]);
	DrawMShape(x,y,&ShapePtr2[2]);
}

/**********************************

	Clear the screen to a specific color

**********************************/

void ClearTheScreen(Word Color)
{
	Word x,y;
	unsigned char *TempPtr;

	TempPtr = VideoPointer;
	y = SCREENHEIGHT;		/* 200 lines high */
	do {
		x = 0;
		do {
			TempPtr[x] = Color;	/* Fill color */
		} while (++x<SCREENWIDTH);
		TempPtr += VideoWidth;	/* Next line down */
	} while (--y);
}

/**********************************

	Palette Manager

**********************************/

/**********************************

	Load and set a palette resource

**********************************/

void SetAPalette(Word PalNum)
{
	SetAPalettePtr(LoadAResource(PalNum));		/* Set the current palette */
	ReleaseAResource(PalNum);					/* Release the resource */
}

/**********************************

	Load and set a palette from a pointer

**********************************/

Byte CurrentPal[768];

void SetAPalettePtr(unsigned char *PalPtr)
{
	memcpy(&CurrentPal, PalPtr, 768);
	SetPalette(PalPtr);
}

/**********************************

	Fade the screen to black

**********************************/

void FadeToBlack(void)
{
	unsigned char MyPal[768];

	memset(MyPal,0,sizeof(MyPal));	/* Fill with black */
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

	Fade the palette

**********************************/

void FadeToPtr(unsigned char *PalPtr)
{
	int DestPalette[768];				/* Dest offsets */
	Byte WorkPalette[768];		/* Palette to draw */
	Byte SrcPal[768];
	Word Count;
	Word i;
	
	if (!memcmp(PalPtr,&CurrentPal,768)) {	/* Same palette? */
		return;
	}
	memcpy(SrcPal,CurrentPal,768);
	i = 0;
	do {		/* Convert the source palette to ints */
		DestPalette[i] = PalPtr[i];			
	} while (++i<768);

	i = 0;
	do {
		DestPalette[i] -= SrcPal[i];	/* Convert to delta's */
	} while (++i<768);

	Count = 1;
	do {
		i = 0;
		do {
			WorkPalette[i] = ((DestPalette[i] * (int)(Count)) / 16) + SrcPal[i];
		} while (++i<768);
		SetAPalettePtr(WorkPalette);
		WaitTicks(1);
	} while (++Count<17);
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

void DLZSSSound(Byte *Dest,Byte *Src,LongWord Length)
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
			RunCount = (Word) Src[1] | ((Word) Src[0]<<8);
			Fun = 0x1000-(RunCount&0xfff);
			BackPtr = Dest-Fun;
			RunCount = ((RunCount>>12) & 0x0f) + 3;
			if (Length >= RunCount) {
				Length -= RunCount;
			} else {
				printf("Overrun: l:%d r:%d\n", Length, RunCount);
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
