#ifndef __ID_HEADS_H__
#define __ID_HEADS_H__

#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <values.h>
#include <sys/types.h>
#include <glob.h>
#include <math.h>

#include <vga.h>
#include <vgakeyboard.h>

#include "misc.h"

#include "version.h"

#define PACKED __attribute__((packed))

#ifndef O_BINARY
#define O_BINARY 0
#endif

int WolfMain(int argc, char *argv[]);

/* ------------------------------------------------------------------------ */

extern char signon;

#define	introscn signon

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


#define GREXT	"VGA"

typedef	enum	{false,true}	boolean;
typedef	unsigned	char		byte;
typedef	unsigned	short int		word;
typedef	unsigned	long		longword;
typedef	byte *					Ptr;

typedef void * memptr;

typedef	struct
		{
			int	x,y;
		} Point;
typedef	struct
		{
			Point	ul,lr;
		} Rect;

#define	nil	((void *)0)

#include "vi_comm.h"

#include "id_ca.h"
#include "id_vh.h"
#include "id_sd.h"
#include "id_us.h"

void	Quit (char *error);		// defined in user program

#define	MAXTICS				10
#define DEMOTICS			4

extern	unsigned	mapwidth,mapheight,tics;

extern	byte		fontcolor,backcolor;

#define SETFONTCOLOR(f,b) fontcolor=f;backcolor=b;

#else
#error "fix me: TODO"
#endif
