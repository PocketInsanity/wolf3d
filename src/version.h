#ifndef __VERSION_H__
#define __VERSION_H__

/* Change WMODE to point to the executable you would like to build: */
/* WL1 = 0 */ 
/* WL6 = 1 */
/* SDM = 2 */
/* SOD = 3 */
#ifndef WMODE
#define WMODE 1
#endif

/* --- End of User-Modifiable Variables --- */

#if WMODE == 0
/* #define SPEAR */
/* #define SPEARDEMO */
#define UPLOAD
#define GAMENAME "Wolfenstein 3D Shareware"

#elif WMODE == 1
/* #define SPEAR */
/* #define SPEARDEMO */
/* #define UPLOAD */
#define GAMENAME "Wolfenstein 3D"

#elif WMODE == 2
#define SPEAR 
#define SPEARDEMO 
/* #define UPLOAD */
#define GAMENAME "Spear of Destiny Demo"

#elif WMODE == 3
#define SPEAR
/* #define SPEARDEMO */
/* #define UPLOAD */
#define GAMENAME "Spear of Destiny"

#else
#error "please edit version.h and fix WMODE"
#endif

#endif
