/* wl_scale.c */

#include "wl_def.h" 

/* Originally from David Haslam -- dch@sirius.demon.co.uk */

/*
=============================================================================

						  GLOBALS

=============================================================================
*/

/* scaling data for a given height */
typedef struct {
    /* number of destination pixels each source pixels maps to in x and y */
    int count[64];
    /* the destination pixel for each source pixel row */
    int desty[64];
} t_scaledata;

static t_scaledata scaledata[MAXSCALEHEIGHT+1];
static int maxscale;

static void BuildCompScale(int height)
{
	long fix, step;
	int src;
	int startpix, endpix, toppix;

	step = ((long)height << 16) / 64;
	toppix = (viewheight - height) / 2;
	fix = 0;
		
	for (src = 0; src < 64; src++)
	{
		startpix = fix >> 16;
		fix += step;
		endpix = fix >> 16;
		
		if (endpix > startpix)
		    scaledata[height].count[src] = endpix - startpix;
		else
		    scaledata[height].count[src] = 0;

		startpix += toppix;
		endpix += toppix;

		if ((startpix == endpix) || (endpix < 0) || (startpix >= viewheight) /*|| (src == 64)*/) {
			/* source pixel goes off screen */
			scaledata[height].desty[src] = -1;
		} else if (startpix < 0) {
			scaledata[height].desty[src] = 0;
		} else {
			scaledata[height].desty[src] = startpix;
			/* Clip if needed */    
			if ((scaledata[height].count[src] + scaledata[height].desty[src]) > viewheight)
				scaledata[height].count[src] = viewheight - scaledata [height].desty[src];
		}
	}

}

/*
==========================
=
= SetupScaling
=
==========================
*/

void SetupScaling(int maxscaleheight)
{
	int i;

	maxscale = maxscaleheight-1;
//
// build the compiled scalers
//
	for (i = 1; i <= maxscaleheight; i++) {
		BuildCompScale(i);
	}

}

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

void xBuildCompScale(unsigned int height, byte *source, int x)
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

/*
=======================
=
= ScaleLine
=
=======================
*/

static int slinex, slinewidth;
static short *linecmds;
static int linescale;
static t_compshape *shapeptr;

/* 
   linecmds - points to line segment data 
   slinewidth - pixels across
   slinex - screen coord of first column
*/

static void ScaleLine()
{
	int x, y, ys;
	int n, ny;
	int y0, y1;
	unsigned char *pixels;
	unsigned char color;

	while (linecmds[0]) {
		y0 = linecmds[2] / 2;
		y1 = linecmds[0] / 2;
		pixels = (unsigned char *)shapeptr + y0 + linecmds[1];

		for (y = y0; y < y1; y++) {
			color = *pixels++;
			ys = scaledata[linescale].desty[y];
			
			if (ys >= 0) {
				for (ny = 0; ny < scaledata[linescale].count[y]; ny++)
					for (n = 0, x = slinex; n < slinewidth; n++, x++)
						VL_Plot(x+xoffset, ys+ny+yoffset, color);
			}
		}
		linecmds += 3;
	}
}

/*
=======================
=
= ScaleShape
=
= Draws a compiled shape at [scale] pixels high
=
=======================
*/

void ScaleShape(int xcenter, int shapenum, unsigned height)
{
	t_compshape	*shape;
	unsigned	scale,srcx,stopx;
	word *cmdptr;
	boolean		leftvis,rightvis;

	shape = PM_GetSpritePage(shapenum);

	scale = height>>2;		// low three bits are fractional
	scale += 4; /* sprites look a bit better pulled up some */
	
	if (!scale || scale>maxscale)
		return;			// too close or far away


	linescale = scale;
	shapeptr = shape;
//
// scale to the left (from pixel 31 to shape->leftpix)
//
	srcx = 32;
	slinex = xcenter;
	stopx = shape->leftpix;
	cmdptr = (word *)&(shape->dataofs[31-stopx]);

	while ( --srcx >= stopx && slinex>0)
	{
		linecmds = (short *)((char *) shapeptr + *cmdptr--);
		if ( !(slinewidth = scaledata[scale].count[srcx]) )
			continue;

		if (slinewidth == 1)
		{
			slinex--;
			if (slinex<viewwidth)
			{
				if (wallheight[slinex] >= height)
					continue;		// obscured by closer wall
				ScaleLine ();
			}
			continue;
		}

		//
		// handle multi pixel lines
		//
		if (slinex>viewwidth)
		{
			slinex -= slinewidth;
			slinewidth = viewwidth-slinex;
			if (slinewidth<1)
				continue;		// still off the right side
		}
		else
		{
			if (slinewidth>slinex)
				slinewidth = slinex;
			slinex -= slinewidth;
		}


		leftvis = (wallheight[slinex] < height);
		rightvis = (wallheight[slinex+slinewidth-1] < height);

		if (leftvis)
		{
			if (rightvis)
				ScaleLine ();
			else
			{
				while (wallheight[slinex+slinewidth-1] >= height)
					slinewidth--;
				ScaleLine ();
			}
		}
		else
		{
			if (!rightvis)
				continue;		// totally obscured

			while (wallheight[slinex] >= height)
			{
				slinex++;
				slinewidth--;
			}
			ScaleLine();
			break;			// the rest of the shape is gone
		}
	}


//
// scale to the right
//
	slinex = xcenter;
	stopx = shape->rightpix;
	if (shape->leftpix<31)
	{
		srcx = 31;
		cmdptr = (word *)&shape->dataofs[32-shape->leftpix];
	}
	else
	{
		srcx = shape->leftpix-1;
		cmdptr = (word *)&shape->dataofs[0];
	}
	slinewidth = 0;

	while ( ++srcx <= stopx && (slinex+=slinewidth)<viewwidth)
	{
		linecmds = (short *)((char *) shapeptr + *cmdptr++);
		if ( !(slinewidth = scaledata[scale].count[srcx]) )
			continue;

		if (slinewidth == 1)
		{
			if (slinex>=0 && wallheight[slinex] < height)
			{
				ScaleLine ();
			}
			continue;
		}

		//
		// handle multi pixel lines
		//
		if (slinex<0)
		{
			if (slinewidth <= -slinex)
				continue;		// still off the left edge

			slinewidth += slinex;
			slinex = 0;
		}
		else
		{
			if (slinex + slinewidth > viewwidth)
				slinewidth = viewwidth-slinex;
		}


		leftvis = (wallheight[slinex] < height);
		rightvis = (wallheight[slinex+slinewidth-1] < height);

		if (leftvis)
		{
			if (rightvis)
			{
				ScaleLine ();
			}
			else
			{
				while (wallheight[slinex+slinewidth-1] >= height)
					slinewidth--;
				ScaleLine ();
				break;			// the rest of the shape is gone
			}
		}
		else
		{
			if (rightvis)
			{
				while (wallheight[slinex] >= height)
				{
					slinex++;
					slinewidth--;
				}
				ScaleLine ();
			}
			else
				continue;		// totally obscured
		}
	}
}



/*
=======================
=
= SimpleScaleShape
=
= NO CLIPPING, height in pixels
=
= Draws a compiled shape at [scale] pixels high
=
=======================
*/

void SimpleScaleShape(int xcenter, int shapenum, unsigned height)
{
	t_compshape	*shape;
	unsigned	scale,srcx,stopx;
	unsigned short	*cmdptr;

	shape = PM_GetSpritePage (shapenum);

	scale = height;

	linescale = scale;
	shapeptr = shape;
//
// scale to the left (from pixel 31 to shape->leftpix)
//
	srcx = 32;
	slinex = xcenter;
	stopx = shape->leftpix;
	cmdptr = (word *)&shape->dataofs[31-stopx];

	while ( --srcx >=stopx )
	{
		linecmds = (short *)((char *) shapeptr + *cmdptr--);
		if ( !(slinewidth = scaledata[scale].count[srcx]) )
			continue;
		slinex -= slinewidth;
		ScaleLine ();
	}


//
// scale to the right
//
	slinex = xcenter;
	stopx = shape->rightpix;
	if (shape->leftpix<31)
	{
		srcx = 31;
		cmdptr = (word *)&shape->dataofs[32-shape->leftpix];
	}
	else
	{
		srcx = shape->leftpix-1;
		cmdptr = (word *)&shape->dataofs[0];
	}
	slinewidth = 0;

	while ( ++srcx <= stopx )
	{
		linecmds = (short *)((char *) shapeptr + *cmdptr++);
		if ( !(slinewidth = scaledata[scale].count[srcx]) )
			continue;

		ScaleLine();
		slinex += slinewidth;
	}
}
