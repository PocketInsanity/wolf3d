/* id_vh.c */

#include "id_heads.h"

/* ======================================================================== */

pictabletype *pictable;

int px, py;
byte fontcolor, backcolor;
int fontnumber;

boolean	screenfaded;

byte palette1[256][3], palette2[256][3];

/* ======================================================================== */

void VW_DrawPropString(char *string)
{
	fontstruct *font;
	int width, step, height;
	byte *source, *dest, *ptrs, *ptrd;
	byte ch;

	font = (fontstruct *)grsegs[STARTFONT+fontnumber];
	height = font->height;
	dest = gfxbuf + py * 320 + px;

	while ((ch = *string++) != 0) {
		width = step = font->width[ch];
		source = ((byte *)font)+font->location[ch];
		while (width--) {
			height = font->height;
			ptrs = source;
			ptrd = dest;
			while (height--) {
				if (*ptrs)
					*ptrd = fontcolor;
				ptrs += step;
				ptrd += 320;
			}
			source++;
			dest++;
		}
	}
}

void VWL_MeasureString(char *string, word *width, word *height, 
	fontstruct *font)
{
	/* proportional width */
	*height = font->height;
	for (*width = 0;*string;string++)
		*width += font->width[*((byte *)string)]; 
}

void VW_MeasurePropString (char *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct *)grsegs[STARTFONT+fontnumber]);
}

void VWB_DrawTile8(int x, int y, int tile)
{
	VL_MemToScreen(grsegs[STARTTILE8]+(tile*64), 8, 8, x, y);
}

void VWB_DrawPic(int x, int y, int chunknum)
{
	int picnum = chunknum - STARTPICS;
	unsigned width,height;

	x &= ~7;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VL_MemToScreen(grsegs[chunknum],width,height,x,y);
}

/*
=====================
=
= LatchDrawPic
=
=====================
*/

void LatchDrawPic(unsigned x, unsigned y, unsigned picnum)
{
	VWB_DrawPic(x*8, y+160, picnum);
}

/*
===================
=
= LoadLatchMem
=
===================
*/

void LoadLatchMem()
{
	int i;

	CA_CacheGrChunk(STARTTILE8);

	for (i = LATCHPICS_LUMP_START; i <= LATCHPICS_LUMP_END; i++) {
		CA_CacheGrChunk(i);
	}

}

/* ======================================================================== */

/*
===================
=
= FizzleFade
=
= returns true if aborted
=
===================
*/

boolean FizzleFade(unsigned xx, unsigned yy, unsigned width,unsigned height, unsigned frames, boolean abortable)
{
	int pixperframe;
	unsigned x, y, p, frame;
	long rndval;
		
	rndval = 1;
	pixperframe = 64000/frames;
	
	IN_StartAck ();

	frame=0;
	set_TimeCount(0);
	
	do {
		if (abortable && IN_CheckAck ())
			return true;
		for (p = 0; p < pixperframe; p++) {
			y = (rndval & 0x00FF) - 1;
			x = (rndval & 0x00FFFF00) >> 8;
			
			if (rndval & 1) {
				rndval >>= 1;
				rndval ^= 0x00012000;
			} else
				rndval >>= 1;
				
			if ((x>width) || (y>height))
				continue;

			VL_DirectPlot(xx+x, yy+y, xx+x, yy+y);
			
			if (rndval == 1) /* entire sequence has been completed */
				return false;

		}
		frame++;
		while (get_TimeCount() < frame)	;
	} while (1);
}

/*
=================
=
= VL_FadeOut
=
= Fades the current palette to the given color in the given number of steps
=
=================
*/

void VL_FadeOut(int start, int end, int red, int green, int blue, int steps)
{
	int i,j,orig,delta;
	byte *origptr, *newptr;

	VL_GetPalette(&palette1[0][0]);
	memcpy(palette2,palette1,768);

//
// fade through intermediate frames
//
	for (i = 0; i < steps; i++)
	{
		origptr = &palette1[start][0];
		newptr = &palette2[start][0];
		for (j=start;j<=end;j++)
		{
			orig = *origptr++;
			delta = red-orig;
			*newptr++ = orig + delta * i / steps;
			orig = *origptr++;
			delta = green-orig;
			*newptr++ = orig + delta * i / steps;
			orig = *origptr++;
			delta = blue-orig;
			*newptr++ = orig + delta * i / steps;
		}

		VL_WaitVBL(1);
		VL_SetPalette(&palette2[0][0]);
	}

//
// final color
//
	VL_FillPalette(red, green, blue);

	screenfaded = true;
}


/*
=================
=
= VL_FadeIn
=
=================
*/

void VL_FadeIn(int start, int end, const byte *palette, int steps)
{
	int		i,j,delta;

	VL_GetPalette (&palette1[0][0]);
	memcpy(&palette2[0][0],&palette1[0][0],sizeof(palette1));

	start *= 3;
	end = end*3+2;

//
// fade through intermediate frames
//
	for (i=0;i<steps;i++)
	{
		for (j=start;j<=end;j++)
		{
			delta = palette[j]-palette1[0][j];
			palette2[0][j] = palette1[0][j] + delta * i / steps;
		}

		VL_WaitVBL(1);
		VL_SetPalette(&palette2[0][0]);
	}

//
// final color
//
	VL_SetPalette(palette);
	screenfaded = false;
}
