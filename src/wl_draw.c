/* wl_draw.c */

#include "wl_def.h" 

/* Originally from David Haslam -- dch@sirius.demon.co.uk */

// the door is the last picture before the sprites
#define DOORWALL	(PMSpriteStart-8)

#define ACTORSIZE	0x4000

unsigned	wallheight[MAXVIEWWIDTH];

#define mindist	MINDIST

//int 		pixelangle[MAXVIEWWIDTH]; /* TODO: i put these in wl_main */
//long		finetangent[FINEANGLES/4];

//
// refresh variables
//
fixed	viewx,viewy;			// the focal point
int		viewangle;

//
// ray tracing variables
//
int			focaltx,focalty;

int			midangle,angle;
unsigned	xpartial,ypartial;
unsigned	xpartialup,xpartialdown,ypartialup,ypartialdown;

unsigned	tilehit;
unsigned	pixx;

int		xtile,ytile;
int		xtilestep,ytilestep;
long	xintercept,yintercept;
long	xstep,ystep;

void AsmRefresh (void);
void xBuildCompScale(int height, byte *source, int x);

//==========================================================================

/*
========================
=
= TransformActor
=
= Takes paramaters:
=   gx,gy		: globalx/globaly of point
=
= globals:
=   viewx,viewy		: point of view
=   viewcos,viewsin	: sin/cos of viewangle
=   scale		: conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
========================
*/

//
// transform actor
//
void TransformActor (objtype *ob)
{
	fixed gx,gy,gxt,gyt,nx,ny;

//
// translate point to view centered coordinates
//
	gx = ob->x-viewx;
	gy = ob->y-viewy;

//
// calculate newx
//
	gxt = FixedByFrac(gx,viewcos);
	gyt = FixedByFrac(gy,viewsin);
	nx = gxt-gyt-ACTORSIZE;		// fudge the shape forward a bit, because
					// the midpoint could put parts of the shape
					// into an adjacent wall

//
// calculate newy
//
	gxt = FixedByFrac(gx,viewsin);
	gyt = FixedByFrac(gy,viewcos);
	ny = gyt+gxt;

//
// calculate perspective ratio
//
	ob->transx = nx;
	ob->transy = ny;

	if (nx < mindist) /* too close, don't overflow the divide */
	{
	  ob->viewheight = 0;
	  return;
	}

	ob->viewx = centerx + ny*scale/nx;	

	ob->viewheight = heightnumerator/(nx>>8);

}

//==========================================================================

/*
========================
=
= TransformTile
=
= Takes paramaters:
=   tx,ty		: tile the object is centered in
=
= globals:
=   viewx,viewy		: point of view
=   viewcos,viewsin	: sin/cos of viewangle
=   scale		: conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
= Returns true if the tile is withing getting distance
=
========================
*/

boolean TransformTile (int tx, int ty, int *dispx, int *dispheight)
{
	fixed gx,gy,gxt,gyt,nx,ny;

//
// translate point to view centered coordinates
//
	gx = ((long)tx<<TILESHIFT)+0x8000-viewx;
	gy = ((long)ty<<TILESHIFT)+0x8000-viewy;

//
// calculate newx
//
	gxt = FixedByFrac(gx,viewcos);
	gyt = FixedByFrac(gy,viewsin);
	nx = gxt-gyt-0x2000;		// 0x2000 is size of object

//
// calculate newy
//
	gxt = FixedByFrac(gx,viewsin);
	gyt = FixedByFrac(gy,viewcos);
	ny = gyt+gxt;


//
// calculate perspective ratio
//
	if (nx<mindist)			/* too close, don't overflow the divide */
	{
		*dispheight = 0;
		return false;
	}

	*dispx = centerx + ny*scale/nx;	

	*dispheight = heightnumerator/(nx>>8);

//
// see if it should be grabbed
//
	if ( (nx<TILEGLOBAL) && (ny>-TILEGLOBAL/2) && (ny<TILEGLOBAL/2) )
		return true;
	else
		return false;
}

//==========================================================================

/*
====================
=
= CalcHeight
=
= Calculates the height of xintercept,yintercept from viewx,viewy
=
====================
*/


int CalcHeight (void)
{
	fixed gxt,gyt,nx,gx,gy;

	gx = xintercept-viewx;
	gxt = FixedByFrac(gx,viewcos);

	gy = yintercept-viewy;
	gyt = FixedByFrac(gy,viewsin);

	nx = gxt-gyt;

  //
  // calculate perspective ratio (heightnumerator/(nx>>8))
  //
	if (nx<mindist)
		nx=mindist;			/* don't let divide overflow */

	return heightnumerator/(nx>>8);
}


//==========================================================================

/*
===================
=
= ScalePost
=
===================
*/

unsigned postx;

void ScalePost(byte *wall, int texture)
{
	int height;
	byte *source;

	height = (wallheight [postx] & 0xfff8) >> 1;
	if (height > maxscaleshl2)
		height = maxscaleshl2;
	
	source = wall+texture;
	xBuildCompScale (height/2, source, postx);
}


/*
====================
=
= HitHorizDoor
=
====================
*/

void HitHorizDoor()
{
	unsigned texture, doorpage = 0, doornum;
	byte *wall;

	doornum = tilehit&0x7f;
	texture = ( (xintercept-doorposition[doornum]) >> 4) &0xfc0;

	wallheight[pixx] = CalcHeight();

		postx = pixx;

		switch (doorobjlist[doornum].lock)
		{
		case dr_normal:
			doorpage = DOORWALL;
			break;
		case dr_lock1:
		case dr_lock2:
		case dr_lock3:
		case dr_lock4:
			doorpage = DOORWALL+6;
			break;
		case dr_elevator:
			doorpage = DOORWALL+4;
			break;
		}

	wall = PM_GetPage (doorpage);
	ScalePost (wall, texture);
}

//==========================================================================

/*
====================
=
= HitVertDoor
=
====================
*/

void HitVertDoor()
{
	unsigned texture, doorpage = 0, doornum;
	byte *wall;

	doornum = tilehit&0x7f;
	texture = ( (yintercept-doorposition[doornum]) >> 4) &0xfc0;

	wallheight[pixx] = CalcHeight();

		postx = pixx;

		switch (doorobjlist[doornum].lock)
		{
		case dr_normal:
			doorpage = DOORWALL;
			break;
		case dr_lock1:
		case dr_lock2:
		case dr_lock3:
		case dr_lock4:
			doorpage = DOORWALL+6;
			break;
		case dr_elevator:
			doorpage = DOORWALL+4;
			break;
		}

 wall = PM_GetPage (doorpage);
 ScalePost (wall, texture);
}

//==========================================================================

unsigned Ceiling[]=
{
#ifndef SPEAR
 0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0xbfbf,
 0x4e4e,0x4e4e,0x4e4e,0x1d1d,0x8d8d,0x4e4e,0x1d1d,0x2d2d,0x1d1d,0x8d8d,
 0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x2d2d,0xdddd,0x1d1d,0x1d1d,0x9898,

 0x1d1d,0x9d9d,0x2d2d,0xdddd,0xdddd,0x9d9d,0x2d2d,0x4d4d,0x1d1d,0xdddd,
 0x7d7d,0x1d1d,0x2d2d,0x2d2d,0xdddd,0xd7d7,0x1d1d,0x1d1d,0x1d1d,0x2d2d,
 0x1d1d,0x1d1d,0x1d1d,0x1d1d,0xdddd,0xdddd,0x7d7d,0xdddd,0xdddd,0xdddd
#else
 0x6f6f,0x4f4f,0x1d1d,0xdede,0xdfdf,0x2e2e,0x7f7f,0x9e9e,0xaeae,0x7f7f,
 0x1d1d,0xdede,0xdfdf,0xdede,0xdfdf,0xdede,0xe1e1,0xdcdc,0x2e2e,0x1d1d,0xdcdc
#endif
};

/*
=====================
=
= ClearScreen
=
=====================
*/

void ClearScreen()
{
    unsigned ceiling = Ceiling[gamestate.episode*10+mapon] & 0xFF;
    unsigned floor = 0x19;

   VL_Bar(xoffset, yoffset, viewwidth, viewheight / 2, ceiling);
   VL_Bar(xoffset, yoffset + viewheight / 2, viewwidth, viewheight / 2, floor);
}

//==========================================================================

/*
=====================
=
= CalcRotate
=
=====================
*/

int CalcRotate(objtype *ob)
{
	int	angle,viewangle;

	/* this isn't exactly correct, as it should vary by a trig value, */
	/* but it is close enough with only eight rotations */

	viewangle = player->angle + (centerx - ob->viewx)/8;

	if (ob->obclass == rocketobj || ob->obclass == hrocketobj)
		angle =  (viewangle-180)- ob->angle;
	else
		angle =  (viewangle-180)- dirangle[ob->dir];

	angle+=ANGLES/16;
	while (angle>=ANGLES)
		angle-=ANGLES;
	while (angle<0)
		angle+=ANGLES;

	if (ob->state->rotate == 2)             // 2 rotation pain frame
		return 4*(angle/(ANGLES/2));        // seperated by 3 (art layout...)

	return angle/(ANGLES/8);
}


/*
=====================
=
= DrawScaleds
=
= Draws all objects that are visable
=
=====================
*/

#define MAXVISABLE	50

typedef struct
{
	int	viewx,
		viewheight,
		shapenum;
} visobj_t;

visobj_t	vislist[MAXVISABLE],*visptr,*visstep,*farthest;

void DrawScaleds (void)
{
	int 		i,least,numvisable,height;
	byte		*tilespot,*visspot;
	unsigned	spotloc;

	statobj_t	*statptr;
	objtype		*obj;

	visptr = &vislist[0];

//
// place static objects
//
	for (statptr = &statobjlist[0] ; statptr !=laststatobj ; statptr++)
	{
		if ((visptr->shapenum = statptr->shapenum) == -1)
			continue;						// object has been deleted

		if (!*statptr->visspot)
			continue;						// not visable

		if (TransformTile (statptr->tilex,statptr->tiley
			,&visptr->viewx,&visptr->viewheight) && statptr->flags & FL_BONUS)
		{
			GetBonus (statptr);
			continue;
		}

		if (!visptr->viewheight)
			continue;						// to close to the object

		if (visptr < &vislist[MAXVISABLE-1])	/* don't let it overflow */
			visptr++;
	}

//
// place active objects
//
	for (obj = player->next;obj;obj=obj->next)
	{
		if (!(visptr->shapenum = obj->state->shapenum))
			continue;  // no shape

		spotloc = (obj->tilex<<6)+obj->tiley;	// optimize: keep in struct?
		visspot = &spotvis[0][0]+spotloc;
		tilespot = &tilemap[0][0]+spotloc;

		//
		// could be in any of the nine surrounding tiles
		//
		if (*visspot
		|| ( *(visspot-1) && !*(tilespot-1) )
		|| ( *(visspot+1) && !*(tilespot+1) )
		|| ( *(visspot-65) && !*(tilespot-65) )
		|| ( *(visspot-64) && !*(tilespot-64) )
		|| ( *(visspot-63) && !*(tilespot-63) )
		|| ( *(visspot+65) && !*(tilespot+65) )
		|| ( *(visspot+64) && !*(tilespot+64) )
		|| ( *(visspot+63) && !*(tilespot+63) ) )
		{
			obj->active = true;
			TransformActor (obj);
			if (!obj->viewheight)
				continue;						// too close or far away

			visptr->viewx = obj->viewx;
			visptr->viewheight = obj->viewheight;
			if (visptr->shapenum == -1)
				visptr->shapenum = obj->temp1;	// special shape

			if (obj->state->rotate)
				visptr->shapenum += CalcRotate (obj);

			if (visptr < &vislist[MAXVISABLE-1])	/* don't let it overflow */
				visptr++;
			obj->flags |= FL_VISABLE;
		}
		else
			obj->flags &= ~FL_VISABLE;
	}

//
// draw from back to front
//
	numvisable = visptr-&vislist[0];

	if (!numvisable)
		return;									// no visable objects

	for (i = 0; i<numvisable; i++)
	{
		least = 32000;
		for (visstep=&vislist[0] ; visstep<visptr ; visstep++)
		{
			height = visstep->viewheight;
			if (height < least)
			{
				least = height;
				farthest = visstep;
			}
		}
		//
		// draw farthest
		//
		ScaleShape(farthest->viewx,farthest->shapenum,farthest->viewheight);

		farthest->viewheight = 32000;
	}

}

//==========================================================================

/*
==============
=
= DrawPlayerWeapon
=
= Draw the player's hands
=
==============
*/

int	weaponscale[NUMWEAPONS] = {SPR_KNIFEREADY,SPR_PISTOLREADY
	,SPR_MACHINEGUNREADY,SPR_CHAINREADY};

void DrawPlayerWeapon (void)
{
	int	shapenum;

#ifndef SPEAR
	if (gamestate.victoryflag)
	{
		if (player->state == &s_deathcam && (get_TimeCount() & 32) )
			SimpleScaleShape(viewwidth/2,SPR_DEATHCAM,viewheight+1);
		return;
	}
#endif

	if (gamestate.weapon != -1)
	{
		shapenum = weaponscale[gamestate.weapon]+gamestate.weaponframe;
		SimpleScaleShape(viewwidth/2,shapenum,viewheight+1);
	}

	if (demorecord || demoplayback)
		SimpleScaleShape(viewwidth/2,SPR_DEMO,viewheight+1);
}


//==========================================================================

/*
====================
=
= WallRefresh
=
====================
*/

void WallRefresh (void)
{
/*
 set up variables for this view
*/
	viewangle = player->angle;
	midangle = viewangle*(FINEANGLES/ANGLES);
	viewsin = sintable[viewangle];
	viewcos = costable[viewangle];
	viewx = player->x - FixedByFrac(focallength,viewcos);
	viewy = player->y + FixedByFrac(focallength,viewsin);

	focaltx = viewx>>TILESHIFT;
	focalty = viewy>>TILESHIFT;

	xpartialdown = viewx&(TILEGLOBAL-1);
	xpartialup = TILEGLOBAL-xpartialdown;
	ypartialdown = viewy&(TILEGLOBAL-1);
	ypartialup = TILEGLOBAL-ypartialdown;

	AsmRefresh();
}

//==========================================================================

/*
========================
=
= ThreeDRefresh
=
========================
*/

void ThreeDRefresh()
{

//
// clear out the traced array
//
	memset(spotvis, 0, sizeof(spotvis));

//
// follow the walls from there to the right, drawwing as we go
//
	DrawPlayBorder();
	ClearScreen();

	WallRefresh ();

//
// draw all the scaled images
//
	DrawScaleds();			// draw scaled stuff
	DrawPlayerWeapon();	/* draw player's hands */

//
// show screen and time last cycle
//
	if (fizzlein)
	{
		FizzleFade(xoffset, yoffset, viewwidth,viewheight,20,false);
		fizzlein = false;

		lasttimecount = 0;		/* don't make a big tic count */
		set_TimeCount(0);

	}

	VW_UpdateScreen ();

	frameon++;
}


//===========================================================================

/* xpartial = 16 bit fraction
   ystep = 32 bit fixed 32,16
   */

#define xpartialbyystep() FixedByFrac(xpartial, ystep)
#define ypartialbyxstep() FixedByFrac(ypartial, xstep)

int samex (int intercept, int tile)
{
    if (xtilestep > 0) {
	if ((intercept>>16) >= tile)
	    return 0;
	else
	    return 1;
    } else {
	if ((intercept>>16) <= tile)
	    return 0;
	else
	    return 1;
    }
}

int samey (int intercept, int tile)
{
    if (ytilestep > 0) {
	if ((intercept>>16) >= tile)
	    return 0;
	else
	    return 1;
    } else {
	if ((intercept>>16) <= tile)
	    return 0;
	else
	    return 1;
    }
}

#define DEG90	900
#define DEG180	1800
#define DEG270	2700
#define DEG360	3600

void HitHorizWall (void);
void HitVertWall (void);
void HitHorizPWall (void);
void HitVertPWall (void);

void AsmRefresh (void)
{
    fixed doorxhit, dooryhit;

    int angle;    /* ray angle through pixx */

    for (pixx = 0; pixx < viewwidth; pixx++) {
	angle = midangle + pixelangle[pixx];

	if (angle < 0) {
	    /* -90 - -1 degree arc */
	    angle += FINEANGLES;
	    goto entry360;
	} else if (angle < DEG90) {
		/* 0-89 degree arc */
	    entry90:
		xtilestep = 1;
		ytilestep = -1;
		xstep = finetangent[DEG90-1-angle];
		ystep = -finetangent[angle];
		xpartial = xpartialup;
		ypartial = ypartialdown;
	    } else if (angle < DEG180) {
		    /* 90-179 degree arc */
		    xtilestep = -1;
		    ytilestep = -1;
		    xstep = -finetangent[angle-DEG90];
		    ystep = -finetangent[DEG180-1-angle];
		    xpartial = xpartialdown;
		    ypartial = ypartialdown;
		} else if (angle < DEG270) {
			/* 180-269 degree arc */
			xtilestep = -1;
			ytilestep = 1;
			xstep = -finetangent[DEG270-1-angle];
			ystep = finetangent[angle-DEG180];
			xpartial = xpartialdown;
			ypartial = ypartialup;
		    } else if (angle < DEG360) {
			    /* 270-359 degree arc */
			entry360:
			    xtilestep = 1;
			    ytilestep = 1;
			    xstep = finetangent[angle-DEG270];
			    ystep = finetangent[DEG360-1-angle];
			    xpartial = xpartialup;
			    ypartial = ypartialup;
			} else {
			    angle -= FINEANGLES;
			    goto entry90;
			}

	yintercept = viewy + xpartialbyystep ();
	xtile = focaltx + xtilestep;

	xintercept = viewx + ypartialbyxstep ();
	ytile = focalty + ytilestep;

/* CORE LOOP */

#define TILE(n) (n>>16)

	/* check intersections with vertical walls */
	vertcheck:
	    if (!samey (yintercept, ytile))
		goto horizentry;
	vertentry:
	    tilehit = tilemap [xtile][TILE(yintercept)];

	    if (tilehit != 0) {
		if (tilehit & 0x80) {
		    if (tilehit & 0x40) {
			/* vertpushwall */
			long ytemp = yintercept + (pwallpos * ystep) / 64;
			ytemp &= ~0x4000000; /* TODO: why? */
			//fprintf(stderr, "VertPushWall@%d: 0x%X vs 0x%X\n", angle, yintercept, ytemp);
			if (TILE(ytemp) != TILE(yintercept)) 
				goto passvert;
			yintercept = ytemp;
			xintercept = xtile << 16;
			HitVertPWall();
		    }
		    else {
			dooryhit = yintercept + ystep / 2;
			if (TILE (dooryhit) != TILE (yintercept))
			    goto passvert;
			/* check door position */
			if ((dooryhit&0xFFFF) < doorposition[tilehit&0x7f])
			    goto passvert;
			yintercept = dooryhit;
			xintercept = (xtile << 16) + 32768;
			HitVertDoor ();
		    }
		}
		else {
		    xintercept = xtile << 16;
		    HitVertWall ();
		}
		goto nextpix;
	    }
    passvert:
	    spotvis [xtile][TILE(yintercept)] = 1;
	    xtile += xtilestep;
	    yintercept += ystep;
	    goto vertcheck;
	horizcheck:
	    /* check intersections with horizontal walls */
	    if (!samex (xintercept, xtile))
		goto vertentry;
	horizentry:
	    tilehit = tilemap [TILE(xintercept)][ytile];

	    if (tilehit != 0) {
		if (tilehit & 0x80) {
		    /* horizdoor */
		    if (tilehit & 0x40) {
		    	long xtemp = xintercept + (pwallpos * xstep) / 64;
		    	xtemp &= ~0x4000000;
			/* horizpushwall */
			if (TILE(xtemp) != TILE(xintercept))
				goto passhoriz;
			xintercept = xtemp;
			yintercept = ytile << 16; 
			HitHorizPWall();
		    }
		    else {
			doorxhit = xintercept + xstep / 2;
			if (TILE (doorxhit) != TILE (xintercept))
			    goto passhoriz;
			/* check door position */
			if ((doorxhit&0xFFFF) < doorposition[tilehit&0x7f])
			    goto passhoriz;
			xintercept = doorxhit;
			yintercept = (ytile << 16) + 32768;
			HitHorizDoor ();
		    }
		}
		else {
		    yintercept = ytile << 16;
		    HitHorizWall ();
		}
		goto nextpix;
	    }
	    passhoriz:
	    spotvis [TILE(xintercept)][ytile] = 1;
	    ytile += ytilestep;
	    xintercept += xstep;
	    goto horizcheck;
    nextpix:
    }
}


void HitVertWall (void)
{
	int			wallpic;
	unsigned	texture;
	byte *wall;

	texture = (yintercept>>4)&0xfc0;
	if (xtilestep == -1)
	{
		texture = 0xfc0-texture;
		xintercept += TILEGLOBAL;
	}
	wallheight[pixx] = CalcHeight();

		postx = pixx;

		if (tilehit & 0x40)
		{								// check for adjacent doors
			ytile = yintercept>>TILESHIFT;
			if ( tilemap[xtile-xtilestep][ytile]&0x80 )
				wallpic = DOORWALL+3;
			else
				wallpic = vertwall[tilehit & ~0x40];
		}
		else
			wallpic = vertwall[tilehit];
		
		wall = PM_GetPage (wallpic);
 ScalePost (wall, texture);

}

void HitHorizWall (void)
{
	int			wallpic;
	unsigned	texture;
	byte *wall;

	texture = (xintercept>>4)&0xfc0;
	if (ytilestep == -1)
		yintercept += TILEGLOBAL;
	else
		texture = 0xfc0-texture;
	wallheight[pixx] = CalcHeight();

		postx = pixx;

		if (tilehit & 0x40)
		{								// check for adjacent doors
			xtile = xintercept>>TILESHIFT;
			if ( tilemap[xtile][ytile-ytilestep]&0x80 )
				wallpic = DOORWALL+2;
			else
				wallpic = horizwall[tilehit & ~0x40];
		}
		else
			wallpic = horizwall[tilehit];

		wall = PM_GetPage (wallpic);
 ScalePost (wall, texture);

}

/*
====================
=
= HitHorizPWall
=
= A pushable wall in action has been hit
=
====================
*/

void HitHorizPWall (void)
{
	int			wallpic;
	unsigned	texture,offset;
	byte *wall;
	
	texture = (xintercept>>4)&0xfc0;
	offset = pwallpos<<10;
	if (ytilestep == -1)
		yintercept += TILEGLOBAL-offset;
	else
	{
		texture = 0xfc0-texture;
		yintercept += offset;
	}

	wallheight[pixx] = CalcHeight();

		postx = pixx;

		wallpic = horizwall[tilehit&63];
	wall = PM_GetPage (wallpic);
	ScalePost (wall, texture);
}


/*
====================
=
= HitVertPWall
=
= A pushable wall in action has been hit
=
====================
*/

void HitVertPWall (void)
{
	int			wallpic;
	unsigned	texture,offset;
	byte *wall;
	
	texture = (yintercept>>4)&0xfc0;
	offset = pwallpos<<10;
	if (xtilestep == -1)
	{
		xintercept += TILEGLOBAL-offset;
		texture = 0xfc0-texture;
	}
	else
		xintercept += offset;

	wallheight[pixx] = CalcHeight();
		postx = pixx;

		wallpic = vertwall[tilehit&63];

	wall = PM_GetPage (wallpic);
	ScalePost (wall, texture);                 
}
