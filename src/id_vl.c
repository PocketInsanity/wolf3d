/* id_vl.c */

#include "id_heads.h"

boolean	screenfaded;

byte palette1[256][3], palette2[256][3];

byte *gfxbuf = NULL;

void VL_WaitVBL(int vbls)
{
	vga_waitretrace();
}

void VW_UpdateScreen()
{
	memcpy(graph_mem, gfxbuf, 64000);
}

/*
=======================
=
= VL_Startup
=
=======================
*/

void VL_Startup (void)
{
	if (gfxbuf == NULL) 
		gfxbuf = malloc(320 * 200 * 1);
		
	vga_init();
	vga_setmode(G320x200x256);
}

/*
=======================
=
= VL_Shutdown
=
=======================
*/

void VL_Shutdown (void)
{
	if (gfxbuf != NULL) {
		free(gfxbuf);
		gfxbuf = NULL;
	}
	vga_setmode(TEXT);
}

//===========================================================================

/*
=================
=
= VL_ClearVideo
=
= Fill the entire video buffer with a given color
=
=================
*/

void VL_ClearVideo(byte color)
{
	memset(gfxbuf, color, 64000);
}

/*
=============================================================================

						PALETTE OPS

		To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/


/*
=================
=
= VL_FillPalette
=
=================
*/

void VL_FillPalette(int red, int green, int blue)
{
	int i;
	
	for (i = 0; i < 256; i++) 
		vga_setpalette(i, red, green, blue);	
}

//===========================================================================

/*
=================
=
= VL_SetColor
=
=================
*/

void VL_SetColor(int color, int red, int green, int blue)
{
	vga_setpalette(color, red, green, blue);
}

//===========================================================================

/*
=================
=
= VL_GetColor
=
=================
*/

void VL_GetColor(int color, int *red, int *green, int *blue)
{
	vga_getpalette(color, red, green, blue);
}

//===========================================================================

/*
=================
=
= VL_SetPalette
=
=================
*/

void VL_SetPalette(byte *palette)
{
	int i;
	
	for (i = 0; i < 256; i++)
		vga_setpalette(i, palette[i*3+0], palette[i*3+1], palette[i*3+2]);
}


//===========================================================================

/*
=================
=
= VL_GetPalette
=
=================
*/

void VL_GetPalette(byte *palette)
{
	int i, r, g, b;
	
	for (i = 0; i < 256; i++) {
		vga_getpalette(i, &r, &g, &b);
		palette[i*3+0] = r;
		palette[i*3+1] = g;
		palette[i*3+2] = b;
	}
}


//===========================================================================

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

	VL_WaitVBL(1);
	VL_GetPalette (&palette1[0][0]);
	memcpy (palette2,palette1,768);

//
// fade through intermediate frames
//
	for (i=0;i<steps;i++)
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
		VL_SetPalette (&palette2[0][0]);
	}

//
// final color
//
	VL_FillPalette (red,green,blue);

	screenfaded = true;
}


/*
=================
=
= VL_FadeIn
=
=================
*/

void VL_FadeIn(int start, int end, byte *palette, int steps)
{
	int		i,j,delta;

	VL_WaitVBL(1);
	VL_GetPalette (&palette1[0][0]);
	memcpy (&palette2[0][0],&palette1[0][0],sizeof(palette1));

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
		VL_SetPalette (&palette2[0][0]);
	}

//
// final color
//
	VL_SetPalette (palette);
	screenfaded = false;
}

/*
==================
=
= VL_ColorBorder
=
==================
*/

void VL_ColorBorder (int color)
{
}

/*
=============================================================================

							PIXEL OPS

=============================================================================
*/

byte	pixmasks[4] = {1,2,4,8};
byte	leftmasks[4] = {15,14,12,8};
byte	rightmasks[4] = {1,3,7,15};

/*
=================
=
= VL_Plot
=
=================
*/

void VL_Plot(int x, int y, int color)
{
	*(gfxbuf + 320 * y + x) = color;
}

/*
=================
=
= VL_Hlin
=
=================
*/

void VL_Hlin(unsigned x, unsigned y, unsigned width, unsigned color)
{
	memset(gfxbuf + 320 * y + x, color, width);
}

/*
=================
=
= VL_Vlin
=
=================
*/

void VL_Vlin (int x, int y, int height, int color)
{
	byte *ptr = gfxbuf + 320 * y + x;
	while (height--) {
		*ptr = color;
		ptr += 320;
	}
}

/*
=================
=
= VL_Bar
=
=================
*/

void VL_Bar(int x, int y, int width, int height, int color)
{
	byte *ptr = gfxbuf + 320 * y + x;
	while (height--) {
		memset(ptr, color, width);
		ptr += 320;
	}
}

/*
============================================================================

							MEMORY OPS

============================================================================
*/

/*
=================
=
= VL_MemToScreen
=
= Draws a block of data to the screen.
=
=================
*/

void VL_MemToScreen(byte *source, int width, int height, int x, int y)
{
	byte *ptr = gfxbuf + 320 * y + x;
	while(height--) {
		memcpy(ptr, source, width);
		source += width;
		ptr += 320;
	}
}

void VL_DeModeXize(byte *buf, int width, int height)
{
	byte *mem, *ptr, *destline;
	int plane, x, y;
	
	if (width & 3) {
		printf("Not divisible by 4?\n");
		return;
	}
	
	mem = malloc(width * height);
	ptr = buf;
	for (plane = 0; plane < 4; plane++) {
		destline = mem;
		for (y = 0; y < height; y++) {
			for (x = 0; x < width / 4; x++)
				*(destline + x*4 + plane) = *ptr++;
			destline += width;
		}
	}
	memcpy(buf, mem, width * height);
	free(mem);
}
