#include "wolfdef.h"

unsigned long lMSB(unsigned long x)
{
	int x1 = (x & 0x000000FF) << 24;
	int x2 = (x & 0x0000FF00) << 8;
	int x3 = (x & 0x00FF0000) >> 8;
	int x4 = (x & 0xFF000000) >> 24;
	
	return x1 | x2 | x3 | x4;
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

void MakeSmallFont(void)
{
}

void KillSmallFont(void)
{
}

void DrawSmall(Word x,Word y,Word tile)
{
}

void ShowGetPsyched(void)
{
}

void DrawPsyched(Word Index)
{
}

void EndGetPsyched(void)
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

void ShareWareEnd(void)
{
        SetAPalette(rGamePal);
        printf("ShareWareEnd()\n");
        SetAPalette(rBlackPal);
}

Word WaitEvent(void)
{
	return 0;
}

LongWord ReadTick()
{
	return 0;
}

void WaitTick()
{
}

void WaitTicks(Word Count)
{
}

Word WaitTicksEvent(Word Time)
{
	return 0;
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

void BlastScreen2(Rect *BlastRect)
{
}

void BlastScreen(void)
{
}

Word NewGameWindow(Word NewVidSize)
{
}

void BailOut()
{
	printf("BailOut()\n");
	exit(1);
}

Word ChooseGameDiff(void)
{
}

void FinishLoadGame(void)
{
}
