/* wl_scale.c */

#include "wl_def.h" 

typedef struct
{
	word leftpix, rightpix;
	word dataofs[64];
        /* table data after dataofs[rightpix-leftpix+1] */
} PACKED t_compshape;

/* ======================================================================== */

/* TODO: this accesses gfxbuf directly! */
static void ScaledDraw(byte *gfx, int scale, byte *vid, unsigned long tfrac, unsigned long tint, unsigned long delta)
{
	unsigned long OldDelta;
	
	while (scale--) {
		*vid = *gfx;
		vid += 320; /* TODO: compiled in constant! */
		OldDelta = delta;
		delta += tfrac;
		gfx += tint;

		if (OldDelta > delta)
			gfx += 1;
	}
}

static void ScaledDrawTrans(byte *gfx, int scale, byte *vid, unsigned long tfrac, unsigned long tint, unsigned long delta)
{
	unsigned long OldDelta;
	
	while (scale--) {
		if (*gfx != 255)
			*vid = *gfx;
		vid += 320; /* TODO: compiled in constant! */
		OldDelta = delta;
		delta += tfrac;
		gfx += tint;

		if (OldDelta > delta)
			gfx += 1;
	}
}

void ScaleLine(unsigned int height, byte *source, int x)
{
	unsigned long TheFrac;
	unsigned long TheInt;
	unsigned long y;
	
	if (height) {
		TheFrac = 0x40000000UL / height;	
		
		if (height < viewheight) {
			y = yoffset + (viewheight - height) / 2;
			TheInt = TheFrac >> 24;
			TheFrac <<= 8;
			
			ScaledDraw(source, height, gfxbuf + (y * 320) + x + xoffset, 
			TheFrac, TheInt, 0);
			
			return;	
		} 
		
		y = (height - viewheight) / 2;
		y *= TheFrac;
		
		TheInt = TheFrac >> 24;
		TheFrac <<= 8;
		
		ScaledDraw(&source[y >> 24], viewheight, gfxbuf + (yoffset * 320) + x + xoffset, 
		TheFrac, TheInt, y << 8);
	}
}

static void ScaleLineTrans(unsigned int height, byte *source, int x)
{
	unsigned long TheFrac;
	unsigned long TheInt;
	unsigned long y;
	
	if (height) {
		TheFrac = 0x40000000UL / height;	
		
		if (height < viewheight) {
			y = yoffset + (viewheight - height) / 2;
			TheInt = TheFrac >> 24;
			TheFrac <<= 8;
			
			ScaledDrawTrans(source, height, gfxbuf + (y * 320) + x + xoffset, 
			TheFrac, TheInt, 0);
			
			return;	
		} 
		
		y = (height - viewheight) / 2;
		y *= TheFrac;
		
		TheInt = TheFrac >> 24;
		TheFrac <<= 8;
		
		ScaledDrawTrans(&source[y >> 24], viewheight, gfxbuf + (yoffset * 320) + x + xoffset, 
		TheFrac, TheInt, y << 8);
	}
}

static unsigned char *spritegfx[SPR_TOTAL];

static void DeCompileSprite(int shapenum)
{
	t_compshape *ptr;
	unsigned char *buf;
	int srcx;
	unsigned short int *cmdptr;
	short int *linecmds;
	unsigned char *pixels;
	int y, y0, y1;
	
	MM_GetPtr((void *)&buf, 64 * 64);
	
	memset(buf, 255, 64 * 64);
	
	ptr = PM_GetSpritePage(shapenum);

	cmdptr = &ptr->dataofs[31 - ptr->leftpix];
	
	for (srcx = 31; srcx >= ptr->leftpix; srcx--) {
		linecmds = (short *)((unsigned char *)ptr + *cmdptr--);
		
		while (linecmds[0]) {
			y0 = linecmds[2] / 2;
			y1 = linecmds[0] / 2;
			pixels = (unsigned char *)ptr + y0 + linecmds[1];
			
			for (y = y0; y < y1; y++) {
				//*(buf + slinex + (y*64)) = *pixels;
				*(buf + (srcx*64) + y) = *pixels;
				pixels++;
			}
			linecmds += 3;
		}
	}
	
	if (ptr->leftpix < 31) {
		srcx = 32;
		cmdptr = &ptr->dataofs[32 - ptr->leftpix];
	} else {
		srcx = ptr->leftpix;
		cmdptr = &ptr->dataofs[0];
	}
	
	for (; srcx <= ptr->rightpix; srcx++) {
		linecmds = (short *)((unsigned char *)ptr + *cmdptr++);
		
		while (linecmds[0]) {
			y0 = linecmds[2] / 2;
			y1 = linecmds[0] / 2;
			pixels = (unsigned char *)ptr + y0 + linecmds[1];
			
			for (y = y0; y < y1; y++) {
				//*(buf + slinex + (y*64)) = *pixels;
				*(buf + (srcx*64) + y) = *pixels;
				pixels++;
			}
			linecmds += 3;
		}
	}
	
	spritegfx[shapenum] = buf;
}

void ScaleShape(int xcenter, int shapenum, unsigned height)
{
	unsigned int scaler = (64 << 16) / (height >> 2);
	unsigned int x, p;

	if (spritegfx[shapenum] == NULL)
		DeCompileSprite(shapenum);
	
	for (p = xcenter - (height >> 3), x = 0; x < (64 << 16); x += scaler, p++) {
		if ((p < 0) || (p >= viewwidth) || (wallheight[p] >= height))
			continue;
		ScaleLineTrans(height >> 2, spritegfx[shapenum] + ((x >> 16) << 6), p);
	}	
}

void SimpleScaleShape(int xcenter, int shapenum, unsigned height)
{
	unsigned int scaler = (64 << 16) / height;
	unsigned int x, p;
	
	if (spritegfx[shapenum] == NULL)
		DeCompileSprite(shapenum);
	
	for (p = xcenter - (height / 2), x = 0; x < (64 << 16); x += scaler, p++) {
		if ((p < 0) || (p >= viewwidth))
			continue;
		ScaleLineTrans(height, spritegfx[shapenum] + ((x >> 16) << 6), p);
	}
}
