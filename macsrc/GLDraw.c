/*
Copyright (C) 1992-1994 Id Software, Inc.
Copyright (C) 2000 Steven Fuller <relnev@atdot.org>

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

#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glext.h>

#include "wolfdef.h"

extern Byte Pal[768];

void ClearTheScreen(Word c)
{
	glClearColor((double)Pal[c*3+0]/256.0, (double)Pal[c*3+1]/256.0, (double)Pal[c*3+2]/256.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void DrawShape(Word x, Word y, void *ShapePtr)
{
}

void DrawXMShape(Word x, Word y, void *ShapePtr)
{
}

void DrawSmall(Word x,Word y,Word tile)
{
}

void MakeSmallFont(void)
{
}

void KillSmallFont(void)
{
}

Byte *Pal256toRGB(Byte *dat, int len, Byte *pal)
{
	Byte *buf;
	int i;
	
	buf = (Byte *)malloc(len * 3);
	
	for (i = 0; i < len; i++) {
		buf[i*3+0] = pal[dat[i]*3+0];
		buf[i*3+1] = pal[dat[i]*3+1];
		buf[i*3+2] = pal[dat[i]*3+2];
	}
	
	return buf;
}

Byte *Pal256toRGBA(Byte *dat, int len, Byte *pal)
{
	Byte *buf;
	int i;
	
	buf = (Byte *)malloc(len * 4);
	
	for (i = 0; i < len; i++) {
		buf[i*4+0] = pal[dat[i]*3+0];
		buf[i*4+1] = pal[dat[i]*3+1];
		buf[i*4+2] = pal[dat[i]*3+2];
	}
	
	return buf;
}

void DrawSprites(void)
{
}

void DrawTopSprite(void)
{
}

GLuint waltex[64];

void InitRenderView()
{
	Byte *pal;
	int i;
	
	glEnable(GL_TEXTURE_2D);	
	if (waltex[0]) {
		glDeleteTextures(64, waltex);
		for (i = 0; i < 64; i++)
			waltex[i] = 0;
	}
	
	glGenTextures(64, waltex);
	
	pal = LoadAResource(rGamePal);
	for (i = 0; i < 64; i++) {
		Byte *buf;
		
		glBindTexture(GL_TEXTURE_2D, waltex[i]);
		if (ArtData[i] == NULL) {
			glDeleteTextures(1, &waltex[i]);
			glEnable(GL_TEXTURE_2D);
			waltex[i] = 0;
			continue;
		}
		
		buf = Pal256toRGB(ArtData[i], 128 * 128, pal);
		
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
				
		free(buf);
	}
	ReleaseAResource(rGamePal);
	
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glFrustum(-0.286751, 0.286751, -0.288675, 0.288675, 0.200000, 182.000000);
	glFrustum(-0.20, 0.20, -0.288675, 0.288675, 0.200000, 182.000000);
	//glFrustum(-0.1, 0.1, -0.1, 0.1, .50000, 182.000000);
	
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
}

void StartRenderView()
{	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(270.0-((double)gamestate.viewangle / (double)ANGLES * 360.0), 0.0, 1.0, 0.0);
	//glTranslatef(-(double)actors[0].x / 256.0, 0, (double)actors[0].y / 256.0);
	glTranslatef((double)actors[0].x / 256.0, 0, (double)actors[0].y / 256.0);
}

int WallSeen = 0;

void WallIsSeen(saveseg_t *seg)
{
/* mark the segment as visible for auto map*/

	seg->dir |= DIR_SEENFLAG;		/* for automap*/
	areavis[seg->area] = 1;			/* for sprite drawing*/

	WallSeen = 1;	
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

void ClipWallSegmentx(Word top, Word bottom, saveseg_t *seg)
{
	screenpost_t *next, *start;
	
/* find the first clippost that touches the source post (adjacent pixels are touching)*/
	start = solidsegs;
	while (start->bottom > top+1) {
		start++;
	}

	if (top > start->top) {
		if (bottom > start->top+1) {	/* post is entirely visible (above start), so insert a new clippost*/
			WallIsSeen(seg);
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
		WallIsSeen(seg);
		start->top = top;		/* adjust the clip size*/
	}
	
	if (bottom >= start->bottom)
		return;			/* bottom contained in start*/
		
	next = start;
	while (bottom <= (next+1)->top+1) {
		/* there is a fragment between two posts*/
		WallIsSeen(seg);
		next++;
		if (bottom >= next->bottom) {	/* bottom is contained in next*/
			start->bottom = next->bottom;	/* adjust the clip size*/
			goto crunch;
		}
	}
	
	/* there is a fragment after *next*/
	WallIsSeen(seg);
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
void P_DrawSegx(saveseg_t *seg);

void P_DrawSeg(saveseg_t *seg)
{
	Word	segplane;
	Word	door;
	door_t	*door_p;
	unsigned short	span, tspan;
	unsigned short	angle1, angle2;
	int		texslide;
	
	WallSeen = 0;
	
	if (seg->dir & DIR_DISABLEDFLAG) {	/* Segment shut down? */
		return;		/* pushwall part*/
	}

	segplane = (LongWord)seg->plane << 7;
	rw_mintex = (LongWord)seg->min << 7;
	rw_maxtex = (LongWord)seg->max << 7;
	
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
		if (viewx <= segplane) {
			return;		/* back side*/
		}
		angle1 = PointToAngle(segplane,rw_maxtex);
		angle2 = PointToAngle(segplane,rw_mintex);
		break;
	case di_south:
		if (segplane <= viewx) {
			return;		/* back side*/
		}
		angle1 = PointToAngle(segplane,rw_mintex);
		angle2 = PointToAngle(segplane,rw_maxtex);
		break;
	case di_east:
		if (viewy <= segplane) {
			return;		/* back side*/
		}
		angle1 = PointToAngle(rw_mintex,segplane);
		angle2 = PointToAngle(rw_maxtex,segplane);
		break;
	case di_west:
		if (segplane <= viewy) {
			return;		/* back side*/
		}
		angle1 = PointToAngle(rw_maxtex,segplane);
		angle2 = PointToAngle(rw_mintex,segplane);
		break;
	}

/* clip to view edges*/

	span = (angle1 - angle2);
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

	angle1 += ANGLE180;		/* adjust so angles are unsigned*/
	angle2 += ANGLE180;
	ClipWallSegmentx(angle1, angle2, seg);
	
	if (WallSeen) P_DrawSegx(seg);
}

void P_DrawSegx(saveseg_t *seg)
{
	GLfloat min, max;
	Byte *tex;
	int i, t;
	
	tex = &textures[seg->texture][0];
	
	if (waltex[i]) 
		glBindTexture(GL_TEXTURE_2D, waltex[i]);
	else
		fprintf(stderr, "ERROR: 0 texture in P_DrawSegx!\n");
	
	max = (seg->max - seg->min) / 2.0f;
	/*
	i = seg->min;
	min = (float)i - seg->min;
	*/
	if (seg->min & 1)
		min = -0.5f;
	else
		min = 0.0f;
	
	for (i = seg->min >> 1; i < (seg->max >> 1); i++) {
	
	t = tex[i];
	
	if (waltex[t]) 
		glBindTexture(GL_TEXTURE_2D, waltex[t]);
	else
		fprintf(stderr, "ERROR: 0 texture in P_DrawSegx!\n");	
	
	if (seg->min & 1)
		min = 0.5f;
	else
		min = 0.0f;
	max = 1.0f;
	
	glBegin(GL_QUADS);
	switch(seg->dir&3) {
	case di_north:
		glTexCoord2f(min, 0.0); glVertex3f(-(seg->plane)/2.0, -1, -min); 
		glTexCoord2f(min, 1.0); glVertex3f(-(seg->plane)/2.0,  1, -min);
		glTexCoord2f(max, 1.0); glVertex3f(-(seg->plane)/2.0,  1, -max);
		glTexCoord2f(max, 0.0); glVertex3f(-(seg->plane)/2.0, -1, -max);
		break;
	case di_south:
		glTexCoord2f(min, 0.0); glVertex3f(-(seg->plane)/2.0, -1, -(seg->max)/2.0); 
		glTexCoord2f(min, 1.0); glVertex3f(-(seg->plane)/2.0,  1, -(seg->max)/2.0);
		glTexCoord2f(max, 1.0); glVertex3f(-(seg->plane)/2.0,  1, -(seg->min)/2.0);
		glTexCoord2f(max, 0.0); glVertex3f(-(seg->plane)/2.0, -1, -(seg->min)/2.0);
		break;
	case di_east:
		glTexCoord2f(min, 0.0); glVertex3f(-(seg->max)/2.0, -1, -(seg->plane)/2.0); 
		glTexCoord2f(min, 1.0); glVertex3f(-(seg->max)/2.0,  1, -(seg->plane)/2.0);
	 	glTexCoord2f(max, 1.0); glVertex3f(-(seg->min)/2.0,  1, -(seg->plane)/2.0);
		glTexCoord2f(max, 0.0); glVertex3f(-(seg->min)/2.0, -1, -(seg->plane)/2.0);
		break;
	case di_west:
		glTexCoord2f(min, 0.0); glVertex3f(-(seg->min)/2.0, -1, -(seg->plane)/2.0); 
		glTexCoord2f(min, 1.0); glVertex3f(-(seg->min)/2.0,  1, -(seg->plane)/2.0);
		glTexCoord2f(max, 1.0); glVertex3f(-(seg->max)/2.0,  1, -(seg->plane)/2.0);
		glTexCoord2f(max, 0.0); glVertex3f(-(seg->max)/2.0, -1, -(seg->plane)/2.0);
		break;	
	}
	glEnd();	
	
	}
}
