/* draw2d.c - 2-D drawing on 4bpp tile-layout surfaces.
 *
 * Pixel (x,y) of a surface lives in byte  buf + (x>>3)*colbytes + y*4 +
 * ((x&7)>>1), high nibble for even x.  An 8-pixel wide strip of a pic is one
 * long per row, so byte-aligned pics are copied as runs of longs. */
#include "draw2d.h"

s16  px,py;
byte fontcolor = WHITE;
boolean fontxor = TRUE;
u16  PrintX,PrintY;
u16  WindowX,WindowY,WindowW,WindowH;
s16  PPT_LeftEdge,PPT_RightEdge = 319;

static inline u8 *pixaddr(Surface *s, s16 x, s16 y)
{
    return s->buf + (x >> 3) * s->colbytes + y * 4 + ((x & 7) >> 1);
}

void VW_Plot(s16 x, s16 y, u16 color)
{
    Surface *s = cursurf;
    u8 *p;
    if (x < 0 || y < 0 || x >= 320 || y >= (s16)s->rows) return;
    p = pixaddr(s, x, y);
    if (x & 1) *p = (*p & 0xF0) | (color & 15);
    else *p = (*p & 0x0F) | ((color & 15) << 4);
    surf_mark(s, x, 1);
}

static const u32 leftmask[8] = {
    0xFFFFFFFF, 0x0FFFFFFF, 0x00FFFFFF, 0x000FFFFF,
    0x0000FFFF, 0x00000FFF, 0x000000FF, 0x0000000F };

void VW_Bar(s16 x, s16 y, s16 w, s16 h, u16 color)
{
    Surface *s = cursurf;
    s16 x2, y2, c, c0, c1, r;
    u32 col = (color & 15) * 0x11111111;

    x2 = x + w;
    y2 = y + h;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x2 > 320) x2 = 320;
    if (y2 > (s16)s->rows) y2 = s->rows;
    if (x >= x2 || y >= y2) return;
    c0 = x >> 3;
    c1 = (x2 - 1) >> 3;
    for (c = c0; c <= c1; c++)
    {
        u32 m = 0xFFFFFFFF;
        u32 *p = (u32 *)(s->buf + c * s->colbytes + y * 4);
        if (c == c0) m &= leftmask[x & 7];
        if (c == c1) m &= ~((x2 & 7) ? leftmask[x2 & 7] : 0);
        if (m == 0xFFFFFFFF)
            for (r = y; r < y2; r++) *p++ = col;
        else
            for (r = y; r < y2; r++, p++) *p = (*p & ~m) | (col & m);
    }
    surf_mark(s, x, x2 - x);
}

void VW_ClearSurface(u16 color)
{
    u32 col = (color & 15) * 0x11111111;
    u32 *p = (u32 *)cursurf->buf;
    u32 n = (u32)cursurf->colbytes * 40 / 4;
    while (n--) *p++ = col;
    surf_markall(cursurf);
}

void VW_DrawPic(u16 xbytes, u16 y, u16 picnum)
{
    const PicDef *pd = &pic_table[picnum];
    Surface *s = cursurf;
    u16 c, r, h = pd->height;
    const u32 *src = pd->data;
    if (!src) return;
    if (y + h > s->rows) h = s->rows - y;
    for (c = 0; c < pd->wbytes; c++)
    {
        u32 *p;
        if (xbytes + c >= 40) break;
        p = (u32 *)(s->buf + (xbytes + c) * s->colbytes + y * 4);
        for (r = 0; r < h; r++) *p++ = src[r];
        src += pd->height;
    }
    surf_mark(s, xbytes * 8, pd->wbytes * 8);
}

void VW_DrawTile8(u16 xbytes, u16 y, u16 tile)
{
    Surface *s = cursurf;
    const u32 *src = &tile8_data[tile * 8];
    u32 *p = (u32 *)(s->buf + xbytes * s->colbytes + y * 4);
    u16 r;
    for (r = 0; r < 8; r++) p[r] = src[r];
    surf_mark(s, xbytes * 8, 8);
}

void VW_DrawMaskedLongs(u16 xbytes, s16 y, const u32 *data, u16 wbytes, u16 h, u16 rows)
{
    /* data: per column 'h' (mask,data) pairs; draw only the first 'rows' */
    Surface *s = cursurf;
    u16 c, r;
    for (c = 0; c < wbytes; c++)
    {
        const u32 *src = data + c * h * 2;
        u32 *p;
        s16 yy = y;
        if (xbytes + c >= 40) break;
        p = (u32 *)(s->buf + (xbytes + c) * s->colbytes + y * 4);
        for (r = 0; r < rows; r++, yy++, p++, src += 2)
        {
            if (yy < 0 || yy >= (s16)s->rows) continue;
            *p = (*p & src[0]) | src[1];
        }
    }
    surf_mark(s, xbytes * 8, wbytes * 8);
}

void VW_DrawTile8M(s16 x, s16 y, u16 tile)
{
    if (x < 0 || x >= 320) return;
    VW_DrawMaskedLongs(x >> 3, y, &tile8m_data[tile * 16], 1, 8, 8);
}

void VW_DrawSpriteNorth(s16 x, s16 y)
{
    u16 i, j;
    for (j = 0; j < NORTHICON_H; j++)
        for (i = 0; i < NORTHICON_W; i++)
        {
            u8 p = northicon_pix[j * NORTHICON_W + i];
            if (p != 0xFF)
                VW_Plot(x + i, y + j, p);
        }
}

/* ------------------------------------------------------------------ font */

void VW_MeasurePropString(const char *s, u16 *w, u16 *h)
{
    u16 ww = 0;
    while (*s)
        ww += font_widths[(u8)*s++];
    *w = ww;
    *h = FONT_HEIGHT;
}

static void drawchar(u8 ch)
{
    Surface *s = cursurf;
    u16 w = font_widths[ch];
    u16 bpr = (w + 7) >> 3;
    const u8 *d = font_data + font_locs[ch];
    u16 r, i;
    u8 col = fontcolor & 15;
    for (r = 0; r < FONT_HEIGHT; r++)
    {
        s16 y = py + r;
        if (y >= 0 && y < (s16)s->rows)
            for (i = 0; i < w; i++)
            {
                if (d[i >> 3] & (0x80 >> (i & 7)))
                {
                    s16 x = px + i;
                    u8 *p;
                    if (x < 0 || x >= 320) continue;
                    p = pixaddr(s, x, y);
                    if (fontxor)
                    {
                        if (x & 1) *p ^= col;
                        else *p ^= col << 4;
                    }
                    else
                    {
                        if (x & 1) *p = (*p & 0xF0) | col;
                        else *p = (*p & 0x0F) | (col << 4);
                    }
                }
            }
        d += bpr;
    }
    surf_mark(s, px, w);
    px += w;
}

void VW_DrawPropString(const char *s)
{
    while (*s)
        drawchar((u8)*s++);
}

/* ---------------------------------------------------------------- US_* */

void US_Print(const char *s)
{
    char buf[128];
    u16 w, h;
    while (*s)
    {
        u16 n = 0;
        while (s[n] && s[n] != '\n' && n < 127) { buf[n] = s[n]; n++; }
        buf[n] = 0;
        VW_MeasurePropString(buf, &w, &h);
        px = PrintX;
        py = PrintY;
        VW_DrawPropString(buf);
        s += n;
        if (*s == '\n')
        {
            s++;
            PrintX = WindowX;
            PrintY += h;
        }
        else
            PrintX += w;
    }
}

void US_PrintUnsigned(u32 n)
{
    char buf[16];
    uintToStr(n, buf, 1);
    US_Print(buf);
}

void US_PrintCentered(const char *s)
{
    u16 w, h;
    VW_MeasurePropString(s, &w, &h);
    px = WindowX + ((WindowW - w) / 2);
    py = WindowY + ((WindowH - h) / 2);
    VW_DrawPropString(s);
}

void US_CPrintLine(const char *s)
{
    u16 w, h;
    VW_MeasurePropString(s, &w, &h);
    px = WindowX + ((s16)(WindowW - w) / 2);
    py = PrintY;
    VW_DrawPropString(s);
    PrintY += h;
}

void US_CPrint(const char *s)
{
    char buf[128];
    while (*s)
    {
        u16 n = 0;
        while (s[n] && s[n] != '\n' && n < 127) { buf[n] = s[n]; n++; }
        buf[n] = 0;
        US_CPrintLine(buf);
        s += n;
        if (*s) s++;
    }
}

void US_ClearWindow(void)
{
    VW_Bar(WindowX, WindowY, WindowW, WindowH, LT_GREY);
    PrintX = WindowX;
    PrintY = WindowY;
}

void US_DrawWindow(u16 x, u16 y, u16 w, u16 h)
{
    u16 i, sx, sy, sw, sh;

    WindowX = x * 8;
    WindowY = y * 8;
    WindowW = w * 8;
    WindowH = h * 8;

    PrintX = WindowX;
    PrintY = WindowY;

    sx = (x - 1) * 8;
    sy = (y - 1) * 8;
    sw = (w + 1) * 8;
    sh = (h + 1) * 8;

    US_ClearWindow();

    VW_DrawTile8M(sx, sy, 0);
    VW_DrawTile8M(sx, sy + sh, 6);
    for (i = sx + 8; i <= sx + sw - 8; i += 8)
    {
        VW_DrawTile8M(i, sy, 1);
        VW_DrawTile8M(i, sy + sh, 7);
    }
    VW_DrawTile8M(i, sy, 2);
    VW_DrawTile8M(i, sy + sh, 8);

    for (i = sy + 8; i <= sy + sh - 8; i += 8)
    {
        VW_DrawTile8M(sx, i, 3);
        VW_DrawTile8M(sx + sw, i, 5);
    }
}

void US_CenterWindow(u16 w, u16 h)
{
    US_DrawWindow(((320 / 8) - w) / 2, ((200 / 8) - h) / 2, w, h);
}

/* PrintPropText() from GELIB.C: word wrapping between PPT_LeftEdge and
 * PPT_RightEdge, '^' centres a line. */
void PrintPropText(const char *text)
{
    char pb[200];
    s16 length, maxend, maxx, curx;
    boolean centerit;

    while (*text)
    {
        if (*text == '^')
            centerit = TRUE, text++;
        else
            centerit = FALSE;

        curx = PPT_LeftEdge;
        length = 0;
        while ((curx + font_widths[(u8)text[length + 1]] < PPT_RightEdge) &&
               (text[length]) && (text[length] != '\n'))
            curx += font_widths[(u8)text[length++]];

        if ((text[length]) && (text[length] != '\n'))
        {
            maxx = curx;
            maxend = length;
            while ((length) && (text[length] != ' '))
                curx -= font_widths[(u8)text[length--]];
            if (!length)
                length = maxend, curx = maxx;
        }

        if (((text[length] == ' ') || (text[length] == '\n')) && (length < PPT_RightEdge))
            length++;

        if (length > 199) length = 199;
        memcpy(pb, text, length);
        pb[length] = 0;
        if (length && pb[length - 1] == '\n')
            pb[length - 1] = 0;

        if (centerit)
            US_CPrintLine(pb);
        else
        {
            PrintX = PPT_LeftEdge;
            US_Print(pb);
            US_Print("\n");
        }
        text += length;
        if (!length) break;
    }
}
