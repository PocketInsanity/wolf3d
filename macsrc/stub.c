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

#include <sys/time.h>
#include <unistd.h>

#include "wolfdef.h"

LongWord LastTick; 

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
	
	MapListPtr = (maplist_t *) LoadAResource(rMapList);
	FixMapList(MapListPtr);
	
	SongListPtr = (unsigned short *)LoadAResource(rSongList);
	WallListPtr = (unsigned short *)LoadAResource(MyWallList);

}

extern int VidSize;

Word ScaleX(Word x) 
{
	switch(VidSize) {
		case 1:
			return x*8/5;
		case 2:
		case 3:
			return x*2;
	}
	return x;
}

Word ScaleY(Word y)
{
	switch(VidSize) {
		case 1:
			y = (y*8/5)+64;
			if (y == 217)
				y++;
			return y;
		case 2:
			return y*2;
		case 3:
			return y*2+80;
	}
	return y;
}

Boolean SetupScalers()
{
	return TRUE;
}

void ReleaseScalers()
{
}

LongWord PsyTime;

void ShowGetPsyched(void)
{
	LongWord *PackPtr;
	Byte *ShapePtr;
	LongWord PackLength;
	Word X,Y;
        
        PsyTime = ReadTick() + 60*2;
          
	ClearTheScreen(BLACK);
	BlastScreen();
	PackPtr = LoadAResource(rGetPsychPic);
	PackLength = lMSB(PackPtr[0]);
	ShapePtr = AllocSomeMem(PackLength);
	DLZSS(ShapePtr,(Byte *) &PackPtr[1],PackLength);
	X = (VidWidth-224)/2;
	Y = (ViewHeight-56)/2;
	DrawShape(X,Y,ShapePtr);
	FreeSomeMem(ShapePtr);
	ReleaseAResource(rGetPsychPic);
	BlastScreen();
	SetAPalette(rGamePal);	
}

void DrawPsyched(Word Index)
{
	/* TODO: blah */
}

void EndGetPsyched(void)
{
	while (PsyTime > ReadTick()) ;
	
	SetAPalette(rBlackPal);
}

void ShareWareEnd(void)
{
        SetAPalette(rGamePal);
        /* printf("ShareWareEnd()\n"); */
        SetAPalette(rBlackPal);
}

Word WaitEvent(void)
{
	while (DoEvents() == 0) ;
	
	return 0;
}

static struct timeval t0;

LongWord ReadTick()
{
	struct timeval t1;
	long secs, usecs;
	
	if (t0.tv_sec == 0) {
		gettimeofday(&t0, NULL);
		return 0;
	}
	
	gettimeofday(&t1, NULL);
	
	secs  = t1.tv_sec - t0.tv_sec;
	usecs = t1.tv_usec - t0.tv_usec;
	if (usecs < 0) {
		usecs += 1000000;
		secs--;
	}
	
	return secs * 60 + usecs * 60 / 1000000;
}

void WaitTick()
{
	do {
		DoEvents();
	} while (ReadTick() == LastTick);
	LastTick = ReadTick();
}

void WaitTicks(Word Count)
{
	LongWord TickMark;
	
	do {
		DoEvents();
		TickMark = ReadTick();
	} while ((TickMark-LastTick)<=Count);
	LastTick = TickMark;
}

Word WaitTicksEvent(Word Time)
{
	LongWord TickMark;
	LongWord NewMark;
	Word RetVal;
	
	TickMark = ReadTick();
	for (;;) {
		RetVal = DoEvents();
		if (RetVal)
			break;
		
		NewMark = ReadTick();
		if (Time) {
			if ((NewMark-TickMark)>=Time) {
				RetVal = 0;
				break;
			}
		}
	}	
	return RetVal;
}

void FinishLoadGame(void)
{
}
