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

Word SystemState=3;

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

	Stop the current song from playing

**********************************/

void StopSong(void)
{
	PlaySong(0);
}

/**********************************

	Play a new song
	
**********************************/

void StartSong(Word songnum)
{
	PlaySong(songnum);		/* Stop the previous song (If any) */
}

void FreeSong()
{
}

void BeginSongLooped(Word Song)
{
}

void EndSong()
{
}

void BeginSound(short int theID, long theRate)
{
}

void EndSound(short int theID)
{
}

void EndAllSound(void)
{
}


/**********************************

	Sound sub-system

**********************************/

/**********************************

	Shut down the sound

**********************************/

void SoundOff(void)
{
	PlaySound(0);
}

/**********************************

	Play a sound resource

**********************************/

void PlaySound(Word SoundNum)
{
	if (SoundNum) {
		SoundNum+=127;
		if (SoundNum&0x8000) {		/* Mono sound */
			EndSound(SoundNum&0x7fff);
		}
		BeginSound(SoundNum&0x7fff,11127<<17L);
	} else {
		EndAllSound();
	}
}

/**********************************

	Stop playing a sound resource

**********************************/

void StopSound(Word SoundNum)
{
	EndSound(SoundNum+127);
}

static Word LastSong = -1;

void PlaySong(Word Song)
{
	if (Song) {
		if (SystemState&MusicActive) {
			if (Song!=LastSong) {
				BeginSongLooped(Song);	
				LastSong = Song;
			}
			return;
		}
	} 
	EndSong();
	LastSong = -1;
}
