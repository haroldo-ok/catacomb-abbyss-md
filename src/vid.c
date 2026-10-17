/* vid.c - Genesis video layer (see vid.h) */
#include "def.h"
#include "vid.h"
#include "snd.h"
#include "input.h"

u8 screenbuf[SCREEN_BYTES] __attribute__((aligned(4)));

Surface viewsurf, statsurf, fullsurf;
Surface *cursurf = &statsurf;

volatile u16 vid_front;
volatile u32 vid_frames;
boolean screenfaded;
static boolean fullmode;

volatile u32 TimeCount;
static u16 tickacc;

/* ------------------------------------------------------------------ DMA */

#define MAXJOBS     48
#define JOB_FLIP    1
#define JOB_SAT     2
typedef struct
{
    const u8 *src;
    u16 dst;
    u16 len;
    u16 flags;
} DmaJob;

static DmaJob jobs[MAXJOBS];
static volatile u16 jhead, jtail;
static u16 vblank_budget = 7000;
static boolean vid_pal;

#define VIEWMAP(y,x) TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, (x) * 15 + (y))
static u16 sat[9*4];
static volatile u16 sat_dirty;
static volatile u16 flip_pending;

void vid_queue(const void *src, u16 dst, u16 len)
{
    u16 n;
    if (!len) return;
    for (;;)
    {
        n = (jhead + 1) % MAXJOBS;
        if (n != jtail) break;
        /* queue full: let the V-Int drain it */
        vid_frame();
    }
    jobs[jhead].src = src;
    jobs[jhead].dst = dst;
    jobs[jhead].len = len;
    jobs[jhead].flags = 0;
    jhead = n;
}

static void queue_flag(u16 flags)
{
    u16 n = (jhead + 1) % MAXJOBS;
    while (n == jtail) { vid_frame(); n = (jhead + 1) % MAXJOBS; }
    jobs[jhead].src = NULL;
    jobs[jhead].len = 0;
    jobs[jhead].dst = 0;
    jobs[jhead].flags = flags;
    jhead = n;
}

boolean vid_busy(void)
{
    return jhead != jtail;
}

void vid_wait_idle(void)
{
    while (jhead != jtail)
        vid_frame();
}

/* The plane is 64x64 cells: rows 0-31 map view buffer 0, rows 32-63 view
 * buffer 1 (both with the status bar and band below), so a page flip is a
 * single vertical scroll write. */
static void do_flip(void)
{
    vid_front ^= 1;
    VDP_setVerticalScroll(BG_A, vid_front ? 256 : 0);
    VDP_setVerticalScroll(BG_B, vid_front ? 256 : 0);
}

static void vint(void)
{
    u16 budget = vblank_budget;

    vid_frames++;
    tickacc += vid_pal ? 84 : 70;     /* TimeCount runs at 70 Hz */
    while (tickacc >= 60) { tickacc -= 60; TimeCount++; }

    if (jhead != jtail || sat_dirty)
    {
        Z80_enableBusProtection();
        while (jhead != jtail && budget)
        {
            DmaJob *j = &jobs[jtail];
            if (j->flags & JOB_FLIP)
            {
                do_flip();
            }
            else
            {
                u16 n = j->len;
                if (n > budget) n = budget & ~1;
                if (!n) break;
                DMA_doDma(DMA_VRAM, (void *)j->src, j->dst, n >> 1, 2);
                j->src += n;
                j->dst += n;
                j->len -= n;
                budget -= n;
                if (j->len) break;
            }
            jtail = (jtail + 1) % MAXJOBS;
        }
        if (sat_dirty)
        {
            DMA_doDma(DMA_VRAM, sat, SAT_ADDR, 9 * 4, 2);
            sat_dirty = 0;
        }
        Z80_disableBusProtection();
    }
    JOY_update();
    snd_tick();
}

/* ------------------------------------------------------------- surfaces */

static void surf_init(Surface *s, u8 *buf, u16 rows, u16 tile)
{
    s->buf = buf;
    s->rows = rows;
    s->width = 320;
    s->colbytes = rows * 4;
    s->vram = tile * 32;
    s->dirty = 0;
    s->dirty_hi = 0;
}

void surf_mark(Surface *s, s16 x, s16 w)
{
    s16 c0, c1, c;
    if (w <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > 320) w = 320 - x;
    if (w <= 0) return;
    c0 = x >> 3;
    c1 = (x + w - 1) >> 3;
    for (c = c0; c <= c1; c++)
    {
        if (c < 32) s->dirty |= 1ul << c;
        else s->dirty_hi |= 1 << (c - 32);
    }
}

void surf_markall(Surface *s)
{
    s->dirty = 0xFFFFFFFF;
    s->dirty_hi = 0xFF;
}

void vid_flush(Surface *s)
{
    u16 c = 0, start;
    u32 d = s->dirty;
    u8 dh = s->dirty_hi;
    u16 vram = s->vram;
    if (s == &viewsurf)
        vram = vid_front ? TILE_VIEW1 * 32 : TILE_VIEW0 * 32;
    s->dirty = 0;
    s->dirty_hi = 0;
    while (c < 40)
    {
        boolean set = (c < 32) ? (d >> c) & 1 : (dh >> (c - 32)) & 1;
        if (!set) { c++; continue; }
        start = c;
        while (c < 40 && ((c < 32) ? (d >> c) & 1 : (dh >> (c - 32)) & 1))
            c++;
        vid_queue(s->buf + start * s->colbytes, vram + start * s->colbytes,
            (c - start) * s->colbytes);
    }
}

void vid_present_view(void)
{
    u16 back = vid_front ^ 1;
    u16 vram = back ? TILE_VIEW1 * 32 : TILE_VIEW0 * 32;
    u16 c;
    vid_flush(&statsurf);
    for (c = 0; c < 40; c += 8)
        vid_queue(viewsurf.buf + c * viewsurf.colbytes, vram + c * viewsurf.colbytes,
            8 * viewsurf.colbytes);
    viewsurf.dirty = 0;
    viewsurf.dirty_hi = 0;
    queue_flag(JOB_FLIP);
}

void vid_update(void)
{
    if (fullmode)
        vid_flush(&fullsurf);
    else
    {
        vid_flush(&statsurf);
        vid_flush(&viewsurf);
    }
    vid_wait_idle();
}

/* --------------------------------------------------------------- setup */

static void load_band(void)
{
    u32 band[8];
    u16 i;
    for (i = 0; i < 8; i++) band[i] = 0x11111111;
    VDP_loadTileData(band, TILE_BAND, 1, CPU);
}

static void set_palettes(void)
{
    u16 pal[16];
    u16 i;
    PAL_setPalette(PAL0, ega_palette, CPU);
    for (i = 0; i < 16; i++) pal[i] = ega_palette[i];
    pal[1] = ega_palette[0];
    PAL_setPalette(PAL1, pal, CPU);
    for (i = 0; i < 16; i++) pal[i] = ega_palette[0];
    PAL_setPalette(PAL2, pal, CPU);
}

static void map_rows(boolean full)
{
    vu32 *ctrl = (vu32 *)VDP_CTRL_PORT;
    vu16 *data = (vu16 *)VDP_DATA_PORT;
    u16 x, y;
    SYS_disableInts();
    for (y = 0; y < 64; y++)
    {
        u16 yy = y & 31;
        *ctrl = VDP_WRITE_VRAM_ADDR(PLANE_ADDR + y * 128);
        for (x = 0; x < 64; x++)
        {
            u16 v = 0;
            if (x < 40)
            {
                if (full)
                {
                    if (yy < 25)
                        v = TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, 1 + x * 25 + yy);
                }
                else if (yy < 15)
                    v = VIEWMAP(yy,x) + (y < 32 ? TILE_VIEW0 : TILE_VIEW1);
                else if (yy < 25)
                    v = TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, TILE_STATUS + x * 10 + (yy - 15));
            }
            if (yy >= 25 && yy < 28)
                v = TILE_ATTR_FULL(PAL2, TRUE, FALSE, FALSE, TILE_BAND);
            *data = v;
        }
    }
    SYS_enableInts();
    VDP_setVerticalScroll(BG_A, (!full && vid_front) ? 256 : 0);
    VDP_setVerticalScroll(BG_B, (!full && vid_front) ? 256 : 0);
}

static void map_game(void) { map_rows(FALSE); }
static void map_full(void) { map_rows(TRUE); }

void vid_init_game(void)
{
    u16 x, y;
    static boolean inited;

    vid_wait_idle();
    if (!inited)
    {
        inited = TRUE;
        vid_pal = IS_PAL_SYSTEM;
        SYS_setVIntCallback(vint);
    }
    SYS_disableInts();
    VDP_setEnable(FALSE);
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    VDP_setPlaneSize(64, 64, FALSE);
    VDP_setBGAAddress(PLANE_ADDR);
    VDP_setBGBAddress(PLANE_ADDR);
    VDP_setWindowAddress(0xE000);
    VDP_setSpriteListAddress(SAT_ADDR);
    VDP_setHScrollTableAddress(HSCROLL_ADDR);
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    VDP_setWindowHPos(FALSE, 0);
    VDP_setWindowVPos(FALSE, 0);
    VDP_setBackgroundColor(0);
    SYS_enableInts();

    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_B, 0);


    surf_init(&viewsurf, screenbuf, VIEW_ROWS, TILE_VIEW0);
    surf_init(&statsurf, screenbuf + VIEW_BYTES, STATUS_ROWS, TILE_STATUS);
    surf_init(&fullsurf, screenbuf, 200, 1);
    fullmode = FALSE;
    vid_front = 0;

    load_band();
    set_palettes();
    for (x = 0; x < 9 * 4; x++) sat[x] = 0;
    sat_dirty = 1;
    map_game();
    memset(screenbuf, 0, SCREEN_BYTES);
    surf_markall(&viewsurf);
    surf_markall(&statsurf);
    vid_flush(&statsurf);
    vid_flush(&viewsurf);
    vid_present_view();
    vid_wait_idle();
    VDP_setEnable(TRUE);
}

void vid_fullscreen(boolean on)
{
    vid_wait_idle();
    if (on && !fullmode)
    {
        fullmode = TRUE;
        memset(screenbuf, 0, SCREEN_BYTES);
        map_full();
        surf_markall(&fullsurf);
        vid_flush(&fullsurf);
        vid_wait_idle();
    }
    else if (!on && fullmode)
    {
        fullmode = FALSE;
        memset(screenbuf, 0, SCREEN_BYTES);
        vid_front = 0;
        map_game();
        surf_markall(&statsurf);
        vid_flush(&statsurf);
        vid_wait_idle();
    }
}

boolean vid_isfull(void)
{
    return fullmode;
}

/* ---------------------------------------------------------------- waits */

void vid_frame(void)
{
    u32 f = vid_frames;
    while (vid_frames == f)
        ;
}

void VW_WaitVBL(u16 n)
{
    vid_flush(&statsurf);
    while (n--)
        vid_frame();
}

/* ------------------------------------------------------------- palette */


void vid_set_palette_black(void)
{
    PAL_setColors(0, palette_black, 48, CPU);
    screenfaded = TRUE;
}

void vid_set_palette_normal(void)
{
    set_palettes();
    screenfaded = FALSE;
}

void VW_FadeOut(void)
{
    u16 i;
    vid_wait_idle();
    PAL_fadeOut(0, 47, 20, FALSE);
    for (i = 0; i < 2; i++) vid_frame();
    screenfaded = TRUE;
}

void VW_FadeIn(void)
{
    u16 pal[48];
    u16 i;
    for (i = 0; i < 16; i++)
    {
        pal[i] = ega_palette[i];
        pal[16 + i] = ega_palette[i];
        pal[32 + i] = ega_palette[0];
    }
    pal[17] = ega_palette[0];
    vid_wait_idle();
    PAL_fadeIn(0, 47, pal, 20, FALSE);
    screenfaded = FALSE;
}

void VW_ColorBorder(s16 color)
{
    PAL_setColor(32 + 1, ega_palette[color & 15]);
}

/* -------------------------------------------------------------- fizzle */

void FizzleFade(boolean abortable)
{
    u32 rndval = 1;
    u16 x, y, n;
    u16 base = vid_front ? TILE_VIEW1 * 32 : TILE_VIEW0 * 32;
    vu32 *ctrl = (vu32 *)VDP_CTRL_PORT;
    vu16 *data = (vu16 *)VDP_DATA_PORT;

    vid_wait_idle();
    do
    {
        if (abortable)
        {
            IN_ReadControl(&control);
            if (control.button0 || control.button1 || in_anykey())
                break;
        }
        for (n = 0; n < 340; n++)
        {
            /* 15 bit maximal LFSR over (x:7, y:8) word positions */
            y = rndval & 0xFF;
            x = rndval >> 8;
            if (rndval & 1)
                rndval = (rndval >> 1) ^ 0x6000;
            else
                rndval >>= 1;
            if (y >= 120 || x >= 80)
                continue;
            {
                u16 off = (x >> 1) * 480 + y * 4 + (x & 1) * 2;
                *ctrl = VDP_WRITE_VRAM_ADDR(base + off);
                *data = *(u16 *)(screenbuf + off);
            }
            if (rndval == 1)
                goto done;
        }
        vid_frame();
    } while (1);
done:
    /* copy whole page (also covers the aborted case) */
    vid_queue(viewsurf.buf, base, VIEW_BYTES);
    vid_wait_idle();
}
