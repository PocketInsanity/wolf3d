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
#include <ctype.h>
#include <stdlib.h>

/**********************************

	Main game introduction

**********************************/

void Intro(void)
{
	LongWord *PackPtr;
	LongWord PackLength;
	Byte *ShapePtr;
	
	NewGameWindow(1);	/* Set to 512 mode */ 

	FadeToBlack();		/* Fade out the video */
	PackPtr = LoadAResource(rMacPlayPic);
	PackLength = lMSB(PackPtr[0]);
	ShapePtr = AllocSomeMem(PackLength);
	DLZSS(ShapePtr,(Byte *) &PackPtr[1],PackLength);
	DrawShape(0,0,ShapePtr);
	FreeSomeMem(ShapePtr);
	ReleaseAResource(rMacPlayPic);
	
	BlastScreen();
	StartSong(SongListPtr[0]);	/* Play the song */
	FadeTo(rMacPlayPal);	/* Fade in the picture */
	WaitTicksEvent(240);		/* Wait for event */
	FadeTo(rIdLogoPal);
	if (toupper(WaitTicksEvent(240))=='B') {		/* Wait for event */
		FadeToBlack();
		ClearTheScreen(BLACK);
		BlastScreen();
		PackPtr = LoadAResource(rYummyPic);
		PackLength = lMSB(PackPtr[0]);
		ShapePtr = AllocSomeMem(PackLength);
		DLZSS(ShapePtr,(Byte *) &PackPtr[1],PackLength);
		DrawShape((SCREENWIDTH-320)/2,(SCREENHEIGHT-200)/2,ShapePtr);
		FreeSomeMem(ShapePtr);
		ReleaseAResource(rYummyPic);
		BlastScreen();
		FadeTo(rYummyPal);
		WaitTicksEvent(600);
	}	
}
