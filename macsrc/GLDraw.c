/*
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

void IO_ScaleMaskedColumn(Word x,Word scale, unsigned short *sprite,Word column)
{
/* TODO: remove stuff from sprites */
}

void StartRenderView()
{
}

void P_DrawSeg(saveseg_t *seg)
{
}
