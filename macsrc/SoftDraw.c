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

#include "wolfdef.h"

void ScaledDraw(Byte *gfx, Word scale, Byte *vid, LongWord TheFrac, Word TheInt, Word Width, LongWord Delta)
{	
	LongWord OldDelta;
	while (scale--) {
		*vid = *gfx;
		vid += Width;
		OldDelta = Delta;
		Delta += TheFrac;
		gfx += TheInt;
		if (OldDelta > Delta)
			gfx += 1;			
	}
}

void IO_ScaleWallColumn(Word x, Word scale, Word tile, Word column)
{
	LongWord TheFrac;
	Word TheInt;
	LongWord y;
	
	Byte *ArtStart;
	
	if (scale) {
		scale*=2;
		TheFrac = 0x80000000UL / scale;

		ArtStart = &ArtData[tile][column<<7];
		if (scale<VIEWHEIGHT) {
			y = (VIEWHEIGHT-scale)/2;
			TheInt = TheFrac>>24;
			TheFrac <<= 8;
			
			ScaledDraw(ArtStart,scale,&VideoPointer[(y*VideoWidth)+x],
			TheFrac,TheInt,VideoWidth, 0);
			
			return;
			
		}
		y = (scale-VIEWHEIGHT)/2;
		y *= TheFrac;
		TheInt = TheFrac>>24;
		TheFrac <<= 8;
		
		ScaledDraw(&ArtStart[y>>24],VIEWHEIGHT,&VideoPointer[x],
		TheFrac,TheInt,VideoWidth,y<<8);
	}
}

typedef struct {
	SWord Topy;
	SWord Boty;
	SWord Shape;
} PACKED SpriteRun;
                        
void IO_ScaleMaskedColumn(Word x,Word scale, unsigned short *CharPtr,Word column)
{
	Byte * CharPtr2;
	int Y1,Y2;
	Byte *Screenad;
	SpriteRun *RunPtr;
	LongWord TheFrac;
	LongWord TFrac;
	LongWord TInt;
	Word RunCount;
	int TopY;
	Word Index;
	LongWord Delta;
	
	if (!scale) 
		return;
		
	CharPtr2 = (Byte *) CharPtr;
	
	TheFrac = 0x40000000/scale;
	
	RunPtr = (SpriteRun *)&CharPtr[sMSB(CharPtr[column+1])/2]; 
	Screenad = &VideoPointer[x];
	TFrac = TheFrac<<8;
	TInt = TheFrac>>24;
	TopY = (VIEWHEIGHT/2)-scale;
	
	while (RunPtr->Topy != 0xFFFF) {
		Y1 = scale*(LongWord)sMSB(RunPtr->Topy)/128+TopY;
		if (Y1 < VIEWHEIGHT) {
			Y2 = scale*(LongWord)sMSB(RunPtr->Boty)/128+TopY;
			if (Y2 > 0) {
				if (Y2 > VIEWHEIGHT) 
					Y2 = VIEWHEIGHT;
				Index = sMSB(RunPtr->Shape)+sMSB(RunPtr->Topy)/2;
				Delta = 0;
				if (Y1 < 0) {
					Delta = (0-(LongWord)Y1)*TheFrac;
					Index += (Delta>>24);
					Delta <<= 8;
					Y1 = 0;
				}
				RunCount = Y2-Y1;
				if (RunCount) 
					ScaledDraw(&CharPtr2[Index],RunCount,
					&Screenad[Y1*VideoWidth],TFrac,TInt,VideoWidth, Delta);
			}
		}
		RunPtr++;
	}
}

Byte *SmallFontPtr;
	
void MakeSmallFont(void)
{
	Word i,j,Width,Height;
	Byte *DestPtr,*ArtStart;
	Byte *TempPtr;
	
	SmallFontPtr = AllocSomeMem(16*16*65);
	if (!SmallFontPtr) {
		return;
	}
	
	memset(SmallFontPtr,0,16*16*65);
	
	i = 0;
	DestPtr = SmallFontPtr;
	do {
		ArtStart = &ArtData[i][0];
		
		if (!ArtStart) {
			DestPtr+=(16*16);
		} else {
			Height = 0;
			do {
				Width = 16;
				j = Height*8;
				do {
					DestPtr[0] = ArtStart[j];
					++DestPtr;
					j+=(WALLHEIGHT*8);
				} while (--Width);
			} while (++Height<16);
		}
	} while (++i<64);
	
	TempPtr = LoadAResource(MyBJFace);
	memcpy(DestPtr,TempPtr,16*16);
	ReleaseAResource(MyBJFace);
}

void KillSmallFont(void)
{
	if (SmallFontPtr) {
		FreeSomeMem(SmallFontPtr);
		SmallFontPtr = 0;
	}
}

void DrawSmall(Word x,Word y,Word tile)
{
	Byte *Screenad;
	Byte *ArtStart;
	Word Width,Height;
	
	if (!SmallFontPtr) {
		return;
	}
	
	x *= 16;
	y *= 16;
	
	Screenad = &VideoPointer[YTable[y]+x];
	ArtStart = &SmallFontPtr[tile*(16*16)];
	Height = 0;
	
	do {
		Width = 16;
		do {
			Screenad[0] = ArtStart[0];
			++Screenad;
			++ArtStart;
		} while (--Width);
		Screenad+=VideoWidth-16;
	} while (++Height<16);	
}
