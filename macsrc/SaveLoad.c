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

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

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
*/

static char id1[4] = "WOLF";
static char id2[4] = "SAVE";
static char id3[4] = "LV00";
static char id4[4] = "0000";

Boolean SaveGame(char *file)
{
	FILE *fp;
	Word PWallWord;
	
	fp = fopen(file, "wb");
	if (fp == NULL)
		goto fail;

	fwrite(id1, 1, sizeof(id1), fp);
	fwrite(id2, 1, sizeof(id2), fp);
	fwrite(id3, 1, sizeof(id3), fp);
	fwrite(id4, 1, sizeof(id4), fp);

	fwrite(&MapListPtr->MaxMap, 	1, sizeof(MapListPtr->MaxMap), 	fp);
	fwrite(&gamestate,		1, sizeof(gamestate), 		fp);
	fwrite(&PushWallRec,		1, sizeof(PushWallRec),		fp);
	fwrite(&nummissiles, 		1, sizeof(nummissiles),		fp);
	if (nummissiles)
		fwrite(missiles,	1, nummissiles*sizeof(missile_t), fp);
	fwrite(&numactors,		1, sizeof(numactors),		fp);
	if (numactors)
		fwrite(actors,		1, numactors*sizeof(actor_t),	fp);
	fwrite(&numdoors,		1, sizeof(numdoors),		fp);
	if (numdoors)
		fwrite(doors,		1, numdoors*sizeof(door_t),	fp);
	fwrite(&numstatics,		1, sizeof(numstatics),		fp);
	if (numstatics)
		fwrite(statics,		1, numstatics*sizeof(static_t),	fp);
	fwrite(MapPtr,			1, 64*64,			fp);
	fwrite(&tilemap,		1, sizeof(tilemap),			fp);
	fwrite(&ConnectCount,		1, sizeof(ConnectCount),	fp);
	if (ConnectCount)
		fwrite(areaconnect,	1, ConnectCount*sizeof(connect_t), fp);
	fwrite(areabyplayer,		1, sizeof(areabyplayer),	fp);
	fwrite(textures,		1, (128+5)*64,			fp);
	PWallWord = 0;
	if (pwallseg)
		PWallWord = (pwallseg-(saveseg_t*)nodes)+1;
	fwrite(&PWallWord,		1, sizeof(PWallWord),		fp);
	fwrite(nodes,			1, MapPtr->numnodes*sizeof(savenode_t), fp);

	fclose(fp);
	
	return TRUE;	
fail:
	if (fp)
		fclose(fp);
		
	return FALSE;
}

FILE *lfp;

Boolean LoadGame(char *file)
{
	FILE *fp;
	unsigned short maxmap;
	char tmp[4];
	
	fp = fopen(file, "rb");
	if (fp == NULL)
		goto fail;
	
	fread(tmp, 1, 4, fp);
	if (strncmp(tmp, id1, 4))
		goto fail;
	
	fread(tmp, 1, 4, fp);	
	if (strncmp(tmp, id2, 4))
		goto fail;
	
	fread(tmp, 1, 4, fp);
	if (strncmp(tmp, id3, 4))
		goto fail;
	
	fread(tmp, 1, 4, fp);
	if (strncmp(tmp, id4, 4))
		goto fail;
	
	fread(&maxmap, 1, sizeof(MapListPtr->MaxMap), fp);
	if (MapListPtr->MaxMap != maxmap)
		goto fail;
		
	fread(&gamestate, 1, sizeof(gamestate), fp);
	
	lfp = fp;
	
	return TRUE;
fail:
	if (fp) 
		fclose(fp);
	
	return FALSE;
}

void FinishLoadGame()
{
	Word PWallWord;
	
	if (lfp == NULL) 
		Quit("FinishLoadGame called without handle!");
	
	fseek(lfp, -sizeof(gamestate), SEEK_CUR);
	fread(&gamestate, 1, sizeof(gamestate), lfp);
	
	fread(&PushWallRec,		1, sizeof(PushWallRec),		lfp);
	fread(&nummissiles, 		1, sizeof(nummissiles),		lfp);
	if (nummissiles)
		fread(missiles,		1, nummissiles*sizeof(missile_t), lfp);
	fread(&numactors,		1, sizeof(numactors),		lfp);
	if (numactors)
		fread(actors,		1, numactors*sizeof(actor_t),	lfp);
	fread(&numdoors,		1, sizeof(numdoors),		lfp);
	if (numdoors)
		fread(doors,		1, numdoors*sizeof(door_t),	lfp);
	fread(&numstatics,		1, sizeof(numstatics),		lfp);
	if (numstatics)
		fread(statics,		1, numstatics*sizeof(static_t),	lfp);
	fread(MapPtr,			1, 64*64,			lfp);
	fread(&tilemap,			1, sizeof(tilemap),			lfp);
	fread(&ConnectCount,		1, sizeof(ConnectCount),	lfp);
	if (ConnectCount)
		fread(areaconnect,	1, ConnectCount*sizeof(connect_t), lfp);
	fread(areabyplayer,		1, sizeof(areabyplayer),	lfp);
	fread(textures,			1, (128+5)*64,			lfp);
	PWallWord = 0;
	fread(&PWallWord,		1, sizeof(PWallWord),		lfp);
	if (PWallWord) {
		pwallseg = (saveseg_t *)nodes;
		pwallseg = &pwallseg[PWallWord-1];
	}
	fread(nodes,			1, MapPtr->numnodes*sizeof(savenode_t), lfp);
	
	fclose(lfp);
	lfp = NULL;
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
