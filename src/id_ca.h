// ID_CA.H
//===========================================================================

#define NUMMAPS		60
#define MAPPLANES	2

#define UNCACHEGRCHUNK(chunk)	{MM_FreePtr(&grsegs[chunk]);grneeded[chunk]&=~ca_levelbit;}

//===========================================================================

typedef	struct
{
	long		planestart[3];
	unsigned	planelength[3];
	unsigned	width,height;
	char		name[16];
} maptype;

//===========================================================================

extern	char		audioname[13];

extern	byte 			*tinf;
extern	int			mapon;

extern	unsigned		*mapsegs[MAPPLANES];
extern	maptype			*mapheaderseg[NUMMAPS];
extern	byte			*audiosegs[NUMSNDCHUNKS];
extern	void			*grsegs[NUMCHUNKS];

extern	byte		grneeded[NUMCHUNKS];
extern	byte		ca_levelbit,ca_levelnum;

extern	char		*titleptr[8];

extern	int			profilehandle,debughandle;

extern	char		extension[5],
			gheadname[10],
			gfilename[10],
			gdictname[10],
			mheadname[10],
			mfilename[10],
			aheadname[10],
			afilename[10];

extern long		*grstarts;	// array of offsets in egagraph, -1 for sparse
extern long		*audiostarts;	// array of offsets in audio / audiot

//===========================================================================

boolean CA_FarRead(int handle, byte *dest, long length);
boolean CA_FarWrite(int handle, byte *source, long length);
boolean CA_ReadFile(char *filename, memptr *ptr);
boolean CA_LoadFile(char *filename, memptr *ptr);
boolean CA_WriteFile(char *filename, void *ptr, long length);

long CA_RLEWCompress (unsigned *source, long length, unsigned *dest,
  unsigned rlewtag);

void CA_RLEWexpand (unsigned *source, unsigned *dest,long length,
  unsigned rlewtag);

void CA_Startup (void);
void CA_Shutdown (void);

void CA_SetGrPurge (void);
void CA_CacheAudioChunk (int chunk);
void CA_LoadAllSounds (void);

void CA_UpLevel (void);
void CA_DownLevel (void);

void CA_SetAllPurge (void);

void CA_ClearMarks (void);
void CA_ClearAllMarks (void);

#define CA_MarkGrChunk(chunk)	grneeded[chunk]|=ca_levelbit

void CA_CacheGrChunk (int chunk);
void CA_CacheMap (int mapnum);

void CA_CacheMarks (void);

void CA_CacheScreen (int chunk);

#define SAVENEARHEAP	0x400		// space to leave in data segment
#define SAVEFARHEAP		0			// space to leave in far heap

#define	BUFFERSIZE		0x1000		// miscelanious, allways available buffer

#define MAXBLOCKS		700


//--------

#define	EMS_INT			0x67

#define	EMS_STATUS		0x40
#define	EMS_GETFRAME	0x41
#define	EMS_GETPAGES	0x42
#define	EMS_ALLOCPAGES	0x43
#define	EMS_MAPPAGE		0x44
#define	EMS_FREEPAGES	0x45
#define	EMS_VERSION		0x46

//--------

#define	XMS_INT			0x2f
#define	XMS_CALL(v)		_AH = (v);\
						asm call [DWORD PTR XMSDriver]

#define	XMS_VERSION		0x00

#define	XMS_ALLOCHMA	0x01
#define	XMS_FREEHMA		0x02

#define	XMS_GENABLEA20	0x03
#define	XMS_GDISABLEA20	0x04
#define	XMS_LENABLEA20	0x05
#define	XMS_LDISABLEA20	0x06
#define	XMS_QUERYA20	0x07

#define	XMS_QUERYFREE	0x08
#define	XMS_ALLOC		0x09
#define	XMS_FREE		0x0A
#define	XMS_MOVE		0x0B
#define	XMS_LOCK		0x0C
#define	XMS_UNLOCK		0x0D
#define	XMS_GETINFO		0x0E
#define	XMS_RESIZE		0x0F

#define	XMS_ALLOCUMB	0x10
#define	XMS_FREEUMB		0x11

//==========================================================================

typedef struct
{
	long	nearheap,farheap,EMSmem,XMSmem,mainmem;
} mminfotype;

//==========================================================================

extern	mminfotype	mminfo;
extern	memptr		bufferseg;
extern	boolean		mmerror;

extern	void		(* beforesort) (void);
extern	void		(* aftersort) (void);

//==========================================================================

void MM_Startup (void);
void MM_Shutdown (void);
void MM_MapEMS (void);

void MM_GetPtr (memptr *baseptr,unsigned long size);
void MM_FreePtr (memptr *baseptr);

void MM_SetPurge (memptr *baseptr, int purge);
void MM_SetLock (memptr *baseptr, boolean locked);
void MM_SortMem (void);

void MM_ShowMemory (void);

long MM_UnusedMemory (void);
long MM_TotalFree (void);

void MM_BombOnError (boolean bomb);

void MML_UseSpace (unsigned segstart, unsigned seglength);

//
//	ID_PM.H
//	Header file for Id Engine's Page Manager
//

//	NOTE! PMPageSize must be an even divisor of EMSPageSize, and >= 1024
#define	EMSPageSize		16384
#define	EMSPageSizeSeg	(EMSPageSize >> 4)
#define	EMSPageSizeKB	(EMSPageSize >> 10)
#define	EMSFrameCount	4
#define	PMPageSize		4096
#define	PMPageSizeSeg	(PMPageSize >> 4)
#define	PMPageSizeKB	(PMPageSize >> 10)
#define	PMEMSSubPage	(EMSPageSize / PMPageSize)

#define	PMMinMainMem	10			// Min acceptable # of pages from main
#define	PMMaxMainMem	100			// Max number of pages in main memory

#define	PMThrashThreshold	1	// Number of page thrashes before panic mode
#define	PMUnThrashThreshold	5	// Number of non-thrashing frames before leaving panic mode

typedef	enum
		{
			pml_Unlocked,
			pml_Locked
		} PMLockType;

typedef	enum
		{
			pmba_Unused = 0,
			pmba_Used = 1,
			pmba_Allocated = 2
		} PMBlockAttr;

typedef	struct
		{
			longword	offset;		// Offset of chunk into file
			word		length;		// Length of the chunk

			int			xmsPage;	// If in XMS, (xmsPage * PMPageSize) gives offset into XMS handle

			PMLockType	locked;		// If set, this page can't be purged
			int			emsPage;	// If in EMS, logical page/offset into page
			int			mainPage;	// If in Main, index into handle array

			longword	lastHit;	// Last frame number of hit
		} PageListStruct;

typedef	struct
		{
			int			baseEMSPage;	// Base EMS page for this phys frame
			longword	lastHit;		// Last frame number of hit
		} EMSListStruct;

extern	boolean			XMSPresent,EMSPresent;
extern	word			XMSPagesAvail,EMSPagesAvail;

extern	word			ChunksInFile,
						PMSpriteStart,PMSoundStart;
extern	PageListStruct *PMPages;

#define	PM_GetSoundPage(v)	PM_GetPage(PMSoundStart + (v))
#define	PM_GetSpritePage(v)	PM_GetPage(PMSpriteStart + (v))

#define	PM_LockMainMem()	PM_SetMainMemPurge(0)
#define	PM_UnlockMainMem()	PM_SetMainMemPurge(3)


extern	char	PageFileName[13];


extern	void	PM_Startup(void),
				PM_Shutdown(void),
				PM_Reset(void),
				PM_Preload(boolean (*update)(word current,word total)),
				PM_NextFrame(void),
				PM_SetPageLock(int pagenum,PMLockType lock),
				PM_SetMainPurge(int level),
				PM_CheckMainMem(void);
extern	memptr	PM_GetPageAddress(int pagenum),
				PM_GetPage(int pagenum);		// Use this one to cache page

void PM_SetMainMemPurge(int level);
