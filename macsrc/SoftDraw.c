/*
Copyright (C) 1992-1994 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "wolfdef.h"

void ScaledDraw(Byte *gfx, Word scale, Byte *vid, LongWord TheFrac, Word TheInt, Word Width, LongWord Delta)
{	
	LongWord OldDelta;
	while (scale--) {
		*vid = *gfx;
		vid += Width;
		OldDelta = Delta;
		Delta += TheFrac;
		gfx += TheInt;
		if (OldDelta > Delta)
			gfx += 1;			
	}
}

void IO_ScaleWallColumn(Word x, Word scale, Word tile, Word column)
{
	LongWord TheFrac;
	Word TheInt;
	LongWord y;
	
	Byte *ArtStart;
	
	if (scale) {
		scale*=2;
		TheFrac = 0x80000000UL / scale;

		ArtStart = &ArtData[tile][column<<7];
		if (scale<VIEWHEIGHT) {
			y = (VIEWHEIGHT-scale)/2;
			TheInt = TheFrac>>24;
			TheFrac <<= 8;
			
			ScaledDraw(ArtStart,scale,&VideoPointer[(y*VideoWidth)+x],
			TheFrac,TheInt,VideoWidth, 0);
			
			return;
			
		}
		y = (scale-VIEWHEIGHT)/2;
		y *= TheFrac;
		TheInt = TheFrac>>24;
		TheFrac <<= 8;
		
		ScaledDraw(&ArtStart[y>>24],VIEWHEIGHT,&VideoPointer[x],
		TheFrac,TheInt,VideoWidth,y<<8);
	}
}

typedef struct {
	SWord Topy;
	SWord Boty;
	SWord Shape;
} PACKED SpriteRun;
                        
void IO_ScaleMaskedColumn(Word x,Word scale, unsigned short *CharPtr,Word column)
{
	Byte * CharPtr2;
	int Y1,Y2;
	Byte *Screenad;
	SpriteRun *RunPtr;
	LongWord TheFrac;
	LongWord TFrac;
	LongWord TInt;
	Word RunCount;
	int TopY;
	Word Index;
	LongWord Delta;
	
	if (!scale) 
		return;
		
	CharPtr2 = (Byte *) CharPtr;
	
	TheFrac = 0x40000000/scale;
	
	RunPtr = (SpriteRun *)&CharPtr[sMSB(CharPtr[column+1])/2]; 
	Screenad = &VideoPointer[x];
	TFrac = TheFrac<<8;
	TInt = TheFrac>>24;
	TopY = (VIEWHEIGHT/2)-scale;
	
	while (RunPtr->Topy != 0xFFFF) {
		Y1 = scale*(LongWord)sMSB(RunPtr->Topy)/128+TopY;
		if (Y1 < VIEWHEIGHT) {
			Y2 = scale*(LongWord)sMSB(RunPtr->Boty)/128+TopY;
			if (Y2 > 0) {
				if (Y2 > VIEWHEIGHT) 
					Y2 = VIEWHEIGHT;
				Index = sMSB(RunPtr->Shape)+sMSB(RunPtr->Topy)/2;
				Delta = 0;
				if (Y1 < 0) {
					Delta = (0-(LongWord)Y1)*TheFrac;
					Index += (Delta>>24);
					Delta <<= 8;
					Y1 = 0;
				}
				RunCount = Y2-Y1;
				if (RunCount) 
					ScaledDraw(&CharPtr2[Index],RunCount,
					&Screenad[Y1*VideoWidth],TFrac,TInt,VideoWidth, Delta);
			}
		}
		RunPtr++;
	}
}

Byte *SmallFontPtr;
	
void MakeSmallFont(void)
{
	Word i,j,Width,Height;
	Byte *DestPtr,*ArtStart;
	Byte *TempPtr;
	
	SmallFontPtr = AllocSomeMem(16*16*65);
	if (!SmallFontPtr) {
		return;
	}
	
	memset(SmallFontPtr,0,16*16*65);
	
	i = 0;
	DestPtr = SmallFontPtr;
	do {
		ArtStart = &ArtData[i][0];
		
		if (!ArtStart) {
			DestPtr+=(16*16);
		} else {
			Height = 0;
			do {
				Width = 16;
				j = Height*8;
				do {
					DestPtr[0] = ArtStart[j];
					++DestPtr;
					j+=(WALLHEIGHT*8);
				} while (--Width);
			} while (++Height<16);
		}
	} while (++i<64);
	
	TempPtr = LoadAResource(MyBJFace);
	memcpy(DestPtr,TempPtr,16*16);
	ReleaseAResource(MyBJFace);
}

void KillSmallFont(void)
{
	if (SmallFontPtr) {
		FreeSomeMem(SmallFontPtr);
		SmallFontPtr = 0;
	}
}

void DrawSmall(Word x,Word y,Word tile)
{
	Byte *Screenad;
	Byte *ArtStart;
	Word Width,Height;
	
	if (!SmallFontPtr) {
		return;
	}
	
	x *= 16;
	y *= 16;
	
	Screenad = &VideoPointer[YTable[y]+x];
	ArtStart = &SmallFontPtr[tile*(16*16)];
	Height = 0;
	
	do {
		Width = 16;
		do {
			Screenad[0] = ArtStart[0];
			++Screenad;
			++ArtStart;
		} while (--Width);
		Screenad+=VideoWidth-16;
	} while (++Height<16);	
}

/**********************************

	Graphics subsystem

**********************************/

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

	Draw a 3-D textured polygon, must be done FAST!
		
**********************************/

void RenderWallLoop(Word x1,Word x2,Word distance)
{
	fixed_t	texturecolumn;
	Word tile,scaler, angle;
	
/* calculate and draw each column */
	
	if (rw_downside) {
		while (x1 < x2) {		/* Time to draw? */
			scaler = rw_scale >> FRACBITS;		/* Get the draw scale */
			xscale[x1] = scaler;		/* Save the scale factor */
			angle = (xtoviewangle[x1]+rw_centerangle)&4095;/* TODO: for some reason i had to add this */
			texturecolumn = rw_midpoint - SUFixedMul(finetangent[angle],distance);	/* Which texture to use? */	
			if ((Word)texturecolumn < rw_mintex) {
				texturecolumn = rw_mintex;
			} else if ((Word)texturecolumn >= rw_maxtex) {
				texturecolumn = rw_maxtex-1;
			}
			tile = rw_texture[texturecolumn>>8];	/* Get the tile to use */
			IO_ScaleWallColumn(x1,scaler,tile,(texturecolumn>>1)&127);	/* Draw the line */
			++x1;						/* Next x */
			rw_scale+=rw_scalestep;		/* Step the scale factor for the wall */
		}
		return;
	}
	while (x1 < x2) {		/* Time to draw? */
		scaler = rw_scale >> FRACBITS;		/* Get the draw scale */
		xscale[x1] = scaler;		/* Save the scale factor */
		angle = (xtoviewangle[x1]+rw_centerangle)&4095; /* TODO: same as above */
		texturecolumn = SUFixedMul(finetangent[angle],distance)+rw_midpoint;	/* Which texture to use? */
		if ((Word)texturecolumn < rw_mintex) {
			texturecolumn = rw_mintex;
		} else if ((Word)texturecolumn >= rw_maxtex) {
			texturecolumn = rw_maxtex-1;
		}
		tile = rw_texture[texturecolumn>>8];	/* Get the tile to use */
		if (!(sMSB(WallListPtr[tile+1]) & 0x4000)) {
			texturecolumn^=0xff;	/* Reverse the tile for N,W walls */
		}
		IO_ScaleWallColumn(x1,scaler,tile,(texturecolumn>>1)&127);	/* Draw the line */
		++x1;						/* Next x */
		rw_scale+=rw_scalestep;		/* Step the scale factor for the wall */
	}
}

/*
=====================
=
= RenderWallRange
=
= Draw a wall segment between start and stop angles (inclusive) (short angles)
= No clipping is needed
=
======================
*/

void RenderWallRange (Word start,Word stop,saveseg_t *seg,Word distance)
{
	LongWord scale2;
	Word vangle;
	Word x1,x2;

/* mark the segment as visible for auto map*/

	seg->dir |= DIR_SEENFLAG;		/* for automap*/
	areavis[seg->area] = 1;			/* for sprite drawing*/
	
	start -= ANGLE180;		/* Adjust the start angle */
	stop -= ANGLE180;		/* Adjust the stop angle */
	vangle = (Word)(start+ANGLE90)>>ANGLETOFINESHIFT;
	x1 = viewangletox[vangle];
	vangle = (Word)(stop+ANGLE90-1)>>ANGLETOFINESHIFT;	/* make non inclusive*/
	x2 = viewangletox[vangle];
	if (x2 == x1) {
		return;		/* less than one column wide*/
	}
	rw_scale = (long) ScaleFromGlobalAngle(start+centershort,distance)<<FRACBITS;
	if (x2>x1+1) {
		scale2 = (long) ScaleFromGlobalAngle(stop+centershort,distance)<<FRACBITS;
		rw_scalestep = (long)(scale2-rw_scale)/(long)(x2-x1);
	}
	RenderWallLoop(x1,x2,distance);
}

/*
===============================================================================
=
= ClipWallSegment
=
= Clips the given screenpost and includes it in newcolumn
===============================================================================
*/

/* a screenpost_t is a solid range of visangles, used to clip and detect*/
/* span exposures / hidings*/

typedef	struct {
	Word top, bottom;
} screenpost_t;

#define	MAXSEGS	16

extern screenpost_t solidsegs[MAXSEGS], *newend;	/* newend is one past the last valid seg */

void ClipWallSegment(Word top,Word bottom,saveseg_t *seg,Word distance)
{
	screenpost_t *next, *start;
	
/* find the first clippost that touches the source post (adjacent pixels are touching)*/
	start = solidsegs;
	while (start->bottom > top+1) {
		start++;
	}

	if (top > start->top) {
		if (bottom > start->top+1) {	/* post is entirely visible (above start), so insert a new clippost*/
			RenderWallRange(top, bottom,seg,distance);
			next = newend;
			newend++;
			while (next != start) {
				*next = *(next-1);
				next--;
			}
			next->top = top;
			next->bottom = bottom;
			return;
		}
		
	/* there is a fragment above *start*/
		RenderWallRange (top, start->top + 1,seg,distance);
		start->top = top;		/* adjust the clip size*/
	}
	
	if (bottom >= start->bottom)
		return;			/* bottom contained in start*/
		
	next = start;
	while (bottom <= (next+1)->top+1) {
		/* there is a fragment between two posts*/
		RenderWallRange (next->bottom - 1, (next+1)->top + 1,seg,distance);
		next++;
		if (bottom >= next->bottom) {	/* bottom is contained in next*/
			start->bottom = next->bottom;	/* adjust the clip size*/
			goto crunch;
		}
	}
	
	/* there is a fragment after *next*/
	RenderWallRange (next->bottom - 1, bottom,seg,distance);
	start->bottom = bottom;		/* adjust the clip size*/
	
	
/* remove start+1 to next from the clip list, because start now covers their area*/
crunch:
	if (next == start) {
		return;			/* post just extended past the bottom of one post*/
	}
	while (next++ != newend)	/* remove a post*/
		*++start = *next;
	newend = start+1;
}

/**********************************

	Clip and draw a given wall segment
		
**********************************/

void P_DrawSeg (saveseg_t *seg)
{
	Word	segplane;
	Word	door;
	door_t	*door_p;
	unsigned short	span, tspan;
	unsigned short	angle1, angle2;
	int		texslide;
	int		distance;
	
	if (seg->dir & DIR_DISABLEDFLAG) {	/* Segment shut down? */
		return;		/* pushwall part*/
	}

	segplane = (Word)seg->plane << 7;
	rw_mintex = (Word)seg->min << 7;
	rw_maxtex = (Word)seg->max << 7;
	
/* adjust pushwall segs */

	if (seg == pwallseg) {		/* Is this the active pushwall? */
		if (seg->dir&1)	{	/* east/west */
			segplane += PushWallRec.pwallychange;
		} else {		/* north/south */
			segplane += PushWallRec.pwallxchange;
		}
	}
	
/* get texture*/

	if (seg->texture >= 129) {	/* segment is a door */
		door = seg->texture - 129;	/* Which door is this? */
		door_p = &doors[door];
		rw_texture = &textures[129 + (door_p->info>>1)][0];
		texslide = door_p->position;
		rw_mintex += texslide;
	} else {
		texslide = 0;
		rw_texture = &textures[seg->texture][0];
	}
	
	switch (seg->dir&3) {	/* mask off the flags*/
	case di_north:
		distance = viewx - segplane;
		if (distance <= 0) {
			return;		/* back side*/
		}
		rw_downside = FALSE;
		rw_midpoint = viewy;
		normalangle = 2*FINEANGLES/4;
		angle1 = PointToAngle(segplane,rw_maxtex);
		angle2 = PointToAngle(segplane,rw_mintex);
		break;
	case di_south:
		distance = segplane - viewx;
		if (distance <= 0) {
			return;		/* back side*/
		}
		rw_downside = TRUE;
		rw_midpoint = viewy;
		normalangle = 0*FINEANGLES/4;
		angle1 = PointToAngle(segplane,rw_mintex);
		angle2 = PointToAngle(segplane,rw_maxtex);
		break;
	case di_east:
		distance = viewy - segplane;
		if (distance <= 0) {
			return;		/* back side*/
		}
		rw_downside = TRUE;
		rw_midpoint = viewx;
		normalangle = 1*FINEANGLES/4;
		angle1 = PointToAngle(rw_mintex,segplane);
		angle2 = PointToAngle(rw_maxtex,segplane);
		break;
	case di_west:
		distance = segplane - viewy;
		if (distance <= 0) {
			return;		/* back side*/
		}
		rw_downside = FALSE;
		rw_midpoint = viewx;
		normalangle = 3*FINEANGLES/4;
		angle1 = PointToAngle(rw_maxtex,segplane);
		angle2 = PointToAngle(rw_mintex,segplane);
		break;
	}

/* clip to view edges*/

	span = angle1 - angle2;
	if (span >= 0x8000) {		/* Test for negative (32 bit clean) */
		return;
	}
	angle1 -= centershort;
	angle2 -= centershort;
	++angle2;	/* make angle 2 non inclusive*/
	
	tspan = angle1 + clipshortangle;
	if (tspan > clipshortangle2) {
		tspan -= clipshortangle2;
		if (tspan >= span) {
			return;	/* totally off the left edge*/
		}
		angle1 = clipshortangle;
	}
	tspan = clipshortangle - angle2;
	if (tspan > clipshortangle2) {
		tspan -= clipshortangle2;
		if (tspan >= span) {
			return;	/* totally off the left edge*/
		}
		angle2 = -clipshortangle;
	}

/* calc center angle for texture mapping*/

	rw_centerangle = (centerangle-normalangle)&FINEMASK;
	if (rw_centerangle > (FINEANGLES/2)) {
		rw_centerangle -= FINEANGLES;
	}
	rw_centerangle += FINEANGLES/4;
	
	rw_midpoint -= texslide;
	rw_mintex -= texslide;
	
	angle1 += ANGLE180;		/* adjust so angles are unsigned*/
	angle2 += ANGLE180;
	ClipWallSegment(angle1, angle2,seg,distance);
}

void StartRenderView()
{
}

void InitRenderView()
{
}
