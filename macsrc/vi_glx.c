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
#include <ctype.h>
#include <getopt.h>
#include <setjmp.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>

#include "wolfdef.h"

Display *dpy;
int screen;
Window win, root;
XVisualInfo *vi;
GLXContext ctx;
Atom wmDeleteWindow;

#ifdef GL_EXT_shared_texture_palette
extern int UseSharedTexturePalette;
extern PFNGLCOLORTABLEEXTPROC pglColorTableEXT;
#endif

extern int CheckToken(const char *str, const char *item);

int VidWidth, VidHeight, ViewHeight;

int HandleEvents();

extern jmp_buf ResetJmp;
extern Boolean JumpOK;

int attrib[] = {
	GLX_RGBA,
	GLX_RED_SIZE,		5,
	GLX_GREEN_SIZE,		5,
	GLX_BLUE_SIZE,		5,
	GLX_DEPTH_SIZE,		16,
	GLX_DOUBLEBUFFER,
	None
};
                                                        
int main(int argc, char *argv[])
{
	XSetWindowAttributes attr;
	Colormap cmap;
	Pixmap bitmap;
	Cursor cursor;
	XColor bg = { 0 };
	XColor fg = { 0 };
	char data[8] = { 0x01 };
	char *display;
	const char *ext;
	int mask, major, minor, verbose = 0;
	int opt;
	
	while ((opt = getopt(argc, argv, "v")) != -1) {
		switch(opt) {
		case 'v':
			verbose = 1;
			break;
		default:
			fprintf(stderr, "%d (%c) is unknown to me\n", opt, opt);
			break;
		}
	}
	
	if ((argc - optind) != 1) {
		fprintf(stderr, "usage: %s <mac wolf3d resource fork>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	if (InitResources(argv[optind])) {
		fprintf(stderr, "could not load %s\n", argv[optind]);
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
	
	if (glXQueryExtension(dpy, NULL, NULL) == False) {
		fprintf(stderr, "Display %s does not support the GLX Extension\n", XDisplayName(display));
		exit(EXIT_FAILURE);
	}
	
	if (glXQueryVersion(dpy, &major, &minor) == False) {
		fprintf(stderr, "glXQueryVersion returned False?\n");
		exit(EXIT_FAILURE);
	} else if (verbose) {
		printf("GLX Version %d.%d\n", major, minor);
		printf("GLX Client:\n");
		printf("GLX_VENDOR: %s\n", glXGetClientString(dpy, GLX_VENDOR));
		printf("GLX_VERSION: %s\n", glXGetClientString(dpy, GLX_VERSION));
		printf("GLX_EXTENSIONS: %s\n", glXGetClientString(dpy, GLX_EXTENSIONS));
		printf("GLX Server:\n");
		printf("GLX_VENDOR: %s\n", glXQueryServerString(dpy, screen, GLX_VENDOR));
		printf("GLX_VERSION: %s\n", glXQueryServerString(dpy, screen, GLX_VERSION));
		printf("GLX_EXTENSIONS: %s\n", glXQueryServerString(dpy, screen, GLX_EXTENSIONS));
		printf("Both:\n");
		printf("GLX_EXTENSIONS: %s\n", glXQueryExtensionsString(dpy, screen));
	}
	
	vi = glXChooseVisual(dpy, screen, attrib);
	if (vi == NULL) {
		fprintf(stderr, "No usable GL visual found on %s:%d\n", XDisplayName(display), screen);
		exit(EXIT_FAILURE);
	}
		
	ctx = glXCreateContext(dpy, vi, NULL, True);
	if (ctx == NULL) {
 		fprintf(stderr, "GLX context creation failed\n");
		exit(EXIT_FAILURE);
	}
	
	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	
	attr.colormap = cmap;
	attr.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask |
	                  StructureNotifyMask;
	mask = CWColormap | CWEventMask;
	
	win = XCreateWindow(dpy, root, 0, 0, 640, 480, 0, CopyFromParent,
			    InputOutput, vi->visual, mask, &attr);
	
	if (win == None) {
		fprintf(stderr, "Unable to create window\n");
		exit(EXIT_FAILURE);
	}
	
	XSetWMProperties(dpy, win, NULL, NULL, argv, argc, None, None, None);
	
	XStoreName(dpy, win, "Wolfenstein 3D");
	XSetIconName(dpy, win, "Wolfenstein 3D");
	
	wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &wmDeleteWindow, 1);
	
	bitmap = XCreateBitmapFromData(dpy, win, data, 8, 8);
	cursor = XCreatePixmapCursor(dpy, bitmap, bitmap, &fg, &bg, 0, 0);
	XDefineCursor(dpy, win, cursor);
	
	glXMakeCurrent(dpy, win, ctx);
	
 	if (verbose) {
		printf("GL Library:\n");
		printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
		printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
		printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
		printf("GL_EXTENSIONS: %s\n", glGetString(GL_EXTENSIONS));
	}
	
	XMapWindow(dpy, win);
	XFlush(dpy);
	
	ext = (const char *)glGetString(GL_EXTENSIONS);
#ifdef GL_EXT_shared_texture_palette
	UseSharedTexturePalette = 0;
	if (CheckToken(ext, "GL_EXT_shared_texture_palette")) {
		pglColorTableEXT = glXGetProcAddressARB((unsigned const char *)"glColorTableEXT");
		if (pglColorTableEXT) {
			UseSharedTexturePalette = 1;
			printf("GL_EXT_shared_texture_palette found...\n");
		}
	}
#endif
	
	glShadeModel(GL_FLAT);
	
	InitData();
	
	SlowDown = 1;
	GameViewSize = 3;	
	NewGameWindow(GameViewSize); 

	ClearTheScreen(BLACK);
	BlastScreen();

	/* NoEnemies = 1; */
		
	return WolfMain(argc, argv);
}

void Quit(char *str)
{	
	FreeResources();
	
	glXDestroyContext(dpy, ctx);
	
	if (str && *str) {
		fprintf(stderr, "%s\n", str);
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}

void BlastScreen2(Rect *BlastRect)
{
	BlastScreen();
}

#define w VidWidth
#define h VidHeight
#define v ViewHeight

void BlastScreen()
{
	GLint error;
	
	glXSwapBuffers(dpy, win);
	
	error = glGetError();
	if (error != GL_NO_ERROR) {
		do {
			fprintf(stderr, "GL Error: %d\n", error);
			error = glGetError();
		} while (error != GL_NO_ERROR);
		exit(EXIT_FAILURE);
	}
}

Word VidXs[] = {320,512,640,640};
Word VidYs[] = {200,384,400,480};
Word VidVs[] = {160,320,320,400};
Word VidPics[] = {rFaceShapes,rFace512,rFace640,rFace640};
Word VidSize = -1;

Word NewGameWindow(Word NewVidSize)
{
	XSizeHints sizehints;
		
	if (NewVidSize == VidSize)
		return VidSize;
	
	if (NewVidSize < 4) {
		w = VidXs[NewVidSize];
		h = VidYs[NewVidSize];
		v = VidVs[NewVidSize];
	} else {
		fprintf(stderr, "Invalid Vid size: %d\n", NewVidSize);
		exit(EXIT_FAILURE);
	}

	sizehints.min_width =  w;
	sizehints.min_height = h;
	sizehints.flags = PMinSize;
	XSetWMNormalHints(dpy, win, &sizehints);
	XResizeWindow(dpy, win, w, h);
	
	SetAPalette(rBlackPal);
	ClearTheScreen(BLACK);
	BlastScreen();
	
	VidSize = NewVidSize;
	
	XSync(dpy, False);
	glXWaitGL();
	glXWaitX();
	HandleEvents();
	
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
{ "XUSCNIELPPA" }, 	{ "IDDQD" },
{ "BURGER" },		{ "WOWZERS" },
{ "LEDOUX" },		{ "SEGER" },
{ "MCCALL" },		{ "APPLEIIGS" }
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
	
	if (press == 0) {
		switch(keycode) {
		case XK_Escape:
			Quit(NULL); /* fast way out */
		case XK_F2:
			if (playstate == EX_STILLPLAYING) {
				if (!SaveGame("wolf3d.sav")) 
					fprintf(stderr, "Unable to save game\n");
			}
			break;
		case XK_F3:
			if (!LoadGame("wolf3d.sav")) {
				fprintf(stderr, "Unable to load game\n");
			} else {
				longjmp(ResetJmp, EX_LOADGAME);
			}
			break;
		default:
			break;
		}
	}
	
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
			joystick1 |= JOYPAD_TR;
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
		
		if ((joystick1 & (JOYPAD_LFT|JOYPAD_RGT)) == (JOYPAD_LFT|JOYPAD_RGT))
			joystick1 &= ~(JOYPAD_LFT|JOYPAD_RGT);
		if ((joystick1 & (JOYPAD_UP|JOYPAD_DN)) == (JOYPAD_UP|JOYPAD_DN))
			joystick1 &= ~(JOYPAD_UP|JOYPAD_DN);
	
		if (joystick1 & JOYPAD_TR) {
			if (joystick1 & JOYPAD_LFT) {
				joystick1 = (joystick1 & ~(JOYPAD_TR|JOYPAD_LFT)) | JOYPAD_TL;
			} else if (joystick1 & JOYPAD_RGT) {
				joystick1 = joystick1 & ~JOYPAD_RGT;
			} else {
				joystick1 &= ~JOYPAD_TR;
			}
		}
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
					ret = 1;
					break; 
				case KeyRelease:
					keyboard_handler(XKeycodeToKeysym(dpy, event.xkey.keycode, 0), 0);
					ret = 0;
					break;
				case Expose:
					RedrawScreen();
					break;
				case ConfigureNotify:
					glViewport(0, 0, event.xconfigure.width, event.xconfigure.height);
					glPixelZoom(1.0f, -1.0f);
					glRasterPos2f(-1.0f, 1.0f);
					RedrawScreen();
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
