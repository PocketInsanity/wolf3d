/*
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

#include "wolfdef.h"

typedef struct ResItem_t
{
	long type;
	Word item;
	
	Byte *dat;
	int size;
	
	Byte *buf;
	
	struct ResItem_t *next;
} ResItem;

ResItem *lr;

int InitResources(char *name)
{	
	FILE *fp;
	ResItem **cur = &lr, *t;
	int resfork, resmap, forklen, maplen;
	int typelist, namelist, typecount;
	int datalen;
	int i;
		
	fp = fopen(name, "rb");
	
	if (fp == NULL) 
		return -1;
	
	resfork = (fgetc(fp) << 24) | (fgetc(fp) << 16) | (fgetc(fp) << 8) | (fgetc(fp) << 0);
	resmap  = (fgetc(fp) << 24) | (fgetc(fp) << 16) | (fgetc(fp) << 8) | (fgetc(fp) << 0);
	forklen = (fgetc(fp) << 24) | (fgetc(fp) << 16) | (fgetc(fp) << 8) | (fgetc(fp) << 0);
	maplen  = (fgetc(fp) << 24) | (fgetc(fp) << 16) | (fgetc(fp) << 8) | (fgetc(fp) << 0);
	
	fseek(fp, resmap, SEEK_SET);
	fseek(fp, 24, SEEK_CUR); /* Skip over reserved */
	
	typelist = (fgetc(fp) << 8) | (fgetc(fp) << 0);
	namelist = (fgetc(fp) << 8) | (fgetc(fp) << 0);
	typecount = ((fgetc(fp) << 8) | (fgetc(fp) << 0)) + 1;
	
	for (i = 0; i < typecount; i++) {
		int type;
		int off, bak, bak2, data, count, x;
		
		type = (fgetc(fp) << 24) | (fgetc(fp) << 16) | (fgetc(fp) << 8) | (fgetc(fp) << 0);
		
		count = ((fgetc(fp) << 8) | fgetc(fp)) + 1;
		off = (fgetc(fp) << 8) | fgetc(fp);
		
		bak = ftell(fp);
		
		fseek(fp, resmap + typelist + off, SEEK_SET);
		for (x = 0; x < count; x++) {
			int id;
			
			id = (fgetc(fp) << 8) | fgetc(fp);
			off = (fgetc(fp) << 8) | fgetc(fp);
			fgetc(fp);
			data = (fgetc(fp) << 16) | (fgetc(fp) << 8) | (fgetc(fp) << 0);
			
			fgetc(fp); fgetc(fp); fgetc(fp); fgetc(fp);
			
			bak2 = ftell(fp);
			fseek(fp, resfork + data, SEEK_SET);
			datalen = (fgetc(fp) << 24) | (fgetc(fp) << 16) | (fgetc(fp) << 8) | (fgetc(fp) << 0);
			
			t = (ResItem *)malloc(sizeof(ResItem));
			
			t->next = NULL;
			
			t->type = type;
			t->item = id;
			
			t->dat = (Byte *)malloc(datalen);
			fread(t->dat, datalen, 1, fp);
			t->size = datalen;
			t->buf = NULL;
			
			*cur = t;
			
			cur = &((*cur)->next);
					
			fseek(fp, bak2, SEEK_SET);
		}
		
		fseek(fp, bak, SEEK_SET);		
	}
			
	fclose(fp);
	
	return 0;
}

void *LoadAResource2(Word RezNum, LongWord Type)
{
	ResItem *c = lr;
	
	while (c != NULL) {
		if ( (c->type == Type) && (c->item == RezNum) ) {
			if (c->buf == NULL) {
				c->buf = malloc(c->size);
				memcpy(c->buf, c->dat, c->size);
			} else {
				/* DEBUG: we want a fresh copy... */
				printf("DEBUG: Item %ld/%d already loaded!\n", Type, RezNum);
				free(c->buf);
				c->buf = malloc(c->size);
				memcpy(c->buf, c->dat, c->size);
			}
			
			if (c->buf == NULL) 
				Quit("MALLOC FAILED?");
				
			return c->buf;
		}
		c = c->next;		
	}
	
	fprintf(stderr, "ERROR (LoadAResource2): %ld/%d was not found!\n", Type, RezNum);
	exit(EXIT_FAILURE);
}

void *FindResource(Word RezNum, LongWord Type)
{
	ResItem *c = lr;
	
	while (c != NULL) {
		if ( (c->type == Type) && (c->item == RezNum) ) {
			if (c->buf == NULL) {
				c->buf = malloc(c->size);
				memcpy(c->buf, c->dat, c->size);
			} else {
				/* DEBUG: we want a fresh copy... */
				printf("DEBUG: Item %ld/%d already loaded!\n", Type, RezNum);
				free(c->buf);
				c->buf = malloc(c->size);
				memcpy(c->buf, c->dat, c->size);
			}
			
			if (c->buf == NULL) 
				Quit("MALLOC FAILED?");
				
			return c->buf;
		}
		c = c->next;		
	}

	return NULL;	
}

void ReleaseAResource2(Word RezNum, LongWord Type)
{
	ResItem *c = lr;
	
	while (c != NULL) {
		if ( (c->type == Type) && (c->item == RezNum) ) {
			if (c->buf)
				free(c->buf);
			c->buf = NULL;
			return;
		}
		c = c->next;
	}
	fprintf(stderr, "ERROR (ReleaseAResource2): %ld/%d was not found!\n", Type, RezNum);
	exit(EXIT_FAILURE);
}

void KillAResource2(Word RezNum, LongWord Type)
{
	ReleaseAResource2(RezNum, Type);
}

void FreeResources()
{
	ResItem *c = lr;
	
	while (c) {
		ResItem *t = c;
		
		if (c->dat)
			free(c->dat);
		
		if (c->buf)
			free(c->buf);
			
		c = c->next;
		
		free(t);		
	}
	
	lr = NULL;
}
