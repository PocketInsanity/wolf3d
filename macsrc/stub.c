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
	
	SongListPtr = (unsigned short *) LoadAResource(rSongList);
	WallListPtr = (unsigned short *) LoadAResource(MyWallList);

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

Boolean SetupScalers()
{
	return TRUE;
}

void ReleaseScalers()
{
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

void FinishLoadGame(void)
{
}
