/* vi_ogl.c */

#include "wl_def.h"

#include <GL/gl.h>

byte *SprToLin(byte *source)
{
	return source;
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
	
	for (i = 0; i < walcount; i++) {
		glBindTexture(GL_TEXTURE_2D, waltex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, Pal_256_RGB(PM_GetPage(i)));
	}
	
	for (i = 0; i < sprcount; i++) {
		glBindTexture(GL_TEXTURE_2D, sprtex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pal_256_RGBA(SprToLin(PM_GetSpritePage(i))));
	}
	
	printf("end init\n");
}

void SetupScaling(int maxheight)
{
}

void DrawPlayerWeapon()
{
}

void ThreeDRefresh()
{	
	DrawPlayerWeapon();
	
	frameon++;
}
