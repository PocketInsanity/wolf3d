/**********************************

	Burger library for the Macintosh.
	Use Think.c or Code Warrior to compile.
	Use SMART linking to link in just what you need

**********************************/

#include "wolfdef.h"		/* Get the prototypes */
#include <string.h>
#include <stdio.h>

unsigned char *VideoPointer;
Word VideoWidth;
LongWord YTable[480]; 

Word FontX;
Word FontY;
unsigned char *FontPtr; 
unsigned char *FontWidths;
Word FontHeight;
Word FontFirst; 
Word FontLast;
Word FontLoaded; 
Word FontInvisible;
unsigned char FontOrMask[16];

Word SystemState=3;

#define BRGR 0x42524752

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

/**********************************

	Graphics subsystem

**********************************/

/**********************************

	Draw a masked shape

**********************************/

void InitYTable(void)
{
	Word i;
	LongWord Offset;

	i = 0;
	Offset = 0;
	do {
		YTable[i] = Offset;
		Offset+=VideoWidth;
	} while (++i<480);
}

/**********************************

	Draw a shape

**********************************/

void DrawShape(Word x,Word y,void *ShapePtr)
{
	unsigned char *ScreenPtr;
	unsigned char *Screenad;
	unsigned char *ShapePtr2;
	unsigned short *ShapePtr3;
	Word Width;
	Word Height;
	Word Width2;

	ShapePtr3 = ShapePtr;
	Width = sMSB(ShapePtr3[0]);		/* 16 bit width */
	Height = sMSB(ShapePtr3[1]);		/* 16 bit height */
	ShapePtr2 = (unsigned char *) &ShapePtr3[2];
	ScreenPtr = (unsigned char *) &VideoPointer[YTable[y]+x];
	
	
	do {
		Width2 = Width;
		Screenad = ScreenPtr;
		do {
			*Screenad++ = *ShapePtr2++;
		} while (--Width2);
		ScreenPtr +=VideoWidth;
	} while (--Height);
}

/**********************************

	Draw a masked shape

**********************************/

void DrawMShape(Word x,Word y,void *ShapePtr)
{
	unsigned char *ScreenPtr;
	unsigned char *Screenad;
	unsigned char *MaskPtr;
	unsigned char *ShapePtr2;
	Word Width;
	Word Height;
	Word Width2;

	ShapePtr2 = ShapePtr;
	Width = ShapePtr2[1]; 
	Height = ShapePtr2[3];

	ShapePtr2 +=4;
	MaskPtr = &ShapePtr2[Width*Height];
	ScreenPtr = (unsigned char *) &VideoPointer[YTable[y]+x];
	do {
		Width2 = Width;
		Screenad = ScreenPtr;
		do {
			*Screenad = (*Screenad & *MaskPtr++) | *ShapePtr2++;
			++Screenad;
		} while (--Width2);
		ScreenPtr +=VideoWidth;
	} while (--Height);
}

/**********************************

	Draw a masked shape with an offset

**********************************/

void DrawXMShape(Word x,Word y,void *ShapePtr)
{
	unsigned short *ShapePtr2;
	ShapePtr2 = ShapePtr;
	x += sMSB(ShapePtr2[0]);
	y += sMSB(ShapePtr2[1]);
	DrawMShape(x,y,&ShapePtr2[2]);
}

/**********************************

	Erase a masked shape

**********************************/

void EraseMBShape(Word x,Word y, void *ShapePtr, void *BackPtr)
{
	unsigned char *ScreenPtr;
	unsigned char *Screenad;
	unsigned char *Backad;
	unsigned char *BackPtr2;
	unsigned char *MaskPtr;
	Word Width;
	Word Height;
	Word Width2;

	MaskPtr = ShapePtr;		/* Get the pointer to the mask */
	Width = sMSB(MaskPtr[1]);		/* Get the width of the shape */
	Height = sMSB(MaskPtr[3]);	/* Get the height of the shape */
	MaskPtr = &MaskPtr[(Width*Height)+4];	/* Index to the mask */
							/* Point to the screen */
	ScreenPtr = (unsigned char *) &VideoPointer[YTable[y]+x];
	BackPtr2 = BackPtr;
	BackPtr2 = &BackPtr2[(y*SCREENWIDTH)+x];	/* Index to the erase buffer */
	do {
		Width2 = Width;		/* Init width count */
		Screenad = ScreenPtr;
		Backad = BackPtr2;
		do {
			if (!*MaskPtr++) {
				*Screenad = *Backad;
			}
			++Screenad;
			++Backad;
		} while (--Width2);
		ScreenPtr +=VideoWidth;
		BackPtr2 += SCREENWIDTH;
	} while (--Height);
}

/**********************************

	Test for a shape collision

**********************************/

Word TestMShape(Word x,Word y,void *ShapePtr)
{
	unsigned char *ScreenPtr;
	unsigned char *Screenad;
	unsigned char *MaskPtr;
	unsigned char *ShapePtr2;
	Word Width;
	Word Height;
	Word Width2;

	ShapePtr2 = ShapePtr;
	Width = sMSB(ShapePtr2[0]);
	Height = sMSB(ShapePtr2[1]);
	ShapePtr2 +=2;
	MaskPtr = &ShapePtr2[Width*Height];
	ScreenPtr = (unsigned char *) &VideoPointer[YTable[y]+x];
	do {
		Width2 = Width;
		Screenad = ScreenPtr;
		do {
			if (!*MaskPtr++) {
				if (*Screenad != *ShapePtr2) {
					return 1;
				}
			}
			++ShapePtr2;
			++Screenad;
		} while (--Width2);
		ScreenPtr +=VideoWidth;
	} while (--Height);
	return 0;
}

/**********************************

	Test for a masked shape collision

**********************************/

Word TestMBShape(Word x,Word y,void *ShapePtr,void *BackPtr)
{
	unsigned char *ScreenPtr;
	unsigned char *Screenad;
	unsigned char *Backad;
	unsigned char *BackPtr2;
	unsigned char *MaskPtr;
	Word Width;
	Word Height;
	Word Width2;

	MaskPtr = ShapePtr;		/* Get the pointer to the mask */
	Width = sMSB(MaskPtr[0]);		/* Get the width of the shape */
	Height = sMSB(MaskPtr[1]);	/* Get the height of the shape */
	MaskPtr = &MaskPtr[(Width*Height)+2];	/* Index to the mask */
							/* Point to the screen */
	ScreenPtr = (unsigned char *) &VideoPointer[YTable[y]+x];
	BackPtr2 = BackPtr;
	BackPtr2 = &BackPtr2[(y*SCREENWIDTH)+x];	/* Index to the erase buffer */
	do {
		Width2 = Width;		/* Init width count */
		Screenad = ScreenPtr;
		Backad = BackPtr2;
		do {
			if (!*MaskPtr++) {
				if (*Screenad != *Backad) {
					return 1;
				}
			}
			++Screenad;
			++Backad;
		} while (--Width2);
		ScreenPtr +=VideoWidth;
		BackPtr2 += SCREENWIDTH;
	} while (--Height);
	return 0;
}

/**********************************

	Show a full screen picture

**********************************/

void ShowPic(Word PicNum)
{
	DrawShape(0,0,LoadAResource(PicNum));	/* Load the resource and show it */
	ReleaseAResource(PicNum);			/* Release it */
	BlastScreen();
}

/**********************************

	Clear the screen to a specific color

**********************************/

void ClearTheScreen(Word Color)
{
	Word x,y;
	unsigned char *TempPtr;

	TempPtr = VideoPointer;
	y = SCREENHEIGHT;		/* 200 lines high */
	do {
		x = 0;
		do {
			TempPtr[x] = Color;	/* Fill color */
		} while (++x<SCREENWIDTH);
		TempPtr += VideoWidth;	/* Next line down */
	} while (--y);
}

/**********************************

	Draw a text string

**********************************/

void DrawAString(char *TextPtr)
{
	while (TextPtr[0]) {		/* At the end of the string? */
		DrawAChar(TextPtr[0]);	/* Draw the char */
		++TextPtr;			/* Continue */
	}
}

/**********************************

	Set the X/Y to the font system

**********************************/

void SetFontXY (Word x,Word y)
{
	FontX = x;
	FontY = y;
}

/**********************************

	Make color zero invisible

**********************************/

void FontUseMask(void)
{
	FontInvisible = 0;
	FontSetColor(0,0);
}

/**********************************

	Make color zero a valid color

**********************************/

void FontUseZero(void)
{
	FontInvisible = -1;
	FontSetColor(0,BLACK);
}

/**********************************

	Set the color entry for the font

**********************************/

void FontSetColor(Word Num,Word Color)
{
	FontOrMask[Num] = Color;
}

/**********************************

	Install a font into memory

**********************************/

typedef struct FontStruct {
	unsigned short FHeight;
	unsigned short FLast;
	unsigned short FFirst;
	unsigned char FData;
} FontStruct;

void InstallAFont(Word FontNum)
{
	FontStruct *FPtr;

	if (FontLoaded) {
		if (FontLoaded == FontNum) {
			return;
		}
		ReleaseAResource(FontLoaded);
	}
	FontLoaded = FontNum;
	FPtr = LoadAResource(FontNum);
	FontHeight = SwapUShort(FPtr->FHeight);
	FontLast = SwapUShort(FPtr->FLast);
	FontFirst = SwapUShort(FPtr->FFirst);
	FontWidths = &FPtr->FData;
	FontPtr = &FontWidths[FontLast];
}

/**********************************

	Draw a char to the screen

**********************************/

void DrawAChar(Word Letter)
{
	Word XWidth;
	Word Offset;
	Word Width;
	Word Height;
	int Width2;
	unsigned char *Font;
	unsigned char *ScreenPtr;
	unsigned char *Screenad;
	unsigned char *FontOr;
	Word Temp;
	Word Temp2;

	Letter -= FontFirst;		/* Offset from the first entry */
	if (Letter>=FontLast) {		/* In the font? */
		return;					/* Exit then! */
	}
	XWidth = FontWidths[Letter];	/* Get the pixel width of the entry */
	Width = (XWidth-1)/2;
	Font = &FontPtr[Letter*2];
	Offset = (Font[1]*256) + Font[0];
	Font = &FontPtr[Offset];
	ScreenPtr = (unsigned char *) &VideoPointer[YTable[FontY]+FontX];
	FontX+=XWidth;
	Height = FontHeight;
	FontOr = &FontOrMask[0];

	do {
		Screenad = ScreenPtr;
		Width2 = Width;
		do {
			Temp = *Font++;
			Temp2 = Temp>>4;
			if (Temp2 != FontInvisible) {
				Screenad[0] = FontOr[Temp2];
			}
			Temp &= 0x0f;
			if (Temp != FontInvisible) {
				Screenad[1] = FontOr[Temp];
			}
			Screenad+=2;		/* Next address */
		} while(--Width2>=0);
		ScreenPtr += VideoWidth;
	} while (--Height);
}

/**********************************

	Palette Manager

**********************************/

/**********************************

	Load and set a palette resource

**********************************/

void SetAPalette(Word PalNum)
{
	SetAPalettePtr(LoadAResource(PalNum));		/* Set the current palette */
	ReleaseAResource(PalNum);					/* Release the resource */
}

/**********************************

	Load and set a palette from a pointer

**********************************/

Byte CurrentPal[768];

void SetAPalettePtr(unsigned char *PalPtr)
{
	memcpy(&CurrentPal, PalPtr, 768);
	SetPalette(PalPtr);
}

/**********************************

	Fade the screen to black

**********************************/

void FadeToBlack(void)
{
	unsigned char MyPal[768];

	memset(MyPal,0,sizeof(MyPal));	/* Fill with black */
	MyPal[0] = MyPal[1] = MyPal[2] = 255;
	FadeToPtr(MyPal);
}

/**********************************

	Fade the screen to a palette

**********************************/

void FadeTo(Word RezNum)
{
	FadeToPtr(LoadAResource(RezNum));
	ReleaseAResource(RezNum);
}

/**********************************

	Fade the palette

**********************************/

void FadeToPtr(unsigned char *PalPtr)
{
	int DestPalette[768];				/* Dest offsets */
	Byte WorkPalette[768];		/* Palette to draw */
	Byte SrcPal[768];
	Word Count;
	Word i;
	
	if (!memcmp(PalPtr,&CurrentPal,768)) {	/* Same palette? */
		return;
	}
	memcpy(SrcPal,CurrentPal,768);
	i = 0;
	do {		/* Convert the source palette to ints */
		DestPalette[i] = PalPtr[i];			
	} while (++i<768);

	i = 0;
	do {
		DestPalette[i] -= SrcPal[i];	/* Convert to delta's */
	} while (++i<768);

	Count = 1;
	do {
		i = 0;
		do {
			WorkPalette[i] = ((DestPalette[i] * (int)(Count)) / 16) + SrcPal[i];
		} while (++i<768);
		SetAPalettePtr(WorkPalette);
		WaitTicks(1);
	} while (++Count<17);
}

/**********************************

	Resource manager subsystem

**********************************/

/**********************************

	Load a personal resource

**********************************/

void *LoadAResource(Word RezNum) 
{
	return(LoadAResource2(RezNum, BRGR));
}

/**********************************

	Allow a resource to be purged

**********************************/

void ReleaseAResource(Word RezNum)
{
	ReleaseAResource2(RezNum, BRGR);
}

/**********************************

	Force a resource to be destroyed

**********************************/

void KillAResource(Word RezNum)
{
	KillAResource2(RezNum, BRGR);
}

unsigned short SwapUShort(unsigned short Val)
{
	return ((Val<<8) | (Val>>8));
}

/**********************************

	Decompress using LZSS

**********************************/

void DLZSS(Byte *Dest,Byte *Src,LongWord Length)
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
			/* TODO - sounds have this line the other way around */
			RunCount = (Word) Src[0] | ((Word) Src[1]<<8);
			Fun = 0x1000-(RunCount&0xfff);
			BackPtr = Dest-Fun;
			RunCount = ((RunCount>>12) & 0x0f) + 3;
			if (Length >= RunCount) {
				Length -= RunCount;
			} else {
				printf("Overrun: l:%d r:%d\n", Length, RunCount);
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

	Allocate some memory

**********************************/

/* TODO - cheap way to find MSB problems */
#include <sys/types.h>
#include <signal.h>
              
void *AllocSomeMem(LongWord Size)
{
	if (Size > 5000000)
		kill(getpid(), SIGSEGV);
		
	return (void *)malloc(Size);
}

/**********************************

	Release some memory

**********************************/

void FreeSomeMem(void *MemPtr)
{
	free(MemPtr);
}
