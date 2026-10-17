/* draw2d.h - replacements for the ID_VW / ID_US drawing and printing
 * routines, drawing into the current Surface (cursurf). */
#ifndef DRAW2D_H
#define DRAW2D_H

#include "def.h"
#include "vid.h"

extern s16  px,py;                  /* prop font pen */
extern byte fontcolor;
extern boolean fontxor;             /* TRUE = XOR like the EGA font code */
extern u16  PrintX,PrintY;
extern u16  WindowX,WindowY,WindowW,WindowH;
extern s16  PPT_LeftEdge,PPT_RightEdge;

void VW_Bar(s16 x, s16 y, s16 w, s16 h, u16 color);
void VW_Plot(s16 x, s16 y, u16 color);
void VW_DrawPic(u16 xbytes, u16 y, u16 picnum);
void VW_DrawTile8(u16 xbytes, u16 y, u16 tile);
void VW_DrawTile8M(s16 x, s16 y, u16 tile);
void VW_DrawSpriteNorth(s16 x, s16 y);
void VW_DrawMaskedLongs(u16 xbytes, s16 y, const u32 *data, u16 wbytes, u16 h, u16 rows);
void VW_DrawPropString(const char *s);
void VW_MeasurePropString(const char *s, u16 *w, u16 *h);
void VW_ClearSurface(u16 color);

void US_Print(const char *s);
void US_PrintUnsigned(u32 n);
void US_PrintCentered(const char *s);
void US_CPrintLine(const char *s);
void US_CPrint(const char *s);
void US_ClearWindow(void);
void US_DrawWindow(u16 x, u16 y, u16 w, u16 h);
void US_CenterWindow(u16 w, u16 h);
void PrintPropText(const char *text);

#endif
