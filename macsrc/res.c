#include <stdio.h>
#include <stdlib.h>

#include "wolfdef.h"

typedef struct ResItem_t
{
	long type;
	Word item;
	
	Byte *dat;
	
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
		int off, bak, bak2, data, count, x, c, z;
		int d1, d2, d3, d4;
		
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
		if ( (c->type == Type) && (c->item == RezNum) ) 
			return c->dat;
		c = c->next;		
	}
	
	fprintf(stderr, "ERROR: %d/%d was not found!\n", Type, RezNum);
	exit(EXIT_FAILURE);
}

void ReleaseAResource2(Word RezNum, LongWord Type)
{
	ResItem *c = lr;
	
	while (c != NULL) {
		c = c->next;
	}
}

void KillAResource2(Word RezNum, LongWord Type)
{
	ResItem *c = lr;
	
	while (c != NULL) {
		c = c->next;
	}
}

void FreeResources()
{
	ResItem *c = lr;
	
	while (c) {
		ResItem *t = c;
		
		if (c->dat)
			free(c->dat);
		
		c = c->next;
		
		free(t);		
	}
	
	lr = NULL;
}
