#include "wl_def.h"

/*
=============================================================================

						   WOLFENSTEIN 3-D

					  An Id Software production

						   by John Carmack

=============================================================================
*/

#define FOCALLENGTH     (0x5700l)               // in global coordinates
#define VIEWGLOBAL      0x10000                 // globals visable flush to wall

char str[80], str2[20];

fixed focallength;

int viewwidth, viewheight;
int viewwidthwin, viewheightwin; /* for borders */
int xoffset, yoffset;
int vwidth, vheight; /* size of screen */
int viewsize;

int centerx;
int shootdelta;                     // pixels away from centerx a target can be
fixed scale;
long heightnumerator;

boolean startgame,loadedgame;
int mouseadjustment;

long frameon;
long lasttimecount;
fixed viewsin, viewcos;
fixed viewx, viewy;                    // the focal point
int pixelangle[MAXVIEWWIDTH];
long finetangent[FINEANGLES/4];
int horizwall[MAXWALLTILES], vertwall[MAXWALLTILES];

char configname[13] = "config.";

fixed sintable[ANGLES+ANGLES/4+1], *costable = sintable+(ANGLES/4);

int _argc;
char **_argv;

/*
========================
=
= FixedByFrac (FixedMul)
=
= multiply two 16/16 bit, 2's complement fixed point numbers
=
========================
*/

fixed FixedByFrac(fixed a, fixed b)
{
	int64_t ra = a;
	int64_t rb = b;
	int64_t r;
	
	r = ra * rb;
	r >>= 16;
	return (fixed)r;
}

/*
=====================
=
= CalcTics
=
=====================
*/

void CalcTics()
{
	int newtime;
	int ticcount;
	
	if (demoplayback || demorecord)
		ticcount = DEMOTICS - 1; /* [70/4] 17.5 Hz */
	else
		ticcount = 0 + 1; /* 35 Hz */
	
	do {
		newtime = get_TimeCount();
		tics = newtime - lasttimecount;
	} while (tics <= ticcount);
	
	lasttimecount = newtime;
	
	if (tics > MAXTICS) {
		tics = MAXTICS;
	}
}

/* ======================================================================== */

#if 0
/*
====================
=
= ReadConfig
=
====================
*/

void ReadConfig()
{
	SDMode sd;
	SMMode sm;
	SDSMode sds;

	int file;
	
	if ((file = open(configname, O_BINARY | O_RDONLY)) != -1)
	{
	//
	// valid config file
	//
		read(file,Scores,sizeof(HighScore) * MaxScores);

		read(file,&sd,sizeof(sd));
		read(file,&sm,sizeof(sm));
		read(file,&sds,sizeof(sds));

		read(file,&mouseenabled,sizeof(mouseenabled));
		read(file,&joystickenabled,sizeof(joystickenabled));
		read(file,&joypadenabled,sizeof(joypadenabled));
		read(file,&joystickport,sizeof(joystickport));

		read(file,&dirscan,sizeof(dirscan));
		read(file,&buttonscan,sizeof(buttonscan));
		read(file,&buttonmouse,sizeof(buttonmouse));
		read(file,&buttonjoy,sizeof(buttonjoy));

		read(file,&viewsize,sizeof(viewsize));
		read(file,&mouseadjustment,sizeof(mouseadjustment));

		close(file);

		MainMenu[6].active=1;
		MainItems.curpos=0;
	}	
	else
	{
	//
	// no config file, so select by hardware
	//
		viewsize = 15;
	}
	
	mouseenabled = false;

	joystickenabled = false;
	joypadenabled = false;
	joystickport = 0;

	mouseadjustment = 5;

	SD_SetMusicMode(smm_AdLib);
	SD_SetSoundMode(sdm_AdLib);
	SD_SetDigiDevice(sds_SoundBlaster);
}


/*
====================
=
= WriteConfig
=
====================
*/

void WriteConfig()
{
	int file;

	file = open(configname, O_CREAT | O_BINARY | O_WRONLY,
				S_IREAD | S_IWRITE | S_IFREG);

	if (file != -1)
	{
		write(file,Scores,sizeof(HighScore) * MaxScores);

		write(file,&SoundMode,sizeof(SoundMode));
		write(file,&MusicMode,sizeof(MusicMode));
		write(file,&DigiMode,sizeof(DigiMode));

		write(file,&mouseenabled,sizeof(mouseenabled));
		write(file,&joystickenabled,sizeof(joystickenabled));
		write(file,&joypadenabled,sizeof(joypadenabled));
		write(file,&joystickport,sizeof(joystickport));

		write(file,&dirscan,sizeof(dirscan));
		write(file,&buttonscan,sizeof(buttonscan));
		write(file,&buttonmouse,sizeof(buttonmouse));
		write(file,&buttonjoy,sizeof(buttonjoy));

		write(file,&viewsize,sizeof(viewsize));
		write(file,&mouseadjustment,sizeof(mouseadjustment));

		close(file);
	}
}

#endif

void DiskFlopAnim(int x, int y)
{
	static char which = 0;
	
	if (!x && !y)
		return;
	
	VWB_DrawPic(x, y, C_DISKLOADING1PIC+which);
	VW_UpdateScreen();
	
	which ^= 1;
}

long DoChecksum(byte *source, unsigned size, long checksum)
{
	int i;

	for (i = 0; i < size-1; i++)
		checksum += source[i]^source[i+1];

	return checksum;
}

int WriteConfig()
{
	int fd;
	
	fd = OpenWrite(configname);
	
	if (fd != -1) {
		CloseWrite(fd);
	}
	
	return 0;
}

int ReadConfig()
{
	int fd;
	
	fd = OpenRead(configname);
	
	if (fd != -1) {
		CloseRead(fd);

#ifdef UPLOAD		
		MainMenu[readthis].active = 1;
		MainItems.curpos = 0;
#endif
	} else {
		viewsize = 15;
	}
	
	viewsize = 15;
	
	mouseenabled = false;

	joystickenabled = false;
	joypadenabled = false;
	joystickport = 0;

	mouseadjustment = 5;

	SD_SetMusicMode(smm_AdLib);
	SD_SetSoundMode(sdm_AdLib);
	SD_SetDigiDevice(sds_SoundBlaster);
	
	return 0;
}

int SaveGame()
{
	return 0;
}

int LoadGame()
{
	return 0;
}

/*
==================
=
= SaveTheGame
=
==================
*/

boolean SaveTheGame(int file, int x, int y)
{
	long checksum;
	objtype *ob,nullobj;

	checksum = 0;

	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void *)&gamestate,sizeof(gamestate));
	checksum = DoChecksum((byte *)&gamestate,sizeof(gamestate),checksum);

	DiskFlopAnim(x,y);
#ifdef SPEAR
	CA_FarWrite (file,(void *)&LevelRatios[0],sizeof(LRstruct)*20);
	checksum = DoChecksum((byte *)&LevelRatios[0],sizeof(LRstruct)*20,checksum);
#else
	CA_FarWrite (file,(void *)&LevelRatios[0],sizeof(LRstruct)*8);
	checksum = DoChecksum((byte *)&LevelRatios[0],sizeof(LRstruct)*8,checksum);
#endif

	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void *)tilemap,sizeof(tilemap));
	checksum = DoChecksum((byte *)tilemap,sizeof(tilemap),checksum);
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void *)actorat,sizeof(actorat));
	checksum = DoChecksum((byte *)actorat,sizeof(actorat),checksum);

	CA_FarWrite (file,(void *)areaconnect,sizeof(areaconnect));
	CA_FarWrite (file,(void *)areabyplayer,sizeof(areabyplayer));

	for (ob = player; ob; ob=ob->next) {
		DiskFlopAnim(x,y);
		CA_FarWrite(file, (void *)ob, sizeof(*ob));
	}
	
	nullobj.active = ac_badobject;          // end of file marker
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void *)&nullobj,sizeof(nullobj));


	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void *)&laststatobj,sizeof(laststatobj));
	checksum = DoChecksum((byte *)&laststatobj,sizeof(laststatobj),checksum);
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void *)statobjlist,sizeof(statobjlist));
	checksum = DoChecksum((byte *)statobjlist,sizeof(statobjlist),checksum);

	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void *)doorposition,sizeof(doorposition));
	checksum = DoChecksum((byte *)doorposition,sizeof(doorposition),checksum);
	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void *)doorobjlist,sizeof(doorobjlist));
	checksum = DoChecksum((byte *)doorobjlist,sizeof(doorobjlist),checksum);

	DiskFlopAnim(x,y);
	CA_FarWrite (file,(void *)&pwallstate,sizeof(pwallstate));
	checksum = DoChecksum((byte *)&pwallstate,sizeof(pwallstate),checksum);
	CA_FarWrite (file,(void *)&pwallx,sizeof(pwallx));
	checksum = DoChecksum((byte *)&pwallx,sizeof(pwallx),checksum);
	CA_FarWrite (file,(void *)&pwally,sizeof(pwally));
	checksum = DoChecksum((byte *)&pwally,sizeof(pwally),checksum);
	CA_FarWrite (file,(void *)&pwalldir,sizeof(pwalldir));
	checksum = DoChecksum((byte *)&pwalldir,sizeof(pwalldir),checksum);
	CA_FarWrite (file,(void *)&pwallpos,sizeof(pwallpos));
	checksum = DoChecksum((byte *)&pwallpos,sizeof(pwallpos),checksum);

	//
	// WRITE OUT CHECKSUM
	//
	CA_FarWrite(file, (void *)&checksum, sizeof(checksum));

	return true;
}

/*
==================
=
= LoadTheGame
=
==================
*/

boolean LoadTheGame(int file,int x,int y)
{
	long checksum,oldchecksum;
	objtype nullobj;

	checksum = 0;

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void *)&gamestate,sizeof(gamestate));
	checksum = DoChecksum((byte *)&gamestate,sizeof(gamestate),checksum);

	DiskFlopAnim(x,y);
#ifdef SPEAR
	CA_FarRead (file,(void *)&LevelRatios[0],sizeof(LRstruct)*20);
	checksum = DoChecksum((byte *)&LevelRatios[0],sizeof(LRstruct)*20,checksum);
#else
	CA_FarRead (file,(void *)&LevelRatios[0],sizeof(LRstruct)*8);
	checksum = DoChecksum((byte *)&LevelRatios[0],sizeof(LRstruct)*8,checksum);
#endif

	DiskFlopAnim(x,y);
	SetupGameLevel ();

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void *)tilemap,sizeof(tilemap));
	checksum = DoChecksum((byte *)tilemap,sizeof(tilemap),checksum);
	DiskFlopAnim(x,y);
	CA_FarRead(file,(void *)actorat,sizeof(actorat));
	checksum = DoChecksum((byte *)actorat,sizeof(actorat),checksum);

	CA_FarRead(file,(void *)areaconnect,sizeof(areaconnect));
	CA_FarRead(file,(void *)areabyplayer,sizeof(areabyplayer));


	InitActorList();
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void *)player,sizeof(*player));

	while (1)
	{
		DiskFlopAnim(x,y);
		CA_FarRead(file,(void *)&nullobj,sizeof(nullobj));
		
		if (nullobj.active == ac_badobject)
			break;
		
		GetNewActor();
	 // don't copy over the links
		memcpy(new, &nullobj, sizeof(nullobj)-4);
	}


	DiskFlopAnim(x,y);
	CA_FarRead (file,(void *)&laststatobj,sizeof(laststatobj));
	checksum = DoChecksum((byte *)&laststatobj,sizeof(laststatobj),checksum);
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void *)statobjlist,sizeof(statobjlist));
	checksum = DoChecksum((byte *)statobjlist,sizeof(statobjlist),checksum);

	DiskFlopAnim(x,y);
	CA_FarRead (file,(void *)doorposition,sizeof(doorposition));
	checksum = DoChecksum((byte *)doorposition,sizeof(doorposition),checksum);
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void *)doorobjlist,sizeof(doorobjlist));
	checksum = DoChecksum((byte *)doorobjlist,sizeof(doorobjlist),checksum);

	DiskFlopAnim(x,y);
	CA_FarRead(file,(void *)&pwallstate,sizeof(pwallstate));
	checksum = DoChecksum((byte *)&pwallstate,sizeof(pwallstate),checksum);
	CA_FarRead(file,(void *)&pwallx,sizeof(pwallx));
	checksum = DoChecksum((byte *)&pwallx,sizeof(pwallx),checksum);
	CA_FarRead(file,(void *)&pwally,sizeof(pwally));
	checksum = DoChecksum((byte *)&pwally,sizeof(pwally),checksum);
	CA_FarRead(file,(void *)&pwalldir,sizeof(pwalldir));
	checksum = DoChecksum((byte *)&pwalldir,sizeof(pwalldir),checksum);
	CA_FarRead(file,(void *)&pwallpos,sizeof(pwallpos));
	checksum = DoChecksum((byte *)&pwallpos,sizeof(pwallpos),checksum);

	CA_FarRead(file, (void *)&oldchecksum, sizeof(oldchecksum));

	if (oldchecksum != checksum)
	{
		Message(STR_SAVECHT1"\n"
			STR_SAVECHT2"\n"
			STR_SAVECHT3"\n"
			STR_SAVECHT4);
			
		IN_ClearKeysDown();
		IN_Ack();

		gamestate.score = 0;
		gamestate.lives = 1;
		gamestate.weapon =
			gamestate.chosenweapon =
			gamestate.bestweapon = wp_pistol;
		gamestate.ammo = 8;
	}

	return true;
}

//===========================================================================

/*
=================
=
= MS_CheckParm
=
=================
*/

int MS_CheckParm(char *check)
{
	int i;
	char *parm;

	for (i = 1; i < _argc; i++) {
		parm = _argv[i];

		while (!isalpha(*parm))       // skip - / \ etc.. in front of parm
			if (!*parm++)
				break;          // hit end of string without an alphanum

		if (!stricmp(check, parm))
			return i;
	}
	return 0;
}

//===========================================================================

/*
=====================
=
= InitDigiMap
=
=====================
*/

static int wolfdigimap[] =
{
	// These first sounds are in the upload version
#ifndef SPEAR
	HALTSND,                0,
	DOGBARKSND,             1,
	CLOSEDOORSND,           2,
	OPENDOORSND,            3,
	ATKMACHINEGUNSND,       4,
	ATKPISTOLSND,           5,
	ATKGATLINGSND,          6,
	SCHUTZADSND,            7,
	GUTENTAGSND,            8,
	MUTTISND,               9,
	BOSSFIRESND,            10,
	SSFIRESND,              11,
	DEATHSCREAM1SND,        12,
	DEATHSCREAM2SND,        13,
	DEATHSCREAM3SND,        13,
	TAKEDAMAGESND,          14,
	PUSHWALLSND,            15,
	LEBENSND,               20,
	NAZIFIRESND,            21,
	SLURPIESND,             22,
	YEAHSND,		32,
#ifndef UPLOAD
	// These are in all other episodes
	DOGDEATHSND,            16,
	AHHHGSND,               17,
	DIESND,                 18,
	EVASND,                 19,
	TOT_HUNDSND,            23,
	MEINGOTTSND,            24,
	SCHABBSHASND,           25,
	HITLERHASND,            26,
	SPIONSND,               27,
	NEINSOVASSND,           28,
	DOGATTACKSND,           29,
	LEVELDONESND,           30,
	MECHSTEPSND,		31,

	SCHEISTSND,		33,
	DEATHSCREAM4SND,	34,		// AIIEEE
	DEATHSCREAM5SND,	35,		// DEE-DEE
	DONNERSND,		36,		// EPISODE 4 BOSS DIE
	EINESND,		37,		// EPISODE 4 BOSS SIGHTING
	ERLAUBENSND,		38,		// EPISODE 6 BOSS SIGHTING
	DEATHSCREAM6SND,	39,		// FART
	DEATHSCREAM7SND,	40,		// GASP
	DEATHSCREAM8SND,	41,		// GUH-BOY!
	DEATHSCREAM9SND,	42,		// AH GEEZ!
	KEINSND,		43,		// EPISODE 5 BOSS SIGHTING
	MEINSND,		44,		// EPISODE 6 BOSS DIE
	ROSESND,		45,		// EPISODE 5 BOSS DIE
#endif
#else
//
// SPEAR OF DESTINY DIGISOUNDS
//
	HALTSND,                0,
	CLOSEDOORSND,           2,
	OPENDOORSND,            3,
	ATKMACHINEGUNSND,       4,
	ATKPISTOLSND,           5,
	ATKGATLINGSND,          6,
	SCHUTZADSND,            7,
	BOSSFIRESND,            8,
	SSFIRESND,              9,
	DEATHSCREAM1SND,        10,
	DEATHSCREAM2SND,        11,
	TAKEDAMAGESND,          12,
	PUSHWALLSND,            13,
	AHHHGSND,               15,
	LEBENSND,               16,
	NAZIFIRESND,            17,
	SLURPIESND,             18,
	LEVELDONESND,           22,
	DEATHSCREAM4SND,	23,		// AIIEEE
	DEATHSCREAM3SND,        23,		// DOUBLY-MAPPED!!!
	DEATHSCREAM5SND,	24,		// DEE-DEE
	DEATHSCREAM6SND,	25,		// FART
	DEATHSCREAM7SND,	26,		// GASP
	DEATHSCREAM8SND,	27,		// GUH-BOY!
	DEATHSCREAM9SND,	28,		// AH GEEZ!
	GETGATLINGSND,		38,		// Got Gat replacement
#ifndef SPEARDEMO
	DOGBARKSND,             1,
	DOGDEATHSND,            14,
	SPIONSND,               19,
	NEINSOVASSND,           20,
	DOGATTACKSND,           21,
	TRANSSIGHTSND,		29,		// Trans Sight
	TRANSDEATHSND,		30,		// Trans Death
	WILHELMSIGHTSND,	31,		// Wilhelm Sight
	WILHELMDEATHSND,	32,		// Wilhelm Death
	UBERDEATHSND,		33,		// Uber Death
	KNIGHTSIGHTSND,		34,		// Death Knight Sight
	KNIGHTDEATHSND,		35,		// Death Knight Death
	ANGELSIGHTSND,		36,		// Angel Sight
	ANGELDEATHSND,		37,		// Angel Death
	GETSPEARSND,		39,		// Got Spear replacement
#endif
#endif
	LASTSOUND
};


void InitDigiMap()
{
	int *map;

	for (map = wolfdigimap; *map != LASTSOUND; map += 2)
		DigiMap[map[0]] = map[1];
}

//===========================================================================

/*
==================
=
= BuildTables
=
= Calculates:
=
= scale                 projection constant
= sintable/costable     overlapping fractional tables
=
==================
*/

static const float radtoint = (float)FINEANGLES/2.0f/PI;

void BuildTables()
{
  int           i;
  float         angle,anglestep;
  double        tang;
  fixed         value;


//
// calculate fine tangents
//

	for (i=0;i<FINEANGLES/8;i++)
	{
		tang = tan((i+0.5)/radtoint);
		finetangent[i] = tang*TILEGLOBAL;
		finetangent[FINEANGLES/4-1-i] = 1/tang*TILEGLOBAL;
	}

//
// costable overlays sintable with a quarter phase shift
// ANGLES is assumed to be divisable by four
//

  angle = 0;
  anglestep = PI/2/ANGLEQUAD;
  for (i=0;i<=ANGLEQUAD;i++)
  {
	value=GLOBAL1*sin(angle);
	sintable[i]=
	  sintable[i+ANGLES]=
	  sintable[ANGLES/2-i] = value;
	sintable[ANGLES-i]=
	  sintable[ANGLES/2+i] = -value;
	angle += anglestep;
  }

}

/*
====================
=
= CalcProjection
=
====================
*/

void CalcProjection(long focal)
{
	int     i;
	long    intang;
	float   angle;
	double  tang;
	int     halfview;
	double  facedist;

	focallength = focal;
	facedist = focal+MINDIST;
	halfview = viewwidth/2;               // half view in pixels

//
// calculate scale value for vertical height calculations
// and sprite x calculations
//
	scale = halfview*facedist/(VIEWGLOBAL/2);

//
// divide heightnumerator by a posts distance to get the posts height for
// the heightbuffer.  The pixel height is height>>2
//
	heightnumerator = (TILEGLOBAL*scale)>>6;

//
// calculate the angle offset from view angle of each pixel's ray
//

	for (i=0;i<halfview;i++)
	{
	// start 1/2 pixel over, so viewangle bisects two middle pixels
		tang = (long)i*VIEWGLOBAL/viewwidth/facedist;
		angle = atan(tang);
		intang = angle*radtoint;
		pixelangle[halfview-1-i] = intang;
		pixelangle[halfview+i] = -intang;
	}
}

/*
===================
=
= SetupWalls
=
= Map tile values to scaled pics
=
===================
*/

void SetupWalls()
{
	int i;

	for (i=1;i<MAXWALLTILES;i++)
	{
		horizwall[i]=(i-1)*2;
		vertwall[i]=(i-1)*2+1;
	}
}

void ShowViewSize(int width)
{
	int oldwidth,oldheight;

	oldwidth = viewwidthwin;
	oldheight = viewheightwin;

	viewwidthwin = width*16;
	viewheightwin = width*16*HEIGHTRATIO;
	DrawPlayBorder();

	viewheightwin = oldheight;
	viewwidthwin = oldwidth;
}

void NewViewSize(int width)
{
	if (width > 20)
		width = 20;
	if (width < 4)
		width = 4;	
	
	width *= vwidth / 320;
	
	if ((width*16) > vwidth)
		width = vwidth / 16;
	
	if ((width*16*HEIGHTRATIO) > (vheight - 40*vheight/200))
		width = (vheight - 40*vheight/200)/8;
	
	viewwidthwin = width*16*320/vwidth;
	viewheightwin = width*16*HEIGHTRATIO*320/vwidth;
	viewsize = width*320/vwidth;
	
	viewwidth = width*16;
	viewheight = width*16*HEIGHTRATIO;
	
	centerx = viewwidth/2-1;
	shootdelta = viewwidth/10;
	
	yoffset = (vheight-STATUSLINES*vheight/200-viewheight)/2;
	xoffset = (vwidth-viewwidth)/2;
	
//
// calculate trace angles and projection constants
//
	CalcProjection(FOCALLENGTH);

}

//===========================================================================

#ifndef SPEARDEMO

#ifndef SPEAR
CP_iteminfo MusicItems={CTL_X,CTL_Y,6,0,32};
CP_itemtype MusicMenu[]=
{
	{1,"Get Them!",0},
	{1,"Searching",0},
	{1,"P.O.W.",0},
	{1,"Suspense",0},
	{1,"War March",0},
	{1,"Around The Corner!",0},

	{1,"Nazi Anthem",0},
	{1,"Lurking...",0},
	{1,"Going After Hitler",0},
	{1,"Pounding Headache",0},
	{1,"Into the Dungeons",0},
	{1,"Ultimate Conquest",0},

	{1,"Kill the S.O.B.",0},
	{1,"The Nazi Rap",0},
	{1,"Twelfth Hour",0},
	{1,"Zero Hour",0},
	{1,"Ultimate Conquest",0},
	{1,"Wolfpack",0}
};
#else
CP_iteminfo MusicItems={CTL_X,CTL_Y-20,9,0,32};
CP_itemtype MusicMenu[]=
{
	{1,"Funky Colonel Bill",0},
	{1,"Death To The Nazis",0},
	{1,"Tiptoeing Around",0},
	{1,"Is This THE END?",0},
	{1,"Evil Incarnate",0},
	{1,"Jazzin' Them Nazis",0},
	{1,"Puttin' It To The Enemy",0},
	{1,"The SS Gonna Get You",0},
	{1,"Towering Above",0}
};
#endif

static int songs[]=
{
#ifndef SPEAR
	GETTHEM_MUS,
	SEARCHN_MUS,
	POW_MUS,
	SUSPENSE_MUS,
	WARMARCH_MUS,
	CORNER_MUS,

	NAZI_OMI_MUS,
	PREGNANT_MUS,
	GOINGAFT_MUS,
	HEADACHE_MUS,
	DUNGEON_MUS,
	ULTIMATE_MUS,

	INTROCW3_MUS,
	NAZI_RAP_MUS,
	TWELFTH_MUS,
	ZEROHOUR_MUS,
	ULTIMATE_MUS,
	PACMAN_MUS
#else
	XFUNKIE_MUS,             // 0
	XDEATH_MUS,              // 2
	XTIPTOE_MUS,             // 4
	XTHEEND_MUS,             // 7
	XEVIL_MUS,               // 17
	XJAZNAZI_MUS,            // 18
	XPUTIT_MUS,              // 21
	XGETYOU_MUS,             // 22
	XTOWER2_MUS              // 23
#endif
};
		
void DoJukebox()
{
	int which,lastsong=-1;
	unsigned start;

	IN_ClearKeysDown();
//	if (!AdLibPresent && !SoundBlasterPresent)
//		return;
	
	MenuFadeOut();

#if !defined(SPEAR) || !defined(UPLOAD)
	start = (US_RndT() % 3) * 6;
#else
	start = 0;
#endif

	CA_CacheGrChunk(STARTFONT+1);
#ifdef SPEAR
	CacheLump(BACKDROP_LUMP_START, BACKDROP_LUMP_END);
#else
	CacheLump(CONTROLS_LUMP_START, CONTROLS_LUMP_END);
#endif
	CA_LoadAllSounds();

	fontnumber=1;
	ClearMScreen();
	VWB_DrawPic(112,184,C_MOUSELBACKPIC);
	DrawStripes(10);
	SETFONTCOLOR (TEXTCOLOR,BKGDCOLOR);

#ifndef SPEAR
	DrawWindow (CTL_X-2,CTL_Y-6,280,13*7,BKGDCOLOR);
#else
	DrawWindow (CTL_X-2,CTL_Y-26,280,13*10,BKGDCOLOR);
#endif

	DrawMenu (&MusicItems,&MusicMenu[start]);

	SETFONTCOLOR (READHCOLOR,BKGDCOLOR);
	PrintY = 15;
	WindowX = 0;
	WindowY = 320;
	US_CPrint("Robert's Jukebox");

	SETFONTCOLOR (TEXTCOLOR,BKGDCOLOR);
	VW_UpdateScreen();
	MenuFadeIn();

	do
	{
		which = HandleMenu(&MusicItems,&MusicMenu[start],NULL);
		if (which>=0)
		{
			if (lastsong >= 0)
				MusicMenu[start+lastsong].active = 1;

			StartCPMusic(songs[start + which]);
			MusicMenu[start+which].active = 2;
			DrawMenu (&MusicItems,&MusicMenu[start]);
			VW_UpdateScreen();
			lastsong = which;
		}
	} while(which>=0);

	MenuFadeOut();
	IN_ClearKeysDown();
#ifdef SPEAR
	UnCacheLump(BACKDROP_LUMP_START, BACKDROP_LUMP_END);
#else
	UnCacheLump(CONTROLS_LUMP_START, CONTROLS_LUMP_END);
#endif
}
#endif

/* ======================================================================== */

/*
==========================
=
= SignonScreen
=
==========================
*/

void SignonScreen()
{
	VL_SetPalette(gamepal);
	VL_MemToScreen(introscn, 320, 200, 0, 0);
	VW_UpdateScreen();
}


/*
==========================
=
= FinishSignon
=
==========================
*/

void FinishSignon()
{
#ifndef SPEAR
	VW_Bar(0, 189, 300, 11, introscn[0]);
	WindowX = 0;
	WindowW = 320;
	PrintY = 190;

	SETFONTCOLOR(14,4);

	US_CPrint("Press a key");
	VW_UpdateScreen();
	
	if (!NoWait)
		IN_Ack ();

	VW_Bar(0, 189, 300, 11, introscn[0]);

	PrintY = 190;
	SETFONTCOLOR(10,4);

	US_CPrint("Working...");
	VW_UpdateScreen();
	
	SETFONTCOLOR(0,15);
#else
	if (!NoWait)
		VW_WaitVBL(3*70);
#endif
}

/* ======================================================================== */

/*
==========================
=
= ShutdownId
=
= Shuts down all ID_?? managers
=
==========================
*/

void ShutdownId()
{
	US_Shutdown();
	SD_Shutdown();
	IN_Shutdown();
	VW_Shutdown();
	CA_Shutdown();
	PM_Shutdown();
	MM_Shutdown();
}

/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/

void NewGame(int difficulty, int episode)
{
	memset(&gamestate, 0, sizeof(gamestate));
	
	gamestate.difficulty = difficulty;
	gamestate.weapon = gamestate.bestweapon
		= gamestate.chosenweapon = wp_pistol;
	gamestate.health = 100;
	gamestate.ammo = STARTAMMO;
	gamestate.lives = 3;
	gamestate.nextextra = EXTRAPOINTS;
	gamestate.episode = episode;

	startgame = true;
}

/*
==========================
=
= InitGame
=
= Load a few things right away
=
==========================
*/

void InitGame()
{
	int i;

	MM_Startup(); 
	PM_Startup();
	CA_Startup();
	VW_Startup();
	IN_Startup();
	SD_Startup();
	US_Startup();
	
//	SignonScreen();
	
//
// build some tables
//
	InitDigiMap();

	for (i = 0;i < MAPSIZE; i++)
	{
		farmapylookup[i] = i*64;
	}

	ReadConfig();

//
// load in and lock down some basic chunks
//

	CA_CacheGrChunk(STARTFONT);

	LoadLatchMem();
	BuildTables();
	SetupWalls();

	NewViewSize(viewsize);


//
// initialize variables
//
	InitRedShifts();

	IN_CheckAck();
//
// HOLDING DOWN 'M' KEY?
//
#ifndef SPEARDEMO
	if (IN_KeyDown(sc_M))
		DoJukebox();
#endif

//	FinishSignon();
}

/*
=====================
=
= DemoLoop
=
=====================
*/

void DemoLoop()
{
	static int LastDemo;
	
	int i;
//
// main game cycle
//

	LastDemo = 0;
	
	StartCPMusic(INTROSONG);

	if (!NoWait)
		PG13();

	i = MS_CheckParm("playdemo");
	if (i && ((i+1) < _argc)) {
		i++;
		for (; i < _argc; i++) {
			if (_argv[i][0] == '-')
				break;
			IN_ClearKeysDown();
			if (PlayDemoFromFile(_argv[i]))
				IN_UserInput(3 * 70);
		}
		VW_FadeOut();
	}
	
	while (1)
	{
		while (!NoWait)
		{
//
// title page
//
			MM_SortMem ();
#ifdef SPEAR
			CA_CacheGrChunk (TITLEPALETTE);

			CA_CacheGrChunk (TITLE1PIC);
			VWB_DrawPic (0,0,TITLE1PIC);
			CA_UnCacheGrChunk (TITLE1PIC);

			CA_CacheGrChunk (TITLE2PIC);
			VWB_DrawPic (0,80,TITLE2PIC);
			CA_UnCacheGrChunk(TITLE2PIC);
			VW_UpdateScreen();
			VL_FadeIn(0,255,grsegs[TITLEPALETTE],30);

			CA_UnCacheGrChunk (TITLEPALETTE);
#else
			VL_CacheScreen(TITLEPIC);
			VW_UpdateScreen ();
			VW_FadeIn();
#endif
			if (IN_UserInput(TickBase*15))
				break;
			VW_FadeOut();
//
// credits page
//
			VL_CacheScreen(CREDITSPIC);
			VW_UpdateScreen();
			VW_FadeIn ();
			if (IN_UserInput(TickBase*10))
				break;
			VW_FadeOut ();
//
// high scores
//
			DrawHighScores();
			VW_UpdateScreen();
			VW_FadeIn();

			if (IN_UserInput(TickBase*10))
				break;
//
// demo
//
			#ifndef SPEARDEMO
			PlayDemo(LastDemo++%4);
			#else
			PlayDemo(0);
			#endif

			if (playstate == ex_abort)
				break;
			StartCPMusic(INTROSONG);
		}

		VW_FadeOut();

		if (IN_KeyDown(sc_Tab) && MS_CheckParm("debugmode"))
			RecordDemo ();
		else
			US_ControlPanel(0);

		if (startgame || loadedgame)
		{
			GameLoop();
			VW_FadeOut();
			StartCPMusic(INTROSONG);
		}
	}
}


//===========================================================================


/*
==========================
=
= WolfMain
=
==========================
*/

int WolfMain(int argc, char *argv[])
{
	_argc = argc;
	_argv = argv;

	if (MS_CheckParm("version")) {
		printf("Game: %s\n", GAMENAME);
		Quit(NULL);
	}
		
	printf("Now Loading %s\n", GAMENAME);
		
	CheckForEpisodes();

	InitGame();

	DemoLoop();

	Quit("Demo loop exited???");
	
	return 0;
}
