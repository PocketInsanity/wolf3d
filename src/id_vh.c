/* id_vh.c */

#include "id_heads.h"

/* ======================================================================== */

pictabletype *pictable;

int px,py;
byte fontcolor,backcolor;
int fontnumber;

/* TODO: more latch nonsense
unsigned freelatch; 
unsigned latchpics[NUMLATCHPICS];
*/

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

/*
=============================================================================

				Double buffer management routines

=============================================================================
*/

void VWB_DrawTile8(int x, int y, int tile)
{
/* TODO: 'latches' do not work */
/*	LatchDrawChar(x,y,tile); */
}

void VWB_DrawPic(int x, int y, int chunknum)
{
	int picnum = chunknum - STARTPICS;
	unsigned width,height;

	x &= ~7;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VL_MemToScreen (grsegs[chunknum],width,height,x,y);
}

void VWB_DrawPropString(char *string)
{
	VW_DrawPropString(string);
}

void VWB_Bar(int x, int y, int width, int height, int color)
{
	VW_Bar(x,y,width,height,color);
}

void VWB_Plot(int x, int y, int color)
{
	VW_Plot(x,y,color);
}

void VWB_Hlin(int x1, int x2, int y, int color)
{
	VW_Hlin(x1,x2,y,color);
}

void VWB_Vlin(int y1, int y2, int x, int color)
{
	VW_Vlin(y1,y2,x,color);
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

/* ======================================================================== */

/*
===================
=
= LoadLatchMem
=
===================
*/

void LoadLatchMem(void)
{
	int	i,j,p,m,width,height,start,end;
	byte *src;
	word	destoff;

/*
   tile 8s
*/
#if 0 /* I can't get these to extract, if at all */
	latchpics[0] = freelatch;
	CA_CacheGrChunk (STARTTILE8);
	src = (byte *)grsegs[STARTTILE8];
	destoff = freelatch;

	for (i=0;i<NUMTILE8;i++)
	{
		VL_MemToLatch (src,8,8,destoff);
		src += 64;
		destoff +=16;
	}
	UNCACHEGRCHUNK (STARTTILE8);
#endif

#if 0 
/*
   tile 16s
*/
	src = (byte *)grsegs[STARTTILE16];
	latchpics[1] = destoff;

	for (i=0;i<NUMTILE16;i++)
	{
		CA_CacheGrChunk (STARTTILE16+i);
		src = (byte *)grsegs[STARTTILE16+i];
		VL_MemToLatch (src,16,16,destoff);
		destoff+=64;
		if (src)
			UNCACHEGRCHUNK (STARTTILE16+i);
	}
#endif

/*
   pics
*/
	start = LATCHPICS_LUMP_START;
	end = LATCHPICS_LUMP_END;

	for (i=start;i<=end;i++)
	{
		/* TODO: this just caches them for eternity */
		CA_CacheGrChunk (i);
	#if 0	
		latchpics[2+i-start] = destoff;
		width = pictable[i-STARTPICS].width;
		height = pictable[i-STARTPICS].height;
		VL_MemToLatch (grsegs[i],width,height,destoff);
		destoff += width/4 *height;
		UNCACHEGRCHUNK(i);
	#endif
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
		for (p=0;p<pixperframe;p++) {
			y = (rndval & 0x00FF) - 1;
			x = (rndval & 0x00FFFF00) >> 8;
			
			if (rndval & 1) {
				rndval >>= 1;
				rndval ^= 0x00012000;
			} else
				rndval >>= 1;
				
			if ((x>width) || (y>height))
				continue;

			*(graph_mem + (xx+x) + (yy+y) * 320) = *(gfxbuf + (xx+x) + (yy+y) * 320);

			if (rndval == 1) /* entire sequence has been completed */
				return false;

		}
		frame++;
		while (get_TimeCount() < frame)	;
	} while (1);
}
