#include "wl_def.h"

#include <windows.h>
#include <ddraw.h>
#include <dinput.h>
#include <direct.h>

#error "This code is out of date/broken"

byte *gfxbuf = NULL;

HWND win;
IDirectInput *lpDI;
IDirectInputDevice *lpDIKeyboard;

int hCrt;
FILE *hf;

IDirectDraw *lpDD;
IDirectDrawSurface *lpDDSPrimary;
IDirectDrawSurface *lpDDSBuffer;
IDirectDrawClipper *lpDDC;

LPDDSCAPS lpDDSCaps;
DDSURFACEDESC  ddsd;

byte mypal[768];

void WolfDirectDrawPaint();
HRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
void CheckEvents();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
	static char szAppName[] = "Wolf3D";
	HWND hwnd;

	WNDCLASSEX wndclass;

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindow(szAppName, GAMENAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 320, 200, NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, iCmdShow);

	UpdateWindow(hwnd);	

		if (DirectDrawCreate(NULL, &lpDD, NULL) != DD_OK) {
			MessageBox(hwnd, "ugh", "ugh", MB_OK);
			return 0;
		}
		if (IDirectDraw_SetCooperativeLevel(lpDD, hwnd, DDSCL_NORMAL) != DD_OK) {
			MessageBox(hwnd, "hi", "hi", MB_OK);
		}
//		if (IDirectDraw_SetDisplayMode(lpDD, 320, 200, 8) != DD_OK) {
//			MessageBox(hwnd, "hi2", "hi", MB_OK);
//		}
		ddsd.dwSize = sizeof(ddsd); 
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		if (IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDSPrimary, NULL) != DD_OK) {
			MessageBox(hwnd, "hi3", "hi", MB_OK);
		}
		if (IDirectDraw_CreateClipper(lpDD, 0, &lpDDC, NULL) != DD_OK) {
			MessageBox(hwnd, "hi4", "hi", MB_OK);
		}
		if (IDirectDrawClipper_SetHWnd(lpDDC, 0, hwnd) != DD_OK) {
			MessageBox(hwnd, "hi5", "hi", MB_OK);
		}
		if (IDirectDrawSurface_SetClipper(lpDDSPrimary, lpDDC) != DD_OK) {
			MessageBox(hwnd, "hi6", "hi", MB_OK);
		}
		ddsd.dwFlags = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		ddsd.dwWidth = 320;
		ddsd.dwHeight = 200;
		if (IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDSBuffer, NULL) != DD_OK) {
			MessageBox(hwnd, "hi7", "hi", MB_OK);
		}

		if(DirectInputCreate(hInstance, DIRECTINPUT_VERSION, &lpDI, NULL) != DI_OK) {
			MessageBox(hwnd, "hi8", "hi", MB_OK);
		}
		
		if (IDirectInput_CreateDevice(lpDI, &GUID_SysKeyboard, &lpDIKeyboard, NULL) != DD_OK) {
			MessageBox(hwnd, "hi9", "hi", MB_OK);
		}

		IDirectInputDevice_SetDataFormat(lpDIKeyboard, &c_dfDIKeyboard);

		if (IDirectInputDevice_SetCooperativeLevel(lpDIKeyboard, hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND) != DD_OK) {
			MessageBox(hwnd, "hi10", "hi", MB_OK);
		}

		win = hwnd;

		return WolfMain(0, NULL); /* TODO ... need to parse commandline */
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
		MessageBox(win, error, "Error!", MB_ICONEXCLAMATION | MB_OK);
 	}
		
	SendMessage(win, WM_CLOSE, 0, 0);

	for (;;) CheckEvents(); /* We sent our death wish, now we wait */
}


void WolfDirectDrawPaint()
{
	int x;
	unsigned short int *ptr;
	HWND hwnd;

	if (!lpDD)
		return;

	if (!gfxbuf)
		return;

	if (IDirectDrawSurface_Lock(lpDDSBuffer, NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR,NULL) != DD_OK) {
		MessageBox(win, "eh?", "hrm", MB_OK);
		return;
	}

	IDirectDrawClipper_GetHWnd(lpDDC, &hwnd);

	ptr = ddsd.lpSurface;
	for (x = 0; x < 64000; x++) {
		*ptr = (mypal[gfxbuf[x]*3+0]>>3) << 13 |
		       (mypal[gfxbuf[x]*3+1]>>3) << 8  |
		       (mypal[gfxbuf[x]*3+2]>>3) << 2;
		ptr++;
	}

//memcpy(ddsd.lpSurface, gfxbuf, 64000);
	
	IDirectDrawSurface_Unlock(lpDDSBuffer, ddsd.lpSurface);
	
	if (IDirectDrawSurface_IsLost(lpDDSPrimary)) {
		printf("eh2?\n");
		exit(1);
	}
/*	
	{
		RECT srcrect={0,0,320, 200};
		RECT dstrect;
		GetWindowRect(hwnd,&dstrect);
		dstrect.top+=42;
		dstrect.bottom-=4;
		dstrect.left+=4;
		dstrect.right-=4;								
		IDirectDrawSurface_Blt(lpDDSPrimary, &dstrect, lpDDSBuffer, &srcrect, 0, NULL);
	}
*/
	{
		/* TODO: need to get and save window border sizes (setting default size needs this too) */
		RECT srcrect={0,0,320, 200};
		RECT dstrect;
		GetWindowRect(hwnd,&dstrect);								
		IDirectDrawSurface_Blt(lpDDSPrimary, &dstrect, lpDDSBuffer, &srcrect, 0, NULL);
	}
}

byte keys[256];
byte oldk[256];

void keyboard_handler(int, int);

int DIKToScancode(int i)
{
	switch(i) {
	case DIK_LEFT:
		return sc_LeftArrow;
	case DIK_RIGHT:
		return sc_RightArrow;
	case DIK_UP:
		return sc_UpArrow;
	case DIK_DOWN:
		return sc_DownArrow;
	default:
		return i;
	}
}

void WolfDirectInputUpdateKeys()
{
	int i;

	if (IDirectInputDevice_GetDeviceState(lpDIKeyboard, 256, keys) != DI_OK) {
		if (IDirectInputDevice_Acquire(lpDIKeyboard) != DI_OK)
			return;
		if (IDirectInputDevice_GetDeviceState(lpDIKeyboard, 256, keys) != DI_OK)
			return;
	}

	for (i = 0; i < 256; i++) 
		if (keys[i] != oldk[i])
			keyboard_handler(DIKToScancode(i), keys[i] ? 1 : 0);
	
	memcpy(oldk, keys, sizeof(oldk));
}

void CheckEvents()
{
	MSG msg;
	
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT)
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	WolfDirectInputUpdateKeys();
	
	if (msg.message == WM_QUIT)
			exit(msg.wParam);
}

HRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (iMsg) {
		case WM_CREATE: 
#ifdef _DEBUG			
			AllocConsole();
			
			hCrt = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
			hf = _fdopen( hCrt, "w" );
			*stdout = *hf;
			setvbuf(stdout, NULL, _IONBF, 0 );
			*stderr = *hf;
			setvbuf(stderr, NULL, _IONBF, 0);
#endif

			/* TODO */
			_chdir("C:\\wolfsrc\\src\\test\\");
			/* - oh well, if path is not found, CWD will still be the same (for end user use) */

			return 0;
		case WM_COMMAND:
			break;
		case WM_ERASEBKGND:
			break;
		case WM_PAINT:
			hdc = BeginPaint(hwnd, &ps);
			WolfDirectDrawPaint(hwnd);
			EndPaint(hwnd, &ps);
			break;
		case WM_CLOSE:
			IDirectDrawSurface_Release(lpDDSBuffer);
			IDirectDrawClipper_Release(lpDDC);
			IDirectDrawSurface_Release(lpDDSPrimary);
			IDirectDraw_Release(lpDD);

#ifdef _DEBUG		
			FreeConsole();
			fclose(hf);
			close(hCrt);
#endif

			DestroyWindow(hwnd);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void VL_WaitVBL(int vbls)
{
}

void VW_UpdateScreen()
{
	/* VL_WaitVBL(1); */
	WolfDirectDrawPaint();
}

/*
=======================
=
= VL_Startup
=
=======================
*/

void VL_Startup (void)
{
	if (gfxbuf == NULL) 
		gfxbuf = malloc(320 * 200 * 1);
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
= VL_SetPalette
=
=================
*/

void VL_SetPalette(const byte *palette)
{
	memcpy(mypal, palette, 768);
	VW_UpdateScreen();
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
	memcpy(palette, mypal, 768);
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

void keyboard_handler(int code, int press)
{
	byte k, c = 0;

	k = code;

	if ( k == 0xE1 )	// Handle Pause key
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

boolean IN_CheckAck (void)
{
	unsigned	i,buttons;
	
	CheckEvents(); /* get all events */

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

void IN_Ack (void)
{
	IN_StartAck ();

//	return; /* TODO: fix when keyboard implemented */
	while (!IN_CheckAck ()) ;
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
	
	IN_StartAck ();
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


