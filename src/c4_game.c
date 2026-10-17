/* Catacomb Abyss Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * C4_GAME.C - Genesis port
 */
#include "def.h"
#include "dbg.h"
#include "snd.h"
#include "vid.h"
#include "draw2d.h"
#include "input.h"
#include "menu.h"

boolean tileneeded[NUMFLOORS];

void ScanInfoPlane (void)
{
    u16 x,y;
    s16 tile;
    const u16 *start;

    InitObjList();

    start = map_defs[gamestate.mapon].plane2;
    for (y=0;y<mapheight;y++)
        for (x=0;x<mapwidth;x++)
        {
            tile = *start++;
            tile &= 0xff;
            if (!tile)
                continue;

            switch (tile)
            {
            case 1:
            case 2:
            case 3:
            case 4:
                SpawnPlayer(x,y,NORTH+tile-1);
                break;

            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                SpawnBonus(x,y,tile-5);
                break;

            case 29:
                SpawnBonus(x,y,B_RKEY2);
            break;

            case 58:
            case 59:
            case 60:
            case 61:
            case 62:
                SpawnBonus(x,y,tile-58+B_RGEM);
            break;

            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
                SpawnBonus(x,y,B_SCROLL1+tile-12);
                break;

            case 21:
                SpawnBonus(x,y,B_CHEST);
            break;

            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
                SpawnWarp (x,y,tile-30);
                break;

            case 41:
                if (gamestate.difficulty <gd_Hard)
                    break;
            case 36:
                if (gamestate.difficulty <gd_Normal)
                    break;
            case 22:
                SpawnTroll (x,y);
                break;

            case 42:
                if (gamestate.difficulty <gd_Hard)
                    break;
            case 37:
                if (gamestate.difficulty <gd_Normal)
                    break;
            case 23:
                SpawnOrc (x,y);
                break;

            case 43:
                if (gamestate.difficulty <gd_Hard)
                    break;
            case 38:
                if (gamestate.difficulty <gd_Normal)
                    break;
            case 25:
                SpawnBat (x,y);
                break;

            case 44:
                if (gamestate.difficulty <gd_Hard)
                    break;
            case 39:
                if (gamestate.difficulty <gd_Normal)
                    break;
            case 26:
                SpawnDemon (x,y);
                break;

            case 45:
                if (gamestate.difficulty <gd_Hard)
                    break;
            case 40:
                if (gamestate.difficulty <gd_Normal)
                    break;
            case 27:
                SpawnMage (x,y);
                break;

            case 28:
                SpawnGrelminar (x,y);
                break;

            case 46:
            case 47:
            case 48:
                SpawnTombstone(x,y,tile-46);
            break;

            case 54:
                SpawnWarp(x,y,0);
                break;

            case 53:
                if (gamestate.difficulty <gd_Normal)
                    break;
            case 52:
                SpawnZombie(x,y);
            break;

            case 51:
                if (gamestate.difficulty <gd_Hard)
                    break;
            case 50:
                if (gamestate.difficulty <gd_Normal)
                    break;
            case 49:
                SpawnSpook(x,y);
                break;

            case 57:
                SpawnFTime(x,y);
                break;

            case 56:
                if (gamestate.difficulty <gd_Normal)
                    break;
            case 55:
                SpawnSkeleton(x,y);
                break;

            case 65:
                if (gamestate.difficulty <gd_Hard)
                    break;
            case 64:
                if (gamestate.difficulty <gd_Normal)
                    break;
            case 63:
                SpawnWetMan(x,y);
                break;

            case 68:
                if (gamestate.difficulty <gd_Hard)
                    break;
            case 67:
                if (gamestate.difficulty <gd_Normal)
                    break;
            case 66:
                SpawnEye(x,y);
                break;

            case 71:
                if (gamestate.difficulty <gd_Hard)
                    break;
            case 70:
                if (gamestate.difficulty <gd_Normal)
                    break;
            case 69:
                SpawnWallSkeleton(x,y);
                break;

            case 20:
            case 24:
            case 30:
                SpawnRedDemon (x,y);
                break;
            }
        }
}

static const char * const levelnames[] =
                {
                    "The Towne Cemetery",
                    "The Garden of Tears",
                    "The Den of Zombies",
                    "The Mausoleum Grounds",
                    "The Main Floor of the Mausoleum",
                    "Mike's Blastable Passage",
                    "The Crypt of Nemesis the Undead",
                    "The Subterranean Vault",
                    "The Ancient Aqueduct",
                    "The Orc Mines",
                    "The Lair of the Troll",
                    "The Demon's Inferno",
                    "The Battleground of the Titans",
                    "The Coven of Mages",
                    "The Inner Sanctum",
                    "The Haunt of Nemesis",
                    "The Passage to the Surface",
                    "Big Jim's Domain",
                    "Nolan's Nasty",
                };

const char *LevelName(s16 n)
{
    if (n >= 0 && n < (s16)(sizeof(levelnames)/sizeof(levelnames[0])))
        return levelnames[n];
    if (n >= 0 && n < NUMMAPDEFS)
        return map_defs[n].name;
    return "";
}

void DrawEnterScreen (void)
{
    s16 width;
    const char *name = LevelName(gamestate.mapon);

    vid_wait_idle();
    cursurf = &viewsurf;
    VW_Bar(0,0,VIEWWIDTH,VIEWHEIGHT,0);
    width = strlen(name);
    if (width < 20)
        width = 20;
    CenterWindow(width,5);
    US_CPrint("\nYou have arrived at\n");
    US_CPrint(name);
    vid_update();
    cursurf = &statsurf;
}

void CacheScaleds (void)
{
    /* all scaled graphics are ROM resident on the Genesis */
}

void SetupGameLevel (void)
{
    u16 x,y;
    u16 tile;
    const MapDef *md;
    const u8 *map;
    const u16 *spotptr;

    memset (tileneeded,0,sizeof(tileneeded));
    US_InitRndT(true);

    if (gamestate.mapon >= NUMMAPDEFS)
        gamestate.mapon = 0;
    md = &map_defs[gamestate.mapon];
    mapwidth = md->w;
    mapheight = md->h;

    memset (tilemap,0,sizeof(tilemap));
    memset (actorat,0,sizeof(actorat));
    map = md->plane0;
    spotptr = md->plane2;
    for (y=0;y<mapheight;y++)
        for (x=0;x<mapwidth;x++)
        {
            tile = *map++;
            MAP0(x,y) = tile;
            MAP2(x,y) = (*spotptr) >> 8;

            if (tile<NUMFLOORS)
            {
                tileneeded[tile] = true;
                tilemap[x][y] = tile;
                if (tile>0)
                    actorat[x][y] = tile;
            }
            spotptr++;
        }

    R_SealMap();
    zombie_base_delay = 0;
    ScanInfoPlane ();
}

void LatchDrawPic (u16 x, u16 y, u16 picnum)
{
    VW_DrawPic(x, y, picnum);
}

void Victory (boolean playsounds)
{
    if (playsounds)
    {
        SD_PlaySound (GETBOLTSND);
        SD_WaitSoundDone ();
        SD_PlaySound (GETNUKESND);
        SD_WaitSoundDone ();
        SD_PlaySound (GETPOTIONSND);
        SD_WaitSoundDone ();
        SD_PlaySound (GETKEYSND);
        SD_WaitSoundDone ();
        SD_PlaySound (GETSCROLLSND);
        SD_WaitSoundDone ();
        SD_PlaySound (GETPOINTSSND);
    }

    FreeUpMemory();

    if (!screenfaded)
        VW_FadeOut();

    vid_wait_idle();
    cursurf = &viewsurf;
    VW_Bar (0,0,320,120,0);
    VW_DrawPic(0, 0, FINALEPIC);
    vid_update();
    cursurf = &statsurf;

    VW_FadeIn();
}

void NormalScreen (void)
{
}

void DrawPlayScreen (void)
{
    vid_wait_idle();
    cursurf = &viewsurf;
    VW_Bar (0,0,320,VIEWHEIGHT,0);
    cursurf = &statsurf;
    VW_Bar (0,0,320,STATUSLINES,0);
    VW_DrawPic (0,0,STATUSPIC);
    RedrawStatusWindow ();
    vid_update();
}

void FizzleOut (s16 showlevel)
{
    if (showlevel)
        DrawEnterScreen ();
}

void FreeUpMemory (void)
{
}

void GameLoop (void)
{
    boolean wait = false;
    dbg_mark(3,1);

    DrawPlayScreen ();
    dbg_mark(4,1);
    IN_ClearKeysDown();

restart:
    if (!loadedgame)
    {
        gamestate.difficulty = restartgame;
        restartgame = gd_Continue;
        DrawEnterScreen ();
        if (gamestate.mapon != 8)
            fizzlein = true;
        wait = true;
    }

    do
    {
        playstate = ex_stillplaying;
        if (!loadedgame)
            SetupGameLevel ();
        else
            loadedgame = false;

        if (EASYMODEON)
            DisplaySMsg("*** NOVICE ***", NULL);
        else
            DisplaySMsg("*** WARRIOR ***", NULL);

        status_delay = 250;

        RedrawStatusWindow();
        if (wait)
        {
            VW_WaitVBL(120);
            wait = false;
        }

        PlayLoop ();
        dbg_mark(7,1);

        switch (playstate)
        {
        case ex_abort:
            FreeUpMemory ();
            return;
        case ex_resetgame:
            NewGame();
        case ex_loadedgame:
        case ex_warped:
            goto restart;
        break;
        default:
        break;
        }

    } while (1);
}
