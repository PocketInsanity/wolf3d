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

void ShowGetPsyched(void)
{
	LongWord *PackPtr;
	Byte *ShapePtr;
	LongWord PackLength;
	Word X,Y;
                                
	ClearTheScreen(BLACK);
	BlastScreen();
	PackPtr = LoadAResource(rGetPsychPic);
	PackLength = lMSB(PackPtr[0]);
	ShapePtr = AllocSomeMem(PackLength);
	DLZSS(ShapePtr,(Byte *) &PackPtr[1],PackLength);
	X = 10; /* TODO */
	Y = 100;
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
	SetAPalette(rBlackPal);
}

void ShareWareEnd(void)
{
        SetAPalette(rGamePal);
        printf("ShareWareEnd()\n");
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
		printf("RESET?\n");
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

void PurgeAllSounds(unsigned long minMemory)
{
}

void BailOut()
{
	printf("BailOut()\n");
	exit(1);
}

Word ChooseGameDiff(void)
{
/* 0 = easy, 1 = normal, 2 = hard, 3 = death incarnate */
	difficulty = 1;
	SetAPalette(rGamePal);
}

void FinishLoadGame(void)
{
}
