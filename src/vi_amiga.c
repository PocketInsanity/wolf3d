#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/layers.h>
#include <libraries/Picasso96.h>
#include <dos/dos.h>
#include <exec/exec.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/Picasso96API.h>

const __attribute__((used)) 
	UBYTE *VERSTAG = "\0$VER: amigawolf3d 0.3 (01.01.2004)";

#include "wl_def.h"

byte *gfxbuf = NULL;

struct Library *P96Base;
struct Library *IntuitionBase;
struct Library *GfxBase;
struct Library *LayersBase;

struct P96IFace *IP96;
struct IntuitionIFace *IIntuition;
struct GraphicsIFace *IGraphics;
struct LayersIFace *ILayers;

static struct Window *window;
static struct Screen *screen;

static struct ScreenBuffer * screenbuf[2];
static int current_screenbuf;

static struct RastPort p96_rastport;
static struct BitMap *p96_bitmap;

static UWORD * pointer_mem;

static unsigned char curpal[768];

static unsigned short int pal16[256];
static unsigned long pal32[256];

static byte mouse_button_status;
static int mousex, mousey;

static int active_window;

static int fullscreen;
static int doublebuf;
static int screen_offset_x;
static int screen_offset_y;

static const unsigned char keytab[128] = {
/* 00 */ sc_None,
/* 01 */ sc_1,
/* 02 */ sc_2,
/* 03 */ sc_3,
/* 04 */ sc_4,
/* 05 */ sc_5,
/* 06 */ sc_6,
/* 07 */ sc_7,
/* 08 */ sc_8,
/* 09 */ sc_9,
/* 0a */ sc_0,
/* 0b */ sc_None,
/* 0c */ sc_None,
/* 0d */ sc_None,
/* 0e */ sc_None,
/* 0f */ sc_None,
/* 10 */ sc_Q,
/* 11 */ sc_W,
/* 12 */ sc_E,
/* 13 */ sc_R,
/* 14 */ sc_T,
/* 15 */ sc_Y,
/* 16 */ sc_U,
/* 17 */ sc_I,
/* 18 */ sc_O,
/* 19 */ sc_P,
/* 1a */ sc_None,
/* 1b */ sc_None,
/* 1c */ sc_None,
/* 1d */ sc_None,
/* 1e */ sc_None,
/* 1f */ sc_None,
/* 20 */ sc_A,
/* 21 */ sc_S,
/* 22 */ sc_D,
/* 23 */ sc_F,
/* 24 */ sc_G,
/* 25 */ sc_H,
/* 26 */ sc_J,
/* 27 */ sc_K,
/* 28 */ sc_L,
/* 29 */ sc_None,
/* 2a */ sc_None,
/* 2b */ sc_None,
/* 2c */ sc_None,
/* 2d */ sc_None,
/* 2e */ sc_None,
/* 2f */ sc_None,
/* 30 */ sc_None,
/* 31 */ sc_Z,
/* 32 */ sc_X,
/* 33 */ sc_C,
/* 34 */ sc_V,
/* 35 */ sc_B,
/* 36 */ sc_N,
/* 37 */ sc_M,
/* 38 */ sc_None,
/* 39 */ sc_None,
/* 3a */ sc_None,
/* 3b */ sc_None,
/* 3c */ sc_None,
/* 3d */ sc_None,
/* 3e */ sc_None,
/* 3f */ sc_None,
/* 40 */ sc_Space,
/* 41 */ sc_BackSpace,
/* 42 */ sc_None,
/* 43 */ sc_None,
/* 44 */ sc_Return,
/* 45 */ sc_Escape,
/* 46 */ sc_None,
/* 47 */ sc_None,
/* 48 */ sc_None,
/* 49 */ sc_None,
/* 4a */ sc_None,
/* 4b */ sc_None,
/* 4c */ sc_UpArrow,
/* 4d */ sc_DownArrow,
/* 4e */ sc_RightArrow,
/* 4f */ sc_LeftArrow,
/* 50 */ sc_None,
/* 51 */ sc_None,
/* 52 */ sc_None,
/* 53 */ sc_None,
/* 54 */ sc_None,
/* 55 */ sc_None,
/* 56 */ sc_None,
/* 57 */ sc_None,
/* 58 */ sc_None,
/* 59 */ sc_None,
/* 5a */ sc_None,
/* 5b */ sc_None,
/* 5c */ sc_None,
/* 5d */ sc_None,
/* 5e */ sc_None,
/* 5f */ sc_None,
/* 60 */ sc_LShift,
/* 61 */ sc_RShift,
/* 62 */ sc_None,
/* 63 */ sc_Control,
/* 64 */ sc_Alt,
/* 65 */ sc_Alt,
/* 66 */ sc_None,
/* 67 */ sc_None,
/* 68 */ sc_None,
/* 69 */ sc_None,
/* 6a */ sc_None,
/* 6b */ sc_None,
/* 6c */ sc_None,
/* 6d */ sc_None,
/* 6e */ sc_None,
/* 6f */ sc_None,
/* 70 */ sc_None,
/* 71 */ sc_None,
/* 72 */ sc_None,
/* 73 */ sc_None,
/* 74 */ sc_None,
/* 75 */ sc_None,
/* 76 */ sc_None,
/* 77 */ sc_None,
/* 78 */ sc_None,
/* 79 */ sc_None,
/* 7a */ sc_None,
/* 7b */ sc_None,
/* 7c */ sc_None,
/* 7d */ sc_None,
/* 7e */ sc_None,
/* 7f */ sc_None
};

#ifdef __amigaos4__
static void swab(const void *from, void *to, ssize_t n)
{
	const unsigned short * src = (const unsigned short *) from;
	unsigned short * dst = (unsigned short *) to;
	
	n /= 2;
	
	while (n--) {
		*dst = (*src >> 8) | (*src << 8);
		
		src++;
		dst++;
	}
}
#endif

/*
==========================
=
= Quit
=
==========================
*/

void Quit(const char *error)
{
	if (!error || !*error) {
		WriteConfig();
	}

	ShutdownId();
		
	if (error && *error) {
		fprintf(stderr, "Quit: %s\n", error);
		exit(EXIT_FAILURE);
 	}
	
	exit(EXIT_SUCCESS);
}

void VL_WaitVBL(int vbls)
{
}

static void Copy8(unsigned char *base, int stride)
{
	int h = 200;
	
	const unsigned char * sptr = gfxbuf;
	
	while (h--) {
		unsigned short * dptr = (unsigned short *) base;
		
		memcpy(dptr, sptr, 320);
		
		sptr += 320;
		base += stride;
		
	}
}

static void XPand8to16(unsigned char *base, int stride)
{
	int w;
	int h = 200;
	
	const unsigned char * sptr = gfxbuf;
	
	while (h--) {
		unsigned short * dptr = (unsigned short *) base;
		
		w = 320;
		
		while (w--) {
			*dptr = pal16[*sptr];
			
			sptr++;
			dptr++;
		}
		
		base += stride;
	}
}

static void XPand8to16Rect(unsigned char *base, int stride, int x, int y, int wc, int hc)
{
	int w;
	
	const unsigned char * sbase = gfxbuf + y * 320 + x;
	
	while (hc--) {
		const unsigned char * sptr = (const unsigned char *) sbase;
		unsigned short * dptr = (unsigned short *) base;

		w = wc;
		
		while (w--) {
			*dptr = pal16[*sptr];
			
			sptr++;
			dptr++;
		}
		
		base += stride;
		sbase += 320;
	}
}

static void UpdateScreenP96(struct RenderInfo *ri, int bpp)
{
	unsigned char *base = (unsigned char *) ri->Memory;
	int stride = ri->BytesPerRow;

	if (base) {
		base = base +
			stride * (window->TopEdge + window->BorderTop + screen_offset_y) +
			bpp * (window->LeftEdge + window->BorderLeft + screen_offset_x);
				
		switch (ri->RGBFormat) {
			case RGBFB_CLUT:
				Copy8(base, stride);
				break;
				
			case RGBFB_R5G5B5:
			case RGBFB_R5G5B5PC:
			case RGBFB_R5G6B5:
			case RGBFB_R5G6B5PC:
				XPand8to16(base, stride);
				break;
				
			default:
				break;
		}
	}
}


static void UpdateScreenRectP96(struct RenderInfo *ri, int bpp, int ox, int oy, struct Rectangle *draw_rect)
{
	unsigned char *base = (unsigned char *) ri->Memory;
	int stride = ri->BytesPerRow;
	int w;
	int h;
	
	if (base) {
		base = base +
			stride * draw_rect->MinY +
			bpp * draw_rect->MinX;

		w = (draw_rect->MaxX - draw_rect->MinX) + 1;
		h = (draw_rect->MaxY - draw_rect->MinY) + 1;
		
		switch (ri->RGBFormat) {
			case RGBFB_R5G5B5:
			case RGBFB_R5G5B5PC:
			case RGBFB_R5G6B5:
			case RGBFB_R5G6B5PC:
				XPand8to16Rect(base, stride, ox, oy, w, h);
				break;
				
			default:
				break;
		}
	}
}

static ULONG ASM SAVEDS DoHookClipRectsFunc(struct Hook *hook, APTR object, APTR message)
{
	struct RastPort * rastport = (struct RastPort *) object;
	struct BackFillMessage * bfm = (struct BackFillMessage *) message;

	struct BitMap * bitmap = rastport->BitMap;
	struct RenderInfo ri;
	ULONG lock;
	int bpp;

 	bpp = IP96->p96GetBitMapAttr(bitmap, P96BMA_BYTESPERPIXEL);

	lock = IP96->p96LockBitMap(bitmap, (UBYTE *) &ri, sizeof(struct RenderInfo));
 	 	
 	UpdateScreenRectP96(&ri, bpp, 
 		bfm->OffsetX-window->BorderLeft, bfm->OffsetY-window->BorderTop, &bfm->Bounds);
 	
 	IP96->p96UnlockBitMap(bitmap, lock);
 	
	return 0;
}

void VW_UpdateScreen()
{
	struct Hook ClipRectsHook;
	
	struct RastPort * rastport = window->RPort;
	struct BitMap * bitmap = rastport->BitMap;

	struct Rectangle draw_bounds;
	
	if (!IP96->p96GetBitMapAttr(bitmap, P96BMA_ISP96)) {
		return;
	}
	
	if (fullscreen) {
		struct RenderInfo ri;
		int bpp;
		ULONG lock;
		
		if (doublebuf) {
			bitmap = screenbuf[current_screenbuf]->sb_BitMap;
		}
		
		bpp = IP96->p96GetBitMapAttr(bitmap, P96BMA_BYTESPERPIXEL);
		
		lock = IP96->p96LockBitMap(bitmap, (UBYTE *) &ri, sizeof(struct RenderInfo));
		UpdateScreenP96(&ri, bpp);
		IP96->p96UnlockBitMap(bitmap, lock);
		
		if (doublebuf) {
			while (IIntuition->ChangeScreenBuffer(
				screen, screenbuf[current_screenbuf]) == 0) { 
				
				/* busy wait... */
			}
			
			current_screenbuf ^= 1;
		}
		
		return;
	}
	
	memset(&ClipRectsHook, 0, sizeof(ClipRectsHook));
	
	ClipRectsHook.h_Entry = (HOOKFUNC) DoHookClipRectsFunc;
	
	draw_bounds.MinX = window->BorderLeft;
	draw_bounds.MaxX = draw_bounds.MinX + 320 - 1;
	draw_bounds.MinY = window->BorderTop;
	draw_bounds.MaxY = draw_bounds.MinY + 200 - 1;
	
	ILayers->DoHookClipRects(&ClipRectsHook, window->RPort, &draw_bounds);
}

static void InitializeLibraries()
{
	IntuitionBase = (struct Library *)
		IExec->OpenLibrary("intuition.library", 0L);
		
	if (IntuitionBase == NULL) {
		Quit("OpenLibrary failed to open intuition.library");
	}

	IIntuition = (struct IntuitionIFace *) 
		IExec->GetInterface((struct Library *) IntuitionBase, "main", 1, 0);

	if (IIntuition == NULL) {
		Quit("GetInterface failed to get intuition.library interface");
	}

	GfxBase = (struct Library *) IExec->OpenLibrary("graphics.library", 0L);
	
	if (GfxBase == NULL) {
		Quit("OpenLibrary failed to open graphics.library");
	}

	IGraphics = (struct GraphicsIFace *) 
		IExec->GetInterface((struct Library *) GfxBase, "main", 1, 0);

	if (IGraphics == NULL) {
		Quit("GetInterface failed to get graphics.library interface");
	}

	LayersBase = (struct LayersBase *) IExec->OpenLibrary("layers.library", 0L);
	
	if (LayersBase == NULL) {
		Quit("OpenLibrary failed to open layers.library");
	}
	
	ILayers = (struct LayersIFace *)
		IExec->GetInterface((struct Library *) LayersBase, "main", 1, 0);
	
	if (ILayers == NULL) {
		Quit("GetInterfacey failed to get layers.library interface");
	}
	
	P96Base = (struct Library *) IExec->OpenLibrary("Picasso96API.library", 0L);
	
	if (P96Base == NULL) {
		Quit("OpenLibrary failed to open Picasso96API.library");
	}
	
	IP96 = (struct P96IFace *)
		IExec->GetInterface((struct Library *) P96Base, "main", 1, 0);
	
	if (IP96 == NULL) {
		Quit("GetInterface failed to get Picasso96API.library interface");
	}	
}

static void CloseLibraries()
{	
	if (IntuitionBase) {
		if (IIntuition) {
			IExec->DropInterface((struct Interface *) IIntuition);
			
			IIntuition = NULL;
		}

		IExec->CloseLibrary((struct Library *) IntuitionBase);
		
		IntuitionBase = NULL;
	}	
	
        if (GfxBase) {
		if (IGraphics) {
			IExec->DropInterface((struct Interface *) IGraphics);
			
			IGraphics = NULL;
		}

        	IExec->CloseLibrary((struct Library *) GfxBase);
        	
        	GfxBase = NULL;
	}

	if (LayersBase) {
		if (ILayers) {
			IExec->DropInterface((struct Interface *) ILayers);
			
			ILayers = NULL;
		}
		
		IExec->CloseLibrary((struct Library *) LayersBase);
		
		LayersBase = NULL;
	}
	
	if (P96Base) {
		if (IP96) {
			IExec->DropInterface((struct Interface *) IP96);
			
			IP96 = NULL;
		}
		
		IExec->CloseLibrary((struct Library *) P96Base);
		
		P96Base = NULL;
	}
}

ULONG FindMode()
{
	struct DisplayInfo buf;
	ULONG modeID;
	ULONG bestID;
	int width, height;
	RGBFTYPE RGBFormat;
	
	bestID = modeID = INVALID_ID;
	
	IGraphics->GetDisplayInfoData(NULL, (APTR) &buf, sizeof(struct DisplayInfo),
		DTAG_DISP, LORES_KEY);

	while ((modeID = IGraphics->NextDisplayInfo(modeID)) != INVALID_ID) {
		if (!IP96->p96GetModeIDAttr(modeID, P96IDA_ISP96)) {
			continue;
		}
		
		if (IGraphics->ModeNotAvailable(modeID)) {
			continue;
		}
		
		width = IP96->p96GetModeIDAttr(modeID, P96IDA_WIDTH);
		height = IP96->p96GetModeIDAttr(modeID, P96IDA_HEIGHT);
		RGBFormat = IP96->p96GetModeIDAttr(modeID, P96IDA_RGBFORMAT);
		
		if (width == 320 && height == 200 && RGBFormat == RGBFB_CLUT) {
			return modeID;
		}
		
		if (width == 320 && height == 240 && RGBFormat == RGBFB_CLUT) {
			bestID = modeID;
		}
	}	

	return bestID;
}

/*
=======================
=
= VL_Startup
=
=======================
*/

void VL_Startup()
{
	ULONG flags;
	ULONG idcmp;
	
	InitializeLibraries();
	
	p96_bitmap = IP96->p96AllocBitMap(320, 200, 8, 
		BMF_USERPRIVATE, NULL, RGBFB_CLUT);
	
	if (p96_bitmap == NULL) {
		Quit("Unable to create Picasso96 Bitmap");
	}
	
	IGraphics->InitRastPort(&p96_rastport);
	
	p96_rastport.BitMap = p96_bitmap;
		
	gfxbuf = (byte *) IP96->p96GetBitMapAttr(p96_bitmap, P96BMA_MEMORY);
	
	if (fullscreen) {
		ULONG ErrorCode;
		ULONG modeID;
		
		modeID = FindMode();
		
		if (modeID == INVALID_ID) {
			Quit("Unable to find a valid mode...\n");
		}
		
		screen = IIntuition->OpenScreenTags(NULL,
			SA_DisplayID,   modeID,
			SA_Depth,	8,
			SA_ShowTitle,	FALSE,
			SA_Quiet,	TRUE,
			SA_FullPalette,	TRUE,
			SA_ErrorCode,	(ULONG) &ErrorCode,
			TAG_DONE);
		
		if (screen == NULL) {
//			fprintf(stderr, "ErrorCode: %d (%08x)\n", 
//				ErrorCode, ErrorCode);
				
			Quit("OpenScreenTags failed...\n");
		}
		
		if (doublebuf) {
			screenbuf[0] = IIntuition->AllocScreenBuffer(
				screen, NULL, SB_SCREEN_BITMAP);
			screenbuf[1] = IIntuition->AllocScreenBuffer(
				screen, NULL, 0);
			
			current_screenbuf = 1;
			
			if (screenbuf[0] == NULL ||
				screenbuf[1] == NULL) {
				
				Quit("Unable to allocate screenbuffers...\n");
			}
		}
	} else {
		screen = IIntuition->LockPubScreen(NULL);
	
		if (screen == NULL) {
			Quit("LockPubScreen failed...\n");
		}
	}
	
	flags = WFLG_REPORTMOUSE | 
		WFLG_NOCAREREFRESH |
		WFLG_ACTIVATE |
		WFLG_RMBTRAP;
		
	if (fullscreen) {
		flags |= (WFLG_BORDERLESS |
			WFLG_BACKDROP |
			WFLG_SIMPLE_REFRESH);
	} else {
		flags |= (WFLG_DEPTHGADGET |
			WFLG_CLOSEGADGET |
			WFLG_DRAGBAR);
	}
	
	idcmp = IDCMP_RAWKEY |
		IDCMP_MOUSEBUTTONS |
		IDCMP_MOUSEMOVE |
		IDCMP_DELTAMOVE |
		IDCMP_ACTIVEWINDOW |
		IDCMP_INACTIVEWINDOW |
		IDCMP_CLOSEWINDOW;
	
	if (fullscreen) {
		window = IIntuition->OpenWindowTags(NULL,
			WA_Flags,		flags,
			WA_Left,		0,
			WA_Top,			0,
			WA_InnerWidth,		screen->Width,
			WA_InnerHeight,		screen->Height,
			WA_CustomScreen,	(ULONG) screen,
			WA_IDCMP, 		idcmp,
			TAG_DONE);
		
		screen_offset_x = (screen->Width - 320) / 2;
		screen_offset_y = (screen->Height - 200) / 2;
	} else {
		window = IIntuition->OpenWindowTags(NULL,
			WA_Flags,	flags,
			WA_InnerWidth,	320,
			WA_InnerHeight,	200,
			WA_PubScreen,	(ULONG) screen,
			WA_Title,	(ULONG) GAMENAME,
			WA_IDCMP, 	idcmp,
			TAG_DONE);
		
		screen_offset_x = 0;
		screen_offset_y = 0;
	}
	
	if (window == NULL) {
		Quit("OpenWindowTags failed...\n");
	}

	if (fullscreen) {
		pointer_mem = IExec->AllocMem(4, MEMF_CHIP|MEMF_CLEAR);
	
		if (pointer_mem) {
			IIntuition->SetPointer(window, pointer_mem, 0, 0, 0, 0);
		}
	}
	
	mouse_button_status = 0;
	mousex = 0;
	mousey = 0;
	
	active_window = 0;
}

/*
=======================
=
= VL_Shutdown
=
=======================
*/

void VL_Shutdown()
{
	if (screenbuf[0]) {
		IIntuition->FreeScreenBuffer(screen, screenbuf[0]);
		
		screenbuf[0] = NULL;
	}
	
	if (screenbuf[1]) {
		IIntuition->FreeScreenBuffer(screen, screenbuf[1]);
		
		screenbuf[1] = NULL;
	}
	
	if (p96_bitmap) {
		IP96->p96FreeBitMap(p96_bitmap);
		
		p96_bitmap = NULL;
	}

	if (pointer_mem) {
		IIntuition->ClearPointer(window);
		
		IExec->FreeMem(pointer_mem, 4);
		
		pointer_mem = NULL;
	}
	
	if (window) {
		IIntuition->CloseWindow(window);
		
		window = NULL;
	}
	
	if (screen) {
		if (fullscreen) {
			IIntuition->CloseScreen(screen);
		} else {
			IIntuition->UnlockPubScreen(NULL, screen);
		}
		
		screen = NULL;
	}

	CloseLibraries();
}

//===========================================================================

static void SetPalette_RGB15(const byte *palette, boolean pc)
{
	int i;
	
	for (i = 0; i < 256; i++) {
		int r = palette[i*3+0] >> 1;
		int g = palette[i*3+1] >> 1;
		int b = palette[i*3+2] >> 1;
		
		pal16[i] = (r << 10) | (g << 5) | (b << 0);
	}
	
	if (pc) {
		swab(pal16, pal16, 512);
	}
}

static void SetPalette_RGB16(const byte *palette, boolean pc)
{
	int i;
	
	for (i = 0; i < 256; i++) {
		int r = palette[i*3+0] >> 1;
		int g = palette[i*3+1] >> 0;
		int b = palette[i*3+2] >> 1;
		
		pal16[i] = (r << 11) | (g << 5) | (b << 0);
	}
	
	if (pc) {
		swab(pal16, pal16, 512);
	}
}

/*
=================
=
= VL_SetPalette
=
=================
*/

void VL_SetPalette(const byte *palette)
{
	struct BitMap *bitmap;
	RGBFTYPE RGBFormat;
	
	memcpy(curpal, palette, 768);
	
	bitmap = window->RPort->BitMap;
	
	if (fullscreen) {
		int i;
		
		for (i = 0; i < 256; i++) {
			IGraphics->SetRGB32(&screen->ViewPort,
				i,
				palette[i*3+0] << 26,
				palette[i*3+1] << 26,
				palette[i*3+2] << 26);
		}
	}
	
	if (IP96->p96GetBitMapAttr(bitmap, P96BMA_ISP96)) {
		RGBFormat = IP96->p96GetBitMapAttr(bitmap, P96BMA_RGBFORMAT);
		
		switch (RGBFormat) {
			case RGBFB_R5G5B5:
				SetPalette_RGB15(palette, false);
				break;
			
			case RGBFB_R5G5B5PC:
				SetPalette_RGB15(palette, true);
				break;
				
			case RGBFB_R5G6B5:
				SetPalette_RGB16(palette, false);
				break;
				
			case RGBFB_R5G6B5PC:
				SetPalette_RGB16(palette, true);
				break;
				
			default:
				break;
		}
	}
}

/*
=================
=
= VL_GetPalette
=
=================
*/

void VL_GetPalette(byte *palette)
{
	memcpy(palette, curpal, 768);
}

static void mouse_handler(UWORD Code)
{
	switch (Code) {
		case SELECTDOWN:
			mouse_button_status |= 1;
			break;
			
		case SELECTUP:
			mouse_button_status &= ~1;
			break;
			
		case MENUDOWN:
			mouse_button_status |= 2;
			break;
			
		case MENUUP:
			mouse_button_status &= ~2;
			break;
			
		case MIDDLEDOWN:
			mouse_button_status |= 4;
			break;
			
		case MIDDLEUP:
			mouse_button_status &= ~4;
			break;
			
		default:
			printf("MOUSEBUTTONS: Code: %04x\n", Code);
			break;
	}
}

void INL_Update()
{
	struct IntuiMessage *imsg;
	int end;
	
	if (window == NULL) {
		printf("DEBUG: window is null\n");
		return;
	}
	
	if (window->UserPort == NULL) {
		printf("DEBUG: userport is null\n");
		return;
	}
	
	end = 0;
	
	if (IExec->SetSignal(0L, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) {
		end = 1;
	}
	
	while ((imsg = (struct IntuiMessage *) IExec->GetMsg(window->UserPort)) != NULL) {
		switch (imsg->Class) {
			case IDCMP_RAWKEY:
				{
					extern void keyboard_handler(int code, int press);
					
					int press = (imsg->Code & 0x80) ? 0 : 1;
					int key = keytab[imsg->Code & 0x7f];
					
					if (key == sc_None) {
						printf("RAWKEY: Code: %04x Qual: %04x\n", 
							imsg->Code, imsg->Qualifier);
					}
					
					keyboard_handler(key, press);
				}
				break;
			
			case IDCMP_MOUSEBUTTONS:
				mouse_handler(imsg->Code);				
				break;
			
			case IDCMP_MOUSEMOVE:
				mousex += imsg->MouseX;
				mousey += imsg->MouseY;
				break;
				
			case IDCMP_ACTIVEWINDOW:
				active_window = 1;
				
				mousex = 0;
				mousey = 0;
				break;
				
			case IDCMP_INACTIVEWINDOW:
				active_window = 0;
				
				mousex = 0;
				mousey = 0;
				break;
				
			case IDCMP_CLOSEWINDOW:
				end = 1;
				break;
				
			default:
				printf("got event: %08lx\n", imsg->Class);
				break;
		}
		
		IExec->ReplyMsg((struct Message *) imsg);
	}
	
	if (end) {
		Quit(NULL);
	}
}

void IN_GetMouseDelta(int *dx, int *dy)
{
	if (!active_window) {
		if (*dx) *dx = 0;
		if (*dy) *dy = 0;
		
		return;
	}
		
	if (dx) {
		*dx = mousex << 4;
		
		if (*dx > 32767) {
			*dx = 32767;
		}
		
		if (*dx < -32768) {
			*dx = -32768;
		}
	}
	
	if (dy) {
		*dy = mousey << 4;
		
		if (*dy > 32767) {
			*dy = 32767;
		}
		
		if (*dy < -32768) {
			*dy = -32768;
		}
	}
	
	mousex = 0;
	mousey = 0;
}

byte IN_MouseButtons()
{
	return mouse_button_status;
}

/*
===================
=
= IN_JoyButtons
=
===================
*/

byte IN_JoyButtons()
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyAbs() - Reads the absolute position of the specified joystick
//
///////////////////////////////////////////////////////////////////////////
void IN_GetJoyAbs(word joy,word *xp,word *yp)
{
	*xp = 0;
	*yp = 0;
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetJoyDelta() - Returns the relative movement of the specified
//		joystick (from +/-127)
//
///////////////////////////////////////////////////////////////////////////
void INL_GetJoyDelta(word joy,int *dx,int *dy)
{
	*dx = 0;
	*dy = 0;
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetJoyButtons() - Returns the button status of the specified
//		joystick
//
///////////////////////////////////////////////////////////////////////////
word INL_GetJoyButtons(word joy)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////
//
//      IN_SetupJoy() - Sets up thresholding values and calls INL_SetJoyScale()
//              to set up scaling values
//
///////////////////////////////////////////////////////////////////////////
void IN_SetupJoy(word joy,word minx,word maxx,word miny,word maxy)
{
}

static void Init()
{
	struct {
		LONG FullScreen;
		LONG DoubleBuf;
	} args;
	
	struct RDArgs * rda;
	
	char * template = "FULLSCREEN/S,DOUBLEBUF/S";
	
	vwidth = 320;
	vheight = 200;
	
	args.FullScreen = fullscreen;
	args.DoubleBuf = doublebuf;
	
	if ((rda = IDOS->ReadArgs(template, (LONG *) &args, NULL)) != NULL) {
		
		fullscreen = args.FullScreen;
		doublebuf = args.DoubleBuf;
		
		IDOS->FreeArgs(rda);
	}
}

int main(int argc, char *argv[])
{
	Init();
		
	return WolfMain(argc, argv);
}
