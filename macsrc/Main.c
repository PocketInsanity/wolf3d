/*
Copyright (C) 1992-1994 Id Software, Inc.

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

#include "wolfdef.h"
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <ctype.h>

/**********************************

	Prepare the screen for game

**********************************/

void SetupPlayScreen (void)
{
	SetAPalette(rBlackPal);		/* Force black palette */
	ClearTheScreen(BLACK);		/* Clear the screen to black */
	BlastScreen();
	firstframe = 1;				/* fade in after drawing first frame */
	GameViewSize = NewGameWindow(GameViewSize);
}

/**********************************

	Display the automap
	
**********************************/

void RunAutoMap(void)
{
	Word vx,vy;
	Word Width,Height;
	Word CenterX,CenterY;
	Word oldjoy,newjoy;
	
	MakeSmallFont();				/* Make the tiny font */
	playstate = EX_AUTOMAP;
	vx = viewx>>8;					/* Get my center x,y */
	vy = viewy>>8;
	Width = (SCREENWIDTH/16);		/* Width of map in tiles */
	Height = (VIEWHEIGHT/16);		/* Height of map in tiles */
	CenterX = Width/2;
	CenterY = Height/2;
	if (vx>=CenterX) {
		vx -= CenterX;
	} else {
		vx = 0;
	}
	if (vy>=CenterY) {
		vy -= CenterY;
	} else {
		vy = 0;
	}
	do {
		ClearTheScreen(BLACK);
		DrawAutomap(vx,vy);
		do {
			oldjoy = joystick1;
			ReadSystemJoystick();
		} while (joystick1==oldjoy);
		oldjoy &= joystick1;
		newjoy = joystick1 ^ oldjoy;

		if (newjoy & (JOYPAD_START|JOYPAD_SELECT|JOYPAD_A|JOYPAD_B|JOYPAD_X|JOYPAD_Y)) {
			playstate = EX_STILLPLAYING;
		} 
		if (newjoy & JOYPAD_UP && vy) {
			--vy;
		}
		if (newjoy & JOYPAD_LFT && vx) {
			--vx;
		}
		if (newjoy & JOYPAD_RGT && vx<(MAPSIZE-1)) {
			++vx;
		}
		if (newjoy & JOYPAD_DN && vy <(MAPSIZE-1)) {
			++vy;
		}
	} while (playstate==EX_AUTOMAP);

	playstate = EX_STILLPLAYING;

	KillSmallFont();			/* Release the tiny font */
	RedrawStatusBar();

	ReadSystemJoystick();
	mousex = 0;
	mousey = 0;
	mouseturn = 0;
}

/**********************************

	Begin a new game
	
**********************************/

void StartGame(void)
{	
	if (playstate!=EX_LOADGAME) {	/* Variables already preset */
		NewGame();				/* init basic game stuff */
	}
	SetupPlayScreen();
	GameLoop();			/* Play the game */
	StopSong();			/* Make SURE music is off */
}

/**********************************

	Show the game logo 

**********************************/

Boolean TitleScreen()
{
	Word RetVal;		/* Value to return */

	playstate = EX_LIMBO;	/* Game is not in progress */
	NewGameWindow(1);	/* Set to 512 mode */
	FadeToBlack();		/* Fade out the video */

	DisplayScreen(rTitlePic);
	BlastScreen();

	StartSong(SongListPtr[0]);
	FadeTo(rTitlePal);	/* Fade in the picture */
	BlastScreen();
	
	RetVal = WaitTicksEvent(0);		/* Wait for event */
	
	playstate = EX_COMPLETED;

	return TRUE;	
}

/**********************************

	Main entry point for the game (Called after InitTools)

**********************************/

jmp_buf ResetJmp;
Boolean JumpOK;
extern Word NumberIndex;

int WolfMain(int argc, char *argv[])
{
	WaitTick();			/* Wait for a system tick to go by */
	playstate = (exit_t)setjmp(ResetJmp);	
	NumberIndex = 36;	/* Force the score to redraw properly */
	IntermissionHack = FALSE;
	if (playstate) {
		goto DoGame;	/* Begin a new game or saved game */
	}
	JumpOK = TRUE;		/* Jump vector is VALID */
	FlushKeys();		/* Allow a system event */
	Intro();			/* Do the game intro */
	for (;;) {
		if (TitleScreen()) {		/* Show the game logo */
			StartSong(SongListPtr[0]);
			ClearTheScreen(BLACK);	/* Blank out the title page */
			BlastScreen();
			SetAPalette(rBlackPal);
			if (ChooseGameDiff()) {	/* Choose your difficulty */
				playstate = EX_NEWGAME;	/* Start a new game */
DoGame:
				FadeToBlack();		/* Fade the screen */
				StartGame();		/* Play the game */
			}
		}
		/* TODO: demos or whatever here */
	}
	
	return 0;
}
