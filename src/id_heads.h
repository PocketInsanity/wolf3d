#ifndef __ID_HEADS_H__
#define __ID_HEADS_H__

#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <values.h>
#include <sys/types.h>
#include <glob.h>
#include <math.h>

#include "misc.h"

#include "version.h"

#define PACKED __attribute__((packed))

#ifndef O_BINARY
#define O_BINARY 0
#endif

/* ------------------------------------------------------------------------ */

#ifndef SPEAR

#ifndef UPLOAD
#include "gfxv_wl6.h"
#include "audiowl6.h"
#else
#include "gfxv_wl1.h"
#include "audiowl1.h"
#endif

#else /* SPEAR */

#ifndef SPEARDEMO
#include "gfxv_sod.h"
#include "audiosod.h"
#else /* SPEARDEMO */
#include "gfxv_sdm.h"
#include "audiosdm.h"
#endif /* SPEARDEMO */

#endif /* SPEAR */

/* ---------------- */

typedef	enum	{false,true}	boolean;
typedef	unsigned	char		byte;
typedef	unsigned	short int	word;
typedef	unsigned	long		longword;

typedef void * memptr;

typedef	struct
		{
			int	x,y;
		} Point;
typedef	struct
		{
			Point	ul,lr;
		} Rect;

#include "vi_comm.h"
#include "sd_comm.h"

#include "id_ca.h"
#include "id_vh.h"
#include "id_us.h"

extern byte signon[];

#define	introscn signon

int MS_CheckParm(char *string);
int WolfMain(int argc, char *argv[]);
void Quit(char *error);

#define	MAXTICS				10
#define DEMOTICS			4

extern	unsigned	mapwidth,mapheight,tics;

extern	byte		fontcolor,backcolor;

#define SETFONTCOLOR(f,b) fontcolor=f;backcolor=b;

#else
#error "fix me: TODO"
#endif
