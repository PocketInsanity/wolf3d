#ifndef __ID_VL_H__
#define __ID_VL_H__

//===========================================================================

extern byte *gfxbuf;

extern boolean screenfaded;

//===========================================================================

void VL_Startup (void);
void VL_Shutdown (void);

void VL_ClearVideo (byte color);

void VL_WaitVBL (int vbls);

void VL_FillPalette (int red, int green, int blue);
void VL_SetColor	(int color, int red, int green, int blue);
void VL_GetColor	(int color, int *red, int *green, int *blue);
void VL_SetPalette (byte *palette);
void VL_GetPalette (byte *palette);
void VL_FadeOut (int start, int end, int red, int green, int blue, int steps);
void VL_FadeIn (int start, int end, byte *palette, int steps);
void VL_ColorBorder (int color);

void VL_Plot (int x, int y, int color);
void VL_Hlin (unsigned x, unsigned y, unsigned width, unsigned color);
void VL_Vlin (int x, int y, int height, int color);
void VL_Bar (int x, int y, int width, int height, int color);

void VL_MemToLatch (byte *source, int width, int height, word dest);
void VL_MemToScreen (byte *source, int width, int height, int x, int y);

void VL_DrawPropString (char *str, unsigned tile8ptr, int printx, int printy);
void VL_SizePropString (char *str, int *width, int *height, char *font);

#elif
#error "fix me: TODO"
#endif
