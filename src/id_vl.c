/* id_vl.c */

#include "id_heads.h"

unsigned	bufferofs;
unsigned	displayofs;

boolean		screenfaded;

byte		palette1[256][3], palette2[256][3];

void VL_WaitVBL(int vbls)
{
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

void VL_ClearVideo (byte color)
{
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

void VL_FillPalette (int red, int green, int blue)
{
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
}

//===========================================================================

/*
=================
=
= VL_SetPalette
=
= If fast palette setting has been tested for, it is used
= (some cards don't like outsb palette setting)
=
=================
*/

void VL_SetPalette(byte *palette)
{
}


//===========================================================================

/*
=================
=
= VL_GetPalette
=
= This does not use the port string instructions,
= due to some incompatabilities
=
=================
*/

void VL_GetPalette(byte *palette)
{
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
= VL_MemToLatch
=
=================
*/

void VL_MemToLatch (byte *source, int width, int height, word dest)
{
#if 0
	unsigned	count;
	byte	plane,mask;

	count = ((width+3)/4)*height;
	mask = 1;
	for (plane = 0; plane<4 ; plane++)
	{
		VGAMAPMASK(mask);
		mask <<= 1;

asm	mov	cx,count
asm mov ax,SCREENSEG
asm mov es,ax
asm	mov	di,[dest]
asm	lds	si,[source]
asm	rep movsb
asm mov	ax,ss
asm	mov	ds,ax

		source+= count;
	}
#endif
}


//===========================================================================


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

/* ======================================================================== */

/*
=================
=
= VL_LatchToScreen
=
=================
*/

void VL_LatchToScreen (unsigned source, int width, int height, int x, int y)
{
#if 0
	VGAWRITEMODE(1);
	VGAMAPMASK(15);

asm	mov	di,[y]				// dest = bufferofs+ylookup[y]+(x>>2)
asm	shl	di,1
asm	mov	di,[WORD PTR ylookup+di]
asm	add	di,[bufferofs]
asm	mov	ax,[x]
asm	shr	ax,2
asm	add	di,ax

asm	mov	si,[source]
asm	mov	ax,[width]
asm	mov	bx,[linewidth]
asm	sub	bx,ax
asm	mov	dx,[height]
asm	mov	cx,SCREENSEG
asm	mov	ds,cx
asm	mov	es,cx

drawline:
asm	mov	cx,ax
asm	rep movsb
asm	add	di,bx
asm	dec	dx
asm	jnz	drawline

asm	mov	ax,ss
asm	mov	ds,ax

	VGAWRITEMODE(0);
#endif
}

/* ======================================================================== */

#if 0

/*
=================
=
= VL_ScreenToScreen
=
=================
*/

void VL_ScreenToScreen (unsigned source, unsigned dest,int width, int height)
{
	VGAWRITEMODE(1);
	VGAMAPMASK(15);

asm	mov	si,[source]
asm	mov	di,[dest]
asm	mov	ax,[width]
asm	mov	bx,[linewidth]
asm	sub	bx,ax
asm	mov	dx,[height]
asm	mov	cx,SCREENSEG
asm	mov	ds,cx
asm	mov	es,cx

drawline:
asm	mov	cx,ax
asm	rep movsb
asm	add	si,bx
asm	add	di,bx
asm	dec	dx
asm	jnz	drawline

asm	mov	ax,ss
asm	mov	ds,ax

	VGAWRITEMODE(0);
}

#endif
