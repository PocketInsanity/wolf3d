#include <stdio.h>
#include <stdlib.h>

#include <vga.h>
#include <vgakeyboard.h>

#include "wolfdef.h"

Byte *gfxbuf;

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

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s <mac wolf3d resource fork>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	vga_init();
	 
	if (InitResources(argv[1])) {
		fprintf(stderr, "could not load %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	
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
	
	NewGameWindow(0); /* 320x200 */
	ClearTheScreen(BLACK);
	BlastScreen();
	
	return WolfMain(argc, argv);
}

void SetPalette(Byte *pal)
{
	int i;

	vga_waitretrace();
	
	for (i = 0; i < 256; i++) 
		vga_setpalette(i, pal[i*3+0] >> 2, pal[i*3+1] >> 2, pal[i*3+2] >> 2);	
}
	
void BlastScreen2(Rect *BlastRect)
{
	BlastScreen();
}

static int w, h;

void BlastScreen()
{
	Byte *ptrs = gfxbuf, *ptrd = graph_mem;
	int i;
	
	for (i = 0; i < 200; i++) {
		memcpy(ptrd, ptrs, 320);
		ptrs += w;
		ptrd += 320;
	}
}

Word NewGameWindow(Word NewVidSize)
{
	LongWord *LongPtr;
	Byte *DestPtr;
	int i;
	
	switch(NewVidSize) {
		case 0:
			w = 320;
			h = 200;
			break;
		case 1:
			w = 512;
			h = 384;
			break;
		case 2:
			w = 640;
			h = 400;
			break;
		case 3:
			w = 640;
			h = 480;
			break;
		default:
			fprintf(stderr, "Vid size: %d\n", NewVidSize);
			exit(EXIT_FAILURE);
	}
	
	if (gfxbuf)
		free(gfxbuf);
		
	gfxbuf = (Byte *)malloc(w * h);
	
	vga_setmode(G320x200x256);
	
	VideoPointer = gfxbuf;
	VideoWidth = w;
	InitYTable();
	SetAPalette(rBlackPal);
	ClearTheScreen(BLACK);
	BlastScreen();
	
	LongPtr = (LongWord *) LoadAResource(rFaceShapes);
	
	GameShapes = (Byte **) AllocSomeMem(lMSB(LongPtr[0]));
	DLZSS((Byte *)GameShapes,(Byte *) &LongPtr[1],lMSB(LongPtr[0]));
	ReleaseAResource(rFaceShapes);
	
	LongPtr = (LongWord *)GameShapes;
	DestPtr = (Byte *)GameShapes;
	for (i = 0; i < ((NewVidSize == 1) ? 57 : 47); i++) 
		GameShapes[i] = DestPtr + lMSB(LongPtr[i]);
		
	return 0;
}

void IO_ScaleWallColumn(Word x,Word scale,Word tile,Word column)
{
}

void IO_ScaleMaskedColumn(Word x,Word scale,unsigned short *sprite,Word column)
{
}

Boolean SetupScalers(void)
{
	return 1;
}

void ReleaseScalers()
{
}

void FlushKeys(void)
{
	/* TODO: read all keys in keyboard buffer */
}

void ReadSystemJoystick(void)
{
	/* TODO: do key stuff here */
}

/* 
Handle events, and return:
last keypress (if any)
mouse button events == 1
zero means none of the above
*/

int DoEvents()
{
	return 0;
}
