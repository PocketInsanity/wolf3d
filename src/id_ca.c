/* id_ca.c */

#include "id_heads.h"

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

typedef struct
{
	/* 0-255 is a character, > is a pointer to a node */
	word bit0, bit1;
} PACKED huffnode;

typedef struct
{
	word RLEWtag;
	long headeroffsets[100];
} PACKED mapfiletype;

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

byte 			*tinf;
int			mapon;

word		*mapsegs[MAPPLANES];
maptype			*mapheaderseg[NUMMAPS];
byte			*audiosegs[NUMSNDCHUNKS];
void			*grsegs[NUMCHUNKS];

byte			grneeded[NUMCHUNKS];
byte		ca_levelbit,ca_levelnum;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

char extension[5],
     gheadname[10]="vgahead.",
     gfilename[10]="vgagraph.",
     gdictname[10]="vgadict.",
     mheadname[10]="maphead.",
     aheadname[10]="audiohed.",
     afilename[10]="audiot.";

long *grstarts;	/* array of offsets in vgagraph, -1 for sparse */
long *audiostarts; /* array of offsets in audio / audiot */

huffnode	grhuffman[255];

int grhandle; /* handle to VGAGRAPH */
int maphandle; /* handle to MAPTEMP / GAMEMAPS */
int audiohandle; /* handle to AUDIOT / AUDIO */

SDMode oldsoundmode;

#define FILEPOSSIZE	3
long GRFILEPOS(int c)
{
	long value;
	int	offset;

	offset = c*3;

	value = *(long *)(((byte *)grstarts)+offset);

	value &= 0x00ffffffl;

	if (value == 0xffffffl)
		value = -1;

	return value;
}

/*
=============================================================================

					   LOW LEVEL ROUTINES

=============================================================================
*/

void CA_CannotOpen(char *string)
{
	/* TODO Ow, string must be a small one else boom */
	char str[30];

	strcpy(str, "Can't open ");
	strcat(str, string);
	strcat(str, "!\n");
	Quit(str);
}

/*
==========================
=
= CA_FarRead
=
= Read from a file to a pointer
=
==========================
*/

boolean CA_FarRead(int handle, byte *dest, long length)
{
	ssize_t l;
	
	l = read(handle, dest, length);
	
	if (l == -1) {
		perror("CA_FarRead");
		return false;
	} else if (l == 0) { 
		fprintf(stderr, "CA_FarRead hit EOF?\n");
		return false;
	} else if (l != length) {
		fprintf(stderr, "CA_FarRead only read %d out of %ld\n", l, length);
		return false;
	}
	return true;
}

/*
==========================
=
= CA_FarWrite
=
= Write from a file to a pointer
=
==========================
*/

boolean CA_FarWrite (int handle, byte *source, long length)
{
	ssize_t l;
	
	l = write(handle, source, length);
	if (l == -1) {
		perror("CA_FarWrite");
		return false;
	} else if (l == 0) {
		fprintf(stderr, "CA_FarWrite hit EOF?\n");
		return false;
	} else if (l != length) {
		fprintf(stderr, "CA_FarWrite only wrote %d out of %ld\n", l, length);
		return false;
	}
	return true;
}

/*
==========================
=
= CA_ReadFile
=
= Reads a file into an allready allocated buffer
=
==========================
*/

boolean CA_ReadFile(char *filename, memptr *ptr)
{
	int handle;
	long size;

	if ((handle = open(filename, O_RDONLY | O_BINARY, S_IREAD)) == -1)
		return false;

	size = filelength(handle);
	if (!CA_FarRead (handle, *ptr, size)) {
		close(handle);
		return false;
	}

	close(handle);
	return true;
}

/*
==========================
=
= CA_WriteFile
=
= Writes a file from a memory buffer
=
==========================
*/

boolean CA_WriteFile(char *filename, void *ptr, long length)
{
	int handle;

	handle = open(filename, O_CREAT | O_BINARY | O_WRONLY, 
			S_IREAD | S_IWRITE | S_IFREG);

	if (handle == -1)
		return false;

	if (!CA_FarWrite(handle, ptr, length)) {
		close(handle);
		return false;
	}
	
	close(handle);
	return true;
}

/*
==========================
=
= CA_LoadFile
=
= Allocate space for and load a file
=
==========================
*/

boolean CA_LoadFile (char *filename, memptr *ptr)
{
	int handle;
	long size;

	if ((handle = open(filename,O_RDONLY | O_BINARY, S_IREAD)) == -1)
		return false;

	size = filelength(handle);
	MM_GetPtr (ptr,size);
	if (!CA_FarRead(handle,*ptr,size))
	{
		close (handle);
		return false;
	}
	close(handle);
	return true;
}

/*
============================================================================

		COMPRESSION routines

============================================================================
*/

/*
======================
=
= CAL_HuffExpand
=
= Length is the length of the EXPANDED data
=
======================
*/

#if 1
/* From Ryan C. Gordon -- ryan_gordon@hotmail.com */
void CAL_HuffExpand(byte *source, byte *dest, long length, huffnode *htable)
{
	huffnode *headptr;          
	huffnode *nodeon;           
	byte      mask = 0x0001;    
	word      path;             
	byte     *endoff = dest + length;    

	nodeon = headptr = htable + 254;  

	do {
		if (*source & mask)
			path = nodeon->bit1;
	        else
			path = nodeon->bit0;
       		mask <<= 1;
	        if (mask == 0x0000) {   
			mask = 0x0001;
			source++;
	        } 
		if (path < 256) {  
			*dest = (byte) path;
			dest++;
			nodeon = headptr;
		} else
			nodeon = (htable + (path - 256));
	} while (dest != endoff);   
} 
#else

void CAL_HuffExpand(byte *source, byte *dest, long length, huffnode *hufftable)
{
        int x;
        huffnode *headptr, *nodeon;
        byte *ptr, *ptrd, *ptrm;
        byte a, mask;
        unsigned short int b;

        ptrd = dest;

        headptr = hufftable + 254; /* head node is always node 254 */

        nodeon = headptr;
        ptr = source;
        a = *ptr;
        ptr++;
        mask = 1;

        for (x = 0; x < length; x++) {
        again:
                if (a & mask)
                        b = nodeon->bit1;
                else
                        b = nodeon->bit0;
                mask <<= 1;
                if (mask == 0) {
                        a = *ptr;
                        ptr++;
                        mask = 1;
                }
                if (b & 0xFF00) {
                        nodeon = hufftable + (b - 256);
                        goto again;
                } else {
                        nodeon = headptr;
                        *ptrd = (b & 0x00FF);
                        ptrd++;
                }
        }
}
#endif

/*
======================
=
= CAL_CarmackExpand
=
= Length is the length of the EXPANDED data
=
======================
*/

#define NEARTAG	0xa7
#define FARTAG	0xa8

void CAL_CarmackExpand(word *source, word *dest, word length)
{
	word ch, chhigh, count, offset;
	word *copyptr, *inptr, *outptr;
	byte **byteinc = (byte **)&inptr;
	
	length /= 2;

	inptr = source;
	outptr = dest;

	while (length) {
		ch = *inptr++;
		chhigh = ch>>8;
		if (chhigh == NEARTAG) {
			count = ch&0xff;
			if (!count) {	
			/* have to insert a word containing the tag byte */
				ch |= **byteinc;
				(*byteinc)++;
				*outptr++ = ch;
				length--;
			} else {
				offset = **byteinc;
				(*byteinc)++;
				copyptr = outptr - offset;
				length -= count;
				while (count--)
					*outptr++ = *copyptr++;
			}
		} else if (chhigh == FARTAG) {
			count = ch&0xff;
			if (!count) {
			/* have to insert a word containing the tag byte */
				ch |= **byteinc;
				(*byteinc)++;
				*outptr++ = ch;
				length --;
			} else {
				offset = *inptr++;
				copyptr = dest + offset;
				length -= count;
				while (count--)
					*outptr++ = *copyptr++;
			}
		} else {
			*outptr++ = ch;
			length--;
		}
	}
}

/*
======================
=
= CA_RLEWcompress
=
======================
*/

/* TODO: actually this isn't used so it can be removed from here */
long CA_RLEWCompress(word *source, long length, word *dest, word rlewtag)
{
	word value, count, i;
	word *start, *end;

	start = dest;

	end = source + (length + 1)/2;

	/* compress it */
	do {
		count = 1;
		value = *source++;
		while ( (*source == value) && (source < end) ) { 
			count++;
			source++;
		}
		if ( (count > 3) || (value == rlewtag) ) {
		 	/* send a tag / count / value string */
			*dest++ = rlewtag;
			*dest++ = count;
			*dest++ = value;
		} else {
			/* send word without compressing */
			for (i = 1; i <= count; i++)
				*dest++ = value;
		}
	} while (source < end);

	return 2*(dest-start);
}

/*
======================
=
= CA_RLEWexpand
= length is EXPANDED length
=
======================
*/

void CA_RLEWexpand(word *source, word *dest, long length, word rlewtag)
{
	word value, count, i;
	word *end = dest + length / 2;
	
	/* expand it */
	do {
		value = *source++;
		
		if (value != rlewtag)
			/* uncompressed */
			*dest++=value;
		else {
			/* compressed string */
			count = *source++;
			value = *source++;
			for (i = 1; i <= count; i++)
				*dest++ = value;
		}
	} while (dest < end);
}

/*
=============================================================================

					 CACHE MANAGER ROUTINES

=============================================================================
*/


/*
============================
=
= CAL_GetGrChunkLength
=
= Gets the length of an explicit length chunk (not tiles)
= The file pointer is positioned so the compressed data can be read in next.
=
============================
*/

long CAL_GetGrChunkLength(int chunk)
{
	long chunkexplen;
	
	lseek(grhandle,GRFILEPOS(chunk),SEEK_SET);
	read(grhandle,&chunkexplen,sizeof(chunkexplen));
	return GRFILEPOS(chunk+1)-GRFILEPOS(chunk)-4;
}

/*
======================
=
= CAL_SetupGrFile
=
======================
*/

void CAL_SetupGrFile (void)
{
	char fname[13];
	int handle;
	memptr compseg;

	long chunkcomplen;
//
// load ???dict.ext (huffman dictionary for graphics files)
//

	strcpy(fname,gdictname);
	strcat(fname,extension);

	if ((handle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);

	read(handle, &grhuffman, sizeof(grhuffman));
	close(handle);
//
// load the data offsets from ???head.ext
//
	MM_GetPtr ((memptr)&grstarts,(NUMCHUNKS+1)*FILEPOSSIZE);

	strcpy(fname,gheadname);
	strcat(fname,extension);

	if ((handle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);

	CA_FarRead(handle, (memptr)grstarts, (NUMCHUNKS+1)*FILEPOSSIZE);

	close(handle);


//
// Open the graphics file, leaving it open until the game is finished
//
	strcpy(fname,gfilename);
	strcat(fname,extension);

	grhandle = open(fname, O_RDONLY | O_BINARY);
	if (grhandle == -1)
		CA_CannotOpen(fname);


//
// load the pic and sprite headers into the arrays in the data segment
//
	MM_GetPtr((memptr)&pictable,NUMPICS*sizeof(pictabletype));
	chunkcomplen = CAL_GetGrChunkLength(STRUCTPIC);
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead(grhandle,compseg,chunkcomplen);
	CAL_HuffExpand(compseg, (byte *)pictable,NUMPICS*sizeof(pictabletype),grhuffman);
	MM_FreePtr(&compseg);
}

//==========================================================================


/*
======================
=
= CAL_SetupMapFile
=
======================
*/

void CAL_SetupMapFile (void)
{
	int	i;
	int handle;
	long length,pos;
	char fname[13];

//
// load maphead.ext (offsets and tileinfo for map file)
//
	strcpy(fname,mheadname);
	strcat(fname,extension);

	if ((handle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);

	length = filelength(handle);
	MM_GetPtr ((memptr)&tinf,length);
	
	CA_FarRead(handle, tinf, length);
	
	close(handle);

//
// open the data file
//
	strcpy(fname,"gamemaps.");
	strcat(fname,extension);

	if ((maphandle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);

//
// load all map header
//
	for (i=0;i<NUMMAPS;i++)
	{
		pos = ((mapfiletype *)tinf)->headeroffsets[i];
		if (pos<0)	/* $FFFFFFFF start is a sparse map */
			continue;

		MM_GetPtr((memptr)&mapheaderseg[i],sizeof(maptype));
		MM_SetLock((memptr)&mapheaderseg[i],true);
		lseek(maphandle,pos,SEEK_SET);
		CA_FarRead (maphandle,(memptr)mapheaderseg[i],sizeof(maptype));
	}

//
// allocate space for 2 64*64 planes
//
	for (i=0;i<MAPPLANES;i++)
	{
		MM_GetPtr ((memptr)&mapsegs[i],64*64*2);
		MM_SetLock ((memptr)&mapsegs[i],true);
	}
}


/* ======================================================================== */

/*
======================
=
= CAL_SetupAudioFile
=
======================
*/

void CAL_SetupAudioFile (void)
{
	int handle;
	long length;
	char fname[13];

/* load maphead.ext (offsets and tileinfo for map file) */

	strcpy(fname,aheadname);
	strcat(fname,extension);

	if ((handle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);

	length = filelength(handle);
	MM_GetPtr((memptr)&audiostarts,length);
	CA_FarRead(handle, (byte *)audiostarts, length);
		
	close(handle);

/* open the data file */

	strcpy(fname,afilename);
	strcat(fname,extension);

	if ((audiohandle = open(fname,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		CA_CannotOpen(fname);
}

/* ======================================================================== */

/*
======================
=
= CA_Startup
=
= Open all files and load in headers
=
======================
*/

void CA_Startup(void)
{
	CAL_SetupMapFile();
	CAL_SetupGrFile();
	CAL_SetupAudioFile();

	mapon = -1;
	ca_levelbit = 1;
	ca_levelnum = 0;
}

//==========================================================================


/*
======================
=
= CA_Shutdown
=
= Closes all files
=
======================
*/

void CA_Shutdown (void)
{
	close (maphandle);
	close (grhandle);
	close (audiohandle);
}

//===========================================================================

/*
======================
=
= CA_CacheAudioChunk
=
======================
*/

void CA_CacheAudioChunk(int chunk)
{
	long pos, length;

	if (audiosegs[chunk])
	{
		MM_SetPurge((memptr)&audiosegs[chunk],0);
		return;	
	}

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = audiostarts[chunk];
	length = audiostarts[chunk+1]-pos;

	lseek(audiohandle, pos, SEEK_SET);

	MM_GetPtr((memptr)&audiosegs[chunk], length);

	CA_FarRead(audiohandle,audiosegs[chunk], length);
}

void CA_UnCacheAudioChunk(int chunk)
{
	/* TODO: For now the warning is ignorable since wl_menu.c does it */
	if (audiosegs[chunk] == 0) {
		fprintf(stderr, "Trying to free null audio chunk %d!\n", chunk);
		return;
	}
	
	MM_FreePtr((memptr *)&audiosegs[chunk]);
	audiosegs[chunk] = 0;
}

//===========================================================================

/*
======================
=
= CA_LoadAllSounds
=
= Purges all sounds, then loads all new ones (mode switch)
=
======================
*/

void CA_LoadAllSounds()
{
	unsigned start, i;

	switch (oldsoundmode)
	{
	case sdm_PC:
		start = STARTPCSOUNDS;
		break;
	case sdm_AdLib:
		start = STARTADLIBSOUNDS;
		break;
	default:
		goto cachein;
	}

	for (i=0;i<NUMSOUNDS;i++,start++)
		if (audiosegs[start])
			MM_SetPurge ((memptr)&audiosegs[start],3);		
			// make purgable

cachein:

	switch (SoundMode)
	{
	case sdm_PC:
		start = STARTPCSOUNDS;
		break;
	case sdm_AdLib:
		start = STARTADLIBSOUNDS;
		break;
	default:
		return;
	}

	for (i=0;i<NUMSOUNDS;i++,start++)
		CA_CacheAudioChunk (start);

	oldsoundmode = SoundMode;
}

//===========================================================================


/*
======================
=
= CAL_ExpandGrChunk
=
= Does whatever is needed with a pointer to a compressed chunk
=
======================
*/

void CAL_ExpandGrChunk(int chunk, byte *source)
{
	int tilecount = 0;
	long expanded;
	
	int width = 0, height = 0;
	
	if (chunk >= STARTTILE8 && chunk < STARTEXTERNS)
	{
	//
	// expanded sizes of tile8/16/32 are implicit
	//
#define BLOCK		64
#define MASKBLOCK	128

		if (chunk<STARTTILE8M) { /* tile 8s are all in one chunk! */
			expanded = BLOCK*NUMTILE8;
			width = 8;
			height = 8;
			tilecount = NUMTILE8;
		} else if (chunk<STARTTILE16) /* TODO: This is removable */
			expanded = MASKBLOCK*NUMTILE8M;
		else if (chunk<STARTTILE16M)	// all other tiles are one/chunk
			expanded = BLOCK*4;
		else if (chunk<STARTTILE32)
			expanded = MASKBLOCK*4;
		else if (chunk<STARTTILE32M)
			expanded = BLOCK*16;
		else
			expanded = MASKBLOCK*16;
		
	} else if (chunk >= STARTPICS && chunk < STARTSPRITES) {
		width = pictable[chunk - STARTPICS].width;
		height = pictable[chunk - STARTPICS].height;
		expanded = *((long *)source);
		source += 4;
	} else {
	//
	// everything else has an explicit size longword
	//
		expanded = *((long *)source);
		source += 4;
	}

//
// allocate final space and decompress it
//
	MM_GetPtr(&grsegs[chunk], expanded);
	CAL_HuffExpand(source, grsegs[chunk], expanded, grhuffman);
	if (width && height) {
		if (tilecount) {
			int i;
			for (i = 0; i < tilecount; i++) 
				VL_DeModeXize(grsegs[chunk]+(width*height)*i, width, height);
		} else			
			VL_DeModeXize(grsegs[chunk], width, height);
	}
}


/*
======================
=
= CA_CacheGrChunk
=
= Makes sure a given chunk is in memory, loadiing it if needed
=
======================
*/

void CA_CacheGrChunk(int chunk)
{
	long	pos,compressed;
	byte	*source;
	int		next;

	/* this is due to Quit wanting to cache the error screen before this has been set up! */
	if ( (grhandle == 0) || (grhandle == -1) ) /* make sure this works ok */
		return;
		
	grneeded[chunk] |= ca_levelbit;	/* make sure it doesn't get removed */
	if (grsegs[chunk])
	{
		MM_SetPurge (&grsegs[chunk], 0);
		return;
	}

//
// load the chunk into a buffer
//
	pos = GRFILEPOS(chunk);
	if (pos < 0) /* $FFFFFFFF start is a sparse tile */
		return;

	next = chunk +1;
	while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
		next++;

	compressed = GRFILEPOS(next)-pos;

	lseek(grhandle,pos,SEEK_SET);

	MM_GetPtr((memptr)&source, compressed);
	CA_FarRead(grhandle, source, compressed);

	CAL_ExpandGrChunk(chunk, source);
	
	MM_FreePtr((memptr)&source);
}

void CA_UnCacheGrChunk(int chunk)
{
	if (grsegs[chunk] == 0) {
		fprintf(stderr, "Trying to free null pointer %d!\n", chunk);
		return;
	}
	
	MM_FreePtr(&grsegs[chunk]);
	grneeded[chunk] &= ~ca_levelbit;
	
	/* Or should MM_FreePtr set it to zero? */
	grsegs[chunk] = 0;
}

//==========================================================================

/*
======================
=
= CA_CacheScreen
=
= Decompresses a chunk from disk straight onto the screen
=
======================
*/

void CA_CacheScreen(int chunk)
{
	long	pos,compressed,expanded;
	memptr	bigbufferseg;
	byte *source, *dest;
	int		next;
	
//
// load the chunk into a buffer
//
	pos = GRFILEPOS(chunk);
	next = chunk +1;
	while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
		next++;
	compressed = GRFILEPOS(next)-pos;

	lseek(grhandle,pos,SEEK_SET);

	MM_GetPtr(&bigbufferseg,compressed);
	MM_SetLock (&bigbufferseg,true);
	
	CA_FarRead(grhandle,bigbufferseg,compressed);
		
	source = bigbufferseg;

	expanded = *(long *)source;
	source += 4;			// skip over length

//
// allocate final space, decompress it, and free bigbuffer
//
	MM_GetPtr((void *)&dest, expanded);
	CAL_HuffExpand(source, dest, expanded, grhuffman);
	VL_DeModeXize(dest, 320, 200);
	VL_MemToScreen(dest, 320, 200, 0, 0);
	MM_FreePtr(&bigbufferseg);
	MM_FreePtr((void *)&dest);
}

//==========================================================================

/*
======================
=
= CA_CacheMap
=
= WOLF: This is specialized for a 64*64 map size
=
======================
*/

void CA_CacheMap(int mapnum)
{
	long	pos,compressed;
	int	plane;
	memptr	*dest,bigbufferseg;
	word	size;
	word	*source;
	memptr	buffer2seg;
	long	expanded;

	mapon = mapnum;

//
// load the planes into the allready allocated buffers
//
	size = 64*64*2;

	for (plane = 0; plane<MAPPLANES; plane++)
	{
		pos = mapheaderseg[mapnum]->planestart[plane];
		compressed = mapheaderseg[mapnum]->planelength[plane];

		dest = (memptr)&mapsegs[plane];

		lseek(maphandle,pos,SEEK_SET);
		MM_GetPtr(&bigbufferseg,compressed);
		MM_SetLock (&bigbufferseg,true);
		source = bigbufferseg;

		CA_FarRead(maphandle,(byte *)source,compressed);
		/*
		 unhuffman, then unRLEW
		 The huffman'd chunk has a two byte expanded length first
		 The resulting RLEW chunk also does, even though it's not really
		 needed
		*/
		expanded = *source;
		source++;
		MM_GetPtr (&buffer2seg,expanded);
		CAL_CarmackExpand (source, (word *)buffer2seg,expanded);
		CA_RLEWexpand (((word *)buffer2seg)+1,*dest,size,
		((mapfiletype *)tinf)->RLEWtag);
		MM_FreePtr (&buffer2seg);

		MM_FreePtr(&bigbufferseg);
	}
}

//===========================================================================

/*
======================
=
= CA_UpLevel
=
= Goes up a bit level in the needed lists and clears it out.
= Everything is made purgable
=
======================
*/

void CA_UpLevel (void)
{
/*
	int	i;

	if (ca_levelnum==7)
		Quit ("CA_UpLevel: Up past level 7!");

	for (i=0;i<NUMCHUNKS;i++)
		if (grsegs[i])
			MM_SetPurge ((memptr)&grsegs[i],3);
	ca_levelbit<<=1;
	ca_levelnum++;
*/
}

//===========================================================================

/*
======================
=
= CA_DownLevel
=
= Goes down a bit level in the needed lists and recaches
= everything from the lower level
=
======================
*/

void CA_DownLevel (void)
{
/*
	if (!ca_levelnum)
		Quit ("CA_DownLevel: Down past level 0!");
	ca_levelbit>>=1;
	ca_levelnum--;
	CA_CacheMarks();
*/
}

//===========================================================================

/*
======================
=
= CA_ClearMarks
=
= Clears out all the marks at the current level
=
======================
*/
#if 0
void CA_ClearMarks (void)
{
	int i;

	for (i=0;i<NUMCHUNKS;i++)
		grneeded[i]&=~ca_levelbit;
}
#endif

//===========================================================================

/*
======================
=
= CA_ClearAllMarks
=
= Clears out all the marks on all the levels
=
======================
*/
#if 0
void CA_ClearAllMarks (void)
{
	memset (grneeded,0,sizeof(grneeded));
	ca_levelbit = 1;
	ca_levelnum = 0;
}
#endif

//===========================================================================

/*
======================
=
= CA_FreeGraphics
=
======================
*/
#if 0
void CA_SetGrPurge (void)
{
	int i;

//
// free graphics
//
	CA_ClearMarks ();

	for (i=0;i<NUMCHUNKS;i++)
		if (grsegs[i])
			MM_SetPurge ((memptr)&grsegs[i],3);
}
#endif
/*
======================
=
= CA_SetAllPurge
=
= Make everything possible purgable
=
======================
*/
#if 0
void CA_SetAllPurge (void)
{
	int i;


//
// free sounds
//
	for (i=0;i<NUMSNDCHUNKS;i++)
		if (audiosegs[i])
			MM_SetPurge ((memptr)&audiosegs[i],3);

//
// free graphics
//
	CA_SetGrPurge ();
}
#endif

//===========================================================================
#if 0
/*
======================
=
= CA_CacheMarks
=
======================
*/
#define MAXEMPTYREAD	1024

void CA_CacheMarks (void)
{
	int 	i,next,numcache;
	long	pos,endpos,nextpos,nextendpos,compressed;
	long	bufferstart,bufferend;	// file position of general buffer
	byte *source;
	memptr	bigbufferseg;

	numcache = 0;
//
// go through and make everything not needed purgable
//
	for (i=0;i<NUMCHUNKS;i++)
		if (grneeded[i]&ca_levelbit)
		{
			if (grsegs[i])					// its allready in memory, make
				MM_SetPurge(&grsegs[i],0);	// sure it stays there!
			else
				numcache++;
		}
		else
		{
			if (grsegs[i])					// not needed, so make it purgeable
				MM_SetPurge(&grsegs[i],3);
		}

	if (!numcache)			// nothing to cache!
		return;


//
// go through and load in anything still needed
//
	bufferstart = bufferend = 0;		// nothing good in buffer now

	for (i=0;i<NUMCHUNKS;i++)
		if ( (grneeded[i]&ca_levelbit) && !grsegs[i])
		{
			pos = GRFILEPOS(i);
			if (pos<0)
				continue;

			next = i +1;
			while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
				next++;

			compressed = GRFILEPOS(next)-pos;
			endpos = pos+compressed;

			if (compressed<=BUFFERSIZE)
			{
				if (bufferstart<=pos
				&& bufferend>= endpos)
				{
				// data is allready in buffer
					source = (byte *)bufferseg+(pos-bufferstart);
				}
				else
				{
				// load buffer with a new block from disk
				// try to get as many of the needed blocks in as possible
					while ( next < NUMCHUNKS )
					{
						while (next < NUMCHUNKS &&
						!(grneeded[next]&ca_levelbit && !grsegs[next]))
							next++;
						if (next == NUMCHUNKS)
							continue;

						nextpos = GRFILEPOS(next);
						while (GRFILEPOS(++next) == -1)	// skip past any sparse tiles
							;
						nextendpos = GRFILEPOS(next);
						if (nextpos - endpos <= MAXEMPTYREAD
						&& nextendpos-pos <= BUFFERSIZE)
							endpos = nextendpos;
						else
							next = NUMCHUNKS;			// read pos to posend
					}

					lseek(grhandle,pos,SEEK_SET);
					CA_FarRead(grhandle,bufferseg,endpos-pos);
					bufferstart = pos;
					bufferend = endpos;
					source = bufferseg;
				}
			}
			else
			{
			// big chunk, allocate temporary buffer
				MM_GetPtr(&bigbufferseg,compressed);
				MM_SetLock (&bigbufferseg,true);
				lseek(grhandle,pos,SEEK_SET);
				CA_FarRead(grhandle,bigbufferseg,compressed);
				source = bigbufferseg;
			}

			CAL_ExpandGrChunk (i,source);

			if (compressed>BUFFERSIZE)
				MM_FreePtr(&bigbufferseg);

		}
}
#endif

/*
===================
=
= MM_Startup
=
===================
*/

void MM_Startup (void)
{
}

/*
====================
=
= MM_Shutdown
=
====================
*/

void MM_Shutdown(void)
{
}

/*
====================
=
= MM_GetPtr
=
====================
*/

void MM_GetPtr(memptr *baseptr, unsigned long size)
{
	/* TODO: add some sort of linked list for purging */
	*baseptr = malloc(size);
}

/*
====================
=
= MM_FreePtr
=
====================
*/

void MM_FreePtr (memptr *baseptr)
{
	/* TODO: add some sort of linked list for purging, etc */
	free(*baseptr);
}

//==========================================================================

/*
=====================
=
= MM_SetPurge
=
= Sets the purge level for a block (locked blocks cannot be made purgable)
=
=====================
*/

void MM_SetPurge (memptr *baseptr, int purge)
{
}

/*
=====================
=
= MM_SetLock
=
= Locks / unlocks the block
=
=====================
*/

void MM_SetLock (memptr *baseptr, boolean locked)
{
}

/*
=====================
=
= MM_SortMem
=
= Throws out all purgable stuff
=
=====================
*/

void MM_SortMem (void)
{
}

	boolean PMStarted;
	char			PageFileName[13] = {"vswap."};
	int				PageFile = -1;
	word			ChunksInFile;
	word			PMSpriteStart,PMSoundStart;

	word PMNumBlocks;
	long PMFrameCount;
	PageListStruct *PMPages, *PMSegPages;

/////////////////////////////////////////////////////////////////////////////
//
//	File management code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PML_ReadFromFile() - Reads some data in from the page file
//
void PML_ReadFromFile(byte *buf, long offset, word length)
{
	if (!buf)
		Quit("PML_ReadFromFile: Null pointer");
	if (!offset)
		Quit("PML_ReadFromFile: Zero offset");
	if (lseek(PageFile,offset,SEEK_SET) != offset)
		Quit("PML_ReadFromFile: Seek failed");
	if (!CA_FarRead(PageFile,buf,length))
		Quit("PML_ReadFromFile: Read failed");
}

//
//	PML_OpenPageFile() - Opens the page file and sets up the page info
//
void PML_OpenPageFile(void)
{
	int				i;
	long			size;
	void			*buf;
	longword		*offsetptr;
	word			*lengthptr;
	PageListStruct *page;

	PageFile = open(PageFileName,O_RDONLY | O_BINARY);
	if (PageFile == -1)
		Quit("PML_OpenPageFile: Unable to open page file");

	// Read in header variables
	read(PageFile,&ChunksInFile,sizeof(ChunksInFile));
	read(PageFile,&PMSpriteStart,sizeof(PMSpriteStart));
	read(PageFile,&PMSoundStart,sizeof(PMSoundStart));

	// Allocate and clear the page list
	PMNumBlocks = ChunksInFile;
	MM_GetPtr((memptr)&PMPages,sizeof(PageListStruct) * PMNumBlocks);
	MM_SetLock((memptr)&PMPages,true);
	memset(PMPages,0,sizeof(PageListStruct) * PMNumBlocks);

	// Read in the chunk offsets
	size = sizeof(longword) * ChunksInFile;
	MM_GetPtr(&buf,size);
	if (!CA_FarRead(PageFile,(byte *)buf,size))
		Quit("PML_OpenPageFile: Offset read failed");
	offsetptr = (longword *)buf;
	for (i = 0,page = PMPages;i < ChunksInFile;i++,page++)
		page->offset = *offsetptr++;
	MM_FreePtr(&buf);

	// Read in the chunk lengths
	size = sizeof(word) * ChunksInFile;
	MM_GetPtr(&buf,size);
	if (!CA_FarRead(PageFile,(byte *)buf,size))
		Quit("PML_OpenPageFile: Length read failed");
	lengthptr = (word *)buf;
	for (i = 0,page = PMPages;i < ChunksInFile;i++,page++)
		page->length = *lengthptr++;
	MM_FreePtr(&buf);
}

//
//  PML_ClosePageFile() - Closes the page file
//
void PML_ClosePageFile(void)
{
	if (PageFile != -1)
		close(PageFile);
		
	if (PMPages) {
		MM_SetLock((memptr)&PMPages,false);
		MM_FreePtr((memptr)&PMPages);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//	Allocation, etc., code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PM_GetPageAddress() - Returns the address of a given page
//		Returns NULL if block is not loaded
//
memptr PM_GetPageAddress(int pagenum)
{
	PageListStruct *page;

	page = &PMPages[pagenum];
	
	return page->addr;
}

//
//	PM_GetPage() - Returns the address of the page, loading it if necessary
//
memptr PM_GetPage(int pagenum)
{
	PageListStruct *page;
	
	if (pagenum >= ChunksInFile)
		Quit("PM_GetPage: Invalid page request");

	page = &PMPages[pagenum];
	if (page->addr == NULL) {
		page->lastHit = 0;
		MM_GetPtr((memptr)&page->addr, PMPageSize);
		PML_ReadFromFile(page->addr, page->offset, page->length);
	}
	page->lastHit++;
	return page->addr;
}

//
//	PM_SetPageLock() - Sets the lock type on a given page
//		pml_Unlocked: Normal, page can be purged
//		pml_Locked: Cannot be purged
//
void PM_SetPageLock(int pagenum,PMLockType lock)
{
	if (pagenum < PMSoundStart)
		Quit("PM_SetPageLock: Locking/unlocking non-sound page");

	PMPages[pagenum].locked = lock;
}

//
//	PM_Preload() - Loads as many pages as possible into all types of memory.
//		Calls the update function after each load, indicating the current
//		page, and the total pages that need to be loaded (for thermometer).
//
void PM_Preload(boolean (*update)(word current,word total))
{
	update(1, 1);
}

/////////////////////////////////////////////////////////////////////////////
//
//	General code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PM_NextFrame() - Increments the frame counter
//
void PM_NextFrame(void)
{
	int	i;

	// Frame count overrun - kill the LRU hit entries & reset frame count
	if (++PMFrameCount >= MAXLONG - 4)
	{
		for (i = 0;i < PMNumBlocks;i++)
			PMPages[i].lastHit = 0;
		PMFrameCount = 0;
	}

}

//
//	PM_Reset() - Sets up caching structures
//
void PM_Reset(void)
{
	int i;
	PageListStruct *page;

	// Initialize page list
	for (i = 0,page = PMPages;i < PMNumBlocks;i++,page++)
	{
		page->addr = NULL;
		page->locked = false;
	}
}

//
//	PM_Startup() - Start up the Page Mgr
//
void PM_Startup(void)
{
	if (PMStarted)
		return;

	PML_OpenPageFile();

	PM_Reset();

	PMStarted = true;
}

//
//	PM_Shutdown() - Shut down the Page Mgr
//
void PM_Shutdown(void)
{
	if (!PMStarted)
		return;

	PML_ClosePageFile();
}
