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

void DisplayScreen(Word res)
{
}

extern Byte Pal[768];

void FadeToPtr(unsigned char *PalPtr)
{
	SetPalette(PalPtr);
}

void SetAPalettePtr(unsigned char *PalPtr)
{
	SetPalette(PalPtr);
}

void ClearTheScreen(Word c)
{
	glClearColor((double)Pal[c*3+0]/256.0, (double)Pal[c*3+1]/256.0, (double)Pal[c*3+2]/256.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
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

void DrawShape(Word x, Word y, void *ShapePtr)
{
}

GLuint LastTexture;

GLuint smltex[65];

void MakeSmallFont()
{
	Byte *buf, *buf2, *pal, *ArtStart;
	Byte *ptr;
	int i, w, h, j;
	
	buf = (Byte *)malloc(16 * 16);
	
	pal = LoadAResource(rGamePal);
	
	LastTexture = 0;
	
	for (i = 0; i < 64; i++) {
		ArtStart = ArtData[i];
		
		if (ArtStart == NULL) {
			if (smltex[i]) {
				glDeleteTextures(1, &smltex[i]);
				smltex[i] = 0;
			}
			continue;
		} else {
			if (smltex[i])
				continue;

			glGenTextures(1, &smltex[i]);
						
			h = 0;
			ptr = buf;
			do {
				w = 16;
				j = h*8;
				do {
					*ptr = ArtStart[j];
					ptr++;
					j += (WALLHEIGHT*8);
				} while (--w);
			} while ((++h) < 16);
			
			buf2 = Pal256toRGB(buf, 16 * 16, pal);
			
			glBindTexture(GL_TEXTURE_2D, smltex[i]);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, buf2);
			
			free(buf2);
		}
	}
	
	free(buf);
	
	buf = LoadAResource(MyBJFace);
	buf2 = Pal256toRGB(buf, 16 * 16, pal);
	
	glGenTextures(1, &smltex[i]);
	glBindTexture(GL_TEXTURE_2D, smltex[i]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, buf2);
	
	free(buf2);
	ReleaseAResource(MyBJFace);
	
	ReleaseAResource(rGamePal);	
}

void KillSmallFont()
{
	int i;
	glDeleteTextures(65, smltex);	
	
	for (i = 0; i < 65; i++)
		smltex[i] = 0;
}

void DrawSmall(Word x, Word y, Word tile)
{
	GLfloat w, h;
	GLfloat xx, yy;
	GLfloat x1, y1, x2, y2;
	
	w = (int)(VidWidth >> 4);
	h = (int)(VidHeight >> 4);
	xx = x;
	yy = y;
	x1 = ((xx * 2.0f) - w) + 1.0;
	x2 = x1 - 2.0;
	y1 = -((yy * 2.0f) - h) + 1.0;
	y2 = y1 - 2.0;
		
	x1 /= w;
	x2 /= w;
	y1 /= h;
	y2 /= h;
		
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	LastTexture = smltex[tile];
	glBindTexture(GL_TEXTURE_2D, smltex[tile]);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); glVertex2f(x2, y2); 
	glTexCoord2f(0.0, 0.0); glVertex2f(x2, y1);
	glTexCoord2f(1.0, 0.0); glVertex2f(x1, y1);
	glTexCoord2f(1.0, 1.0); glVertex2f(x1, y2);
	glEnd();
	
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}


Byte *FlipWall(Byte *dat, int w, int h)
{
	Byte *buf;
	int i, j;
	
	buf = (Byte *)malloc(w * h);
	
	for (j = 0; j < h; j++) 
		for (i = 0; i < w; i++) 
			buf[(h-j-1)*w+(w-i-1)] = dat[i*w+j];
	return buf;
}

typedef struct {
	unsigned short Topy;
	unsigned short Boty;
	unsigned short Shape;
} PACKED SpriteRun;

Byte *DeSprite(Byte *data, Byte *pal)
{
	Byte *buf;
	unsigned short *dat = (unsigned short *)data;
	int i, x, width, offset;
	
	buf = (Byte *)malloc(128 * 128 * 4);
	memset(buf, 0, 128 * 128 * 4);
	
	width = sMSB(dat[0]);
	
	offset = 64 - width / 2;
	for (x = 0; x < width; x++) {
		SpriteRun *p = (SpriteRun *)&dat[ sMSB(dat[x+1]) / 2 ];
		
		while (p->Topy != 0xFFFF) {
			for (i = sMSB(p->Topy) / 2; i < sMSB(p->Boty) / 2; i++) {
				*(buf + (i * 128 + x + offset) * 4 + 0) = pal[data[sMSB(p->Shape)+sMSB(p->Topy)/2 + (i-sMSB(p->Topy)/2)]*3+0];
				*(buf + (i * 128 + x + offset) * 4 + 1) = pal[data[sMSB(p->Shape)+sMSB(p->Topy)/2 + (i-sMSB(p->Topy)/2)]*3+1];
				*(buf + (i * 128 + x + offset) * 4 + 2) = pal[data[sMSB(p->Shape)+sMSB(p->Topy)/2 + (i-sMSB(p->Topy)/2)]*3+2];
				*(buf + (i * 128 + x + offset) * 4 + 3) = 255;
			}
			p++;
		}
	}
		
	return buf;
}

Byte *DeXMShape(Byte *data, Byte *pal)
{
	Byte *buf, *mask, *ptr;
	int x, y, w, h;
	
	buf = (Byte *)malloc(128 * 128 * 4);
	memset(buf, 0, 128 * 128 * 4);
	
	x = data[0] << 8 | data[1];
	y = data[2] << 8 | data[3];
	w = data[4] << 8 | data[5];
	h = data[6] << 8 | data[7];
	
	data += 8;
	mask = data + w*h;
	ptr = buf + 512*y + x*4;
	
	do {
		int w2 = w;
		do {
			if (*mask == 0) {
				ptr[0] = pal[data[0]*3+0];
				ptr[1] = pal[data[0]*3+1];
				ptr[2] = pal[data[0]*3+2];
				ptr[3] = 255;
			}
			ptr += 4;
			data++;
			mask++;
		} while (--w2);
		ptr += 4*(128 - w);
	} while (--h);
	
	return buf;
}

void IO_ClearViewBuffer()
{
	LastTexture = 0;
	glBindTexture(GL_TEXTURE_2D, 0);
	
	/* glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); */
	glClear(GL_DEPTH_BUFFER_BIT);
	
	/* IO_AttackShape disables depth */
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
		
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glColor3ub(Pal[0x2F*3+0], Pal[0x2F*3+1], Pal[0x2F*3+2]);
	glRectf(-1, 0, 1, 1);
	
	glColor3ub(Pal[0x2A*3+0], Pal[0x2A*3+1], Pal[0x2A*3+2]);
	glRectf(1, -1, -1, 0);
		
	glColor3f(1.0, 1.0, 1.0);

	glEnable(GL_DEPTH_TEST);
	
	glPopMatrix();	
}

GLuint waltex[64];
GLuint sprtex[S_LASTONE];
GLuint weptex[NUMWEAPONS*4];

void InitRenderView()
{
	Byte *pal;
	int i;
	
	LastTexture = 0;
	glEnable(GL_TEXTURE_2D);	
		
	pal = LoadAResource(rGamePal);
	for (i = 0; i < 64; i++) {
		Byte *buf, *buf2;
		
		if (waltex[i]) {
			if (ArtData[i] == NULL) {
				glDeleteTextures(1, &waltex[i]);
				waltex[i] = 0;
			}
			continue;
		} else if (ArtData[i]) {
			glGenTextures(1, &waltex[i]);
		} else {
			continue;
		}
		
		glBindTexture(GL_TEXTURE_2D, waltex[i]);
		
		buf2 = FlipWall(ArtData[i], 128, 128);
		buf = Pal256toRGB(buf2, 128 * 128, pal);
		free(buf2);
		
		/*
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);		
		*/
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
				
		free(buf);
	}
	
	for (i = 1; i < S_LASTONE; i++) {
		Byte *buf;
				
		if (sprtex[i]) {
			if (SpriteArray[i] == NULL) {
				glDeleteTextures(1, &sprtex[i]);
				sprtex[i] = 0;
			} else {
			}
			continue;
		} else if (SpriteArray[i]) {
			glGenTextures(1, &sprtex[i]);
		} else {
			continue;
		}
		glBindTexture(GL_TEXTURE_2D, sprtex[i]);
		
		buf = DeSprite(SpriteArray[i], pal);
		/*
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);		
		*/
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
				
		free(buf);
	}
	
	if (weptex[0] == 0) {
		glGenTextures(NUMWEAPONS*4, weptex);
		for (i = 0; i < NUMWEAPONS*4; i++) {
			Byte *buf = DeXMShape(GameShapes[12+i], pal);
			glBindTexture(GL_TEXTURE_2D, weptex[i]);
			/*
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);		
			*/
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
				
			free(buf);
		}	
	}
	
	ReleaseAResource(rGamePal);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5);
	/* glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); */
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glFrustum(-0.20, 0.20, -0.288675, 0.288675, 0.2, 182.0);
/*	
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
*/
}

void IO_AttackShape(Word shape)
{
	LastTexture = weptex[shape];
	if (weptex[shape] == 0) {
		fprintf(stderr, "Weapon shape %d is zero texture!\n", shape);
	}
	
	glDisable(GL_DEPTH_TEST);
	
	glBindTexture(GL_TEXTURE_2D, weptex[shape]);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex2f( 0.2, -0.36);
	glTexCoord2f(1.0, 1.0); glVertex2f( 0.2, -1.00);
	glTexCoord2f(0.0, 1.0); glVertex2f(-0.2, -1.00);
	glTexCoord2f(0.0, 0.0); glVertex2f(-0.2, -0.36);
	glEnd();
	
	glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void StartRenderView()
{	

/*	glViewport(0, VidHeight - ViewHeight, VidWidth, VidHeight); */

	glMatrixMode(GL_MODELVIEW);
	/* glLoadIdentity(); */
	
	glRotatef(270.0-((double)gamestate.viewangle / (double)ANGLES * 360.0), 0.0, 1.0, 0.0);
	glTranslatef((double)actors[0].x / 256.0, 0, (double)actors[0].y / 256.0);
}

void DrawSprite(thing_t *t)
{
	glPushMatrix();
	
	glTranslatef(-(double)t->x / 256.0, 0, -(double)t->y / 256.0);
	glRotatef(90.0+((double)gamestate.viewangle / (double)ANGLES * 360.0), 0.0, 1.0, 0.0);
	
	if (sprtex[t->sprite] == 0) {
		fprintf(stderr, "ERROR: 0 texture in DrawSprite (%d)\n", t->sprite);
	}

	if (LastTexture != sprtex[t->sprite]) {
		LastTexture = sprtex[t->sprite];
		glBindTexture(GL_TEXTURE_2D, sprtex[t->sprite]);
	}
	
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex2f( 0.5,  1); 
	glTexCoord2f(1.0, 1.0); glVertex2f( 0.5, -1);
	glTexCoord2f(0.0, 1.0); glVertex2f(-0.5, -1);
	glTexCoord2f(0.0, 0.0); glVertex2f(-0.5,  1);
	glEnd();
	
	glPopMatrix();
}

void DrawTopSprite()
{
	GLfloat z = -128.0 / (GLfloat)topspritescale;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	if (LastTexture != sprtex[topspritenum]) {
		LastTexture = sprtex[topspritenum];
		glBindTexture(GL_TEXTURE_2D, sprtex[topspritenum]);
	}
	
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f( 0.5,  1, z);
	glTexCoord2f(1.0, 1.0); glVertex3f( 0.5, -1, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(-0.5, -1, z);
	glTexCoord2f(0.0, 0.0); glVertex3f(-0.5,  1, z);
	glEnd();

	glPopMatrix();
}

Word *src1,*src2,*dest;		/* Used by the sort */

/**********************************
	
	Merges src1/size1 and src2/size2 to dest
	Both Size1 and Size2 MUST be non-zero
	
**********************************/

void Merge(Word Size1, Word Size2)
{
	Word *XDest,*XSrc1,*XSrc2;
	
/* merge two parts of the unsorted array to the sorted array */

	XDest = dest;
	dest = &XDest[Size1+Size2];
	XSrc1 = src1;
	src1 = &XSrc1[Size1];
	XSrc2 = src2;
	src2 = &XSrc2[Size2];
	
	if (XSrc1[0] < XSrc2[0]) {		/* Which sort to use? */
mergefrom1:
		do {
			XDest[0] = XSrc1[0];	/* Copy one entry */
			++XDest;
			++XSrc1;
			if (!--Size1) {			/* Any more? */
				do {	/* Dump the rest */
					XDest[0] = XSrc2[0];	/* Copy the rest of data */
					++XDest;
					++XSrc2;
				} while (--Size2);
				return;
			}
		} while (XSrc1[0] < XSrc2[0]);
	}
	do {
		XDest[0] = XSrc2[0];
		++XDest;
		++XSrc2;
		if (!--Size2) {
			do {
				XDest[0] = XSrc1[0];
				++XDest;
				++XSrc1;
			} while (--Size1);
			return;
		}
	} while (XSrc1[0] >= XSrc2[0]);
	goto mergefrom1;
}

/**********************************
	
	Sorts the events from xevents[0] to xevent_p
	firstevent will be set to the first sorted event (either xevents[0] or sortbuffer[0])
	
**********************************/

void SortEvents(void)
{
	Word	count;	/* Number of members to sort */
	Word	size;	/* Entry size to sort with */
	Word	sort;	/* Sort count */
	Word	remaining;	/* Temp merge count */
	Word	*sorted,*unsorted,*temp;
    
	count = numvisspr;		/* How many entries are there? */
	if (count<2) {
		firstevent = xevents;	/* Just return the 0 or 1 entries */
		return;				/* Leave now */
	}
	
	size = 1;		/* source size		(<<1 / loop)*/
	sort = 1;		/* iteration number (+1 / loop)*/
	sorted = xevents;
	unsorted = sortbuffer;
	
	do {
		remaining = count>>sort;	/* How many times to try */
		
		/* pointers incremented by the merge */
		src1 = sorted;		/* Sorted array */
		src2 = &sorted[remaining<<(sort-1)];	/* Half point */
		dest = unsorted;	/* Dest array */
		
		/* merge paired blocks*/
		if (remaining) {	/* Any to sort? */
			do {
				Merge(size,size);	/* All groups equal size */
			} while (--remaining);
		}
		
		/* copy or merge the leftovers */
		remaining = count&((size<<1)-1);	/* Create mask (1 bit higher) */
		if (remaining > size) {	/* one complete block and one fragment */
			src1 = &src2[size];
			Merge(remaining-size,size);
		} else if (remaining) {	/* just a single sorted fragment */
			memcpy(dest,src2,remaining*sizeof(Word));	/* Copy it */
		}
		
		/* get ready to sort back to the other array */
		
		size <<= 1;		/* Double the entry size */
		++sort;			/* Increase the shift size */
		temp = sorted;	/* Swap the pointers */
		sorted = unsorted;
		unsorted = temp;
	} while (size<count);
	firstevent = sorted;
}

/**********************************
	
	Add a sprite entry to the render list
	
**********************************/

void AddSprite (thing_t *thing, Word actornum)
{
	fixed_t tx;		/* New X coord */
	fixed_t tz;		/* New z coord (Size) */
	Word scale;		/* Scaled size */
	fixed_t trx,try;	/* x,y from the camera */
	vissprite_t *VisPtr;	/* Local pointer to visible sprite record */
	int px;			/* Center X coord */
	unsigned short *patch;  /* Pointer to sprite data */
	int x1, x2;		/* Left,Right x */
	Word width;		/* Width of sprite */
		
/* transform the origin point */
	
	if (numvisspr>=(MAXVISSPRITES-1)) {
		return;
	}
	trx = thing->x - viewx;		/* Adjust from the camera view */
	try = viewy - thing->y;		/* Adjust from the camera view */
	tz = R_TransformZ(trx,try);	/* Get the distance */

	if (tz < MINZ) {		/* Too close? */
		return;
	}
		
	if (tz>=MAXZ) {		/* Force smallest */
		tz = MAXZ-1;
	}
	scale = scaleatzptr[tz];	/* Get the scale at the z coord */
	tx = R_TransformX(trx,try);	/* Get the screen x coord */
	px = ((tx*(long)scale)>>7) + CENTERX; /* Use 32 bit precision! */
	
/* calculate edges of the shape */

	patch = SpriteArray[thing->sprite];     /* Pointer to the sprite info */
	width =((LongWord)sMSB(patch[0])*scale)>>6; /* Get the width of the sprite */
	if (!width)
		return; 	/* too far away */
	
	x1 = px - (width>>1);   /* Get the left edge */
	if (x1 >= (int) SCREENWIDTH)
		return;         /* off the right side */
	
	x2 = x1 + width - 1;                    /* Get the right edge */
	if (x2 < 0) 
		return;		/* off the left side */	
	
	VisPtr = &vissprites[numvisspr];
	VisPtr->actornum = actornum;	/* Actor who this is (0 for static) */
	VisPtr->x1 = x1;
	VisPtr->x2 = x2;
	VisPtr->pos = thing;
	
/* pack the vissprite number into the low 6 bits of the scale for sorting */

	xevents[numvisspr] = (scale<<6) | numvisspr;		/* Pass the scale in the upper 10 bits */
	++numvisspr;		/* 1 more valid record */
}

/**********************************
	
	Draw all the character sprites
	
**********************************/

void DrawSprites(void)
{
	vissprite_t	*dseg;		/* Pointer to visible sprite record */
	Word i;					/* Index */
	static_t *stat;			/* Pointer to static sprite record */
	actor_t	*actor;			/* Pointer to active actor record */
	missile_t *MissilePtr;	/* Pointer to active missile record */
	Word *xe;				/* Pointer to sort value */

	numvisspr = 0;			/* Init the sprite count */
	
/* add all sprites in visareas*/

	if (numstatics) {		/* Any statics? */
		i = numstatics;
		stat = statics;			/* Init my pointer */
		do {
			if (areavis[stat->areanumber]) {	/* Is it in a visible area? */
				AddSprite((thing_t *) stat,0);	/* Add to my list */
			}
			++stat;		/* Next index */
		} while (--i);	/* Count down */
	}
	
	if (numactors>1) {		/* Any actors? */
		i = 1;				/* Index to the first NON-PLAYER actor */
		actor = &actors[1];	/* Init pointer */
		do {
			if (areavis[actor->areanumber]) {	/* Visible? */
				AddSprite ((thing_t *)actor, i);	/* Add it */
			}
			++actor;		/* Next actor */
		} while (++i<numactors);	/* Count up */
	}
	
	if (nummissiles) {		/* Any missiles? */
		i = nummissiles;	/* Get the missile count */
		MissilePtr = missiles;	/* Get the pointer to the first missile */
		do {
			if (areavis[MissilePtr->areanumber]) {	/* Visible? */
				AddSprite((thing_t *)MissilePtr,0);	/* Show it */
			}
			++MissilePtr;	/* Next missile */
		} while (--i);		/* Count down */
	}

	i = numvisspr;
	if (i) {			/* Any sprites? */

/* sort sprites from back to front*/

		SortEvents();

/* draw from smallest scale to largest */

		/* TODO: GL should raw back to front with depth */
		xe = &firstevent[i-1];
		do {
			dseg = &vissprites[xe[0]&(MAXVISSPRITES-1)];	/* Which one? */
			DrawSprite(dseg->pos);
			--xe;
		} while (--i);
	}
}

static int WallSeen = 0;

void WallIsSeen(saveseg_t *seg)
{
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
	Word    door;
	door_t  *door_p;
	unsigned short	span, tspan;
	unsigned short	angle1, angle2;
	
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

	if (seg->texture >= 129) {	/* segment is a door */
		door = seg->texture - 129;	/* Which door is this? */
		door_p = &doors[door];
		rw_mintex += door_p->position;
	} 
	
/* get texture*/
	
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
	GLfloat min, max, plane, pos, texslide;
	GLfloat smin, smax;
	Word door = -1;
	door_t *door_p;
	Byte *tex;
	int i, t;

	plane = -((float)seg->plane)/2.0;

	if (seg == pwallseg) {		/* Is this the active pushwall? */
		if (seg->dir&1)	{	/* east/west */
			plane += -(float)PushWallRec.pwallychange / 256.0;
		} else {		/* north/south */
			plane += -(float)PushWallRec.pwallxchange / 256.0;
		}
	}
	
	min = 0.0f;
	max = 1.0f;
	
	if (seg->texture >= 129) {
		door = seg->texture - 129;
		door_p = &doors[door];
		texslide = (float)door_p->position / 256.0f;
		tex = &textures[129 + (door_p->info>>1)][0];
	} else {
		texslide = 0.0f;
		tex = &textures[seg->texture][0];
	}
		
	for (i = seg->min; i < seg->max;) {
	
	t = tex[i >> 1];
		
	pos = ((double)i) / 2.0f;
	
	if (i == seg->min) {
		if (i & 1) {
			min = 0.5;
			i++;
		} else {
			i+=2;
			min = 0.0;
		}
		max = 1.0f;
	} /* else if (i == (seg->max-1)) {
		min = 0.0f;
		max = 0.5f;		
		i += 2;	
	} */ else {
		min = 0.0f;
		max = 1.0f;
		i += 2;
	}
		        		
	smin = -((float)pos + texslide); 
	smax = -((float)pos + (max - min));
	
	if (waltex[t] == 0)
		fprintf(stderr, "ERROR: 0 texture in P_DrawSegx!\n");	
		
	if (LastTexture != waltex[t]) {
		LastTexture = waltex[t];
		glBindTexture(GL_TEXTURE_2D, waltex[t]);
	}
	
	glBegin(GL_QUADS);
	switch(seg->dir&3) {
	case di_north:
		if (door != -1) {
			min += texslide;
			glTexCoord2f(max, 0.0); glVertex3f(plane, -1, smin); 
			glTexCoord2f(min, 0.0); glVertex3f(plane, -1, smax);
			glTexCoord2f(min, 1.0); glVertex3f(plane,  1, smax);
			glTexCoord2f(max, 1.0); glVertex3f(plane,  1, smin);
		} else {
			glTexCoord2f(min, 0.0); glVertex3f(plane, -1, smin); 
			glTexCoord2f(max, 0.0); glVertex3f(plane, -1, smax);
			glTexCoord2f(max, 1.0); glVertex3f(plane,  1, smax);
			glTexCoord2f(min, 1.0); glVertex3f(plane,  1, smin);
		}
		break;
	case di_south:
		if (texslide == 0.0 && min == 0.5)
			{ min -= 0.5; max -= 0.5; }
		else
			min += texslide;
		glTexCoord2f(max, 0.0); glVertex3f(plane, -1, smin); 
		glTexCoord2f(min, 0.0); glVertex3f(plane, -1, smax);
		glTexCoord2f(min, 1.0); glVertex3f(plane,  1, smax);
		glTexCoord2f(max, 1.0); glVertex3f(plane,  1, smin);
		break;
	case di_east:
		if (texslide == 0.0 && min == 0.5) 
			{ min -= 0.5; max -= 0.5; }
		else
			min += texslide;
		glTexCoord2f(max, 0.0); glVertex3f(smin, -1, plane);
		glTexCoord2f(min, 0.0); glVertex3f(smax, -1, plane);
	 	glTexCoord2f(min, 1.0); glVertex3f(smax,  1, plane);
		glTexCoord2f(max, 1.0); glVertex3f(smin,  1, plane);		
		break;
	case di_west:
		if (door != -1) {
			min += texslide;
			glTexCoord2f(max, 0.0); glVertex3f(smin, -1, plane); 
			glTexCoord2f(min, 0.0); glVertex3f(smax, -1, plane);
	 		glTexCoord2f(min, 1.0); glVertex3f(smax,  1, plane);
			glTexCoord2f(max, 1.0); glVertex3f(smin,  1, plane);
		} else {
			glTexCoord2f(min, 0.0); glVertex3f(smin, -1, plane);
			glTexCoord2f(max, 0.0); glVertex3f(smax, -1, plane);
		 	glTexCoord2f(max, 1.0); glVertex3f(smax,  1, plane);
			glTexCoord2f(min, 1.0); glVertex3f(smin,  1, plane);
		}
		break;
	}
	glEnd();
	}
}
