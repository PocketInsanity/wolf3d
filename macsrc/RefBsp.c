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

static Word checkcoord[11][4] = {	/* Indexs to the bspcoord table */
{3,0,2,1},
{3,0,2,0},
{3,1,2,0},
{0,0,0,0},
{2,0,2,1},
{0,0,0,0},		/* Not valid */
{3,1,3,0},
{0,0,0,0},
{2,0,3,1},
{2,1,3,1},
{2,1,3,0}};

typedef	struct {
	Word top, bottom;
} screenpost_t;

#define	MAXSEGS	16

screenpost_t solidsegs[MAXSEGS], *newend;

/**********************************

	Clear out the edge segments for the ray cast
	(Assume full viewing angle)
	
**********************************/

void ClearClipSegs(void)
{
	solidsegs[0].top = -1;		/* Maximum angle */
	solidsegs[0].bottom = ANGLE180 + clipshortangle;	/* First edge */
	solidsegs[1].top = ANGLE180 - clipshortangle;		/* Left edge */
	solidsegs[1].bottom = 0;		/* Minimum angle */
	newend = solidsegs+2;
}

/**********************************

	Returns True if some part of the BSP dividing line might be visible
		
**********************************/

Boolean CheckBSPNode(Word boxpos)
{
	short 	angle1, angle2;
	unsigned short	span, tspan;
	unsigned short	uangle1, uangle2;
	screenpost_t	*start;
	Word *PosPtr;
	int x1,y1,x2,y2;
	
	PosPtr = &checkcoord[boxpos][0];
	x1 = bspcoord[PosPtr[0]];
	y1 = bspcoord[PosPtr[1]];
	x2 = bspcoord[PosPtr[2]];
	y2 = bspcoord[PosPtr[3]];
	
	angle1 = PointToAngle(x1,y1) - centershort;
	angle2 = PointToAngle(x2,y2) - centershort;
	
/* check clip list for an open space */

	span = angle1 - angle2;
	if (span >= 0x8000) {
		return TRUE;	/* sitting on a line*/
	}
	tspan = angle1 + clipshortangle;
	if (tspan > clipshortangle2) {
		tspan -= clipshortangle2;
		if (tspan >= span) {
			return FALSE;	/* totally off the left edge*/
		}
		angle1 = clipshortangle;
	}
	tspan = clipshortangle - angle2;
	if (tspan > clipshortangle2) {
		tspan -= clipshortangle2;
		if (tspan >= span) {
			return FALSE;	/* totally off the left edge*/
		}
		angle2 = -clipshortangle;
	}

/* find the first clippost that touches the source post (adjacent pixels are touching)*/
	uangle1 = angle1 + ANGLE180;
	uangle2 = angle2 + ANGLE180;
	start = solidsegs;
	while (start->bottom > uangle1+1) {
		++start;
	}
	if (uangle1 <= start->top && uangle2 >= start->bottom) {
		return FALSE;	/* the clippost contains the new span*/
	}
	return TRUE;
}


/**********************************

	Draw one or more wall segments
		
**********************************/

void TerminalNode (saveseg_t *seg)
{
	for (;;) {				/* Forever? */
		P_DrawSeg(seg);		/* Draw the wall segment (If visible) */
		if (seg->dir & DIR_LASTSEGFLAG) {	/* Last segment in list? */
			return;			/* Exit now */
		}
		++seg;				/* Index to the next wall segment */
	}
}

/**********************************

	Render the 3D view by recursivly following the BSP tree
	
**********************************/

void RenderBSPNode(Word bspnum)
{
	savenode_t *bsp;	/* Pointer to the current BSP node */
	Word side;			/* decision line */
	Word coordinate;	/* Current coord */
	Word savednum;		/* Save index */
	Word savedcoordinate;	/* Save value */
	Word boxpos;		/* Index to the coord table */

	bsp = &nodes[bspnum];		/* Get pointer to the current tree node */
	if (bsp->dir & DIR_SEGFLAG) {		/* There is a segment here... */	
		TerminalNode((saveseg_t *)bsp);	/* Render it */
		return;							/* Exit */
	}
	
/* decision node */

	coordinate = bsp->plane<<7;		/* stored as half tiles*/
	
	if (bsp->dir&1) {				/* True for vertical tiles */
		side = viewx > coordinate;	/* vertical decision line*/
		savednum = BSPLEFT + (side^1);	/* Left or right */
	} else {
		side = viewy > coordinate;	/* horizontal decision line*/
		savednum = BSPTOP + (side^1);	/* Top or bottom */
	}
	
	savedcoordinate = bspcoord[savednum];	/* Save this coord */
	bspcoord[savednum] = coordinate;	/* Set my new coord boundary */
	RenderBSPNode(bsp->children[side^1]);	/* recursively divide front space*/

	bspcoord[savednum] = savedcoordinate;	/* Restore the coord */
	savednum ^= 1;				/* Negate the index */
	savedcoordinate = bspcoord[savednum];	/* Save the other side */
	bspcoord[savednum] = coordinate;	/* Set the new side */
	
/* if the back side node is a single seg, don't bother explicitly checking visibility */

	if ( ! ( nodes[bsp->children[side]].dir & DIR_LASTSEGFLAG ) ) {
	
	/* don't flow into the back space if it is not going to be visible */

		if (viewx <= bspcoord[BSPLEFT]) {
			boxpos = 0;
		} else if (viewx < bspcoord[BSPRIGHT]) {
			boxpos = 1;
		} else {
			boxpos = 2;
		}
		if (viewy > bspcoord[BSPTOP]) {
			if (viewy < bspcoord[BSPBOTTOM]) {
				boxpos += 4;
			} else {
				boxpos += 8;
			}
		}
		if (!CheckBSPNode(boxpos)) {	/* Intersect possible? */
			goto skipback;				/* Exit now then */
		}
	}
	RenderBSPNode(bsp->children[side]);	/* recursively divide back space */
skipback:
	bspcoord[savednum] = savedcoordinate;
}
