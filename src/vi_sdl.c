#include "wl_def.h"

#include "SDL.h"

byte *gfxbuf = NULL;
SDL_Surface *surface;

/*
==========================
=
= Quit
=
==========================
*/

int main (int argc, char *argv[])
{
	return WolfMain(argc, argv);
}

void DisplayTextSplash(byte *text);

void Quit(char *error)
{
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
		//DisplayTextSplash(screen);
	}
	
	if (error && *error) {
		fprintf(stderr, "Quit: %s\n", error);
		exit(EXIT_FAILURE);
 	}
	exit(EXIT_SUCCESS);
}

void VL_WaitVBL(int vbls)
{
	long last = get_TimeCount() + vbls;
	while (last > get_TimeCount()) ;
}

void VW_UpdateScreen()
{
	//VL_WaitVBL(1); 
	memcpy(surface->pixels, gfxbuf, vwidth*vheight);
	SDL_UpdateRect(surface, 0, 0, 0, 0);
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
	vwidth = 320;
	vheight = 200;
	
	if (MS_CheckParm("x2")) {
		vwidth *= 2;
		vheight *= 2;
	} else if (MS_CheckParm("x3")) {
		vwidth *= 3;
		vheight *= 3;
	}
	
	if (gfxbuf == NULL) 
		gfxbuf = malloc(vwidth * vheight * 1);
		
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		Quit("Couldn't init SDL");
	}

	if (MS_CheckParm("fullscreen"))
		surface = SDL_SetVideoMode(vwidth, vheight, 8, SDL_SWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN);
	else
		surface = SDL_SetVideoMode(vwidth, vheight, 8, SDL_SWSURFACE|SDL_HWPALETTE);
		
	if (surface == NULL) {
		SDL_Quit();
		Quit ("Couldn't set 320x200 mode");
	}
	
	SDL_WM_SetCaption(GAMENAME, GAMENAME);

	SDL_ShowCursor(0);	
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
	if (gfxbuf != NULL) {
		free(gfxbuf);
		gfxbuf = NULL;
	}
	SDL_Quit();
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
	SDL_Color colors[256];
	int i;
	
	for (i = 0; i < 256; i++)
	{
		colors[i].r = palette[i*3+0] << 2;
		colors[i].g = palette[i*3+1] << 2;
		colors[i].b = palette[i*3+2] << 2;
	}
	SDL_SetColors(surface, colors, 0, 256);
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
	int i;
	for (i=0;i<256;i++)
	{
		palette[i*3+0] = surface->format->palette->colors[i].r >> 2;
		palette[i*3+1] = surface->format->palette->colors[i].g >> 2;
		palette[i*3+2] = surface->format->palette->colors[i].b >> 2;
	}
}

void VL_DirectPlot(int x1, int y1, int x2, int y2)
{
	*(((Uint8 *)surface->pixels) + x1 + y1 * vwidth) = *(gfxbuf + x2 + y2 * vwidth);
}

void VL_DirectPlotFlush()
{
	SDL_UpdateRect(surface, 0, 0, 0, 0);
}

/*
=============================================================================

					GLOBAL VARIABLES

=============================================================================
*/

//
// configuration variables
//
boolean		MousePresent;
boolean		JoysPresent[MaxJoys];

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
		case SDLK_1:
			return sc_1;
		case SDLK_2:
			return sc_2;
		case SDLK_3:
			return sc_3;
		case SDLK_4:
			return sc_4;
		case SDLK_a:
			return sc_A;
		case SDLK_b:
			return sc_B;
		case SDLK_c:
			return sc_C;
		case SDLK_h:
			return sc_H;
		case SDLK_i:
			return sc_I;
		case SDLK_l:
			return sc_L;
		case SDLK_m:
			return sc_M;
		case SDLK_n:
			return sc_N;
		case SDLK_t:
			return sc_T;
		case SDLK_y:
			return sc_Y;
		case SDLK_LEFT:
		case SDLK_KP4:
			return sc_LeftArrow;
		case SDLK_RIGHT:
		case SDLK_KP6:
			return sc_RightArrow;
		case SDLK_UP:
		case SDLK_KP8:
			return sc_UpArrow;
		case SDLK_DOWN:
		case SDLK_KP2:
			return sc_DownArrow;
		case SDLK_LCTRL:
			return sc_Control;
		case SDLK_LALT:
			return sc_Alt;
		case SDLK_LSHIFT:
			return sc_LShift;
		case SDLK_RSHIFT:
			return sc_RShift;
		case SDLK_ESCAPE:
			return sc_Escape;
		case SDLK_SPACE:
			return sc_Space;
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			return sc_Enter;
		case SDLK_TAB:
			return sc_Tab;
		case SDLK_BACKSPACE:
			return sc_BackSpace;
		case SDLK_PAUSE:
			return 0xE1;
		default:
			//printf("unknown: %s\n", XKeysymToString(keysym));
			return sc_None;
	}
}
			
void keyboard_handler(int code, int press)
{
	byte k, c = 0;

 	k = code;

	if (k == 0xe1)	// Handle Pause key
		Paused = true;
	else
	{
		if (press == 0)	
		{
			Keyboard[k] = false;
		}
		else			// Make code
		{
			LastCode = CurCode;
			CurCode = LastScan = k;
			Keyboard[k] = true;

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
			
			if (c)
				LastASCII = c;
		}
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
	SDL_Event event;
	
	unsigned i, buttons;
	
	if (SDL_PollEvent(&event)) {
		do {
			switch(event.type) {
				case SDL_KEYDOWN:
					keyboard_handler(XKeysymToScancode(event.key.keysym.sym), 1);
					break;
				case SDL_KEYUP:
					keyboard_handler(XKeysymToScancode(event.key.keysym.sym), 0);
					break;
				default:
					break;
			}
		} while (SDL_PollEvent(&event));
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

byte IN_MouseButtons()
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

byte IN_JoyButtons()
{
	return 0;
}
