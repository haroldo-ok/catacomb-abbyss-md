/* Catacomb Abyss Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * C4_MAIN.C - Genesis port.  Saved games go to battery backed SRAM.
 */
#include "def.h"
#include "dbg.h"
#include "snd.h"
#include "vid.h"
#include "draw2d.h"
#include "input.h"
#include "menu.h"
#include "save.h"

GameDiff restartgame;
boolean loadedgame,abortgame,ingame;
boolean quitgame;

char        str[80],str2[20];
gametype    gamestate;
exittype    playstate;
s16         starting_level;
boolean     EASYMODEON;
u16         Flags=0;
boolean     restore_requested;

/* ------------------------------------------------------------ randomness */

static const u8 rndtable[256] = {
      0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
     74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
     95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
     52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
    149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
    145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
    175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
     25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
     94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
    136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
    135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
     80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
     24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
    145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
     28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
     71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
     17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
    197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
    120, 163, 236, 249 };
static u16 rndindex;

void US_InitRndT(boolean randomize)
{
    rndindex = randomize ? (random() & 0xff) : 0;
}

u16 US_RndT(void)
{
    rndindex = (rndindex + 1) & 0xff;
    return rndtable[rndindex];
}

s16 rnd(s16 n)
{
    if (n <= 0)
        return 0;
    return (s16)(((u32)random() * (u16)n) >> 16);
}

/* ----------------------------------------------------------------------- */

void NewGame (void)
{
    if (!loadedgame)
    {
        memset (&gamestate,0,sizeof(gamestate));
        gamestate.mapon = starting_level;
        gamestate.body = MAXBODY;
    }

    BGFLAGS = 0;
    Flags &= FL_CLEAR;

    boltsleft = bolttimer = 0;
    FreezeTime = 0;
}

/* --------------------------------------------------------- save / load */

extern const statetype * const all_states[];
extern const u16 num_states;

#define SAVE_MAGIC  0x43413133      /* 'CA13' */

typedef struct
{
    u32 magic;
    u16 used;
    u16 mapon;
    char desc[32];
} SaveHeader;

static u16 state_index(const statetype *st)
{
    u16 i;
    for (i = 0; i < num_states; i++)
        if (all_states[i] == st)
            return i;
    return 0xFFFF;
}

#define OBJ_SAVE_BYTES  ((u16)__builtin_offsetof(objtype, next))

boolean SaveTheGame(u16 slot)
{
    u32 a = save_slot_base(slot);
    SaveHeader h;
    objtype *o, tmp;
    u16 n = 0;

    h.magic = SAVE_MAGIC;
    h.used = 1;
    h.mapon = gamestate.mapon;
    memset(h.desc, 0, sizeof(h.desc));
    strncpy(h.desc, LevelName(gamestate.mapon), 31);

    save_begin();
    a = save_write(a, &h, sizeof(h));
    a = save_write(a, &FreezeTime, sizeof(FreezeTime));
    a = save_write(a, &gamestate, sizeof(gamestate));
    a = save_write(a, &EASYMODEON, sizeof(EASYMODEON));
    a = save_write(a, &BGFLAGS, sizeof(BGFLAGS));
    a = save_write(a, mapplane0, mapwidth * mapheight);
    a = save_write(a, mapplane2, mapwidth * mapheight);
    for (o = player; o; o = o->next) n++;
    a = save_write(a, &n, sizeof(n));
    for (o = player; o; o = o->next)
    {
        u16 si;
        tmp = *o;
        si = state_index(o->state);
        tmp.state = (const statetype *)(u32)si;
        a = save_write(a, &tmp, OBJ_SAVE_BYTES);
    }
    save_end();
    return true;
}

boolean LoadTheGame(u16 slot)
{
    u32 a = save_slot_base(slot);
    SaveHeader h;
    u16 i,x,y,n;
    objtype *prev,*next;

    save_begin();
    a = save_read(a, &h, sizeof(h));
    if (h.magic != SAVE_MAGIC || !h.used)
    {
        save_end();
        return false;
    }

    playstate = ex_loadedgame;
    a = save_read(a, &FreezeTime, sizeof(FreezeTime));
    a = save_read(a, &gamestate, sizeof(gamestate));
    a = save_read(a, &EASYMODEON, sizeof(EASYMODEON));
    a = save_read(a, &BGFLAGS, sizeof(BGFLAGS));
    save_end();

    SetupGameLevel ();

    save_begin();
    a = save_read(a, mapplane0, mapwidth * mapheight);
    a = save_read(a, mapplane2, mapwidth * mapheight);

    memset (tilemap,0,sizeof(tilemap));
    memset (actorat,0,sizeof(actorat));
    for (y=0;y<mapheight;y++)
        for (x=0;x<mapwidth;x++)
        {
            u16 tile = MAP0(x,y);
            if (tile<NUMFLOORS)
            {
                tilemap[x][y] = tile;
                if (tile>0)
                    actorat[x][y] = tile;
            }
        }

    R_SealMap();
    a = save_read(a, &n, sizeof(n));
    InitObjList ();
    new = player;
    for (i = 0; i < n; i++)
    {
        u16 si;
        prev = new->prev;
        next = new->next;
        a = save_read(a, new, OBJ_SAVE_BYTES);
        si = (u16)(u32)new->state;
        new->state = (si < num_states) ? all_states[si] : all_states[0];
        new->prev = prev;
        new->next = next;
        actorat[new->tilex][new->tiley] = ACTOROBJ(new);
        if (i + 1 < n)
            GetNewObj (false);
    }
    /* the player marker isn't kept in actorat */
    actorat[player->tilex][player->tiley] = 0;
    save_end();

    return(true);
}

void ResetGame(void)
{
    NewGame ();
}

/* ----------------------------------------------------------------------- */

void InitGame (void)
{
    static boolean done;
    if (!done)
    {
        done = TRUE;
        SD_Startup ();
        BuildTables ();
        save_init();
    }
    vid_init_game();
    fontcolor = WHITE;
    fontxor = TRUE;
}

void Quit (const char *error)
{
    if (error && *error)
        SYS_die((char *)error);
    quitgame = TRUE;
}

/* main game cycle, started by the intro with the chosen difficulty;
 * returns FALSE when a restore from the title screen was cancelled */
boolean DemoLoop (void)
{
    dbg_mark(1,1);
    InitGame();
    dbg_mark(2,1);
    vid_set_palette_normal();       /* a fresh EGA mode: black screen */
    loadedgame = false;
    playstate = ex_stillplaying;

    if (restore_requested)
    {
        restore_requested = FALSE;
        PreFullDisplay();
        if (!GE_LoadGame())
            return FALSE;
        loadedgame = true;
        playstate = ex_loadedgame;
        vid_fullscreen(false);
        cursurf = &statsurf;
        vid_set_palette_black();
    }

    restartgame = gd_Normal;
    NewGame();
    GameLoop();
    VW_FadeOut();
    return TRUE;
}
