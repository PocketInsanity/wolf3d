/* vi_ogl.c */

#include "wl_def.h"

#include <GL/gl.h>

byte *SprToLin(byte *source)
{
	byte *buf = malloc(64 * 64);
	t_compshape *ptr = (t_compshape *)source;
	
	memset(buf, '\0', 64 * 64);
	{
		int srcx = 32;
		int slinex = 31;
		int stopx = ptr->leftpix;
		word *cmdptr = &ptr->dataofs[31-stopx];
		while ( --srcx >=stopx ) {
			short *linecmds = (short *)((unsigned char *)ptr + *cmdptr--);
			slinex--;
			while (linecmds[0]) {
				int y;
				int y0 = linecmds[2] / 2;
				int y1 = linecmds[0] / 2 - 1;
				unsigned char *pixels = (unsigned char *)ptr + y0 + linecmds[1];
				for (y = y0; y <= y1; y++) {
					unsigned char color = *pixels++;
					*(buf + slinex + (y*64)) = color;
				}
				linecmds += 3;
			}	
		}
		slinex = 31;
		stopx = ptr->rightpix;
		if (ptr->leftpix < 31) {
			srcx = 31;
			cmdptr = &ptr->dataofs[32 - ptr->leftpix];
		} else {
			srcx = ptr->leftpix - 1;
			cmdptr = &ptr->dataofs[0];
		}
		while (++srcx <= stopx) {
			short *linecmds = (short *)((unsigned char *)ptr + *cmdptr++);
			while (linecmds[0]) {
				int y;
				int y0 = linecmds[2] / 2;
				int y1 = linecmds[0] / 2 - 1;
				unsigned char *pixels = (unsigned char *)ptr + y0 + linecmds[1];
				for (y = y0; y <= y1; y++) {
					unsigned char color = *pixels++;
					*(buf + slinex + (y*64)) = color;
				}
				linecmds += 3;
			}
			slinex++;
		}
	}
	
	return buf;
}

byte *Pal_256_RGB(byte *source)
{
	byte *dest = (byte *)malloc(64 * 64 * 3);
	int i;
	
	for (i = 0; i < 4096; i++) {
		dest[i*3+0] = gamepal[source[i]*3+0] << 2;
		dest[i*3+1] = gamepal[source[i]*3+1] << 2;
		dest[i*3+2] = gamepal[source[i]*3+2] << 2;
	}
		
	return dest;
}

byte *Pal_256_RGBA(byte *source)
{
	byte *dest = (byte *)malloc(64 * 64 * 4);
	int i;
	
	for (i = 0; i < 4096; i++) {
		dest[i*4+0] = gamepal[source[i]*3+0] << 2;
		dest[i*4+1] = gamepal[source[i]*3+0] << 2;
		dest[i*4+2] = gamepal[source[i]*3+0] << 2;
		if (source[i] == 0) 
			dest[i*4+3] = 0;
		else
			dest[i*4+3] = 255;
	}
	
	return dest;
}
	
int *sprtex, sprcount;
int *waltex, walcount;

void Init3D()
{
	int i;
	
	printf("start init\n");
	
	walcount = PMSpriteStart;
	waltex = (int *)malloc(sizeof(int) * walcount);
	sprcount = PMSoundStart - PMSpriteStart;
	sprtex = (int *)malloc(sizeof(int) * sprcount);
	
	glGenTextures(walcount, waltex);
	glGenTextures(sprcount, sprtex);
	
	for (i = 0; i < 2 /*walcount*/; i++) {
		glBindTexture(GL_TEXTURE_2D, waltex[i]);
	        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);                        		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, Pal_256_RGB(PM_GetPage(i)));
	}
	
	for (i = 0; i < 2 /*sprcount*/; i++) {
		glBindTexture(GL_TEXTURE_2D, sprtex[i]);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);                        		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pal_256_RGBA(SprToLin(PM_GetSpritePage(i))));
	}
	
	printf("end init\n");
}

void SetupScaling(int maxheight)
{
}

int weaponscale[NUMWEAPONS] = {SPR_KNIFEREADY,SPR_PISTOLREADY,SPR_MACHINEGUNREADY,SPR_CHAINREADY};

void DrawPlayerWeapon()
{
	int shapenum;
	
	if (gamestate.weapon != -1) {
		shapenum = weaponscale[gamestate.weapon]+gamestate.weaponframe;
		
		glBindTexture(GL_TEXTURE_2D, waltex[0]);
		glBegin(GL_QUADS);
		glTexCoord2d(0.0,1.0); glVertex2d(-1.0,-1.0);
		glTexCoord2d(1.0,1.0); glVertex2d(+1.0,-1.0);
		glTexCoord2d(1.0,0.0); glVertex2d(+1.0,+1.0);
		glTexCoord2d(0.0,0.0); glVertex2d(-1.0,+1.0);
		glEnd();                                                                                                		
	}	
}

void ThreeDRefresh()
{	
	int error;
	
	glViewport(xoffset, yoffset+viewheight, viewwidth, viewheight);
	
	DrawPlayerWeapon();
	
	VW_UpdateScreen();
	
	error = glGetError();
	
	if (error != GL_NO_ERROR) {
		do {
			fprintf(stderr, "GL Error: %d\n", error);
			error = glGetError();
		} while (error != GL_NO_ERROR);
		exit(EXIT_FAILURE);
	}
	
	frameon++;
}
