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

#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#include "wolfdef.h"

LongWord LastTick; 

TimeCounter gametime, rendtime;

void InitTimeCounter(TimeCounter *t)
{
	t->frames = 0;
	t->mintime = 0xFFFFFFFF;
	t->maxtime = 0;
	t->total = 0;
}

void StartTimeCounter(TimeCounter *t)
{
	struct timeval t0;
	
	gettimeofday(&t0, NULL);
	
	t->secs = t0.tv_sec;
	t->usecs = t0.tv_usec;
}

void EndTimeCounter(TimeCounter *t)
{
	struct timeval t0;
	unsigned long curtime;
	long secs, usecs;
	
	gettimeofday(&t0, NULL);
	
	secs = t0.tv_sec - t->secs;
	usecs = t0.tv_usec - t->usecs;
	
	curtime = secs * 1000000 + usecs;
	
	if (curtime > t->maxtime)
		t->maxtime = curtime;
	if (curtime < t->mintime)
		t->mintime = curtime;
		
	t->total += curtime;
	t->frames++;
}

void PrintTimeCounter(TimeCounter *t, char *header)
{
	double avg;
	 
	if (t->frames == 0)
		return;
		
	avg = (double)t->total / (double)t->frames;
	printf("%s:\n", header);
	printf("Frames: %lu, time %lu, avg %f\n", t->frames, t->total, avg);
	printf("Min: %lu, max:%lu\n", t->mintime, t->maxtime);
}

void ShareWareEnd(void)
{
        SetAPalette(rGamePal);
        /* printf("ShareWareEnd()\n"); */
        SetAPalette(rBlackPal);
}

Word WaitEvent(void)
{
	while (DoEvents() == 0) ;
	
	return 0;
}

static struct timeval t0;

LongWord ReadTick()
{
	struct timeval t1;
	long secs, usecs;
	
	if (t0.tv_sec == 0) {
		gettimeofday(&t0, NULL);
		return 0;
	}
	
	gettimeofday(&t1, NULL);
	
	secs  = t1.tv_sec - t0.tv_sec;
	usecs = t1.tv_usec - t0.tv_usec;
	if (usecs < 0) {
		usecs += 1000000;
		secs--;
	}
	
	return secs * 60 + usecs * 60 / 1000000;
}

void WaitTick()
{
	do {
		DoEvents();
	} while (ReadTick() == LastTick);
	LastTick = ReadTick();
}

void WaitTicks(Word Count)
{
	LongWord TickMark;
	
	do {
		DoEvents();
		TickMark = ReadTick();
	} while ((TickMark-LastTick)<=Count);
	LastTick = TickMark;
}

Word WaitTicksEvent(Word Time)
{
	LongWord TickMark;
	LongWord NewMark;
	Word RetVal;
	
	TickMark = ReadTick();
	for (;;) {
		RetVal = DoEvents();
		if (RetVal)
			break;
		
		NewMark = ReadTick();
		if (Time) {
			if ((NewMark-TickMark)>=Time) {
				RetVal = 0;
				break;
			}
		}
	}	
	return RetVal;
}
