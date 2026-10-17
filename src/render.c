/* render.c - the 3-D refresh (C4_DRAW.C / C4_SCALE.C replacement).
 *
 * The original traces wall *segments* (FollowWalls) and scales 64x64 EGA
 * textures with compiled 8086 scalers.  Here every 2-pixel screen column
 * casts a ray through the tile map (DDA) from the same focal point, with the
 * same projection constants (camera FOCALLENGTH behind the player, scale
 * 145, MINRATIO 16, heights clamped to MAXSCALEHEIGHT), and the walls are
 * drawn by compiled 68000 scalers generated from BuildCompScale()'s exact
 * pixel mapping.  Sprites use the original TransformActor()/ScaleShape()
 * maths and the column z-buffer.
 */
#include "def.h"
#include "vid.h"
#include "draw2d.h"
#include "input.h"

#define MINRATIO    16
#define WALLPIC(p)  (wall_tex + (u32)((p) - FIRSTWALLPIC) * 8192)

const fixed *sintable = sintable_init;
const fixed *costable = sintable_init + (ANGLES/4);

fixed   viewx,viewy;
s16     viewangle;
fixed   viewsin,viewcos;
s32     lasttimecount;
boolean fizzlein;
s16     zbuffer[VIEWWIDTH/2];

typedef struct { const u8 *tex; u16 scale; u8 u, side; u8 *hitp; } WallCol;
WallCol wallcols[160];

/* per frame parameters of R_CastCol() (layout shared with render_a.s) */
typedef struct
{
    s16 fine;
    u16 fxp, fxn, fyp, fyn;
    u16 vxl, vyl, pad;
    u8 *tp0;
    const s16 *light, *dark;
} CastFrame;
CastFrame cast_frame;
extern void R_CastCol_(u16 c);
#ifdef PROFILE
static u32 prof_casts, prof_steps;
#define R_CastCol(c) (prof_casts++, R_CastCol_(c))
#else
#define R_CastCol(c) R_CastCol_(c)
#endif

extern void R_FillSkyGround(u8 *buf, u32 sky, u32 ground);
extern void R_DrawWalls(const WallCol *cols, u8 *buf);
extern void R_ScaleColumn(const u8 *seg, u8 *dest, const u8 *rowsrc, const s16 *starts);

/* EAST / WEST WALLS */
static const s16 walllight1[NUMFLOORS] = {0,
    W_WARP1EWPIC,
    W_NEMPICEWPIC,W_PENTAEWPIC,W_ALTER_LFPIC,W_ALTER_RTPIC,
    W_SUB1EWPIC,W_SUB2EWPIC,W_SUB3EWPIC,
    W_TORCH1PIC,W_TORCH2PIC,
    W_LSUB_STONEPIC,
    W_BLOODY_LSUB_STONEPIC,
    W_BREATH_LWALL1PIC,W_BREATH_LWALL2PIC,
    EXPWALL1PIC,EXPWALL2PIC,EXPWALL3PIC,
    W_WOOD_DOORWAYPIC,W_WOOD_DOORWAY_GLOWPIC,
    W_WATER1EWPIC,W_DRAIN1EWPIC,
    W_WATER2EWPIC,W_DRAIN2EWPIC,
    W_WOODEN_DOORPIC,W_WOOD_DOOREWPIC,W_METAL_DOORPIC,W_GLOW_DOORPIC,
    W_FINALEXITPIC,
    W_WATER_EXP1PIC,W_WATER_EXP2PIC,W_WATER_EXP3PIC,
    W_PRE_CHEATSPIC,W_CHEAT_WARPPIC,W_CHEAT_FREEZEPIC,W_SURFACE_PLAQPIC,
    W_WATER_GATEEW1PIC,
    WALL8LPIC,WALL9LPIC,WALL10DPIC,WALL11LPIC,WALL12LPIC,WALL13LPIC,WALL14LPIC,WALL15LPIC,
    WALL16LPIC,WALL17LPIC,W_WINDOWEWPIC,WALL19LPIC,WALL20LPIC,WALL21LPIC,
    WALL22LPIC,WALL23LPIC,WALL24LPIC,WALL25LPIC,WALL26LPIC,WALL27LPIC,
    WALL28LPIC,WALL29LPIC,WALL30LPIC,WALL31LPIC,
    W_BREATH_LWALL4PIC,W_BREATH_LWALL3PIC,
    MAGE_STATUEPIC,ZOMBIE_STATUEPIC,EYE_STATUEPIC,NEM_STATUEPIC,
    SKELETON_STATUEPIC,SPOOK_STATUEPIC,ORCH_STATUEPIC,
    };

/* NORTH / SOUTH WALLS */
static const s16 walldark1[NUMFLOORS] = {0,
    W_WARP1NSPIC,
    W_NEMPICEWPIC,W_PENTANSPIC,1,1,
    W_SUB1NSPIC,W_SUB2NSPIC,W_SUB3NSPIC,
    W_TORCH1PIC,W_TORCH2PIC,
    W_DSUB_STONEPIC,
    W_BLOODY_DSUB_STONEPIC,
    W_BREATH_DWALL1PIC,W_BREATH_DWALL2PIC,
    EXPWALL1PIC,EXPWALL2PIC,EXPWALL3PIC,
    W_WOOD_DOORWAYPIC,W_WOOD_DOORWAY_GLOWPIC,
    W_WATER1NSPIC,W_DRAIN1NSPIC,
    W_WATER2NSPIC,W_DRAIN2NSPIC,
    W_WOODEN_DOORPIC,W_WOOD_DOORNSPIC,W_METAL_DOORPIC,W_GLOW_DOORPIC,
    W_FINALEXITPIC,
    W_WATER_EXP1PIC,W_WATER_EXP2PIC,W_WATER_EXP3PIC,
    W_CHEAT_GODPIC,W_CHEAT_ITEMSPIC,W_POST_CHEATPIC,W_SURFACE_PLAQPIC,
    W_WATER_GATENS1PIC,
    WALL8DPIC,WALL9DPIC,WALL10LPIC,WALL11DPIC,WALL12DPIC,WALL13DPIC,WALL14DPIC,WALL15DPIC,
    WALL16DPIC,WALL17DPIC,W_WINDOWNSPIC,WALL19DPIC,WALL20DPIC,WALL21DPIC,
    WALL22DPIC,WALL23DPIC,WALL24DPIC,WALL25DPIC,WALL26DPIC,WALL27DPIC,
    WALL28DPIC,WALL29DPIC,WALL30DPIC,WALL31DPIC,
    W_BREATH_DWALL4PIC,W_BREATH_DWALL3PIC,
    MAGE_STATUEPIC,ZOMBIE_STATUEPIC,EYE_STATUEPIC,NEM_STATUEPIC,
    SKELETON_STATUEPIC,SPOOK_STATUEPIC,ORCH_STATUEPIC,
    };

static const s16 walllight2[NUMFLOORS] = {0,
    W_WARP2EWPIC,
    W_NEMPICEWPIC,W_PENTAEWPIC,W_ALTER_LFPIC,W_ALTER_RTPIC,
    W_SUB1EWPIC,W_SUB2EWPIC,W_SUB3EWPIC,
    W_TORCH2PIC,W_TORCH1PIC,
    W_LSUB_STONEPIC,
    W_BLOODY_LSUB_STONEPIC,
    W_BREATH_LWALL2PIC,W_BREATH_LWALL1PIC,
    EXPWALL2PIC,EXPWALL1PIC,EXPWALL3PIC,
    W_WOOD_DOORWAYPIC,W_WOOD_DOORWAY_GLOWPIC,
    W_WATER2EWPIC,W_DRAIN2EWPIC,
    W_WATER1EWPIC,W_DRAIN1EWPIC,
    W_WOODEN_DOORPIC,W_WOOD_DOOREWPIC,W_METAL_DOORPIC,W_GLOW_DOORPIC,
    W_FINALEXITPIC,
    W_WATER_EXP2PIC,W_WATER_EXP1PIC,W_WATER_EXP1PIC,
    W_PRE_CHEATSPIC,W_CHEAT_WARPPIC,W_CHEAT_FREEZEPIC,W_SURFACE_PLAQPIC,
    W_WATER_GATEEW2PIC,
    WALL8LPIC,WALL9LPIC,WALL10DPIC,WALL11LPIC,WALL12LPIC,WALL13LPIC,WALL14LPIC,WALL15LPIC,
    WALL16LPIC,WALL17LPIC,W_WINDOWEWPIC,WALL19LPIC,WALL20LPIC,WALL21LPIC,
    WALL22LPIC,WALL23LPIC,WALL24LPIC,WALL25LPIC,WALL26LPIC,WALL27LPIC,
    WALL28LPIC,WALL29LPIC,WALL30LPIC,WALL31LPIC,
    W_BREATH_LWALL3PIC,W_BREATH_LWALL4PIC,
    MAGE_STATUEPIC,ZOMBIE_STATUEPIC,EYE_STATUEPIC,NEM_STATUEPIC,
    SKELETON_STATUEPIC,SPOOK_STATUEPIC,ORCH_STATUEPIC,
    };

static const s16 walldark2[NUMFLOORS] = {0,
    W_WARP2NSPIC,
    W_NEMPICEWPIC,W_PENTANSPIC,1,1,
    W_SUB1NSPIC,W_SUB2NSPIC,W_SUB3NSPIC,
    W_TORCH2PIC,W_TORCH1PIC,
    W_DSUB_STONEPIC,
    W_BLOODY_DSUB_STONEPIC,
    W_BREATH_DWALL2PIC,W_BREATH_DWALL1PIC,
    EXPWALL2PIC,EXPWALL1PIC,EXPWALL3PIC,
    W_WOOD_DOORWAYPIC,W_WOOD_DOORWAY_GLOWPIC,
    W_WATER2NSPIC,W_DRAIN2NSPIC,
    W_WATER1NSPIC,W_DRAIN1NSPIC,
    W_WOODEN_DOORPIC,W_WOOD_DOORNSPIC,W_METAL_DOORPIC,W_GLOW_DOORPIC,
    W_FINALEXITPIC,
    W_WATER_EXP2PIC,W_WATER_EXP1PIC,W_WATER_EXP1PIC,
    W_CHEAT_GODPIC,W_CHEAT_ITEMSPIC,W_POST_CHEATPIC,W_SURFACE_PLAQPIC,
    W_WATER_GATENS2PIC,
    WALL8DPIC,WALL9DPIC,WALL10LPIC,WALL11DPIC,WALL12DPIC,WALL13DPIC,WALL14DPIC,WALL15DPIC,
    WALL16DPIC,WALL17DPIC,W_WINDOWNSPIC,WALL19DPIC,WALL20DPIC,WALL21DPIC,
    WALL22DPIC,WALL23DPIC,WALL24DPIC,WALL25DPIC,WALL26DPIC,WALL27DPIC,
    WALL28DPIC,WALL29DPIC,WALL30DPIC,WALL31DPIC,
    W_BREATH_DWALL3PIC,W_BREATH_DWALL4PIC,
    MAGE_STATUEPIC,ZOMBIE_STATUEPIC,EYE_STATUEPIC,NEM_STATUEPIC,
    SKELETON_STATUEPIC,SPOOK_STATUEPIC,ORCH_STATUEPIC,
    };

/* ---------------------------------------------------------------- maths */

/* keep a value in a data register (avoids GCC narrowing a shifted 32 bit
 * memory operand into a word load at an odd address - illegal on 68000) */
static inline u32 inreg(u32 v)
{
    __asm__ ("" : "+d"(v));
    return v;
}

static inline u32 mulu16(u16 a, u16 b)
{
    u32 r = a;
    __asm__ ("mulu.w %1,%0" : "+d"(r) : "d"(b));
    return r;
}

static inline u16 divu16(u32 a, u16 b)
{
    u32 r = a;
    __asm__ ("divu.w %1,%0" : "+d"(r) : "d"(b));
    return (u16)r;
}

/* frac (0..65535) * d (< 2^24, 16.16) >> 16 */
static inline u32 fracmul(u16 frac, u32 d)
{
    return mulu16(frac, (u16)(d >> 16)) + (mulu16(frac, (u16)d) >> 16);
}

/* 16 bit unsigned x 32 bit unsigned >> 16 using two 16x16 multiplies */
static inline u32 mul32x16(u32 a, u16 b)
{
    a = inreg(a);
    return mulu16((u16)(a >> 16), b) + (mulu16((u16)a, b) >> 16);
}

/* FixedByFrac: multiply a 16.16 number by a fraction; like the original the
 * fraction's magnitude only uses its low 16 bits (sin 90 = 65535). */
fixed FixedByFrac(fixed a, fixed b)
{
    boolean neg = FALSE;
    u32 ua;
    u16 ub;
    if (a < 0) { ua = -a; neg = TRUE; } else ua = a;
    if (b < 0) { ub = (u16)(-b); neg ^= 1; } else ub = (u16)b;
    ua = mul32x16(ua, ub);
    return neg ? -(fixed)ua : (fixed)ua;
}

void BuildTables(void)
{
}

/* ------------------------------------------------------------- tracing */

static u16 animframe;

void R_SealMap(void)
{
    u16 x, y;
    memset(mapa.pad, SEALTILE, sizeof(mapa.pad));
    for (x = 0; x < MAPSIZE; x++)
        for (y = 0; y < MAPSIZEY; y++)
            if (x >= mapwidth || y >= mapheight)
                tilemap[x][y] = SEALTILE;
}

/* two casts hit the same wall face: the columns between them can be
 * interpolated (scale is linear in screen x across a plane) */
static inline boolean SameFace(const WallCol *a, const WallCol *b)
{
    return a->hitp && a->hitp == b->hitp && a->side == b->side;
}

static void Interp(u16 c, u16 shift)
{
    WallCol *a = &wallcols[c], *b = &wallcols[c + (1 << shift)];
    const u8 *base = a->tex - ((u16)a->u << 7);
    s16 ds = (s16)b->scale - (s16)a->scale;
    s16 du = (s16)b->u - (s16)a->u;
    u16 k, n = 1 << shift;
    for (k = 1; k < n; k++)
    {
        WallCol *w = a + k;
        u16 sc = a->scale + ((s16)(ds * k) >> shift);
        u16 u = a->u + ((s16)(du * k) >> shift);
        w->scale = sc;
        w->u = u;
        w->side = a->side;
        w->hitp = a->hitp;
        w->tex = base + (u << 7);
#ifdef PROFILE
        prof_steps++;
#endif
        zbuffer[c + k] = sc;
    }
}

static void CastRays(void)
{
    u16 c;
    s16 cx = viewx >> 16, cy = viewy >> 16;
    u16 fx = viewx & 0xFFFF, fy = viewy & 0xFFFF;

    memset(spotvis, 0, sizeof(spotvis));
#ifdef PROFILE
    prof_casts = prof_steps = 0;
#endif
    if (cx < 0) cx = 0;
    if (cy < 0) cy = 0;
    cast_frame.fine = viewangle * (FINEANGLES / ANGLES);
    cast_frame.fxp = 0xFFFF - fx;
    cast_frame.fxn = fx;
    cast_frame.fyp = 0xFFFF - fy;
    cast_frame.fyn = fy;
    cast_frame.vxl = fx;
    cast_frame.vyl = fy;
    cast_frame.tp0 = &tilemap[cx][cy];
    cast_frame.light = animframe ? walllight2 : walllight1;
    cast_frame.dark = animframe ? walldark2 : walldark1;

    /* coarse grid every 4th column, refined where the face changes */
    for (c = 0; c < 160; c += 4)
        R_CastCol(c);
    R_CastCol(159);
    for (c = 0; c < 156; c += 4)
    {
        WallCol *w = &wallcols[c];
        if (SameFace(w, w + 4))
            Interp(c, 2);
        else
        {
            R_CastCol(c + 2);
            if (SameFace(w, w + 2))
                Interp(c, 1);
            else
                R_CastCol(c + 1);
            if (SameFace(w + 2, w + 4))
                Interp(c + 2, 1);
            else
                R_CastCol(c + 3);
        }
    }
    R_CastCol(158);
    if (SameFace(&wallcols[156], &wallcols[158]))
        Interp(156, 1);
    else
        R_CastCol(157);
}

/* ------------------------------------------------------------- sprites */

static void TransformActor(objtype *ob)
{
    s32 ratio;
    fixed gx,gy,gxt,gyt,nx,ny;
    u32 any;
    s16 vx;

    gx = ob->x-viewx;
    gy = ob->y-viewy;

    gxt = FixedByFrac(gx,viewcos);
    gyt = FixedByFrac(gy,viewsin);
    nx = gxt-gyt-ob->size;

    gxt = FixedByFrac(gx,viewsin);
    gyt = FixedByFrac(gy,viewcos);
    ny = gyt+gxt;

    if (nx<0)
        nx = 0;

    /* nx (< 128 tiles) * PROJ_SCALE >> 16 without a 32 bit multiply */
    ratio = mulu16((u16)(inreg(nx) >> 16), PROJ_SCALE)
          + (mulu16((u16)nx, PROJ_SCALE) >> 16);

    if (ratio<=MINRATIO)
        ratio = MINRATIO;

    /* ny/ratio with a 16 bit divide; quotients beyond +-32000 are off
     * screen anyway */
    any = ny < 0 ? -ny : ny;
    if ((any >> 15) >= (u32)ratio)
        vx = 32000;
    else
        vx = divu16(any, ratio);
    ob->viewx = CENTERX + (ny < 0 ? -vx : vx);
    ob->viewheight = divu16(TILEGLOBAL, ratio);
}

static void ScaleShape(s16 xcenter, u16 shapenum, u16 viewheight)
{
    const u8 *shape;
    u16 scale, width, scalewidth, i;
    s16 toppix;
    s16 pixel, c0, c1, c;
    const s16 *starts;
    const u8 *rowsrc;
    u32 inv;

    if (shape_offsets[shapenum - FIRSTSCALEPIC] == 0xFFFFFFFF)
        return;
    shape = shape_blob + shape_offsets[shapenum - FIRSTSCALEPIC];

    scale = (viewheight + 1) / 2;
    if (!scale)
        return;
    if (scale > MAXSCALE)
        scale = MAXSCALE;

    width = (shape[0] << 8) | shape[1];
    starts = scale_starts + scale * 65;
    rowsrc = scale_rowsrc + scale * VIEWHEIGHT;
    toppix = (s16)(VIEWHEIGHT - 2 * (s16)scale) / 2;
    scalewidth = starts[width] - (s16)toppix;
    pixel = xcenter - scalewidth / 2;

    if (pixel >= VIEWWIDTH || pixel + (s16)scalewidth <= 0)
        return;

    /* 2-pixel columns covered by the shape */
    c0 = pixel >> 1;
    c1 = (pixel + scalewidth - 1) >> 1;
    if (c0 < 0) c0 = 0;
    if (c1 > 159) c1 = 159;
    inv = scale_hinv[scale];

    for (c = c0; c <= c1; c++)
    {
        s16 off;
        u16 src;
        if (zbuffer[c] > (s16)scale)
            continue;
        off = c * 2 + 1 - pixel;
        if (off < 0) off = 0;
        if (off >= (s16)scalewidth) off = scalewidth - 1;
        src = (u16)(mulu16((u16)(inv >> 16), off) + (mulu16((u16)inv, off) >> 16));
        if (src >= width) src = width - 1;
        i = (shape[2 + src * 2] << 8) | shape[3 + src * 2];
        R_ScaleColumn(shape + i, screenbuf + (c >> 2) * 480 + (c & 3), rowsrc, starts);
    }
}

static objtype *depthsort[MAXACTORS];

static void DrawScaleds(void)
{
    s16 i,j,least,numvisable,height;
    objtype *obj,**vislist,*farthest = NULL;

    numvisable = 0;
    vislist = &depthsort[0];

    for (obj = player->next;obj;obj=obj->next)
    {
        s16 tx = obj->tilex, ty = obj->tiley;
        boolean vis = FALSE;
        s16 ox, oy;
        if (tx < 1 || ty < 1 || tx >= MAPSIZE - 1 || ty >= MAPSIZEY - 1)
            vis = spotvis[tx & 63][ty & 31];
        else if (spotvis[tx][ty])
            vis = TRUE;
        else
        {
            for (ox = -1; ox <= 1 && !vis; ox++)
                for (oy = -1; oy <= 1; oy++)
                    if (spotvis[tx+ox][ty+oy] && !tilemap[tx+ox][ty+oy])
                    {
                        vis = TRUE;
                        break;
                    }
        }

        if (vis)
        {
            if ((obj->active == noalways) || (obj->active == always))
                obj->active = always;
            else
                obj->active = yes;
            TransformActor (obj);
            if (!obj->viewheight || obj->viewheight > VIEWWIDTH)
                continue;

            if (!obj->state->shapenum)
                continue;

            *vislist++ = obj;
            numvisable++;
        }
        else
            if ((obj->active != always) && (obj->active != noalways))
                obj->active = no;
    }

    if (vislist == &depthsort[0])
        return;

    for (i = 0; i<numvisable; i++)
    {
        least = 32000;
        for (j=0;j<numvisable;j++)
        {
            height = depthsort[j]->viewheight;
            if (height < least)
            {
                least = height;
                farthest = depthsort[j];
            }
        }
        ScaleShape(farthest->viewx,farthest->state->shapenum,farthest->viewheight);
        farthest->viewheight = 32000;
    }
}

/* ------------------------------------------------------------- timing */

void CalcTics (void)
{
    s32 newtime;

    if (lasttimecount > (s32)TimeCount)
        TimeCount = lasttimecount;

    newtime = TimeCount;
    /* the Genesis can't render faster than this in practice; still never
     * let the game run with 0 tics */
    while (newtime == lasttimecount)
    {
        vid_frame();
        newtime = TimeCount;
    }
    realtics = tics = newtime-lasttimecount;
    lasttimecount = newtime;

    if (tics>MAXTICS)
    {
        TimeCount -= (tics-MAXTICS);
        tics = MAXTICS;
    }

    if (realtics>MAXREALTICS)
        realtics = MAXREALTICS;
}

/* ---------------------------------------------------------------- hand */

static void DrawHand(void)
{
    cursurf = &viewsurf;
    VW_DrawMaskedLongs((VIEWWIDTH/16)-(10/2), VIEWHEIGHT - handheight,
        hand_data, HAND_WBYTES, HAND_H, handheight);
    cursurf = &statsurf;
}

/* ------------------------------------------------------- ThreeDRefresh */

#ifdef PROFILE
static u32 prof_t[9];
extern u16 wall_wpairs;
static inline u32 lines_now(void)
{
    u32 f;
    u16 v;
    SYS_disableInts();
    f = vid_frames;
    v = GET_VCOUNTER & 0xFF;
    SYS_enableInts();
    if (v >= 0xE0) f--;
    return f * 262 + v;
}

static void prof_show(void)
{
    u16 i;
    char b[8];
    for (i = 0; i < 8; i++)
    {
        u32 d = (i == 6) ? prof_casts : (i == 7) ? wall_wpairs : prof_t[i + 1] - prof_t[i];
        if (d > 9999) d = 9999;
        intToStr(d, b, 4);
        cursurf = &statsurf;
        {
            u16 k;
            for (k = 0; k < 4; k++)
                VW_DrawTile8(k + 1, 14 + i * 8, NUMBERCHARS + b[k] - '0');
        }
    }
}
#define PROF(n) prof_t[n] = lines_now()
#else
#define PROF(n)
#endif

void ThreeDRefresh (void)
{
    u16 sky, gnd;
    PROF(0);

    viewangle = player->angle;
    viewsin = sintable[viewangle];
    viewcos = costable[viewangle];
    viewx = player->x - FixedByFrac(FOCALLENGTH,viewcos);
    viewy = player->y + FixedByFrac(FOCALLENGTH,viewsin);
    viewx &= 0xfffffc00;
    viewy &= 0xfffffc00;
    viewx += 0x180;
    viewy += 0x180;

    animframe = (TimeCount&8)>>3;

    CastRays();
    PROF(1);

    /* the previous frame must be out of the RAM buffer before we draw */
    vid_wait_idle();
    PROF(2);

    sky = *skycolor & 15;
    gnd = *groundcolor & 15;
    R_FillSkyGround(screenbuf, sky * 0x11111111, gnd * 0x11111111);
    PROF(3);
    R_DrawWalls(wallcols, screenbuf);
    PROF(4);
    DrawScaleds();
    PROF(5);

    if (handheight)
        DrawHand();

    if (fizzlein)
    {
        fizzlein = false;
        FizzleFade(TRUE);
        lasttimecount = TimeCount;
    }
    else
        vid_present_view();

    CalcTics ();
    PROF(6);
#ifdef PROFILE
    prof_show();
#endif
}
