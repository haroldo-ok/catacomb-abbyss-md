/* vid.h - Genesis video layer for the Catacomb Abyss port.
 *
 * The original draws into EGA memory: a 320x120 3-D view page (triple
 * buffered) and a 320x80 status bar shown below it through the EGA split
 * screen.  Here both are 4bpp RAM bitmaps stored in the VDP tile layout
 * (column-major, one "tile column" = rows*4 bytes) and copied to VRAM by DMA
 * from the vertical-blank interrupt, so the CPU never waits for the
 * transfer.  The 3-D view is double buffered in VRAM and flipped by
 * rewriting the plane A tilemap.
 */
#ifndef VID_H
#define VID_H

#include <genesis.h>

#define SCREEN_BYTES    32000
#define VIEW_ROWS       120
#define STATUS_ROWS     80
#define VIEW_BYTES      (40*VIEW_ROWS*4)        /* 19200 */
#define STATUS_BYTES    (40*STATUS_ROWS*4)      /* 12800 */

#define TILE_VIEW0      1
#define TILE_VIEW1      601
#define TILE_STATUS     1201
#define TILE_BAND       1601
#define PLANE_ADDR      0xE000
#define SAT_ADDR        0xD000
#define HSCROLL_ADDR    0xD400

typedef struct
{
    u8  *buf;
    u16 rows;           /* pixel rows */
    u16 width;          /* pixels */
    u16 colbytes;       /* bytes per 8 pixel tile column */
    u16 vram;           /* VRAM byte address of tile column 0 (front) */
    u32 dirty;          /* one bit per tile column needing upload (40 cols) */
    u8  dirty_hi;
} Surface;

extern u8 screenbuf[SCREEN_BYTES];
extern Surface viewsurf, statsurf, fullsurf;
extern Surface *cursurf;            /* where 2-D drawing goes ("bufferofs") */

extern volatile u16 vid_front;      /* VRAM view buffer being displayed (0/1) */
extern boolean screenfaded;

void vid_init_game(void);           /* set up VRAM/planes for game mode */
void vid_fullscreen(boolean on);    /* 320x200 2-D surface (help, menus) */
boolean vid_isfull(void);

/* DMA job queue serviced in V-Int */
void vid_queue(const void *src, u16 vramdst, u16 len);
void vid_queue_flip(void);
boolean vid_busy(void);
void vid_wait_idle(void);

/* surfaces */
void surf_mark(Surface *s, s16 x, s16 w);
void surf_markall(Surface *s);
void vid_flush(Surface *s);         /* queue dirty columns to VRAM (front) */
void vid_present_view(void);        /* queue whole view to back buffer + flip */
void vid_update(void);              /* flush status + view overlays, wait */

/* waits a number of 60Hz frames while servicing DMA/sound */
void VW_WaitVBL(u16 n);
void vid_frame(void);               /* wait one frame */

/* palette / effects */
void VW_FadeOut(void);
void VW_FadeIn(void);
void VW_ColorBorder(s16 color);
void vid_set_palette_black(void);
void vid_set_palette_normal(void);

/* fizzle fade the view from the RAM buffer onto the displayed page */
void FizzleFade(boolean abortable);

extern volatile u32 vid_frames;     /* 60 Hz counter */

#endif
