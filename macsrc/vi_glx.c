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

/*
This is NOT the OpenGL version!
This is a COPY of vi_xlib.c as a placeholder!
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "wolfdef.h"

Display *dpy;
int screen;
Window win, root;
XVisualInfo *vi;
GC gc;
XImage *img;
Colormap cmap;
Atom wmDeleteWindow;

XColor clr[256];

Byte *gfxbuf;

int main(int argc, char *argv[])
{
	XSetWindowAttributes attr;
	XVisualInfo vitemp;
	XGCValues gcvalues;
	Pixmap bitmap;
	Cursor cursor;
	XColor bg = { 0 };
	XColor fg = { 0 };
	char data[8] = { 0x01 };
	char *display;
	int mask, i;
	
	if (argc != 2) {
		fprintf(stderr, "usage: %s <mac wolf3d resource fork>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	if (InitResources(argv[1])) {
		fprintf(stderr, "could not load %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	
	display = getenv("DISPLAY");
	dpy = XOpenDisplay(getenv(display));
	if (dpy == NULL) {
		fprintf(stderr, "Unable to open display %s\n", XDisplayName(display));
		exit(EXIT_FAILURE);
	}
	
	screen = DefaultScreen(dpy);
	
	root = RootWindow(dpy, screen);
	
	vitemp.screen = screen;
	vitemp.depth = 8;
	vitemp.class = PseudoColor;
	mask = VisualScreenMask | VisualDepthMask | VisualClassMask;
	
	vi = XGetVisualInfo(dpy, mask, &vitemp, &i);
	
	if ( !(vi && i) ) {
		fprintf(stderr, "Unable to get a depth 8 PseudoColor visual on screen %d\n", screen);
		exit(EXIT_FAILURE);
	}
	
	cmap = XCreateColormap(dpy, root, vi->visual, AllocAll);
	for (i = 0; i < 256; i++) {
		clr[i].pixel = i;
		clr[i].flags = DoRed | DoGreen | DoBlue;
	}
	
	attr.colormap = cmap;
	attr.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask;
	mask = CWColormap | CWEventMask;
	
	win = XCreateWindow(dpy, root, 0, 0, 320, 200, 0, CopyFromParent,
			    InputOutput, vi->visual, mask, &attr);
	
	if (win == None) {
		fprintf(stderr, "Unable to create window\n");
		exit(EXIT_FAILURE);
	}
	
	gcvalues.foreground = BlackPixel(dpy, screen);
	gcvalues.background = WhitePixel(dpy, screen);
	mask = GCForeground | GCBackground;
	
	gc = XCreateGC(dpy, win, mask, &gcvalues);
	
	XSetWMProperties(dpy, win, NULL, NULL, argv, argc, None, None, None);
	
	XStoreName(dpy, win, "Wolfenstein 3D");
	XSetIconName(dpy, win, "Wolfenstein 3D");
	
	wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &wmDeleteWindow, 1);
	
	bitmap = XCreateBitmapFromData(dpy, win, data, 8, 8);
	cursor = XCreatePixmapCursor(dpy, bitmap, bitmap, &fg, &bg, 0, 0);
	XDefineCursor(dpy, win, cursor);
	
	XMapWindow(dpy, win);
	XFlush(dpy);
	
	InitData();
	
	GameViewSize = 2;	
	NewGameWindow(GameViewSize); 

	ClearTheScreen(BLACK);
	BlastScreen();
	
	return WolfMain(argc, argv);
}

void Quit(char *str)
{	
	FreeResources();
	
	if (img)
		XDestroyImage(img);
	
	if (str && *str) {
		fprintf(stderr, "%s\n", str);
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}

void SetPalette(Byte *pal)
{
	int i;
	
	for (i = 0; i < 256; i++) {
		clr[i].red = pal[i*3+0] << 8;
		clr[i].green = pal[i*3+1] << 8;
		clr[i].blue = pal[i*3+2] << 8;
	}
	XStoreColors(dpy, cmap, clr, 256);
}
	
void BlastScreen2(Rect *BlastRect)
{
	BlastScreen();
}

int VidWidth, VidHeight, ViewHeight;
#define w VidWidth
#define h VidHeight
#define v ViewHeight

void BlastScreen()
{
	XPutImage(dpy, win, gc, img, 0, 0, 0, 0, w, h);
}

Word VidXs[] = {320,512,640,640};       /* Screen sizes to play with */
Word VidYs[] = {200,384,400,480};
Word VidVs[] = {160,320,320,400};
Word VidPics[] = {rFaceShapes,rFace512,rFace640,rFace640};
Word VidSize = -1;

Word NewGameWindow(Word NewVidSize)
{
	XSizeHints sizehints;
	LongWord *LongPtr;
	Byte *DestPtr;
	int i;
	
	printf("Called: %d\n", NewVidSize);
	
	if (NewVidSize == VidSize)
		return VidSize;
	
	printf("Setting Size: %d (from %d)\n", NewVidSize, VidSize);
		
	if (NewVidSize < 4) {
		w = VidXs[NewVidSize];
		h = VidYs[NewVidSize];
		v = VidVs[NewVidSize];
	} else {
		fprintf(stderr, "Invalid Vid size: %d\n", NewVidSize);
		exit(EXIT_FAILURE);
	}
	
	if (img) {
		XDestroyImage(img);
		/* free(gfxbuf); */
	}
	
	sizehints.min_width = sizehints.max_width = sizehints.base_width = w;
	sizehints.min_height = sizehints.max_height = sizehints.base_height = h;
	sizehints.flags = PMinSize | PMaxSize | PBaseSize;
	XSetWMNormalHints(dpy, win, &sizehints);
	XResizeWindow(dpy, win, w, h);
	
	gfxbuf = (Byte *)malloc(w * h);
	
	img = XCreateImage(dpy, vi->visual, vi->depth, ZPixmap, 0, 
			   (char *)gfxbuf, w, h, 8, w);
	
	if (img == NULL) {
		fprintf(stderr, "XCreateImage returned NULL, Unable to create an XImage\n");
		exit(EXIT_FAILURE);
	}
	
	VideoPointer = gfxbuf;
	VideoWidth = w;
	InitYTable();
	SetAPalette(rBlackPal);
	ClearTheScreen(BLACK);
	BlastScreen();
	
	LongPtr = (LongWord *) LoadAResource(VidPics[NewVidSize]);

	if (GameShapes)
		FreeSomeMem(GameShapes);
		
	GameShapes = (Byte **)AllocSomeMem(lMSB(LongPtr[0]));
	DLZSS((Byte *)GameShapes, (Byte *)&LongPtr[1], lMSB(LongPtr[0]));
	ReleaseAResource(VidPics[NewVidSize]);
	
	LongPtr = (LongWord *)GameShapes;
	DestPtr = (Byte *)GameShapes;
	for (i = 0; i < ((NewVidSize == 1) ? 57 : 47); i++) 
		GameShapes[i] = DestPtr + lMSB(LongPtr[i]);
	
	VidSize = NewVidSize;
	
	return VidSize;
}

/* Keyboard Hack */
static int RSJ;

static int keys[128];

void FlushKeys()
{
	joystick1 = 0;
	memset(keys, 0, sizeof(keys));
}

struct {
	char code[13];
} CheatCodes[] = {
{ "XUSCNIELPPA" }, /* "XUSCNIELPPA" */
{ "IDDQD" }, /* "IDDQD" */
{ "BURGER" }, /* "BURGER" */
{ "WOWZERS" }, /* "WOWZERS" */
{ "LEDOUX" }, /* "LEDOUX" */
{ "SEGER" }, /* "SEGER" */
{ "MCCALL" }, /* "MCCALL" */
{ "APPLEIIGS" } /* "APPLEIIGS" */
};
const int CheatCount = sizeof(CheatCodes) / sizeof(CheatCodes[0]);
int CheatIndex;

#define SC_CURSORUPLEFT 	1
#define SC_CURSORUP	  	2
#define SC_CURSORUPRIGHT	3
#define SC_CURSORRIGHT		4
#define SC_CURSORDOWNRIGHT	5
#define SC_CURSORDOWN		6
#define SC_CURSORDOWNLEFT	7
#define SC_CURSORLEFT		8
#define SC_CURSORBLOCKLEFT	9
#define SC_CURSORBLOCKRIGHT	10
#define SC_CURSORBLOCKUP	11
#define SC_CURSORBLOCKDOWN	12
#define SC_KEYPADENTER		13
#define SC_ENTER		14
#define SC_SPACE		15
#define SC_LEFTALT		16
#define SC_RIGHTALT		17
#define SC_LEFTCONTROL		18
#define SC_RIGHTCONTROL		19
#define SC_LEFTSHIFT		20
#define SC_RIGHTSHIFT		21
#define SC_B			22

void UpdateKeys(KeySym key, int press)
{
	switch(key) {
		case XK_KP_Home:
			keys[SC_CURSORUPLEFT] = press;
			break;
		case XK_KP_Up:
			keys[SC_CURSORUP] = press;
			break;
		case XK_KP_Page_Up:
			keys[SC_CURSORUPRIGHT] = press;
			break;
		case XK_KP_Right:
			keys[SC_CURSORRIGHT] = press;
			break;
		case XK_KP_Page_Down:
			keys[SC_CURSORDOWNRIGHT] = press;
			break;
		case XK_KP_Down:
			keys[SC_CURSORDOWN] = press;
			break;
		case XK_KP_End:
			keys[SC_CURSORDOWNLEFT] = press;
			break;
		case XK_KP_Left:
			keys[SC_CURSORLEFT] = press;
			break;
			
		case XK_Up:
			keys[SC_CURSORBLOCKUP] = press;
			break;
		case XK_Down:
			keys[SC_CURSORBLOCKDOWN] = press;
			break;
		case XK_Left:
			keys[SC_CURSORBLOCKLEFT] = press;
			break;
		case XK_Right:
			keys[SC_CURSORBLOCKRIGHT] = press;
			break;
			
		case XK_KP_Enter:
			keys[SC_KEYPADENTER] = press;
			break;
		case XK_Return:
			keys[SC_ENTER] = press;
			break;
		case XK_space:
			keys[SC_SPACE] = press;
			break;
		
		case XK_Alt_L:
			keys[SC_LEFTALT] = press;
			break;
		case XK_Alt_R:
			keys[SC_RIGHTALT] = press;
			break;
		
		case XK_Control_L:
			keys[SC_LEFTCONTROL] = press;
			break;
		case XK_Control_R:
			keys[SC_RIGHTCONTROL] = press;
			break;
		
		case XK_Shift_L:
			keys[SC_LEFTSHIFT] = press;
			break;
		case XK_Shift_R:
			keys[SC_RIGHTSHIFT] = press;
			break;
				
		case XK_b:
			keys[SC_B] = press;
			break;
	}
}

void keyboard_handler(KeySym keycode, int press)
{
	int i;
	
	UpdateKeys(keycode, press);
	
	if (RSJ) {		
		if (press == 0) {
			for (i = 0; i < CheatCount; i++) {
				char *key = XKeysymToString(keycode);
				if (key == NULL)
					break;
				if (strlen(key) != 1)
					break;
				if (CheatCodes[i].code[CheatIndex] == toupper(key[0])) {
					CheatIndex++;
					if (CheatCodes[i].code[CheatIndex] == 0) {
						PlaySound(SND_BONUS);
						switch (i) {
						case 0:
						case 4:
							GiveKey(0);
							GiveKey(1);
							gamestate.godmode = TRUE;
							break;
						case 1:
							gamestate.godmode^=TRUE;
							break;
						case 2:
							gamestate.machinegun = TRUE;
							gamestate.chaingun = TRUE;
							gamestate.flamethrower = TRUE;
							gamestate.missile = TRUE;
							GiveAmmo(gamestate.maxammo);
							GiveGas(99);
							GiveMissile(99);
							break;
						case 3:
							gamestate.maxammo = 999;
							GiveAmmo(999);
							break;
						case 5:
							GiveKey(0);
							GiveKey(1);
							break;
						case 6:
							playstate=EX_WARPED;
							nextmap = gamestate.mapon+1;
							if (MapListPtr->MaxMap<=nextmap) 
								nextmap = 0;
							break;
						case 7:
							ShowPush ^= TRUE;
							break;
						}
						CheatIndex = 0;
					}
					break;
				} 
			}	
			if (i == CheatCount) 
				CheatIndex = 0;
		}
				
		joystick1 = 0;
		
		if (press == 0) {
			switch(keycode) {
			case XK_1:
				gamestate.pendingweapon = WP_KNIFE;
				break;
			case XK_2:
				if (gamestate.ammo) {
					gamestate.pendingweapon = WP_PISTOL;
				}	
				break;
			case XK_3:
				if (gamestate.ammo && gamestate.machinegun) {
					gamestate.pendingweapon = WP_MACHINEGUN;
				}
				break;
			case XK_4:
				if (gamestate.ammo && gamestate.chaingun) {
					gamestate.pendingweapon = WP_CHAINGUN;
				}
				break;
			case XK_5:
				if (gamestate.gas && gamestate.flamethrower) {
					gamestate.pendingweapon = WP_FLAMETHROWER;
				}
				break;
			case XK_6:
				if (gamestate.missiles && gamestate.missile) {
					gamestate.pendingweapon = WP_MISSILE;
				}
				break;
			case XK_period:
			case XK_slash:
				joystick1 = JOYPAD_START;
				break;
			case XK_Escape:
				Quit(NULL); /* fast way out */
			}
		}
		
		if (keys[SC_CURSORUPLEFT])
			joystick1 |= (JOYPAD_UP|JOYPAD_LFT);
		if (keys[SC_CURSORUP])
			joystick1 |= JOYPAD_UP;
		if (keys[SC_CURSORUPRIGHT])
			joystick1 |= (JOYPAD_UP|JOYPAD_RGT);
		if (keys[SC_CURSORRIGHT])
			joystick1 |= JOYPAD_RGT;
		if (keys[SC_CURSORDOWNRIGHT])
			joystick1 |= (JOYPAD_DN|JOYPAD_RGT);
		if (keys[SC_CURSORDOWN])
			joystick1 |= JOYPAD_DN;
		if (keys[SC_CURSORDOWNLEFT])
			joystick1 |= (JOYPAD_DN|JOYPAD_LFT);
		if (keys[SC_CURSORLEFT])
			joystick1 |= JOYPAD_LFT;	
		
		if (keys[SC_CURSORBLOCKLEFT]) 
			joystick1 |= JOYPAD_LFT;
		if (keys[SC_CURSORBLOCKRIGHT])
			joystick1 |= JOYPAD_RGT;
		if (keys[SC_CURSORBLOCKUP])
			joystick1 |= JOYPAD_UP;
		if (keys[SC_CURSORBLOCKDOWN])
			joystick1 |= JOYPAD_DN;
		
		if (keys[SC_KEYPADENTER])
			joystick1 |= JOYPAD_A;	
		if (keys[SC_ENTER])
			joystick1 |= JOYPAD_A;
		if (keys[SC_SPACE])
			joystick1 |= JOYPAD_A;
		
		if (keys[SC_LEFTALT]) 
			joystick1 |= JOYPAD_TL;
		if (keys[SC_RIGHTALT])
			joystick1 |= JOYPAD_TR;
			
		if (keys[SC_LEFTCONTROL])
			joystick1 |= JOYPAD_B;
		if (keys[SC_RIGHTCONTROL])
			joystick1 |= JOYPAD_B;
		
		if (keys[SC_LEFTSHIFT])
			joystick1 |= (JOYPAD_X|JOYPAD_Y);
		if (keys[SC_RIGHTSHIFT])
			joystick1 |= (JOYPAD_X|JOYPAD_Y);
	}
							
}

int HandleEvents()
{
	XEvent event;
	int ret = 0;
	
	if (XPending(dpy)) {
		do {
			XNextEvent(dpy, &event);
			switch(event.type) {
				case KeyPress:
					keyboard_handler(XKeycodeToKeysym(dpy, event.xkey.keycode, 0), 1);
					/* ret = 1; */
					break; 
				case KeyRelease:
					keyboard_handler(XKeycodeToKeysym(dpy, event.xkey.keycode, 0), 0);
					ret = 1;
					break;
				case Expose:
					BlastScreen();
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
	return ret;
}

void ReadSystemJoystick()
{
	RSJ = 1;
	HandleEvents();
}

int DoEvents()
{
	RSJ = 0;
	if (HandleEvents()) {
		if (keys[SC_B]) { /* Special */
			return 'B';
		}
		return 1;
	}
	return 0;
}

Word ChooseGameDiff()
{
/* 0 = easy, 1 = normal, 2 = hard, 3 = death incarnate */
	difficulty = 1;
	SetAPalette(rGamePal);

	return 1;
}
