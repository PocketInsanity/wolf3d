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

void BeginSongLooped(Song)
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
