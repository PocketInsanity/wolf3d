/* id_vl.c */

#include "wl_def.h"

#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/extensions/xf86dga.h>

byte *gfxbuf = NULL;
byte *disbuf = NULL;

Display *dpy;
int screen;
Window root, win;
XVisualInfo *vi;
GC gc;
XImage *img;
Colormap cmap;
Atom wmDeleteWindow;

XShmSegmentInfo shminfo;
XF86VidModeModeInfo vidmode;
XF86VidModeModeInfo **vmmi;
XColor clr[256];

int indexmode;
int shmmode;
int fullscreen;
int dga;
byte *dgabuf;
int dgawidth, dgabank, dgamem, vwidth, vheight;
unsigned char mypal[768];

int main(int argc, char *argv[])
{
	return WolfMain(argc, argv);
}

/*
==========================
=
= Quit
=
==========================
*/

void Quit(char *error)
{
	/* TODO: blah blah blah */
	memptr screen = NULL;

	if (!error || !*error) {
		CA_CacheGrChunk(ORDERSCREEN);
		screen = grsegs[ORDERSCREEN];
		WriteConfig();
	} else if (error) {
		CA_CacheGrChunk(ERRORSCREEN);
		screen = grsegs[ERRORSCREEN];
	}
	
	ShutdownId();
	
	if (screen) {
		printf("TODO: spiffy ansi screen goes here..\n");
	}
	
	if (error && *error) {
		fprintf(stderr, "Quit: %s", error);
		exit(EXIT_FAILURE);
 	}
	exit(EXIT_SUCCESS);
}

/*
=======================
=
= VL_Startup
=
=======================
*/

void GetVisual()
{
	XVisualInfo vitemp;
		
	int i, numVisuals;
		
	vitemp.screen = screen;
	vitemp.depth = 8;
	vitemp.class = PseudoColor;
	
	vi = XGetVisualInfo(dpy, VisualScreenMask | VisualDepthMask |
			    VisualClassMask, &vitemp, &numVisuals);
	
	if (vi && (numVisuals > 0)) {
		indexmode = 1;
	
		cmap = XCreateColormap(dpy, root, vi->visual, AllocAll);
		for (i = 0; i < 256; i++) {
			clr[i].pixel = i;
			clr[i].flags = DoRed|DoGreen|DoBlue;
		}
		
		return;	
	}
	
	vitemp.depth = 15;
	vitemp.class = TrueColor;
	vitemp.red_mask = 0x7C00;
	vitemp.green_mask = 0x03E0;
	vitemp.blue_mask = 0x001F;
	vi = XGetVisualInfo(dpy, VisualScreenMask | VisualDepthMask |
			    VisualClassMask, &vitemp, &numVisuals);
	
	if (vi && (numVisuals > 0)) {
		indexmode = 0;
		
		printf("15: rm:%04lX gm:%04lX bm:%04lX cs:%04X bpr:%04X\n", vi->red_mask,
			vi->green_mask, vi->blue_mask, vi->colormap_size,
			vi->bits_per_rgb);
			
		cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		
		return;
	} 
	
	vitemp.depth = 16;
	vitemp.red_mask = 0xF800;
	vitemp.green_mask = 0x07E0;
	vitemp.blue_mask = 0x001F;
	vi = XGetVisualInfo(dpy, VisualScreenMask | VisualDepthMask |
			    VisualClassMask, &vitemp, &numVisuals);
	
	if (vi && (numVisuals > 0)) {
		indexmode = 0;

		printf("16: rm:%04lX gm:%04lX bm:%04lX cs:%04X bpr:%04X\n", vi->red_mask,
			vi->green_mask, vi->blue_mask, vi->colormap_size,
			vi->bits_per_rgb);
		
		cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		
		return;
	}
	
	vitemp.depth = 24;
	vitemp.red_mask = 0xFF0000;
	vitemp.green_mask = 0x00FF00;
	vitemp.blue_mask = 0x0000FF;
	vi = XGetVisualInfo(dpy, VisualScreenMask | VisualDepthMask |
			    VisualClassMask, &vitemp, &numVisuals);
	
	if (vi && (numVisuals > 0)) {
		indexmode = 0;

		printf("24: rm:%04lX gm:%04lX bm:%04lX cs:%04X bpr:%04X\n", vi->red_mask,
			vi->green_mask, vi->blue_mask, vi->colormap_size,
			vi->bits_per_rgb);
		
		cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		
		return;
	}

#if 0	
	vitemp.depth = 32;
	vitemp.class = TrueColor;
	
	vi = XGetVisualInfo(dpy, VisualScreenMask | VisualDepthMask |
			    VisualClassMask, &vitemp, &numVisuals);
	
	if (vi && (numVisuals > 0)) {
		indexmode = 0;

		printf("32: rm:%04lX gm:%04lX bm:%04lX cs:%04X bpr:%04X\n", vi->red_mask,
			vi->green_mask, vi->blue_mask, vi->colormap_size,
			vi->bits_per_rgb);
		
		cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		
		return;
	}
#endif	

	Quit("No usable visual found!");		
}

int BPP(int d)
{
	switch(d) {
		case 8:
			return 1;
		case 15:
		case 16:
			return 2;
		case 24: /* TODO: ??? the nvidia xserver really gave me AGBR? */
			/* need to check what the image says */
			return 4;
		case 32:
			return 4;
		default:
			Quit("Sorry, BPP doesn't like that...");
			return 0; /* heh */
	}
}
	
void VL_Startup()
{
	XSetWindowAttributes attr;
	XSizeHints sizehints;	
	XGCValues gcvalues;
	Pixmap bitmap;
	Cursor cursor;
	XColor bg = { 0 };
	XColor fg = { 0 };
	char data[8] = { 0x01 };
	
	char *disp;
	int attrmask, eventn, errorn, i, vmc;
	
	disp = getenv("DISPLAY");
	dpy = XOpenDisplay(disp);
	if (dpy == NULL) {
		/* TODO: quit function with vsnprintf */
		fprintf(stderr, "Unable to open display %s!\n", XDisplayName(disp));
		exit(EXIT_FAILURE);
	}
	
	screen = DefaultScreen(dpy);
	
	root = RootWindow(dpy, screen);
	
	GetVisual(); /* GetVisual will quit for us if no visual.. */                      	
	
	fullscreen = 0;
	if (MS_CheckParm("fullscreen") && XF86VidModeQueryExtension(dpy, &eventn, &errorn)) {
				
		XF86VidModeGetAllModeLines(dpy, screen, &vmc, (XF86VidModeModeInfo ***)&vmmi);
		
		printf("VidMode: eventn = %d, error = %d, vmc = %d\n", eventn, errorn, vmc);
		
		for (i = 0; i < vmc; i++) {
			if ( (vmmi[i]->hdisplay == 320) && (vmmi[i]->vdisplay == 200) ) {
				//XF86VidModeGetModeLine(dpy, screen, &errorn, (XF86VidModeModeLine *)&vidmode); /* TODO: 3rd parm? */
				//memcpy(&vidmode, vmmi[0], sizeof(XF86VidModeModeInfo)); /* TODO: bah, why doesn't above work? */
				//printf("%d, %d, %d\n", vidmode.hdisplay, vidmode.vdisplay, errorn);
				if (XF86VidModeSwitchToMode(dpy, screen, vmmi[i]) == True)  {
					XF86VidModeLockModeSwitch(dpy, screen, True);
					printf("Using VidMode!\n");
					fullscreen = 1;
					break;
				}
			}
		}
		
		dga = 0;
		if (fullscreen && MS_CheckParm("dga") && XF86DGAQueryExtension(dpy, &eventn, &errorn)) {
			if (geteuid()) {
				fprintf(stderr, "must be root to use dga\n");
			} else {
				printf("DGA %d %d\n", eventn, errorn);
				XF86DGAQueryVersion(dpy, &eventn, &errorn);
				printf("DGA Version %d.%d\n", eventn, errorn);
				
				XF86DGAQueryDirectVideo(dpy, screen, &i);
			
				if (i & XF86DGADirectPresent) {
					XF86DGAGetVideo(dpy, screen, (char **)&dgabuf, &dgawidth, &dgabank, &dgamem);
					printf("addr = %p, width = %d, bank = %d, mem = %d\n", dgabuf, dgawidth, dgabank, dgamem);
					gfxbuf = disbuf = dgabuf;
					XF86DGAGetViewPortSize(dpy, screen, &vwidth, &vheight);
					printf("width = %d, height = %d\n", vwidth, vheight);

					gfxbuf = (byte *)malloc(320 * 200);
					if (!indexmode)
						disbuf = (byte *)malloc(320 * 4);
					dga = 1;
				}
			}		
		}		
	}
	
	//attr.colormap = cmap;		   
	attr.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask;
	attrmask = /*CWColormap |*/ CWEventMask;
	
	if (dga) {
		attrmask |= CWOverrideRedirect;
		attr.override_redirect = True;	
	}
	
	win = XCreateWindow(dpy, root, 0, 0, 320, 200, 0, CopyFromParent, 
			    InputOutput, vi->visual, attrmask, &attr);
	
	if (win == None) {
		Quit("Unable to create window!");
	}
	
	if (dga) {
		XMapWindow(dpy, win);
		XRaiseWindow(dpy, win);
		XGrabKeyboard(dpy, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
	}
	
	gcvalues.foreground = BlackPixel(dpy, screen);
	gcvalues.background = WhitePixel(dpy, screen);
	gc = XCreateGC(dpy, win, GCForeground | GCBackground, &gcvalues);
	
	sizehints.min_width = 320;
	sizehints.min_height = 200;
	sizehints.max_width = 320;
	sizehints.max_height = 200;
	sizehints.base_width = 320;
	sizehints.base_height = 200;
	sizehints.flags = PMinSize | PMaxSize | PBaseSize;
	
	XSetWMProperties(dpy, win, NULL, NULL, _argv, _argc, &sizehints, None, None); 
	
	XStoreName(dpy, win, GAMENAME);
	XSetIconName(dpy, win, GAMENAME);
	
	wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &wmDeleteWindow, 1);

	bitmap = XCreateBitmapFromData(dpy, win, data, 8, 8);
	cursor = XCreatePixmapCursor(dpy, bitmap, bitmap, &fg, &bg, 0, 0);
	XDefineCursor(dpy, win, cursor);
		
	shmmode = 0;
	
	if ( !dga && (XShmQueryExtension(dpy) == True) ) {
		img = XShmCreateImage(dpy, vi->visual, vi->depth, ZPixmap, 
				      NULL, &shminfo, 320, 200);
		printf("Shm: bpl = %d, h = %d, bp = %d\n", img->bytes_per_line, img->height, img->bitmap_pad);
		if ( img->bytes_per_line != (320 * BPP(vi->depth)) ) {
			printf("Currently cannot handle irregular shm sizes...\n");
		} else {
			shminfo.shmid = shmget(IPC_PRIVATE, img->bytes_per_line * img->height, IPC_CREAT | 0777);
			shminfo.shmaddr = img->data = shmat(shminfo.shmid, 0, 0);	
			shminfo.readOnly = False;
			disbuf = (byte *)img->data;
			
			if (indexmode)
				gfxbuf = disbuf;
			else
				gfxbuf = malloc(320 * 200 * 1);
				
			if (XShmAttach(dpy, &shminfo) == True) {
				printf("Using XShm Extension...\n");
				shmmode = 1;
			} else {
				printf("Error with XShm...\n");
			}
		}
	}
				
	if ( !dga && (img == NULL) ) {
		printf("Falling back on XImage...\n");
		
		if (gfxbuf == NULL) 
			gfxbuf = malloc(320 * 200 * 1);
		if (indexmode) 
			disbuf = gfxbuf;
		else 
			disbuf = malloc(320 * 200 * BPP(vi->depth));
		
		img = XCreateImage(dpy, vi->visual, vi->depth, ZPixmap, 0, (char *)disbuf, 320, 200,
			   8, 320 * BPP(vi->depth));
	
		if (img == NULL) {
			Quit("XCreateImage returned NULL");
		}
	}				   
	XMapWindow(dpy, win);

	if (fullscreen) {
		XMoveWindow(dpy, win, 0, 0);
		XRaiseWindow(dpy, win);
		XWarpPointer(dpy, None, win, 0, 0, 0, 0, 0, 0);
		XF86VidModeSetViewPort(dpy, screen, 0, 0);
	}

	if (dga) {
		XF86DGADirectVideo(dpy, screen, XF86DGADirectGraphics | XF86DGADirectKeyb);
		XF86DGASetViewPort(dpy, screen, 0, 0);
	}
	
	XSetWindowColormap(dpy, win, cmap);
	
	XFlush(dpy);
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
	if (dga) {
		XF86DGADirectVideo(dpy, screen, 0);
		XUngrabKeyboard(dpy, CurrentTime);
		free(gfxbuf);
		free(disbuf);
		gfxbuf = disbuf = NULL;
	}
	
	if (fullscreen) {
		XF86VidModeLockModeSwitch(dpy, screen, False);
		//printf("%d, %d\n", vidmode.hdisplay, vidmode.vdisplay);
		//XF86VidModeSwitchToMode(dpy, screen, &vidmode);
		XF86VidModeSwitchToMode(dpy, screen, vmmi[0]);
	}
	
	if ( !shmmode && (gfxbuf != NULL) ) {
		free(gfxbuf);
		gfxbuf = NULL;
	}
	
	if ( shmmode && !indexmode && (gfxbuf != NULL) ) {
		free(gfxbuf);
		gfxbuf = NULL;
	}
	
	if (shmmode) {
		XShmDetach(dpy, &shminfo);
		XDestroyImage(img);
		shmdt(shminfo.shmaddr);
		shmctl(shminfo.shmid, IPC_RMID, 0);
	} else if ( (indexmode == 0) && (disbuf != NULL) ) {
		free(disbuf);
		disbuf = NULL;
	}
}

void VL_WaitVBL(int vbls)
{
	/* hack - but it works for me */
	long last = get_TimeCount() + 1;
	while (last > get_TimeCount()) ;
}

void VW_UpdateScreen()
{
	word *ptrs;
	byte *ptrb, *ptrbd;
	
	int i, j;

	if (dga) {
		switch(vi->depth) {
			case 8:
				ptrb = dgabuf;
				ptrbd = gfxbuf;
				for(i = 0; i < 200; i++) {
					memcpy(ptrb, ptrbd, 320);
					ptrb += dgawidth;
					ptrbd += 320;
				}
				return;
			#if 0
			case 15:
				ptrs = (word *)disbuf;
				for (i = 0; i < 64000; i++) {
					*ptrs = (mypal[gfxbuf[i]*3+0] >> 1) << 10 |
						(mypal[gfxbuf[i]*3+1] >> 1) << 5  |
						(mypal[gfxbuf[i]*3+2] >> 1);
					ptrs++;
				}
				break;
			case 16:
				ptrs = (word *)disbuf;
				for (i = 0; i < 64000; i++) {
					*ptrs = (mypal[gfxbuf[i]*3+0] >> 1) << 11 |
						(mypal[gfxbuf[i]*3+1] >> 0) << 5  |
						(mypal[gfxbuf[i]*3+2] >> 1);
					ptrs++;
				}
				break;
			case 24:
				ptrb = disbuf;
				for (i = 0; i < 64000; i++) {
					*ptrb = mypal[gfxbuf[i]*3+2] << 2; ptrb++;
					*ptrb = mypal[gfxbuf[i]*3+1] << 2; ptrb++;
					*ptrb = mypal[gfxbuf[i]*3+0] << 2; ptrb++;
					ptrb++;
				}
				break;
			#endif
		} 	
	}
	if (indexmode == 0) {
		switch(vi->depth) {
		case 15:
			ptrs = (word *)disbuf;
			for (i = 0; i < 64000; i++) {
				*ptrs = (mypal[gfxbuf[i]*3+0] >> 1) << 10 |
					(mypal[gfxbuf[i]*3+1] >> 1) << 5  |
					(mypal[gfxbuf[i]*3+2] >> 1);
				ptrs++;
			}
			break;
		case 16:
			ptrs = (word *)disbuf;
			for (i = 0; i < 64000; i++) {
				*ptrs = (mypal[gfxbuf[i]*3+0] >> 1) << 11 |
					(mypal[gfxbuf[i]*3+1] >> 0) << 5  |
					(mypal[gfxbuf[i]*3+2] >> 1);
				ptrs++;
			}
			break;
		case 24:
			ptrb = disbuf;
			for (i = 0; i < 64000; i++) {
				*ptrb = mypal[gfxbuf[i]*3+2] << 2; ptrb++;
				*ptrb = mypal[gfxbuf[i]*3+1] << 2; ptrb++;
				*ptrb = mypal[gfxbuf[i]*3+0] << 2; ptrb++;
				ptrb++;
			}
			break;
		}
	}

	if (shmmode)
		XShmPutImage(dpy, win, gc, img, 0, 0, 0, 0, 320, 200, False);
	else
		XPutImage(dpy, win, gc, img, 0, 0, 0, 0, 320, 200);
}

/*
=============================================================================

						PALETTE OPS

		To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/


/*
=================
=
= VL_FillPalette
=
=================
*/

void VL_FillPalette(int red, int green, int blue)
{
	int i;
	
	if (indexmode) {
		for (i = 0; i < 256; i++) {
			clr[i].red = red << 10;
			clr[i].green = green << 10;
			clr[i].blue = blue << 10;
		}
	
		XStoreColors(dpy, cmap, clr, 256);	
		if (dga) XF86DGAInstallColormap(dpy, screen, cmap); 
	} else {
		for (i = 0; i < 256; i++) {
			mypal[i*3+0] = red;
			mypal[i*3+1] = green;
			mypal[i*3+2] = blue;
		}
	}
}	

//===========================================================================

/*
=================
=
= VL_SetPalette
=
=================
*/

void VL_SetPalette(const byte *palette)
{
	int i;
	
	if (indexmode) {
		for (i = 0; i < 256; i++) {
			clr[i].red = palette[i*3+0] << 10;
			clr[i].green = palette[i*3+1] << 10;
			clr[i].blue = palette[i*3+2] << 10;
		}
		XStoreColors(dpy, cmap, clr, 256);
		if (dga) XF86DGAInstallColormap(dpy, screen, cmap);
	} else {
		memcpy(mypal, palette, 768);
		VW_UpdateScreen();
	}		
}


//===========================================================================

/*
=================
=
= VL_GetPalette
=
=================
*/

void VL_GetPalette(byte *palette)
{	
	int i;
	
	if (indexmode) {
		for (i = 0; i < 256; i++) {
			palette[i*3+0] = clr[i].red >> 10;
			palette[i*3+1] = clr[i].green >> 10;
			palette[i*3+2] = clr[i].blue >> 10;
		}
	} else {
		memcpy(palette, mypal, 768);
	}
}

/*
=============================================================================

							PIXEL OPS

=============================================================================
*/

/*
=================
=
= VL_ClearVideo
=
= Fill the entire video buffer with a given color
=
=================
*/

void VL_ClearVideo(byte color)
{
	memset(gfxbuf, color, 64000);
}

/*
=================
=
= VL_Plot
=
=================
*/

void VL_Plot(int x, int y, int color)
{
	*(gfxbuf + 320 * y + x) = color;
}

/*
=================
=
= VL_Hlin
=
=================
*/

void VL_Hlin(unsigned x, unsigned y, unsigned width, unsigned color)
{
	memset(gfxbuf + 320 * y + x, color, width);
}

/*
=================
=
= VL_Vlin
=
=================
*/

void VL_Vlin (int x, int y, int height, int color)
{
	byte *ptr = gfxbuf + 320 * y + x;
	while (height--) {
		*ptr = color;
		ptr += 320;
	}
}

/*
=================
=
= VL_Bar
=
=================
*/

void VL_Bar(int x, int y, int width, int height, int color)
{
	byte *ptr = gfxbuf + 320 * y + x;
	while (height--) {
		memset(ptr, color, width);
		ptr += 320;
	}
}

/*
============================================================================

							MEMORY OPS

============================================================================
*/

/*
=================
=
= VL_MemToScreen
=
= Draws a block of data to the screen.
=
=================
*/

void VL_MemToScreen(const byte *source, int width, int height, int x, int y)
{
	byte *ptr = gfxbuf + 320 * y + x;
	while(height--) {
		memcpy(ptr, source, width);
		source += width;
		ptr += 320;
	}
}

void VL_DeModeXize(byte *buf, int width, int height)
{
	byte *mem, *ptr, *destline;
	int plane, x, y;
	
	if (width & 3) {
		printf("Not divisible by 4?\n");
		return;
	}
	
	/* TODO: can this malloc be removed, and have this func swap each pixel? */
	mem = malloc(width * height);
	ptr = buf;
	for (plane = 0; plane < 4; plane++) {
		destline = mem;
		for (y = 0; y < height; y++) {
			for (x = 0; x < width / 4; x++)
				*(destline + x*4 + plane) = *ptr++;
			destline += width;
		}
	}
	memcpy(buf, mem, width * height);
	free(mem);
}

void VL_DirectPlot(int x1, int y1, int x2, int y2)
{
	if (dga) {
		switch(vi->depth) {
			case 8:
				*(dgabuf + x2 + y2 * dgawidth) = *(gfxbuf + x1 + y1 * 320);
				return;
		}
		return;
	}
	if (indexmode) {
		XSetForeground(dpy, gc, *(gfxbuf + x1 + y1 * 320));
		XDrawPoint(dpy, win, gc, x2, y2);
	} else {
		#if 0
		unsigned char pix = *(gfxbuf + x1 + y1 * 320);
		XColor c;
		c.pixel = 0;
		c.flags = DoRed|DoGreen|DoBlue;
		c.red = mypal[pix*3+0] << 10;
		c.green = mypal[pix*3+1] << 10;
		c.blue = mypal[pix*3+2] << 10;
		XStoreColor(dpy, cmap, &c);
		XSetForeground(dpy, gc, 0);
		XDrawPoint(dpy, win, gc, x2, y2);
		#endif
	#if 0
		unsigned char pix = *(gfxbuf + x1 + y1 * 320);
		XColor c;
		c.pixel = 0;
		c.flags = DoRed|DoGreen|DoBlue;
		c.red = mypal[pix*3+0] << 10;
		c.green = mypal[pix*3+1] << 10;
		c.blue = mypal[pix*3+2] << 10;
		XAllocColor(dpy, cmap, &c);
		//XStoreColor(dpy, cmap, &c);
		XSetForeground(dpy, gc, c.pixel);
		XDrawPoint(dpy, win, gc, x2, y2);
	#endif
	}
}

/*
=============================================================================

					GLOBAL VARIABLES

=============================================================================
*/

//
// configuration variables
//
boolean			MousePresent;
boolean			JoysPresent[MaxJoys];

// 	Global variables
		boolean		Keyboard[NumCodes];
		boolean		Paused;
		char		LastASCII;
		ScanCode	LastScan;

KeyboardDef KbdDefs = {sc_Control, sc_Alt, sc_Home, sc_UpArrow, sc_PgUp, sc_LeftArrow, sc_RightArrow, sc_End, sc_DownArrow, sc_PgDn};

		ControlType	Controls[MaxPlayers];

/*
=============================================================================

					LOCAL VARIABLES

=============================================================================
*/
static byte ASCIINames[] =		// Unshifted ASCII for scan codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	 0  ,27 ,'1','2','3','4','5','6','7','8','9','0','-','=',8  ,9  ,	// 0
	'q','w','e','r','t','y','u','i','o','p','[',']',13 ,0  ,'a','s',	// 1
	'd','f','g','h','j','k','l',';',39 ,'`',0  ,92 ,'z','x','c','v',	// 2
	'b','n','m',',','.','/',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0		// 7
					},
					 ShiftNames[] =		// Shifted ASCII for scan codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'!','@','#','$','%','^','&','*','(',')','_','+',8  ,9  ,	// 0
	'Q','W','E','R','T','Y','U','I','O','P','{','}',13 ,0  ,'A','S',	// 1
	'D','F','G','H','J','K','L',':',34 ,'~',0  ,'|','Z','X','C','V',	// 2
	'B','N','M','<','>','?',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
					},
					 SpecialNames[] =	// ASCII for 0xe0 prefixed codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 0
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,13 ,0  ,0  ,0  ,	// 1
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 2
	0  ,0  ,0  ,0  ,0  ,'/',0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 4
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
					};


static	boolean		IN_Started;
static	boolean		CapsLock;
static	ScanCode	CurCode,LastCode;

static	Direction	DirTable[] =		// Quick lookup for total direction
					{
						dir_NorthWest,	dir_North,	dir_NorthEast,
						dir_West,		dir_None,	dir_East,
						dir_SouthWest,	dir_South,	dir_SouthEast
					};

int XKeysymToScancode(unsigned int keysym)
{
	switch (keysym) {
		case XK_Left:
		case XK_KP_Left:
			return sc_LeftArrow;
		case XK_Right:
		case XK_KP_Right:
			return sc_RightArrow;
		case XK_Up:
		case XK_KP_Up:
			return sc_UpArrow;
		case XK_Down:
		case XK_KP_Down:
			return sc_DownArrow;
		case XK_Control_L:
			return sc_Control;
		case XK_Alt_L:
			return sc_Alt;
		case XK_Shift_L:
			return sc_LShift;
		case XK_Shift_R:
			return sc_RShift;
		case XK_Escape:
			return sc_Escape;
		case XK_space:
		case XK_KP_Space:
			return sc_Space;
		case XK_KP_Enter:
		case XK_Return:
			return sc_Enter;
		case XK_y:
			return sc_Y;
		case XK_n:
			return sc_N;
		case XK_Pause:
			return 0xE1;
		default:
			printf("unknown: %s\n", XKeysymToString(keysym));
			return sc_None;
	}
}
			
void keyboard_handler(int code, int press)
{
	static boolean special;
	byte k, c;

	k = code;

	if (k == 0xe0)		// Special key prefix
		special = true;
	else if (k == 0xe1)	// Handle Pause key
		Paused = true;
	else
	{
		if (press == 0)	
		{

// DEBUG - handle special keys: ctl-alt-delete, print scrn

			Keyboard[k] = false;
		}
		else			// Make code
		{
			LastCode = CurCode;
			CurCode = LastScan = k;
			Keyboard[k] = true;

			if (special)
				c = SpecialNames[k];
			else
			{
				if (k == sc_CapsLock)
				{
					CapsLock ^= true;
				}

				if (Keyboard[sc_LShift] || Keyboard[sc_RShift])	// If shifted
				{
					c = ShiftNames[k];
					if ((c >= 'A') && (c <= 'Z') && CapsLock)
						c += 'a' - 'A';
				}
				else
				{
					c = ASCIINames[k];
					if ((c >= 'a') && (c <= 'z') && CapsLock)
						c -= 'a' - 'A';
				}
			}
			if (c)
				LastASCII = c;
		}

		special = false;
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseDelta() - Gets the amount that the mouse has moved from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static void INL_GetMouseDelta(int *x,int *y)
{
	*x = 0;
	*y = 0;
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseButtons() - Gets the status of the mouse buttons from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static word INL_GetMouseButtons(void)
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
static word INL_GetJoyButtons(word joy)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartKbd() - Sets up my keyboard stuff for use
//
///////////////////////////////////////////////////////////////////////////
static void INL_StartKbd(void)
{
	IN_ClearKeysDown();
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutKbd() - Restores keyboard control to the BIOS
//
///////////////////////////////////////////////////////////////////////////
static void INL_ShutKbd(void)
{
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartMouse() - Detects and sets up the mouse
//
///////////////////////////////////////////////////////////////////////////
static boolean INL_StartMouse(void)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutMouse() - Cleans up after the mouse
//
///////////////////////////////////////////////////////////////////////////
static void INL_ShutMouse(void)
{
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetupJoy() - Sets up thresholding values and calls INL_SetJoyScale()
//		to set up scaling values
//
///////////////////////////////////////////////////////////////////////////
void IN_SetupJoy(word joy,word minx,word maxx,word miny,word maxy)
{
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartJoy() - Detects & auto-configures the specified joystick
//					The auto-config assumes the joystick is centered
//
///////////////////////////////////////////////////////////////////////////
static boolean INL_StartJoy(word joy)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutJoy() - Cleans up the joystick stuff
//
///////////////////////////////////////////////////////////////////////////
static void INL_ShutJoy(word joy)
{
	JoysPresent[joy] = false;
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_Startup() - Starts up the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void IN_Startup(void)
{
	boolean	checkjoys,checkmouse;
	word	i;

	if (IN_Started)
		return;

	checkjoys = true;
	checkmouse = true;
	
	if (MS_CheckParm("nojoy"))
		checkjoys = false;
	if (MS_CheckParm("nomouse"))
		checkmouse = false;
		
	INL_StartKbd();
	MousePresent = checkmouse ? INL_StartMouse() : false;

	for (i = 0;i < MaxJoys;i++)
		JoysPresent[i] = checkjoys ? INL_StartJoy(i) : false;

	IN_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Shutdown() - Shuts down the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void IN_Shutdown(void)
{
	word i;

	if (!IN_Started)
		return;

	INL_ShutMouse();
	for (i = 0;i < MaxJoys;i++)
		INL_ShutJoy(i);
	INL_ShutKbd();

	IN_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_ClearKeysDown() - Clears the keyboard array
//
///////////////////////////////////////////////////////////////////////////
void IN_ClearKeysDown(void)
{
	LastScan = sc_None;
	LastASCII = key_None;
	memset(Keyboard, 0, sizeof(Keyboard));
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_ReadControl() - Reads the device associated with the specified
//		player and fills in the control info struct
//
///////////////////////////////////////////////////////////////////////////
void IN_ReadControl(int player,ControlInfo *info)
{
			boolean		realdelta = false;
			word		buttons;
			int			dx,dy;
			Motion		mx,my;
			ControlType	type;
			KeyboardDef	*def;

	dx = dy = 0;
	mx = my = motion_None;
	buttons = 0;

IN_CheckAck();

		switch (type = Controls[player])
		{
		case ctrl_Keyboard:
			def = &KbdDefs;

			if (Keyboard[def->upleft])
				mx = motion_Left,my = motion_Up;
			else if (Keyboard[def->upright])
				mx = motion_Right,my = motion_Up;
			else if (Keyboard[def->downleft])
				mx = motion_Left,my = motion_Down;
			else if (Keyboard[def->downright])
				mx = motion_Right,my = motion_Down;

			if (Keyboard[def->up])
				my = motion_Up;
			else if (Keyboard[def->down])
				my = motion_Down;

			if (Keyboard[def->left])
				mx = motion_Left;
			else if (Keyboard[def->right])
				mx = motion_Right;

			if (Keyboard[def->button0])
				buttons += 1 << 0;
			if (Keyboard[def->button1])
				buttons += 1 << 1;
			realdelta = false;
			break;
		case ctrl_Joystick1:
		case ctrl_Joystick2:
			INL_GetJoyDelta(type - ctrl_Joystick,&dx,&dy);
			buttons = INL_GetJoyButtons(type - ctrl_Joystick);
			realdelta = true;
			break;
		case ctrl_Mouse:
			INL_GetMouseDelta(&dx,&dy);
			buttons = INL_GetMouseButtons();
			realdelta = true;
			break;
		}

	if (realdelta)
	{
		mx = (dx < 0)? motion_Left : ((dx > 0)? motion_Right : motion_None);
		my = (dy < 0)? motion_Up : ((dy > 0)? motion_Down : motion_None);
	}
	else
	{
		dx = mx * 127;
		dy = my * 127;
	}

	info->x = dx;
	info->xaxis = mx;
	info->y = dy;
	info->yaxis = my;
	info->button0 = buttons & (1 << 0);
	info->button1 = buttons & (1 << 1);
	info->button2 = buttons & (1 << 2);
	info->button3 = buttons & (1 << 3);
	info->dir = DirTable[((my + 1) * 3) + (mx + 1)];
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Ack() - waits for a button or key press.  If a button is down, upon
// calling, it must be released for it to be recognized
//
///////////////////////////////////////////////////////////////////////////

boolean	btnstate[8];

void IN_StartAck(void)
{
	unsigned	i,buttons;

//
// get initial state of everything
//
	IN_ClearKeysDown();
	memset (btnstate,0,sizeof(btnstate));

	buttons = IN_JoyButtons () << 4;
	if (MousePresent)
		buttons |= IN_MouseButtons ();

	for (i=0;i<8;i++,buttons>>=1)
		if (buttons&1)
			btnstate[i] = true;
}

boolean IN_CheckAck()
{
	XEvent event;
	
	unsigned i, buttons;
	
	if (XPending(dpy)) {
		do {
			XNextEvent(dpy, &event);
			switch(event.type) {
				case KeyPress:
					keyboard_handler(XKeysymToScancode(XKeycodeToKeysym(dpy, event.xkey.keycode, 0)), 1);
					break;
				case KeyRelease:
					keyboard_handler(XKeysymToScancode(XKeycodeToKeysym(dpy, event.xkey.keycode, 0)), 0);
					break;
				case Expose:
					VW_UpdateScreen();
					break;
				case ClientMessage:
					if (event.xclient.data.l[0] == wmDeleteWindow)
						Quit(NULL);
					break;
				default:
					break;
			}
		} while (XPending(dpy));
	}
	
	if (LastScan)
		return true;

	buttons = IN_JoyButtons () << 4;
	if (MousePresent)
		buttons |= IN_MouseButtons ();

	for (i=0;i<8;i++,buttons>>=1)
		if ( buttons&1 )
		{
			if (!btnstate[i])
				return true;
		}
		else
			btnstate[i]=false;

	return false;
}

void IN_Ack()
{
	IN_StartAck();

	while(!IN_CheckAck()) ;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_UserInput() - Waits for the specified delay time (in ticks) or the
//		user pressing a key or a mouse button. If the clear flag is set, it
//		then either clears the key or waits for the user to let the mouse
//		button up.
//
///////////////////////////////////////////////////////////////////////////
boolean IN_UserInput(longword delay)
{
	longword	lasttime;

	lasttime = get_TimeCount();
	
	IN_StartAck();
	do {
		if (IN_CheckAck())
			return true;
	} while ( (get_TimeCount() - lasttime) < delay );
	
	return false;
}

//===========================================================================

/*
===================
=
= IN_MouseButtons
=
===================
*/

byte IN_MouseButtons (void)
{
	return 0;
}

/*
===================
=
= IN_JoyButtons
=
===================
*/

byte IN_JoyButtons (void)
{
	return 0;
}
