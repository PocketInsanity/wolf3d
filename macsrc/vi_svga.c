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
#include <string.h>
#include <getopt.h>

#include <vga.h>
#include <vgakeyboard.h>

#include "wolfdef.h"

Byte *gfxbuf;

void keyboard_handler(int key, int press);

int VidModes[] = { G320x200x256, G512x384x256, G640x400x256, G640x480x256 };

int main(int argc, char *argv[])
{
	int opt, vidmode = 0;
	int i;
	
	while ((opt = getopt(argc, argv, "s:")) != -1) {
		switch(opt) {
		case 's': /* Set video mode (resolution) */
			vidmode = atoi(optarg);
			if ((vidmode < 0) && (vidmode > 3)) {
				fprintf(stderr, "Invalid video mode %d\n", vidmode);
				exit(EXIT_FAILURE);
			}
			break;
		case ':':
			fprintf(stderr, "option needs a parameter: %c\n", optopt);
			break;
		case '?':
			fprintf(stderr, "unknown option: %c\n", optopt);
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
	
	vga_init();
	
	if (!vga_hasmode(G320x200x256)) {
		fprintf(stderr, "SVGAlib says 320x200x256 is not supported...\n");
		exit(EXIT_FAILURE);
	}
	
	for (i = 1; i < 4; i++) {
		if (!vga_hasmode(VidModes[i]))
			VidModes[i] = -1;
	}
	
	if (VidModes[vidmode] == -1) {
		fprintf(stderr, "Video mode %d is not supported...\n", vidmode);
		exit(EXIT_FAILURE);
	}
	
	if (InitResources(argv[optind])) {
		fprintf(stderr, "could not load %s\n", argv[optind]);
		exit(EXIT_FAILURE);
	}
	
	InitData();
	
	GameViewSize = vidmode;
	NewGameWindow(GameViewSize);

	keyboard_init(); /* keyboard must be init'd after vga_setmode .. */
	keyboard_seteventhandler(keyboard_handler);

	ClearTheScreen(BLACK);
	BlastScreen();
	
	return WolfMain(argc, argv);
}

void Quit(char *str)
{
	keyboard_close();
	vga_setmode(TEXT);
	
	FreeResources();
	
	if (gfxbuf)
		free(gfxbuf);
	
	if (str && *str) {
		fprintf(stderr, "%s\n", str);
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}

void SetPalette(Byte *pal)
{
	int i;

	vga_waitretrace();

	for (i = 0; i < 256; i++) 
		vga_setpalette(i, pal[i*3+0] >> 2, pal[i*3+1] >> 2, pal[i*3+2] >> 2);	
}
	
void BlastScreen2(Rect *BlastRect)
{
	BlastScreen();
}

int VidWidth, VidHeight, ViewHeight;
#define w VidWidth
#define h VidHeight
#define v ViewHeight

int vh, vw;

void BlastScreen()
{
	Byte *ptrs = gfxbuf, *ptrd = vga_getgraphmem();
 	int i, hm, wm;

	wm = (w > vw) ? vw : w;
	hm = (h > vh) ? vh : h;
	
	for (i = 0; i < hm; i++) {
		memcpy(ptrd, ptrs, wm);
		ptrs += w;
		ptrd += vw;
	}
}

Word VidXs[] = {320,512,640,640};       /* Screen sizes to play with */
Word VidYs[] = {200,384,400,480};
Word VidVs[] = {160,320,320,400};
Word VidPics[] = {rFaceShapes,rFace512,rFace640,rFace640};
Word VidSize = -1;
Word x512Hack = 0;

Word NewGameWindow(Word NewVidSize)
{
	LongWord *LongPtr;
	Byte *DestPtr;
	int i;
	
	/* printf("Called: %d\n", NewVidSize); */

	if (NewVidSize == -1) {
		fprintf(stderr, "Invalid vid size: %d\n", NewVidSize);
		exit(EXIT_FAILURE);
	}	
	
	if (!x512Hack && (NewVidSize == VidSize))
		return VidSize;

	if ((NewVidSize == 1) && (VidModes[1] == -1)) {		
		if (VidSize == -1) {
			fprintf(stderr, "Trying to set 512Hack before setting a real value!\n");
			exit(EXIT_FAILURE);
		}
		
		if (x512Hack)
			return VidSize;
			
		x512Hack = 1;
	} else {
		x512Hack = 0;
	}
	
 	/* printf("Setting Size: %d (from %d)\n", NewVidSize, VidSize); */
		
	if (NewVidSize < 4) {
		if (!x512Hack && VidModes[NewVidSize] == -1) {
			fprintf(stderr, "Trying to set to an unsupported mode (%d)!\n", NewVidSize);
			exit(EXIT_FAILURE);
		}
		w = VidXs[NewVidSize];
		h = VidYs[NewVidSize];
		v = VidVs[NewVidSize];	
	} else {
		fprintf(stderr, "Invalid vid size: %d\n", NewVidSize);
		exit(EXIT_FAILURE);
	}

	if (gfxbuf)
		free(gfxbuf);
		
	gfxbuf = (Byte *)malloc(w * h);
	
	if (!x512Hack) {
		vw = w;
		vh = h;
		vga_setmode(VidModes[NewVidSize]);
		vga_setlinearaddressing();
		vga_clear();
	}
	
	VideoPointer = gfxbuf;
	VideoWidth = w;
	InitYTable();
	SetAPalette(rBlackPal);
	ClearTheScreen(BLACK);
	BlastScreen();
	
	LongPtr = (LongWord *)LoadAResource(VidPics[NewVidSize]);

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
	while (keyboard_update()) ;
	keyboard_clearstate();
	
	gamestate.keys = 0;
	joystick1 = 0;
	memset(keys, 0, sizeof(keys));
}

#define SC(x) SCANCODE_##x

struct {
	int code[13];
	int flag;
} CheatCodes[] = {
{ { SC(X), SC(U), SC(S), SC(C), SC(N), SC(I), SC(E), SC(L), SC(P), SC(P), SC(A) }, 0 }, /* "XUSCNIELPPA" */
{ { SC(I), SC(D), SC(D), SC(Q), SC(D) }, 0 }, /* "IDDQD" */
{ { SC(B), SC(U), SC(R), SC(G), SC(E), SC(R) }, 0 }, /* "BURGER" */
{ { SC(W), SC(O), SC(W), SC(Z), SC(E), SC(R), SC(S) }, 0 }, /* "WOWZERS" */
{ { SC(L), SC(E), SC(D), SC(O), SC(U), SC(X) }, 0 }, /* "LEDOUX" */
{ { SC(S), SC(E), SC(G), SC(E), SC(R) }, 0 }, /* "SEGER" */
{ { SC(M), SC(C), SC(C), SC(A), SC(L), SC(L) }, 0 }, /* "MCCALL" */
{ { SC(A), SC(P), SC(P), SC(L), SC(E), SC(I), SC(I), SC(G), SC(S) }, 0 } /* "APPLEIIGS" */
};
const int CheatCount = sizeof(CheatCodes) / sizeof(CheatCodes[0]);
int CheatIndex;

void keyboard_handler(int key, int press)
{
	int i;
	
	if (key == SCANCODE_ESCAPE) 
		Quit(0);

	keys[key] = press;
	
	if (RSJ) {		
		if (press == 0) {
			for (i = 0; i < CheatCount; i++) {
				if (CheatCodes[i].code[CheatIndex] == key) {
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
			switch(key) {
			case SCANCODE_1:
				gamestate.pendingweapon = WP_KNIFE;
				break;
			case SCANCODE_2:
				if (gamestate.ammo) {
					gamestate.pendingweapon = WP_PISTOL;
				}	
				break;
			case SCANCODE_3:
				if (gamestate.ammo && gamestate.machinegun) {
					gamestate.pendingweapon = WP_MACHINEGUN;
				}
				break;
			case SCANCODE_4:
				if (gamestate.ammo && gamestate.chaingun) {
					gamestate.pendingweapon = WP_CHAINGUN;
				}
				break;
			case SCANCODE_5:
				if (gamestate.gas && gamestate.flamethrower) {
					gamestate.pendingweapon = WP_FLAMETHROWER;
				}
				break;
			case SCANCODE_6:
				if (gamestate.missiles && gamestate.missile) {
					gamestate.pendingweapon = WP_MISSILE;
				}
				break;
			case SCANCODE_PERIOD:
			case SCANCODE_SLASH:
				joystick1 = JOYPAD_START;
				break;
			}
		}
		
		if (keys[SCANCODE_CURSORUPLEFT])
			joystick1 |= (JOYPAD_UP|JOYPAD_LFT);
		if (keys[SCANCODE_CURSORUP])
			joystick1 |= JOYPAD_UP;
		if (keys[SCANCODE_CURSORUPRIGHT])
			joystick1 |= (JOYPAD_UP|JOYPAD_RGT);
		if (keys[SCANCODE_CURSORRIGHT])
			joystick1 |= JOYPAD_RGT;
		if (keys[SCANCODE_CURSORDOWNRIGHT])
			joystick1 |= (JOYPAD_DN|JOYPAD_RGT);
		if (keys[SCANCODE_CURSORDOWN])
			joystick1 |= JOYPAD_DN;
		if (keys[SCANCODE_CURSORDOWNLEFT])
			joystick1 |= (JOYPAD_DN|JOYPAD_LFT);
		if (keys[SCANCODE_CURSORLEFT])
			joystick1 |= JOYPAD_LFT;	
		
		if (keys[SCANCODE_CURSORBLOCKLEFT]) 
			joystick1 |= JOYPAD_LFT;
		if (keys[SCANCODE_CURSORBLOCKRIGHT])
			joystick1 |= JOYPAD_RGT;
		if (keys[SCANCODE_CURSORBLOCKUP])
			joystick1 |= JOYPAD_UP;
		if (keys[SCANCODE_CURSORBLOCKDOWN])
			joystick1 |= JOYPAD_DN;
		
		if (keys[SCANCODE_KEYPADENTER])
			joystick1 |= JOYPAD_A;	
		if (keys[SCANCODE_ENTER])
			joystick1 |= JOYPAD_A;
		if (keys[SCANCODE_SPACE])
			joystick1 |= JOYPAD_A;
		
		if (keys[SCANCODE_LEFTALT]) 
			joystick1 |= JOYPAD_TR;
		if (keys[SCANCODE_RIGHTALT])
			joystick1 |= JOYPAD_TR;
			
		if (keys[SCANCODE_LEFTCONTROL])
			joystick1 |= JOYPAD_B;
		if (keys[SCANCODE_RIGHTCONTROL])
			joystick1 |= JOYPAD_B;
		
		if (keys[SCANCODE_LEFTSHIFT])
			joystick1 |= (JOYPAD_X|JOYPAD_Y);
		if (keys[SCANCODE_RIGHTSHIFT])
			joystick1 |= (JOYPAD_X|JOYPAD_Y);	
	}
	
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

void ReadSystemJoystick(void)
{
	RSJ = 1;
	/* keyboard_update(); */
	while (keyboard_update()) ;
}

int DoEvents()
{
	RSJ = 0;
	if (keyboard_update()) {
		while (keyboard_update()) ;
		if (keys[SCANCODE_B]) {
			return 'B';
		}
		return 1;
	}
	return 0;
}

Word ChooseGameDiff(void)
{
/* 0 = easy, 1 = normal, 2 = hard, 3 = death incarnate */
	difficulty = 3;
	SetAPalette(rGamePal);
                
	return 1;
}
