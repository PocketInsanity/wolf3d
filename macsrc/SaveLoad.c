/*
Copyright (C) 1992-1994 Id Software, Inc.
Copyright (C) 2000 Steven Fuller <relnev@atdot.org>

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

/*
Utility functions
*/

unsigned short int sMSB(unsigned short int x)
{
	int x1 = (x & 0x00FF) << 8;
	int x2 = (x & 0xFF00) >> 8;
	
	return x1 | x2;
}

unsigned long lMSB(unsigned long x)
{
	int x1 = (x & 0x000000FF) << 24;
	int x2 = (x & 0x0000FF00) << 8;
	int x3 = (x & 0x00FF0000) >> 8;
	int x4 = (x & 0xFF000000) >> 24;
	
	return x1 | x2 | x3 | x4;
}

/*
Save and load save games
These functions must be updated if the saved structures are ever changed!
*/

Boolean SaveGame(char *file)
{
	return FALSE;
}

Boolean LoadGame(char *file)
{
	return FALSE;
}

void FinishLoadGame(void)
{
}

/*
Load initial game data
*/

void FixMapList(maplist_t *m)
{
	int i;
	
	m->MaxMap = sMSB(m->MaxMap);
	m->MapRezNum = sMSB(m->MapRezNum);
	
	for (i = 0; i < m->MaxMap; i++) {
		m->InfoArray[i].NextLevel = sMSB(m->InfoArray[i].NextLevel);
		m->InfoArray[i].SecretLevel = sMSB(m->InfoArray[i].SecretLevel);
		m->InfoArray[i].ParTime = sMSB(m->InfoArray[i].ParTime);
		m->InfoArray[i].ScenarioNum = sMSB(m->InfoArray[i].ScenarioNum);
		m->InfoArray[i].FloorNum = sMSB(m->InfoArray[i].FloorNum);
	}
}

void InitData()
{	

/*
InitSoundMusicSystem(8,8,5, 11025);
SoundListPtr = (Word *) LoadAResource(MySoundList);	
RegisterSounds(SoundListPtr,FALSE);
ReleaseAResource(MySoundList);
*/

	GetTableMemory();
	
	MapListPtr = (maplist_t *)LoadAResource(rMapList);
	FixMapList(MapListPtr);
	
	SongListPtr = (unsigned short *)LoadAResource(rSongList);
	WallListPtr = (unsigned short *)LoadAResource(MyWallList);

}
