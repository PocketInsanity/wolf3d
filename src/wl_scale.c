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
    unsigned count [65];
    /* the destination pixel for each source pixel row */
    short desty [65];
} t_scaledata;

t_scaledata scaledata [MAXSCALEHEIGHT+1];

int maxscale,maxscaleshl2;

void BuildCompScale(int height)
{
	long		fix,step;
	unsigned	src;
	int		startpix,endpix,toppix;


	step = ((long)height<<16) / 64;
	toppix = (viewheight-height)/2;
	fix = 0;

	for (src=0;src<=64;src++)
	{
		startpix = fix>>16;
		fix += step;
		endpix = fix>>16;
		if (endpix>startpix)
		    scaledata [height].count[src] = endpix-startpix;
		else
		    scaledata [height].count[src] = 0;

		startpix+=toppix;
		endpix+=toppix;

		if (startpix == endpix || endpix < 0 || startpix >= viewheight || src == 64)
		    /* source pixel goes off screen */
		    scaledata [height].desty [src] = -1;
		else
		    scaledata [height].desty [src] = startpix;
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
	maxscaleshl2 = maxscale<<2;

//
// build the compiled scalers
//
	for (i=1;i<=maxscaleheight;i++)
	{
		BuildCompScale (i);
	}

}

//===========================================================================

/*
========================
=
= BuildCompScale
=
= Builds a compiled scaler object that will scale a 64 tall object to
= the given height (centered vertically on the screen)
=
= height should be even
=
========================
*/

void xBuildCompScale(int height, byte *source, int x)
{

	int			i;
	long		fix,step;
	int	 startpix,endpix,toppix;
	unsigned char al;
	
	step = height << 10; 
	
	toppix = (viewheight-height) >> 1;
		
	fix = 0;
		
	for (i = 0; i < 64; i++)
	{
		startpix = fix>>16;
		fix += step;
		endpix = fix>>16;
		
		startpix+=toppix;
		endpix+=toppix;

		if (startpix == endpix || endpix < 0 || startpix >= viewheight)
			continue;

	       al = source[i];

		for (;startpix<endpix;startpix++)
		{
		    if (startpix >= viewheight)
			break;	   /* off the bottom of the view area */
		    if (startpix < 0)
			continue;	/* not into the view area */
		    VL_Plot(x+xoffset, startpix+yoffset, al);
		}

	}
}


/*
=======================
=
= ScaleLine
=
=======================
*/

int slinex, slinewidth;
short *linecmds;
int linescale;
t_compshape *shapeptr;

/* 
   linecmds - points to line segment data 
   slinewidth - pixels across
   slinex - screen coord of first column
*/

void ScaleLine()
{
	int x, y, ys;
	int n, ny;
	int y0, y1;
	unsigned char *pixels;
	unsigned char color;

    while (linecmds[0]) {
	y0 = linecmds[2]/2;
	y1 = linecmds[0]/2 - 1;
	pixels = (unsigned char *) shapeptr + y0 + linecmds[1];

	for (y=y0; y<=y1; y++) {
	    ys = scaledata[linescale].desty[y];
	    color = *pixels++;
	    if (ys >= 0) {
		for (ny=0; ny<scaledata[linescale].count[y]; ny++) {
		if ( (ys+ny) >= viewheight ) break; /* TODO: fix BuildCompScale so this isn't necessary */
		    for (n = 0, x = slinex; n < slinewidth; n++, x++) {
			VL_Plot(x+xoffset, ys+ny+yoffset, color);
		    }
		}
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
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
=======================
*/

void ScaleShape(int xcenter, int shapenum, unsigned height)
{
	t_compshape	*shape;
	unsigned	scale,srcx,stopx;
	word *cmdptr;
	boolean		leftvis,rightvis;

	shape = PM_GetSpritePage (shapenum);

	scale = height>>2;		// low three bits are fractional
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

	while ( --srcx >=stopx && slinex>0)
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
			ScaleLine ();
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
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
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

		ScaleLine ();
		slinex+=slinewidth;
	}
}
