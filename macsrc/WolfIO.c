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

#include <string.h>

#include "wolfdef.h"

/**********************************

	Read the keyboard/mouse
	
**********************************/

void IO_CheckInput(void)
{
	ReadSystemJoystick();	/* Set the variable "joystick1" */
	
/* check for auto map */

    if (joystick1 & JOYPAD_START) {
         RunAutoMap();		/* Do the auto map */
    }

/*
** get game control flags from joypad
*/
    memset(buttonstate,0,sizeof(buttonstate));	/* Zap the buttonstates */
    if (joystick1 & JOYPAD_UP)
         buttonstate[bt_north] = 1;
    if (joystick1 & JOYPAD_DN)
         buttonstate[bt_south] = 1;
    if (joystick1 & JOYPAD_LFT)
         buttonstate[bt_west] = 1;
    if (joystick1 & JOYPAD_RGT)
         buttonstate[bt_east] = 1;
    if (joystick1 & JOYPAD_TL)
         buttonstate[bt_left] = 1;
    if (joystick1 & JOYPAD_TR)
         buttonstate[bt_right] = 1;
    if (joystick1 & JOYPAD_B)
         buttonstate[bt_attack] = 1;
    if (joystick1 & (JOYPAD_Y|JOYPAD_X) )
         buttonstate[bt_run] = 1;
    if (joystick1 & JOYPAD_A)
         buttonstate[bt_use] = 1;
    if (joystick1 & JOYPAD_SELECT) {
         buttonstate[bt_select] = 1;
	}
}
