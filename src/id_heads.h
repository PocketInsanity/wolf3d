#ifndef __ID_HEADS_H__
#define __ID_HEADS_H__

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <values.h>

#include "version.h"

#define PACKED __attribute__((packed))

#ifndef O_BINARY
#define O_BINARY
#endif


/* ------------------------------------------------------------------------ */

extern char signon;

#define	introscn signon

#ifdef JAPAN

#ifdef JAPDEMO
#include "FOREIGN\JAPAN\GFXV_WJ1.H"
#else /* JAPDEMO */
#include "FOREIGN\JAPAN\GFXV_WJ6.H"
#endif /* JAPDEMO */

#include "audiowl6.h"
#include "mapswl6.h"

#else /* JAPAN */

#ifndef SPEAR

#ifndef UPLOAD
#include "gfxv_wl6.h"
#include "audiowl6.h"
#include "mapswl6.h"
#else
#include "gfxv_wl1.h"
#include "audiowl1.h"
#include "mapswl1.h"
#endif

#else /* SPEAR */

#ifndef SPEARDEMO
#include "gfxv_sod.h"
#include "audiosod.h"
#include "mapssod.h"
#else /* SPEARDEMO */
#include "gfxv_sdm.h"
#include "audiosdm.h"
#include "mapssdm.h"
#endif /* SPEARDEMO */

#endif /* SPEAR */
#endif /* JAPAN */

/* ---------------- */


#define GREXT	"VGA"

typedef	enum	{false,true}	boolean;
typedef	unsigned	char		byte;
typedef	unsigned	int			word;
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

#include "id_ca.h"
#include "id_vl.h"
#include "id_vh.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"

void	Quit (char *error);		// defined in user program

//
// replacing refresh manager with custom routines
//

#define	PORTTILESWIDE		20      // all drawing takes place inside a
#define	PORTTILESHIGH		13		// non displayed port of this size

#define UPDATEWIDE			PORTTILESWIDE
#define UPDATEHIGH			PORTTILESHIGH

#define	MAXTICS				10
#define DEMOTICS			4

extern	unsigned	mapwidth,mapheight,tics;

extern	byte		*updateptr;
extern	unsigned	uwidthtable[UPDATEHIGH];
extern	unsigned	blockstarts[UPDATEWIDE*UPDATEHIGH];

extern	byte		fontcolor,backcolor;

#define SETFONTCOLOR(f,b) fontcolor=f;backcolor=b;

#elif
#error "fix me: TODO"
#endif
