#include "wl_def.h"

typedef struct
{
	/* 0-255 is a character, > is a pointer to a node */
	int bit0, bit1;
} huffnode;

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

int RLEWtag;
int mapon;

word	*mapsegs[MAPPLANES];
maptype	*mapheaderseg[NUMMAPS];
byte	*audiosegs[NUMSNDCHUNKS];
byte	*grsegs[NUMCHUNKS];

char extension[5];
#define gheadname "vgahead."
#define gfilename "vgagraph."
#define gdictname "vgadict."
#define mheadname "maphead."
#define gmapsname "gamemaps."
#define aheadname "audiohed."
#define afilename "audiot."
#define pfilename "vswap."

static long *grstarts;	/* array of offsets in vgagraph */
static long *audiostarts; /* array of offsets in audiot */

static huffnode grhuffman[256];

static int grhandle = -1;	/* handle to VGAGRAPH */
static int maphandle = -1;	/* handle to GAMEMAPS */
static int audiohandle = -1;	/* handle to AUDIOT */

/*
=============================================================================

					   LOW LEVEL ROUTINES

=============================================================================
*/

static void CA_CannotOpen(char *string)
{
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

boolean CA_FarWrite(int handle, byte *source, long length)
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

boolean CA_LoadFile(char *filename, memptr *ptr)
{
	int handle;
	long size;

	if ((handle = open(filename, O_RDONLY | O_BINARY)) == -1)
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
= Length is the length of the EXPANDED data
=
======================
*/

/* From Ryan C. Gordon -- ryan_gordon@hotmail.com */
void CAL_HuffExpand(byte *source, byte *dest, long length, huffnode *htable)
{
	huffnode *headptr;          
	huffnode *nodeon;           
	byte      mask = 0x01;    
	word      path;             
	byte     *endoff = dest + length;    

	nodeon = headptr = htable + 254;  

	do {
		if (*source & mask)
			path = nodeon->bit1;
	        else
			path = nodeon->bit0;
       		mask <<= 1;
	        if (mask == 0x00) {   
			mask = 0x01;
			source++;
	        } 
		if (path < 256) {  
			*dest = (byte)path;
			dest++;
			nodeon = headptr;
		} else
			nodeon = (htable + (path - 256));
	} while (dest != endoff);   
} 

/*
======================
=
= CAL_CarmackExpand
= Length is the length of the EXPANDED data
=
======================
*/

#define NEARTAG	0xa7
#define FARTAG	0xa8

void CAL_CarmackExpand(word *source, word *dest, word length)
{
	unsigned int offset;
	word *copyptr, *outptr;	
	byte chhigh, chlow, *inptr;
	
	length /= 2;

	inptr = (byte *)source;
	outptr = dest;

	while (length) {		
		chlow = *inptr++; /* count */
		chhigh = *inptr++;
		
		if (chhigh == NEARTAG) {
			if (!chlow) {	
				/* have to insert a word containing the tag byte */
				*outptr++ = (chhigh << 8) | *inptr;
				inptr++;
				
				length--;
			} else {
				offset = *inptr;
				inptr++;
				
				copyptr = outptr - offset;
				
				length -= chlow;
				while (chlow--)
					*outptr++ = *copyptr++;
			}
		} else if (chhigh == FARTAG) {
			if (!chlow) {
				/* have to insert a word containing the tag byte */
				*outptr++ = (chhigh << 8) | *inptr;
				inptr++;
				
				length--;
			} else {
				offset = *inptr | (*(inptr+1) << 8);
				inptr += 2;
				
				copyptr = dest + offset;
				length -= chlow;
				while (chlow--)
					*outptr++ = *copyptr++;
			}
		} else {
			*outptr++ = (chhigh << 8) | chlow;
			length--;
		}
	}
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
======================
=
= CAL_SetupGrFile
=
======================
*/

static void CAL_SetupGrFile()
{
	char fname[13];
	int handle;
	memptr compseg;
	long chunkcomplen;
	byte *grtemp;
	int i;

//
// load vgadict.ext (huffman dictionary for graphics files)
//

	strcpy(fname, gdictname);
	strcat(fname, extension);

	handle = OpenRead(fname);
	if (handle == -1)
		CA_CannotOpen(fname);

	for (i = 0; i < 256; i++) {
		grhuffman[i].bit0 = ReadInt16(handle);
		grhuffman[i].bit1 = ReadInt16(handle);
	}
	
	CloseRead(handle);
	
//
// load the data offsets from vgahead.ext
//
	MM_GetPtr((memptr)&grstarts, (NUMCHUNKS+1)*4);
	MM_GetPtr((memptr)&grtemp, (NUMCHUNKS+1)*3);
	
	strcpy(fname, gheadname);
	strcat(fname, extension);

	handle = OpenRead(fname);
	if (handle == -1)
		CA_CannotOpen(fname);
	
	ReadBytes(handle, grtemp, (NUMCHUNKS+1)*3);

	for (i = 0; i < NUMCHUNKS+1; i++)
		grstarts[i] = (grtemp[i*3+0]<<0)|(grtemp[i*3+1]<<8)|(grtemp[i*3+2]<<16);

	MM_FreePtr((memptr)&grtemp);
	
	CloseRead(handle);
	
//
// Open the graphics file, leaving it open until the game is finished
//
	strcpy(fname, gfilename);
	strcat(fname, extension);

	grhandle = OpenRead(fname);
	if (grhandle == -1)
		CA_CannotOpen(fname);

//
// load the pic headers into pictable
//
	MM_GetPtr((memptr)&pictable, NUMPICS*sizeof(pictabletype));
	chunkcomplen = grstarts[STRUCTPIC+1] - grstarts[STRUCTPIC];
	ReadSeek(grhandle, grstarts[STRUCTPIC], SEEK_SET);

	MM_GetPtr(&compseg, chunkcomplen);

	ReadBytes(grhandle, compseg, chunkcomplen);
	
	CAL_HuffExpand((byte *)compseg+4, (byte *)pictable, NUMPICS*sizeof(pictabletype), grhuffman);
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

static void CAL_SetupMapFile()
{
	int i;
	int handle;
	long pos;
	char fname[13];
	
	strcpy(fname, mheadname);
	strcat(fname, extension);

	handle = OpenRead(fname);
	if (handle == -1)
		CA_CannotOpen(fname);

	RLEWtag = ReadInt16(handle);

/* open the data file */
	strcpy(fname, gmapsname);
	strcat(fname, extension);

	maphandle = OpenRead(fname);
	if (maphandle == -1)
		CA_CannotOpen(fname);

//
// load all map header
//
	for (i = 0; i < NUMMAPS; i++)
	{
		pos = ReadInt32(handle);
		if (pos == 0) {
			mapheaderseg[i] = NULL;
			continue;
		}
			
		MM_GetPtr((memptr)&mapheaderseg[i], sizeof(maptype));
		MM_SetLock((memptr)&mapheaderseg[i], true);

		ReadSeek(maphandle, pos, SEEK_SET);
		
		mapheaderseg[i]->planestart[0] = ReadInt32(maphandle);
		mapheaderseg[i]->planestart[1] = ReadInt32(maphandle);
		mapheaderseg[i]->planestart[2] = ReadInt32(maphandle);
		
		mapheaderseg[i]->planelength[0] = ReadInt16(maphandle);
		mapheaderseg[i]->planelength[1] = ReadInt16(maphandle);
		mapheaderseg[i]->planelength[2] = ReadInt16(maphandle);
		mapheaderseg[i]->width = ReadInt16(maphandle);
		mapheaderseg[i]->height = ReadInt16(maphandle);
		ReadBytes(maphandle, (byte *)mapheaderseg[i]->name, 16);
		
	}

	CloseRead(handle);
//
// allocate space for 2 64*64 planes
//
	for (i = 0;i < MAPPLANES; i++) {
		MM_GetPtr((memptr)&mapsegs[i], 64*64*2);
		MM_SetLock((memptr)&mapsegs[i], true);
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

static void CAL_SetupAudioFile()
{
	int handle;
	long length;
	char fname[13];
	int i;
	
	strcpy(fname, aheadname);
	strcat(fname, extension);

	handle = OpenRead(fname);
	if (handle == -1)
		CA_CannotOpen(fname);
	
	length = ReadLength(handle);
	
	MM_GetPtr((memptr)&audiostarts, length);
	
	for (i = 0; i < (length/4); i++)
		audiostarts[i] = ReadInt32(handle);

	CloseRead(handle);	

/* open the data file */

	strcpy(fname, afilename);
	strcat(fname, extension);

	audiohandle = OpenRead(fname);
	if (audiohandle == -1)
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

void CA_Startup()
{
	CAL_SetupMapFile();
	CAL_SetupGrFile();
	CAL_SetupAudioFile();

	mapon = -1;
}

/*
======================
=
= CA_Shutdown
=
= Closes all files
=
======================
*/

void CA_Shutdown()
{
	CloseRead(maphandle);
	CloseRead(grhandle);
	CloseRead(audiohandle);
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

	if (audiosegs[chunk]) {
		return;	
	}

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = audiostarts[chunk];
	length = audiostarts[chunk+1]-pos;

	ReadSeek(audiohandle, pos, SEEK_SET);

	MM_GetPtr((memptr)&audiosegs[chunk], length);

	ReadBytes(audiohandle, audiosegs[chunk], length);
}

void CA_UnCacheAudioChunk(int chunk)
{
	if (audiosegs[chunk] == 0) {
		fprintf(stderr, "Trying to free null audio chunk %d!\n", chunk);
		return;
	}
	
	MM_FreePtr((memptr *)&audiosegs[chunk]);
	audiosegs[chunk] = 0;
}

/*
======================
=
= CA_LoadAllSounds
=
======================
*/

void CA_LoadAllSounds()
{
	int start, i;

	for (start = STARTADLIBSOUNDS, i = 0; i < NUMSOUNDS; i++, start++)
		CA_CacheAudioChunk(start);
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

static void CAL_ExpandGrChunk(int chunk, byte *source)
{
	int tilecount = 0, i;
	long expanded;
	
	int width = 0, height = 0;
	
	if (chunk >= STARTTILE8 && chunk < STARTEXTERNS)
	{
	//
	// expanded sizes of tile8 are implicit
	//
		expanded = (8*8)*NUMTILE8;
		width = 8;
		height = 8;
		tilecount = NUMTILE8;
	} else if (chunk >= STARTPICS && chunk < STARTTILE8) {
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
	MM_GetPtr((void *)&grsegs[chunk], expanded);
	CAL_HuffExpand(source, grsegs[chunk], expanded, grhuffman);
	if (width && height) {
		if (tilecount) {
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
	long pos, compressed;
	byte *source;

	/* this is due to Quit() wanting to cache the error screen before this has been set up! */
	if (grhandle == -1)
		return;
		
	if (grsegs[chunk]) {
		return;
	}

//
// load the chunk into a buffer
//
	pos = grstarts[chunk];

	compressed = grstarts[chunk+1]-pos;

	ReadSeek(grhandle, pos, SEEK_SET);

	MM_GetPtr((memptr)&source, compressed);
	ReadBytes(grhandle, source, compressed);

	CAL_ExpandGrChunk(chunk, source);
	
	MM_FreePtr((memptr)&source);
}

void CA_UnCacheGrChunk(int chunk)
{
	if (grsegs[chunk] == 0) {
		fprintf(stderr, "Trying to free null pointer %d!\n", chunk);
		return;
	}
	
	MM_FreePtr((memptr)&grsegs[chunk]);
	
	grsegs[chunk] = 0;
}

/* ======================================================================== */

/*
======================
=
= CA_CacheMap
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

	for (plane = 0; plane < MAPPLANES; plane++)
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
		MM_GetPtr(&buffer2seg, expanded);
		CAL_CarmackExpand(source, (word *)buffer2seg,expanded);
		CA_RLEWexpand(((word *)buffer2seg)+1,*dest,size, RLEWtag);
		MM_FreePtr(&buffer2seg);

		MM_FreePtr(&bigbufferseg);
	}
}

/* ======================================================================== */

void CA_UpLevel()
{
}

void CA_DownLevel()
{
}

void MM_Startup()
{
}

void MM_Shutdown()
{
}

void MM_GetPtr(memptr *baseptr, unsigned long size)
{
	/* add some sort of linked list for purging */
	*baseptr = malloc(size);
}

void MM_FreePtr(memptr *baseptr)
{
	/* add some sort of linked list for purging, etc */
	free(*baseptr);
}

void MM_SetPurge(memptr *baseptr, int purge)
{
}

void MM_SetLock(memptr *baseptr, boolean locked)
{
}

void MM_SortMem()
{
}

static boolean PMStarted;

static int PageFile = -1;
int ChunksInFile, PMSpriteStart, PMSoundStart;

PageListStruct *PMPages;

//
//	PML_ReadFromFile() - Reads some data in from the page file
//
static void PML_ReadFromFile(byte *buf, long offset, word length)
{
	if (!buf)
		Quit("PML_ReadFromFile: Null pointer");
	if (!offset)
		Quit("PML_ReadFromFile: Zero offset");
	if (ReadSeek(PageFile, offset, SEEK_SET) != offset)
		Quit("PML_ReadFromFile: Seek failed");
	if (ReadBytes(PageFile, buf, length) != length)
		Quit("PML_ReadFromFile: Read failed");
}

//
//	PML_OpenPageFile() - Opens the page file and sets up the page info
//
static void PML_OpenPageFile()
{
	int i;
	PageListStruct *page;
	char fname[13];
	
	strcpy(fname, pfilename);
	strcat(fname, extension);
	
	PageFile = OpenRead(fname);
	if (PageFile == -1)
		Quit("PML_OpenPageFile: Unable to open page file");

	// Read in header variables
	ChunksInFile = ReadInt16(PageFile);
	PMSpriteStart = ReadInt16(PageFile);
	PMSoundStart = ReadInt16(PageFile);

	// Allocate and clear the page list
	MM_GetPtr((memptr)&PMPages, sizeof(PageListStruct) * ChunksInFile);
	MM_SetLock((memptr)&PMPages, true);
	
	memset(PMPages, 0, sizeof(PageListStruct) * ChunksInFile);

	// Read in the chunk offsets
	for (i = 0, page = PMPages; i < ChunksInFile; i++, page++)
		page->offset = ReadInt32(PageFile);
		
	// Read in the chunk lengths
	for (i = 0, page = PMPages; i < ChunksInFile; i++, page++)
		page->length = ReadInt16(PageFile);
}

//
//  PML_ClosePageFile() - Closes the page file
//
static void PML_ClosePageFile()
{
	if (PageFile != -1)
		CloseRead(PageFile);
		
	if (PMPages) {
		MM_SetLock((memptr)&PMPages,false);
		MM_FreePtr((memptr)&PMPages);
	}
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
		MM_GetPtr((memptr)&page->addr, PMPageSize);
		PML_ReadFromFile(page->addr, page->offset, page->length);
	}
	return page->addr;
}

//
//	PM_Preload() - Loads as many pages as possible into all types of memory.
//		Calls the update function after each load, indicating the current
//		page, and the total pages that need to be loaded (for thermometer).
//
void PM_Preload(boolean (*update)(word current, word total))
{
	int i;
	
	for (i = 0; i < 50; i++)
		update(i, 50); /* yay */
	update(50, 50);
}

//
//	PM_Startup() - Start up the Page Mgr
//
void PM_Startup()
{
	if (PMStarted)
		return;

	PML_OpenPageFile();

	PMStarted = true;
}

//
//	PM_Shutdown() - Shut down the Page Mgr
//
void PM_Shutdown()
{
	if (!PMStarted)
		return;

	PML_ClosePageFile();
}
