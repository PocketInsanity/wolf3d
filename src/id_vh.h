#ifndef __ID_VH_H__
#define __ID_VH_H__

#define WHITE			15
#define BLACK			0

/* ======================================================================== */

typedef struct
{
	word width, height;
} PACKED pictabletype;

typedef struct
{
	word height;
	word location[256];
	char width[256];
} PACKED fontstruct;

/* ======================================================================== */

extern pictabletype *pictable;

extern	byte	fontcolor;
extern	int	fontnumber;
extern	int	px,py;

void VW_UpdateScreen();

void VWB_DrawTile8(int x, int y, int tile);
void VWB_DrawPic(int x, int y, int chunknum);

extern boolean screenfaded;

#define VW_Startup		VL_Startup
#define VW_Shutdown		VL_Shutdown
#define VW_Hlin(x,z,y,c)	VL_Hlin(x,y,(z)-(x)+1,c)
#define VW_Vlin(y,z,x,c)	VL_Vlin(x,y,(z)-(y)+1,c)
#define VW_WaitVBL		VL_WaitVBL
#define VW_FadeIn()		VL_FadeIn(0,255,gamepal,30);
#define VW_FadeOut()		VL_FadeOut(0,255,0,0,0,30);
void	VW_MeasurePropString(char *string, word *width, word *height);

void VW_DrawPropString(char *string);

boolean FizzleFade(unsigned xoffset, unsigned yoffset, unsigned width,unsigned height, unsigned frames,boolean abortable);

void VL_FadeOut(int start, int end, int red, int green, int blue, int steps);
void VL_FadeIn(int start, int end, const byte *palette, int steps);

void LoadLatchMem();

void VL_CacheScreen(int chunk);

void VW_Bar(int x, int y, int width, int height, int color);

#endif
