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

#ifndef __BURGER_H__
#define __BURGER_H__

typedef unsigned int Word;
typedef unsigned long LongWord;
typedef unsigned char Byte;
typedef unsigned char Boolean;
typedef unsigned short int ShortWord;

#define BLACK 255
#define DARKGREY 250
#define BROWN 101
#define PURPLE 133
#define BLUE 210
#define DARKGREEN 229
#define ORANGE 23
#define RED 216
#define BEIGE 14
#define YELLOW 5
#define GREEN 225
#define LIGHTBLUE 150
#define LILAC 48
#define PERIWINKLE 120
#define LIGHTGREY 43
#define WHITE 0

#define SfxActive 1
#define MusicActive 2

extern unsigned char *VideoPointer;
extern Word SystemState;
extern Word VideoWidth;
extern LongWord LastTick;
extern LongWord YTable[480];

void DLZSS(Byte *Dest, Byte *Src,LongWord Length);

void WaitTick(void);
void WaitTicks(Word TickCount);
Word WaitTicksEvent(Word TickCount);
Word WaitEvent(void);
LongWord ReadTick(void);
void *AllocSomeMem(LongWord Size);
void FreeSomeMem(void *MemPtr);

Word WaitKey(void);
void FlushKeys(void);

void SoundOff(void);
void PlaySound(Word SndNum);
void StopSound(Word SndNum);
void PlaySong(Word SongNum);

void ClearTheScreen(Word Color);

void InitYTable(void);
void DrawShape(Word x,Word y,void *ShapePtr);
void DrawXMShape(Word x,Word y,void *ShapePtr);
void DrawMShape(Word x,Word y,void *ShapePtr);

void SetAPalette(Word PalNum);
void SetAPalettePtr(unsigned char *PalPtr);
void FadeTo(Word PalNum);
void FadeToBlack(void);
void FadeToPtr(unsigned char *PalPtr);

void *LoadAResource(Word RezNum);
void ReleaseAResource(Word RezNum);
void KillAResource(Word RezNum);
void *LoadAResource2(Word RezNum,LongWord Type);
void *FindResource(Word RezNum, LongWord Type);
void ReleaseAResource2(Word RezNum,LongWord Type);
void KillAResource2(Word RezNum,LongWord Type);

#endif
