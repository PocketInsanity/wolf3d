#ifndef __ID_HEADS_H__
#define __ID_HEADS_H__

#ifdef _WIN32

/* TODO: rename dosism, because for example djgpp has glob() */
#define DOSISM /* for junk which isn't abstracted (namely stuff in wl_menu.c with glob/findfirst and misc.c) */
#undef HAVE_FFBLK /* TODO: what to do with hacks like this */

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <io.h>
#include <dos.h>

#ifdef __cplusplus
typedef bool boolean;
#else
#define boolean BOOLEAN
#define false FALSE
#define true TRUE
#endif

#define PACKED
#pragma pack(1) /* TODO: this unfortunately packs every struct... */

#define ssize_t SSIZE_T

#else 

#undef DOSISM
#include <unistd.h>
#include <sys/time.h>
#include <values.h>
#include <glob.h>

#define PACKED __attribute__((packed))
#define LONGLONG long long

#ifdef __cplusplus
typedef bool boolean;
#else
typedef enum { false, true } boolean;
#endif

#endif

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include "misc.h"
#include "version.h"

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

typedef unsigned char byte;
typedef unsigned short int word;
typedef unsigned int longword;
typedef unsigned long dword;

typedef long fixed;

typedef void * memptr;

typedef	struct {
	int x, y;
} Point;

typedef	struct {
	Point ul, lr;
} Rect;

#include "vi_comm.h"
#include "sd_comm.h"

#include "id_ca.h"
#include "id_vh.h"
#include "id_us.h"

extern const byte introscn[];
extern const byte gamepal[];

int MS_CheckParm(char *string);
void Quit(char *error);

#undef PI
#define PI      3.141592657

#define	MAXTICS		10
#define DEMOTICS	4

extern unsigned tics;

#define mapwidth	64
#define mapheight	64

extern byte fontcolor, backcolor;

#define SETFONTCOLOR(f, b) { fontcolor = f; backcolor = b; }

#endif
