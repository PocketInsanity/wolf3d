/* id_vl.c */

#include "id_heads.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>

#include <GL/gl.h>
#include <GL/glx.h>

byte *gfxbuf = NULL;

Display *dpy;
int screen;
Window root, win;
XVisualInfo *vi;
Colormap cmap;
Atom wmDeleteWindow;
GLXContext ctx;

int attrib[] = {
	GLX_RGBA,
	GLX_RED_SIZE,           5,
	GLX_GREEN_SIZE,         5,
	GLX_BLUE_SIZE,          5,
	GLX_DEPTH_SIZE,         16,
	GLX_DOUBLEBUFFER,
        None
};

int main(int argc, char *argv[])
{
	/* TODO: move this to the proper functions */
	
	XSetWindowAttributes attr;
	XSizeHints sizehints;	
	Pixmap bitmap;
	Cursor cursor;
	XColor bg = { 0 };
	XColor fg = { 0 };
	char data[8] = { 0x01 };
		
	char *disp, *ext;
	int attrmask;
	int major, minor;
		
	disp = getenv("DISPLAY");
	dpy = XOpenDisplay(disp);
	if (dpy == NULL) {
		/* TODO: quit function with vsnprintf */
		fprintf(stderr, "Unable to open display %s!\n", XDisplayName(disp));
		exit(EXIT_FAILURE);
	}
	
	screen = DefaultScreen(dpy);
	
	root = RootWindow(dpy, screen);
	
	if (glXQueryExtension(dpy, NULL, NULL) == False) {
		fprintf(stderr, "X server %s does not support GLX\n", XDisplayName(disp));
		exit(EXIT_FAILURE);
	}
	
	if (glXQueryVersion(dpy, &major, &minor) == False) {
		fprintf(stderr, "glXQueryVersion returned False?\n");
		exit(EXIT_FAILURE);
	} else {
		printf("GLX Version %d.%d\n", major, minor);
		printf("GLX Client:\n");
		printf("GLX_VENDOR: %s\n", glXGetClientString(dpy, GLX_VENDOR));
		printf("GLX_VERSION: %s\n", glXGetClientString(dpy, GLX_VERSION));
		printf("GLX_EXTENSIONS: %s\n", glXGetClientString(dpy, GLX_EXTENSIONS));
		printf("GLX Server:\n");
		printf("GLX_VENDOR: %s\n", glXQueryServerString(dpy, DefaultScreen(dpy), GLX_VENDOR));
		printf("GLX_VERSION: %s\n", glXQueryServerString(dpy, DefaultScreen(dpy), GLX_VERSION));
		printf("GLX_EXTENSIONS: %s\n", glXQueryServerString(dpy, DefaultScreen(dpy), GLX_EXTENSIONS));
		printf("Both:\n");
		printf("GLX_EXTENSIONS: %s\n", glXQueryExtensionsString(dpy, DefaultScreen(dpy)));
	}
	
	vi = glXChooseVisual(dpy, DefaultScreen(dpy), attrib);
	
	if (vi == NULL) {
		Quit("No suitable GL visual found!");
	}

	ctx = glXCreateContext(dpy, vi, NULL, True);	
	
	if (ctx == NULL) {
		Quit("glx context create failed");
	}
	
	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	                      	
	attr.colormap = cmap;		   
	attr.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask |
			  StructureNotifyMask;
	attrmask = CWColormap | CWEventMask;
	win = XCreateWindow(dpy, root, 0, 0, 320, 200, 0, CopyFromParent, 
			    InputOutput, vi->visual, attrmask, &attr);
	
	if (win == None) {
		Quit("Unable to create window!");
	}
		
	sizehints.min_width = 320;
	sizehints.min_height = 200;
	/*
	sizehints.max_width = 320;
	sizehints.max_height = 200;
	sizehints.base_width = 320;
	sizehints.base_height = 200;
	sizehints.flags = PMinSize | PMaxSize | PBaseSize;
	*/
	sizehints.flags = PMinSize;
	
	XSetWMProperties(dpy, win, NULL, NULL, argv, argc, &sizehints, None, None); 
	
	XStoreName(dpy, win, GAMENAME);
	XSetIconName(dpy, win, GAMENAME);
	
	wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &wmDeleteWindow, 1);

	bitmap = XCreateBitmapFromData(dpy, win, data, 8, 8);
	cursor = XCreatePixmapCursor(dpy, bitmap, bitmap, &fg, &bg, 0, 0);
	XDefineCursor(dpy, win, cursor);
	
	XFlush(dpy);
	
	glXMakeCurrent(dpy, win, ctx);
	
	printf("GL Library:\n");
	printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
	printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
	printf("GL_EXTENSIONS: %s\n", glGetString(GL_EXTENSIONS));
	
	return WolfMain(argc, argv);
}

void VL_WaitVBL(int vbls)
{
	/* hack - but it works for me */
	long last = get_TimeCount() + 1;
	while (last > get_TimeCount()) ;
}

void VW_UpdateScreen()
{
}

/*
=======================
=
= VL_Startup
=
=======================
*/

void Init3D();

void VL_Startup()
{
	if (gfxbuf == NULL) 
		gfxbuf = malloc(320 * 200 * 1);
				   
	XMapWindow(dpy, win);
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Init3D();		
}

/*
=======================
=
= VL_Shutdown
=
=======================
*/

void VL_Shutdown (void)
{
	if (gfxbuf != NULL) {
		free(gfxbuf);
		gfxbuf = NULL;
	}
}

//===========================================================================

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
}	

//===========================================================================

/*
=================
=
= VL_SetPalette
=
=================
*/

void VL_SetPalette(byte *palette)
{
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
}

/*
=============================================================================

							PIXEL OPS

=============================================================================
*/

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

void VL_MemToScreen(byte *source, int width, int height, int x, int y)
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

//	Internal routines

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
				case ConfigureNotify:
					glViewport(0, 0, event.xconfigure.width, event.xconfigure.height);
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
