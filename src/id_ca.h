#ifndef __ID_CA_H__
#define __ID_CA_H__

//===========================================================================

#define NUMMAPS		60
#define MAPPLANES	2

#define UNCACHEGRCHUNK CA_UnCacheGrChunk

//===========================================================================

typedef	struct
{
	long planestart[3];
	word planelength[3];
	word width, height;
	char name[16];
} PACKED maptype;

//===========================================================================

extern	byte 			*tinf;
extern	int			mapon;

extern	word *mapsegs[MAPPLANES];
extern	maptype			*mapheaderseg[NUMMAPS];
extern	byte			*audiosegs[NUMSNDCHUNKS];
extern	void			*grsegs[NUMCHUNKS];

extern	byte		grneeded[NUMCHUNKS];
extern	byte		ca_levelbit,ca_levelnum;

extern	char		extension[5],
			gheadname[10],
			gfilename[10],
			gdictname[10],
			mheadname[10],
			aheadname[10],
			afilename[10];

extern long		*grstarts;	// array of offsets in vgagraph, -1 for sparse
extern long		*audiostarts;	// array of offsets in audio / audiot

//===========================================================================

boolean CA_FarRead(int handle, byte *dest, long length);
boolean CA_FarWrite(int handle, byte *source, long length);
boolean CA_ReadFile(char *filename, memptr *ptr);
boolean CA_LoadFile(char *filename, memptr *ptr);
boolean CA_WriteFile(char *filename, void *ptr, long length);

long CA_RLEWCompress(word *source, long length, word *dest, word rlewtag);
void CA_RLEWexpand(word *source, word *dest, long length, word rlewtag);

void CA_Startup (void);
void CA_Shutdown (void);

void CA_SetGrPurge (void);
void CA_CacheAudioChunk(int chunk);
void CA_UnCacheAudioChunk(int chunk);
void CA_LoadAllSounds (void);

void CA_CacheMap (int mapnum);
void CA_CacheGrChunk(int chunk);
void CA_UnCacheGrChunk(int chunk);

void CA_UpLevel (void);
void CA_DownLevel (void);
/*
void CA_SetAllPurge (void);

void CA_ClearMarks (void);
void CA_ClearAllMarks (void);

#define CA_MarkGrChunk(chunk)	grneeded[chunk]|=ca_levelbit

void CA_CacheMarks (void);
*/

void CA_CacheScreen (int chunk);

//==========================================================================

void MM_Startup (void);
void MM_Shutdown (void);

void MM_GetPtr (memptr *baseptr, unsigned long size);
void MM_FreePtr (memptr *baseptr);

void MM_SetPurge (memptr *baseptr, int purge);
void MM_SetLock (memptr *baseptr, boolean locked);
void MM_SortMem (void);

#define PMPageSize              4096

typedef	enum
		{
			pml_Unlocked,
			pml_Locked
		} PMLockType;

typedef	struct {
	longword offset;		// Offset of chunk into file
	word length;		// Length of the chunk
	PMLockType locked;	// If set, this page cannot be purged
	memptr addr;
	longword lastHit;	// Last frame number of hit
} PageListStruct;

extern	word			ChunksInFile,
						PMSpriteStart,PMSoundStart;
extern	PageListStruct *PMPages;

#define	PM_GetSoundPage(v)	PM_GetPage(PMSoundStart + (v))
#define	PM_GetSpritePage(v)	PM_GetPage(PMSpriteStart + (v))

extern	char	PageFileName[13];


void	PM_Startup(void),
				PM_Shutdown(void),
				PM_Reset(void),
				PM_Preload(boolean (*update)(word current,word total)),
				PM_NextFrame(void),
				PM_SetPageLock(int pagenum,PMLockType lock),
				PM_SetMainPurge(int level),
				PM_CheckMainMem(void);
memptr	PM_GetPageAddress(int pagenum), PM_GetPage(int pagenum);

#else 
#error "fix me TODO"
#endif
