/* wl_debug.c */

#include "wl_def.h"

/*
==================
=
= CountObjects
=
==================
*/

void CountObjects (void)
{
	int	i,total,count,active,inactive,doors;
	objtype	*obj;

	CenterWindow (16,7);
	active = inactive = count = doors = 0;

	US_Print ("Total statics :");
	total = laststatobj-&statobjlist[0];
	US_PrintUnsigned (total);

	US_Print ("\nIn use statics:");
	for (i=0;i<total;i++)
		if (statobjlist[i].shapenum != -1)
			count++;
		else
			doors++;	//debug
	US_PrintUnsigned (count);

	US_Print ("\nDoors         :");
	US_PrintUnsigned (doornum);

	for (obj=player->next;obj;obj=obj->next)
	{
		if (obj->active)
			active++;
		else
			inactive++;
	}

	US_Print ("\nTotal actors  :");
	US_PrintUnsigned (active+inactive);

	US_Print ("\nActive actors :");
	US_PrintUnsigned (active);

	VW_UpdateScreen();
	IN_Ack ();
}

//===========================================================================

/*
================
=
= PicturePause
=
================
*/

void PicturePause (void)
{

	FinishPaletteShifts ();

	IN_Ack();
	
	if (LastScan != sc_Enter)
	{
		return;
	}

	/* TODO: save picture to file */
	
	VL_SetPalette(gamepal);

	VW_WaitVBL(70);
	VW_WaitVBL(70);
	Quit(NULL);
}


//===========================================================================


/*
================
=
= ShapeTest
=
================
*/

void ShapeTest (void)
{
#if 0 /* TODO: this code want to access the raycasting renderer directly */
extern	word	NumDigi;
extern	word	*DigiList;
static	char	buf[10];

	boolean			done;
	ScanCode		scan;
	int				i,j,k,x;
	longword		l;
	memptr			addr;
	PageListStruct *page;

	CenterWindow(20,16);
	VW_UpdateScreen();
	for (i = 0,done = false;!done;)
	{
		US_ClearWindow();

		page = &PMPages[i];
		US_Print(" Page #");
		US_PrintUnsigned(i);
		if (i < PMSpriteStart)
			US_Print(" (Wall)");
		else if (i < PMSoundStart)
			US_Print(" (Sprite)");
		else if (i == ChunksInFile - 1)
			US_Print(" (Sound Info)");
		else
			US_Print(" (Sound)");

		US_Print("\n XMS: ");
		US_Print("No");

		US_Print("\n Main: ");
		US_Print("No");

		US_Print("\n Last hit: ");
		US_PrintUnsigned(page->lastHit);

		US_Print("\n Address: ");
		addr = PM_GetPageAddress(i);
		sprintf(buf,"%p", addr); /* TODO: might wanna check */
		US_Print(buf);

		if (addr)
		{
			if (i < PMSpriteStart)
			{
			//
			// draw the wall
			//
				postx = 128;
				for (x=0;x<64;x++,postx++)
				{
					wallheight[postx] = 256;
					ScalePost((byte *)addr, x);
				}
			}
			else if (i < PMSoundStart)
			{
			//
			// draw the sprite
			//
				SimpleScaleShape (160, i-PMSpriteStart, 64);
			}
			else if (i == ChunksInFile - 1)
			{
				US_Print("\n\n Number of sounds: ");
				US_PrintUnsigned(NumDigi);
				for (l = j = k = 0;j < NumDigi;j++)
				{
					l += DigiList[(j * 2) + 1];
					k += (DigiList[(j * 2) + 1] + (PMPageSize - 1)) / PMPageSize;
				}
				US_Print("\n Total bytes: ");
				US_PrintUnsigned(l);
				US_Print("\n Total pages: ");
				US_PrintUnsigned(k);
			}
			else
			{
				byte *dp = addr;
				for (j = 0;j < NumDigi;j++)
				{
					k = (DigiList[(j * 2) + 1] + (PMPageSize - 1)) / PMPageSize;
					if
					(
						(i >= PMSoundStart + DigiList[j * 2])
					&&	(i < PMSoundStart + DigiList[j * 2] + k)
					)
						break;
				}
				if (j < NumDigi)
				{
					US_Print("\n Sound #");
					US_PrintUnsigned(j);
					US_Print("\n Segment #");
					US_PrintUnsigned(i - PMSoundStart - DigiList[j * 2]);
				}
				for (j = 0;j < page->length;j += 32)
				{
					byte v = dp[j];
					int v2 = (unsigned)v;
					v2 -= 128;
					v2 /= 4;
					if (v2 < 0)
						VW_Vlin(WindowY + WindowH - 32 + v2,
								WindowY + WindowH - 32,
								WindowX + 8 + (j / 32),BLACK);
					else
						VW_Vlin(WindowY + WindowH - 32,
								WindowY + WindowH - 32 + v2,
								WindowX + 8 + (j / 32),BLACK);
				}
			}
		}

		VW_UpdateScreen();

		while (!(scan = LastScan)) {
			SD_Poll();
			IN_CheckAck();
		}

		IN_ClearKey(scan);
		switch (scan)
		{
		case sc_LeftArrow:
			if (i)
				i--;
			break;
		case sc_RightArrow:
			if (++i >= ChunksInFile)
				i--;
			break;
		case sc_W:	// Walls
			i = 0;
			break;
		case sc_S:	// Sprites
			i = PMSpriteStart;
			break;
		case sc_D:	// Digitized
			i = PMSoundStart;
			break;
		case sc_I:	// Digitized info
			i = ChunksInFile - 1;
			break;
		case sc_L:	// Load all pages
			for (j = 0;j < ChunksInFile;j++)
				PM_GetPage(j);
			break;
		case sc_P:
/* TODO: this would play the current digital sound */
			break;
		case sc_Escape:
			done = true;
			break;
		case sc_Enter:
			PM_GetPage(i);
			break;
		}
	}
#endif
}

//===========================================================================


/*
================
=
= DebugKeys
=
================
*/

int DebugKeys()
{
	boolean esc;
	int level;

	if (Keyboard[sc_C])		// C = count objects
	{
		CountObjects();
		return 1;
	}

	if (Keyboard[sc_E])		// E = quit level
	{
		playstate = ex_completed;
//		gamestate.mapon++;
	}

	if (Keyboard[sc_F])		// F = facing spot
	{
		CenterWindow (14,4);
		US_Print ("X:");
		US_PrintUnsigned (player->x);
		US_Print ("\nY:");
		US_PrintUnsigned (player->y);
		US_Print ("\nA:");
		US_PrintUnsigned (player->angle);
		VW_UpdateScreen();
		IN_Ack();
		return 1;
	}

	if (Keyboard[sc_G])		// G = god mode
	{
		CenterWindow (12,2);
		if (godmode)
		  US_PrintCentered ("God mode OFF");
		else
		  US_PrintCentered ("God mode ON");
		VW_UpdateScreen();
		IN_Ack();
		godmode ^= 1;
		return 1;
	}
	if (Keyboard[sc_H])		// H = hurt self
	{
		IN_ClearKeysDown ();
		TakeDamage (16,NULL);
	}
	else if (Keyboard[sc_I])			// I = item cheat
	{
		CenterWindow (12,3);
		US_PrintCentered ("Free items!");
		VW_UpdateScreen();
		GivePoints (100000);
		HealSelf (99);
		if (gamestate.bestweapon<wp_chaingun)
			GiveWeapon (gamestate.bestweapon+1);
		gamestate.ammo += 50;
		if (gamestate.ammo > 99)
			gamestate.ammo = 99;
		DrawAmmo ();
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[sc_N])			// N = no clip
	{
		noclip^=1;
		CenterWindow (18,3);
		if (noclip)
			US_PrintCentered ("No clipping ON");
		else
			US_PrintCentered ("No clipping OFF");
		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[sc_P])			// P = pause with no screen disruptioon
	{
		PicturePause ();
		return 1;
	}
	else if (Keyboard[sc_Q])			// Q = fast quit
		Quit(NULL);
	else if (Keyboard[sc_S])			// S = slow motion
	{
		singlestep^=1;
		CenterWindow (18,3);
		if (singlestep)
			US_PrintCentered ("Slow motion ON");
		else
			US_PrintCentered ("Slow motion OFF");
		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[sc_T])			// T = shape test
	{
		ShapeTest ();
		return 1;
	}
	else if (Keyboard[sc_V])			// V = extra VBLs
	{
		CenterWindow(30,3);
		PrintY+=6;
		US_Print("  Add how many extra VBLs(0-8):");
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);
			if (level>=0 && level<=8)
				extravbls = level;
		}
		return 1;
	}
	else if (Keyboard[sc_W])			// W = warp to level
	{
		CenterWindow(26,3);
		PrintY+=6;
	/* TODO: wouldn't work on sod demo etc */
#ifndef SPEAR
		US_Print("  Warp to which level(1-10):");
#elif defined(SPEARDEMO)
		US_Print("  Warp to which level(1-2):");
#else
		US_Print("  Warp to which level(1-21):");
#endif
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);
#ifndef SPEAR
			if (level>0 && level<11)
#elif defined(SPEARDEMO)
			if (level>0 && level<2)
#else
			if (level>0 && level<22)
#endif
			{
				gamestate.mapon = level-1;
				playstate = ex_warped;
			}
		}
		return 1;
	}

	return 0;
}
