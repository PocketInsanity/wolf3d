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

void VW_UpdateScreen (void);

void VWB_DrawTile8(int x, int y, int tile);
void VWB_DrawPic(int x, int y, int chunknum);
void VWB_Bar(int x, int y, int width, int height, int color);

void VWB_DrawPropString(char *string);
void VWB_Plot(int x, int y, int color);
void VWB_Hlin(int x1, int x2, int y, int color);
void VWB_Vlin(int y1, int y2, int x, int color);

extern byte gamepal;
extern boolean screenfaded;

#define VW_Startup		VL_Startup
#define VW_Shutdown		VL_Shutdown
#define VW_Bar			VL_Bar
#define VW_Plot			VL_Plot
#define VW_Hlin(x,z,y,c)	VL_Hlin(x,y,(z)-(x)+1,c)
#define VW_Vlin(y,z,x,c)	VL_Vlin(x,y,(z)-(y)+1,c)
#define VW_WaitVBL		VL_WaitVBL
#define VW_FadeIn()		VL_FadeIn(0,255,&gamepal,30);
#define VW_FadeOut()		VL_FadeOut(0,255,0,0,0,30);
void	VW_MeasurePropString(char *string, word *width, word *height);

boolean FizzleFade(unsigned xoffset, unsigned yoffset, unsigned width,unsigned height, unsigned frames,boolean abortable);

void VL_FadeOut(int start, int end, int red, int green, int blue, int steps);
void VL_FadeIn(int start, int end, byte *palette, int steps);

void LatchDrawPic(unsigned x, unsigned y, unsigned picnum);
void LoadLatchMem(void);

#if 0 /* TODO: find some way to remove this ... */
#define LatchDrawChar(x,y,p) VL_LatchToScreen(latchpics[0]+(p)*16,2,8,x,y)
#define LatchDrawTile(x,y,p) VL_LatchToScreen(latchpics[1]+(p)*64,4,16,x,y)

#define NUMLATCHPICS	100
extern	unsigned	latchpics[NUMLATCHPICS];
extern	unsigned freelatch;
#endif

#else
#error "fix me: TODO"
#endif
