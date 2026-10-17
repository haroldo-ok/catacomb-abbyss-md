/* Catacomb Abyss Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * C4_WIZ.C - player, shots, status bar.  Genesis port.
 */
#include "def.h"
#include "snd.h"
#include "vid.h"
#include "draw2d.h"
#include "input.h"
#include "menu.h"

/* forward declarations of every state in this file */
extern const statetype s_pshot1;
extern const statetype s_pshot2;
extern const statetype s_pshot_exp1;
extern const statetype s_pshot_exp2;
extern const statetype s_pshot_exp3;
extern const statetype s_player;
extern const statetype s_explode;


#define NUMSCROLLS  8
#define NUKETIME    40
#define NUMBOLTS    10
#define BOLTTICS    6

#define STATUSCOLOR 8
#define TEXTCOLOR   14

#define SHOTDAMAGE      1
#define BIGSHOTDAMAGE   3

#define PLAYERSPEED 5120
#define SHOTSPEED   10000

#define LASTTILE  (LASTWALLPIC-FIRSTWALLPIC)

#define HANDPAUSE   60

s32     lastnuke,lasthand;
s16     lasttext;
s16     handheight;
s16     boltsleft,bolttimer;
s16     RadarXY[MAX_RADAR_BLIPS][3]={{-1,-1,-1}};
static const s16 radarx=RADARX,radary=RADARY,radar_xcenter=RADAR_XCENTER,radar_ycenter=RADAR_YCENTER;
static const s16 key_x[4]={20,23,23,20},key_y[4]={30,57,30,57};

boolean redraw_gems,button0down;
boolean autofire;

s16 FreezeTime = 0;

void ClipMove (objtype *ob, s32 xmove, s32 ymove);
boolean VerifyGateExit(void);
void DrawGems(void);
void DrawNum(s16 x,s16 y,s16 value,s16 maxdigits);
void DrawScrolls(void);
void TakeBolt (void);
void TakeNuke (void);
void TakePotion (void);
void TakeKey (s16 keytype);
void Shoot (void);
void T_Player (objtype *ob);
void T_Pshot (objtype *ob);
void CacheScaleds (void);
void FreeUpMemory (void);

/* ---------------------------------------------------------------------- */

void DrawChar (u16 x, u16 y, u16 tile)
{
    VW_DrawTile8(x, y, tile);
}

void RedrawStatusWindow (void)
{
    s16 keytype;

    cursurf = &statsurf;
    for (keytype=0; keytype<4; keytype++)
        DrawNum(key_x[keytype],key_y[keytype],gamestate.keys[keytype],2);
    DrawNum(17,54,gamestate.potions,2);
    DrawNum(17,36,gamestate.nukes,2);
    DrawNum(17,18,gamestate.bolts,2);

    DrawHealth();
    DrawRadar();
    DrawGems();
    DrawScrolls();
    redraw_gems = false;
}

void GiveBolt (void)
{
    if (gamestate.bolts == 99)
        return;

    SD_PlaySound (GETBOLTSND);
    DrawNum(17,18,++gamestate.bolts,2);
}

void TakeBolt (void)
{
    SD_PlaySound (USEBOLTSND);
    DrawNum(17,18,--gamestate.bolts,2);
}

void GiveNuke (void)
{
    if (gamestate.nukes == 99)
        return;

    SD_PlaySound (GETNUKESND);
    DrawNum(17,36,++gamestate.nukes,2);
}

void TakeNuke (void)
{
    SD_PlaySound (USENUKESND);
    DrawNum(17,36,--gamestate.nukes,2);
}

void GivePotion (void)
{
    if (gamestate.potions == 99)
        return;

    SD_PlaySound (GETPOTIONSND);
    DrawNum(17,54,++gamestate.potions,2);
}

void TakePotion (void)
{
    SD_PlaySound (USEPOTIONSND);
    DrawNum(17,54,--gamestate.potions,2);
}

void GiveKey (s16 keytype)
{
    if (gamestate.keys[keytype] == 99)
        return;

    SD_PlaySound (GETKEYSND);
    DrawNum(key_x[keytype],key_y[keytype],++gamestate.keys[keytype],2);
}

void TakeKey (s16 keytype)
{
    static const char * const key_colors[] = {"a RED key",
                                 "a YELLOW key",
                                 "a GREEN key",
                                 "a BLUE key"};

    SD_PlaySound (USEKEYSND);
    DrawNum(key_x[keytype],key_y[keytype],--gamestate.keys[keytype],2);
    CenterWindow(20,5);
    US_CPrint("\nYou use\n");
    US_CPrint(key_colors[keytype]);
    vid_update();
    VW_WaitVBL(120);
}

void DrawGems(void)
{
    s16 loop;

    redraw_gems = false;

    cursurf = &statsurf;
    LatchDrawPic (31,51,RADAR_BOTTOMPIC);
    for (loop=0; loop<5; loop++)
        if (gamestate.gems[loop])
            LatchDrawPic (32+loop,53,RADAR_RGEMPIC+loop);
}

void GiveScroll (s16 scrolltype,boolean show)
{
    s16 x,y;

    SD_PlaySound (GETSCROLLSND);
    gamestate.scrolls[scrolltype] = true;

    y = 30 + ((scrolltype > 3) * 10);
    x = 26 + (scrolltype % 4);
    cursurf = &statsurf;
    DrawChar(x,y,SCROLLCHARS+scrolltype);

    if (show)
        ReadScroll(scrolltype);
}

void DrawScrolls(void)
{
    s16 loop,x,y;

    cursurf = &statsurf;
    VW_Bar(210,30,30,18,0xf);

    for (loop=0;loop<8;loop++)
        if (gamestate.scrolls[loop])
        {
            y = 30 + ((loop > 3) * 10);
            x = 26 + (loop % 4);
            DrawChar(x,y,SCROLLCHARS+loop);
        }
}

void DrawHealth(void)
{
    u16 picnum;
    s16 percentage;

    percentage = (u16)(100*((gamestate.body<<9)/MAXBODY))>>9;

    DrawNum(9,57,percentage,3);

    if (percentage > 75)
        picnum = FACE1PIC;
    else
    if (percentage > 50)
        picnum = FACE2PIC;
    else
    if (percentage > 25)
        picnum = FACE3PIC;
    else
    if (percentage)
        picnum = FACE4PIC;
    else
        picnum = FACE5PIC;

    cursurf = &statsurf;
    if (!percentage)
        VW_DrawPic(8,14,picnum);
    else
        LatchDrawPic(8,14,picnum);
}

void DrawFreezeTime(void)
{
    s32 percentage;
    percentage = (u16)(100*(((s32)FreezeTime<<7)/MAXFREEZETIME))>>7;
    DrawNum(23,70,percentage,3);
}

void DrawNum(s16 x,s16 y,s16 value,s16 maxdigits)
{
    char str[10];
    s16 len,i;

    intToStr(value,str,1);
    len=strlen(str);

    cursurf = &statsurf;
    for (i=len; i<maxdigits; i++)
        DrawChar(x++,y,BLANKCHAR);

    for (i=0;i<len;i++)
        DrawChar(x++,y,NUMBERCHARS+str[i]-'0');
}

void GiveChest(void)
{
    s16 i;

    for (i=0;i<rnd(4);i++)
    {
        GiveBolt();
        SD_WaitSoundDone();
    }

    for (i=0;i<rnd(3);i++)
    {
        GiveNuke();
        SD_WaitSoundDone();
    }

    for (i=0;i<rnd(2);i++)
    {
        GivePotion();
        SD_WaitSoundDone();
    }
}

void GiveGoal (void)
{
    SD_PlaySound (GETPOINTSSND);
    playstate = ex_victorious;
}

void DrawText (boolean draw_text_whether_it_needs_it_or_not)
{
    u16 number;
    char str[81];

    number = MAP0(player->tilex,player->tiley)-NAMESTART;

    if ( number>26 )
        number = 0;

    if ((number == lasttext) && (!draw_text_whether_it_needs_it_or_not))
        return;

    lasttext = number;

    strncpy(str, level_texts[gamestate.mapon][number], 80);
    str[80] = 0;
    DisplayMsg(str,NULL);
}

/* The original DisplayMsg/DisplaySMsg print on the status bar and, given a
 * list of choice keys, wait for one of them.  On the Genesis the choices
 * are picked with the pad (see menu.c). */
char DisplayMsg(const char *text,const char *choices)
{
    char ch=true;
    byte temp;

    cursurf = &statsurf;
    PrintY = 1;
    WindowX = 20;
    WindowW = 270;

    VW_Bar (WindowX,2,WindowW,8,STATUSCOLOR);
    temp = fontcolor;
    fontcolor = TEXTCOLOR^STATUSCOLOR;
    if (text && !choices)
        US_CPrintLine (text);
    fontcolor = temp;

    if (choices)
        ch = MenuChoice(text, choices);

    return(ch);
}

char DisplaySMsg(const char *text,const char *choices)
{
    char ch=true;
    byte temp;

    cursurf = &statsurf;
    PrintY = 69;
    WindowX = 98;
    WindowW = 115;

    VW_Bar(WindowX,PrintY+1,WindowW,8,STATUSCOLOR);
    temp = fontcolor;
    fontcolor = TEXTCOLOR^STATUSCOLOR;
    if (text)
        US_CPrintLine (text);
    fontcolor = temp;

    if (choices)
        ch = MenuChoice(text, choices);

    return(ch);
}

void DrawRadar (void)
{
    s16 objnum;

    cursurf = &statsurf;
    LatchDrawPic (radarx,radary,RADAR_TOPPIC);

    objnum = 0;
    while (RadarXY[objnum][2] != -1)
    {
        VW_Plot(radar_xcenter+RadarXY[objnum][0],radar_ycenter+RadarXY[objnum][1],RadarXY[objnum][2]);
        objnum++;
    }
}

void DrawNSEWIcons(void)
{
    s16 x,y;

    x = -FixedByFrac(RADAR_X_IRADIUS,costable[player->angle]);
    y = -FixedByFrac(RADAR_Y_IRADIUS,sintable[player->angle]);

    cursurf = &statsurf;
    VW_DrawSpriteNorth(radar_xcenter+x-3,radar_ycenter+y-3);
}

/*
=============================================================================

                            SHOTS

=============================================================================
*/

extern const statetype s_pshot1;
extern const statetype s_pshot2;

const statetype s_pshot1 = {PSHOT1PIC,8,&T_Pshot,&s_pshot2};
const statetype s_pshot2 = {PSHOT2PIC,8,&T_Pshot,&s_pshot1};

const statetype s_pshot_exp1 = {PSHOT_EXP1PIC,7,NULL,&s_pshot_exp2};
const statetype s_pshot_exp2 = {PSHOT_EXP2PIC,7,NULL,&s_pshot_exp3};
const statetype s_pshot_exp3 = {PSHOT_EXP3PIC,7,NULL,NULL};

void SpawnPShot (void)
{
    DSpawnNewObjFrac (player->x,player->y,&s_pshot1,PIXRADIUS*7);
    new->obclass = pshotobj;
    new->speed = SHOTSPEED;
    new->angle = player->angle;
    new->active = always;
}

boolean JimsShotClipMove (objtype *ob, s32 xmove, s32 ymove)
{
    s16 xl,yl,xh,yh,x,y;
    objtype *check;

    xl = ob->xl>>TILESHIFT;
    yl = ob->yl>>TILESHIFT;

    xh = ob->xh>>TILESHIFT;
    yh = ob->yh>>TILESHIFT;

    for (y=yl;y<=yh;y++)
        for (x=xl;x<=xh;x++)
        {
            u16 v = actorat[x][y];

            if (!ISACTOROBJ(v))
                continue;
            check = ACTORPTR(v);
            if ((check == player) || (!(check->flags & of_shootable)))
                continue;

            ob->x -= xmove;
            ob->y -= ymove;

            if (check->obclass != solidobj)
            {
                SD_PlaySound (SHOOTMONSTERSND);
                if (ob->obclass == bigpshotobj)
                    ShootActor (check,BIGSHOTDAMAGE);
                else
                    ShootActor (check,SHOTDAMAGE);
            }
            ob->state = &s_pshot_exp1;
            ob->ticcount = ob->state->tictime;
            return(true);
        }

    return(false);
}

void T_Pshot (objtype *ob)
{
    objtype *check;
    s32 xmove,ymove,speed;

    for (check = player->next; check; check=check->next)
        if ((check->flags & of_shootable)
        && ob->xl <= check->xh
        && ob->xh >= check->xl
        && ob->yl <= check->yh
        && ob->yh >= check->yl)
        {
            if (check->obclass != solidobj)
            {
                SD_PlaySound (SHOOTMONSTERSND);
                if (ob->obclass == bigpshotobj)
                    ShootActor (check,BIGSHOTDAMAGE);
                else
                    ShootActor (check,SHOTDAMAGE);
            }

            ob->state = &s_pshot_exp1;
            ob->obclass = expobj;
            ob->ticcount = ob->state->tictime;
            return;
        }

    speed = ob->speed*tics;

    xmove = FixedByFrac(speed,costable[ob->angle]);
    ymove = -FixedByFrac(speed,sintable[ob->angle]);

    if (ShotClipMove(ob,xmove,ymove))
    {
        ob->state = &s_pshot_exp1;
        ob->obclass = expobj;
        ob->ticcount = ob->state->tictime;
        return;
    }

    ob->tilex = ob->x >> TILESHIFT;
    ob->tiley = ob->y >> TILESHIFT;

    /* the original passes the global 'obj' (the object being thought) */
    JimsShotClipMove(obj,xmove,ymove);
}

/*
=============================================================================

                            PLAYER ACTIONS

=============================================================================
*/

void ClearShotPower (void)
{
    gamestate.shotpower = 0;
}

void Shoot (void)
{
    ClearShotPower ();
    SD_PlaySound (SHOOTSND);
    SpawnPShot ();
}

void CastBolt (void)
{
    if (!gamestate.bolts)
    {
        SD_PlaySound (NOITEMSND);
        return;
    }

    TakeBolt ();
    boltsleft = NUMBOLTS;
    bolttimer = BOLTTICS;
    Shoot ();
}

void ContinueBolt (void)
{
    bolttimer-=realtics;
    if (bolttimer<0)
    {
        boltsleft--;
        bolttimer = BOLTTICS;
        Shoot ();
    }
}

void CastNuke (void)
{
    s16 angle;

    if (!gamestate.nukes)
    {
        SD_PlaySound (NOITEMSND);
        return;
    }

    TakeNuke ();
    lastnuke = TimeCount;

    for (angle = 0; angle < ANGLES; angle+= ANGLES/16)
    {
        DSpawnNewObjFrac (player->x,player->y,&s_pshot1,24*PIXRADIUS);
        new->obclass = bigpshotobj;
        new->speed = SHOTSPEED;
        new->angle = angle;
        new->active = always;
    }
}

void DrinkPotion (void)
{
    if (!gamestate.potions)
    {
        SD_PlaySound (NOITEMSND);
        return;
    }

    DisplaySMsg("Curing", NULL);
    TakePotion ();
    gamestate.body = MAXBODY;
    VW_WaitVBL(30);
    status_flag    = S_NONE;
}

void ReadScroll (s16 scroll)
{
    const u16 *skytemp,*gndtemp;
    static const u16 blackcolor=0;

    DisplaySMsg("Reading Scroll", NULL);

    if (status_flag != S_TIMESTOP)
        status_flag = S_NONE;

    skytemp = skycolor;
    gndtemp = groundcolor;
    skycolor = groundcolor = &blackcolor;

    vid_wait_idle();
    cursurf = &viewsurf;
    VW_Bar(0,0,VIEWWIDTH,VIEWHEIGHT,0);
    VW_DrawPic (10,0,SCROLLTOPPIC);
    VW_DrawPic (10,32,SCROLL1PIC + scroll);
    VW_DrawPic (10,88,SCROLLBOTTOMPIC);
    cursurf = &statsurf;

    skycolor = skytemp;
    groundcolor = gndtemp;

    lasttext = -1;
    DisplayMsg("Press B or START to exit.",NULL);
    vid_update();
    IN_ClearKeysDown ();
    while (!(IN_WaitPress() & (BUTTON_B|BUTTON_START|BUTTON_C)))
        ;
    IN_ClearKeysDown ();

    if (status_flag == S_TIMESTOP)
        DisplaySMsg("Time Stopped:     ",NULL);
}

void StopTime(void)
{
    FreezeTime = MAXFREEZETIME;
    SD_PlaySound(FREEZETIMESND);
    DisplaySMsg("Time Stopped:     ",NULL);
    status_flag = S_TIMESTOP;
}

void TakeDamage (s16 points)
{
    if (!gamestate.body || (bordertime && bcolor==FLASHCOLOR) || godmode)
        return;

    if (points >= gamestate.body)
    {
        points = gamestate.body;
        Flags |= FL_DEAD;
    }

    bordertime = FLASHTICS<<2;
    bcolor = FLASHCOLOR;
    VW_ColorBorder (FLASHCOLOR);

    DisplaySMsg("Damaging blows!", NULL);
    status_flag  = S_NONE;
    status_delay = 80;

    if (gamestate.body<MAXBODY/3)
        SD_PlaySound (TAKEDMGHURTSND);
    else
        SD_PlaySound (TAKEDAMAGESND);

    gamestate.body -= points;
}

/*
=============================================================================

                            INTERACTION

=============================================================================
*/

boolean HitSpecialTile (u16 x, u16 y, u16 tile)
{
    s16 keyspot;
    u16 spot,newlevel = 0;
    static const char * const key_colors[] = {"a RED key",
                                 "a YELLOW key",
                                 "a GREEN key",
                                 "a BLUE key"};

    switch (tile)
    {
        case 28:
            playstate = ex_victorious;
        break;

        case 36:            // Water gate
        case 24:            // Steel gate (BLUE KEY REQ.)
        case 25:            // Steel gate (RED KEY REQ.)
        case 26:            // Steel gate (YELLOW KEY REQ.)
        case 27:            // HOT Steel gate (YELLOW KEY REQ.)
        case 18:            // Wooden Doorway
        case 19:            // Wooden doorway with a GLOW
        case 1:             // tile warp
        case 54:            // DOWN stairs
        case 56:            // UP stairs
        case 51:            // generic door
            if (!playstate && !FreezeTime)
            {
                spot = MAP2(x,y);
                if (spot == CANT_OPEN_CODE)
                {
                    CenterWindow(20,4);
                    US_CPrint("\nThe door is blocked");
                    vid_update();
                    IN_ClearKeysDown();
                    IN_Ack();
                    return true;
                }

                if (TILE_FLAGS(tile) & tf_EMBEDDED_KEY_COLOR)
                    keyspot = GATE_KEY_COLOR(tile);
                else
                    keyspot = MAP2(x,y+1);

                if (keyspot--)
                    if (!gamestate.keys[keyspot])
                    {
                        SD_PlaySound(HIT_GATESND);
                        CenterWindow(20,5);
                        US_CPrint("\nYou need\n");
                        US_CPrint(key_colors[keyspot]);
                        vid_update();
                        IN_ClearKeysDown();
                        IN_Ack();
                        return true;
                    }

                switch (spot)
                {
                    case NEXT_LEVEL_CODE:
                        newlevel = gamestate.mapon+1;
                        playstate = ex_warped;
                    break;

                    case REMOVE_DOOR_CODE:
                        actorat[x][y] = tilemap[x][y] = MAP0(x,y) = 0;
                        MAP2(x,y+1) = 0;
                        if (keyspot>=0)
                            TakeKey(keyspot);
                    break;

                    default:
                        newlevel = spot;
                        playstate = ex_warped;
                    break;
                }

                if (playstate == ex_warped)
                {
                    SD_PlaySound(HIT_GATESND);

                    if (!VerifyGateExit())
                    {
                        IN_ClearKeysDown ();
                        playstate = ex_stillplaying;
                        break;
                    }

                    if (keyspot>=0)
                        TakeKey(keyspot);
                    MAP2(x,y+1) = 0;

                    gamestate.mapon = newlevel;
                    SD_PlaySound(WARPUPSND);
                    IN_ClearKeysDown ();
                }
            }
        break;
    }

    return true;
}

boolean VerifyGateExit(void)
{
    char ch;

    ch=DisplayMsg("Pass this way?","YN");
    DrawText(true);

    return(ch == 'Y');
}

boolean TouchActor (objtype *ob, objtype *check)
{
    if (ob->xh < check->xl || ob->xl > check->xh ||
        ob->yh < check->yl || ob->yl > check->yh)
        return false;

    switch (check->obclass)
    {
        case bonusobj:
            switch (check->temp1)
            {
                case B_BOLT:        GiveBolt ();        break;

                case B_NUKE:        GiveNuke ();        break;

                case B_POTION:      GivePotion ();      break;

                case B_RKEY2:       GiveKey(B_RKEY-B_RKEY);                 break;

                case B_RKEY:
                case B_YKEY:
                case B_GKEY:
                case B_BKEY:        GiveKey (check->temp1-B_RKEY);      break;

                case B_SCROLL1:
                case B_SCROLL2:
                case B_SCROLL3:
                case B_SCROLL4:
                case B_SCROLL5:
                case B_SCROLL6:
                case B_SCROLL7:
                case B_SCROLL8: GiveScroll (check->temp1-B_SCROLL1,true);   break;

                case B_CHEST:       GiveChest ();       break;

                case B_RGEM:
                case B_YGEM:
                case B_GGEM:
                case B_BGEM:
                case B_PGEM:
                    SD_PlaySound(GETGEMSND);
                    gamestate.gems[check->temp1-B_RGEM] = GEM_DELAY_TIME;
                    redraw_gems = true;
                break;

                default:
                    Quit("TouchActor(): INVALID BONUS");
                break;
            }

            actorat[check->tilex][check->tiley] = 0;
            RemoveObj (check);

            return false;
        break;

        case freezeobj:
            StopTime();
            actorat[check->tilex][check->tiley] = 0;
            RemoveObj(check);
            return(false);
        break;
    }

    return  true;
}

void CalcBounds (objtype *ob)
{
  ob->xl = ob->x - ob->size;
  ob->xh = ob->x + ob->size;
  ob->yl = ob->y - ob->size;
  ob->yh = ob->y + ob->size;
}

boolean LocationInActor (objtype *ob)
{
    s16 x,y,xmin,ymin,xmax,ymax;
    objtype *check;

    CalcBounds (ob);

    xmin = (ob->x >> TILESHIFT)-2;
    ymin = (ob->y >> TILESHIFT)-2;
    xmax = xmin+5;
    ymax = ymin+5;

    for (x=xmin;x<xmax;x++)
        for (y=ymin;y<ymax;y++)
        {
            u16 v;
            if (x < 0 || y < 0 || x >= MAPSIZE || y >= MAPSIZEY)
                continue;
            v = actorat[x][y];
            if (!ISACTOROBJ(v))
                continue;
            check = ACTORPTR(v);
            if ((check->flags & of_shootable)
                && ob->xl <= check->xh
                && ob->xh >= check->xl
                && ob->yl <= check->yh
                && ob->yh >= check->yl)
                    return true;
        }

    return false;
}

void ClipXMove (objtype *ob, s32 xmove)
{
    s16 xl,yl,xh,yh,x,y;
    s32 basex,basey;
    boolean moveok;

    basex = ob->x;
    basey = ob->y;

    ob->x += xmove;

    CalcBounds (ob);

    xl = ob->xl>>TILESHIFT;
    yl = ob->yl>>TILESHIFT;

    xh = ob->xh>>TILESHIFT;
    yh = ob->yh>>TILESHIFT;

    for (y=yl;y<=yh;y++)
        for (x=xl;x<=xh;x++)
        {
            u16 check = actorat[x][y];

            if (!check)
                continue;

            if (check <= LASTTILE)
            {
                if (TILE_FLAGS(check) & tf_SPECIAL)
                {
                    HitSpecialTile(x,y,check);
                    goto blockmove;
                }

                if (TILE_FLAGS(check) & tf_SOLID)
                {
                    goto blockmove;
                }
            }

            if (ISACTOROBJ(check))
                TouchActor(ob,ACTORPTR(check));
        }

    if (LocationInActor(ob))
    {
        ob->x -= xmove;
        if (LocationInActor(ob))
        {
            ob->x += xmove;
            if (LocationInActor(ob))
                ob->x -= xmove;
        }
    }
    return;

blockmove:

    moveok = false;

    do
    {
        xmove /= 2;
        if (moveok)
        {
            ob->x += xmove;
        }
        else
        {
            ob->x -= xmove;
        }
        CalcBounds (ob);
        xl = ob->xl>>TILESHIFT;
        yl = ob->yl>>TILESHIFT;
        xh = ob->xh>>TILESHIFT;
        yh = ob->yh>>TILESHIFT;
        if (tilemap[xl][yl] || tilemap[xh][yl]
        || tilemap[xh][yh] || tilemap[xl][yh] )
        {
            moveok = false;
            if (xmove>=-2048 && xmove <=2048)
            {
                ob->x = basex;
                ob->y = basey;
                return;
            }
        }
        else
        {
            if (xmove>=-2048 && xmove <=2048)
                return;
            moveok = true;
        }
    } while (1);
}

void ClipYMove (objtype *ob, s32 ymove)
{
    s16 xl,yl,xh,yh,x,y;
    s32 basex,basey;
    boolean moveok;

    basex = ob->x;
    basey = ob->y;

    ob->y += ymove;

    CalcBounds (ob);

    xl = ob->xl>>TILESHIFT;
    yl = ob->yl>>TILESHIFT;

    xh = ob->xh>>TILESHIFT;
    yh = ob->yh>>TILESHIFT;

    for (y=yl;y<=yh;y++)
        for (x=xl;x<=xh;x++)
        {
            u16 check = actorat[x][y];
            if (!check)
                continue;

            if (check <= LASTTILE)
            {
                if (TILE_FLAGS(check) & tf_SPECIAL)
                {
                    HitSpecialTile (x,y,check);
                    goto blockmove;
                }

                if (TILE_FLAGS(check) & tf_SOLID)
                {
                    goto blockmove;
                }
            }

            if (ISACTOROBJ(check))
                TouchActor(ob,ACTORPTR(check));
        }

    if (LocationInActor(ob))
    {
        if (LocationInActor(ob))
        {
            ob->y -= ymove;
        }
    }
    return;

blockmove:

    moveok = false;

    do
    {
        ymove /= 2;
        if (moveok)
        {
            ob->y += ymove;
        }
        else
        {
            ob->y -= ymove;
        }
        CalcBounds (ob);
        xl = ob->xl>>TILESHIFT;
        yl = ob->yl>>TILESHIFT;
        xh = ob->xh>>TILESHIFT;
        yh = ob->yh>>TILESHIFT;
        if (tilemap[xl][yl] || tilemap[xh][yl]
        || tilemap[xh][yh] || tilemap[xl][yh] )
        {
            moveok = false;
            if (ymove>=-2048 && ymove <=2048)
            {
                ob->x = basex;
                ob->y = basey;
                return;
            }
        }
        else
        {
            if (ymove>=-2048 && ymove <=2048)
                return;
            moveok = true;
        }
    } while (1);
}

boolean ShotClipMove (objtype *ob, s32 xmove, s32 ymove)
{
    s16 xl,yl,xh,yh,x,y;
    s32 basex,basey;
    u16 spot,tile;
    boolean moveok;

    basex = ob->x;
    basey = ob->y;

    ob->x += xmove;
    ob->y += ymove;

    CalcBounds (ob);

    xl = ob->xl>>TILESHIFT;
    yl = ob->yl>>TILESHIFT;

    xh = ob->xh>>TILESHIFT;
    yh = ob->yh>>TILESHIFT;

    for (y=yl;y<=yh;y++)
        for (x=xl;x<=xh;x++)
        {
            spot = MAP2(x,y);
            if (spot == EXP_WALL_CODE)
                switch (ob->obclass)
                {
                    case pshotobj:
                    case bigpshotobj:
                        ExplodeWall (x,y);
                        goto blockmove;
                    break;
                }

            tile = MAP0(x,y);
            if (TILE_FLAGS(tile) & tf_SOLID)
                goto blockmove;
        }
    return false;

blockmove:

    SD_PlaySound (SHOOTWALLSND);

    moveok = false;

    do
    {
        xmove /= 2;
        ymove /= 2;
        if (moveok)
        {
            ob->x += xmove;
            ob->y += ymove;
        }
        else
        {
            ob->x -= xmove;
            ob->y -= ymove;
        }
        CalcBounds (ob);
        xl = ob->xl>>TILESHIFT;
        yl = ob->yl>>TILESHIFT;
        xh = ob->xh>>TILESHIFT;
        yh = ob->yh>>TILESHIFT;
        if (tilemap[xl][yl] || tilemap[xh][yl]
        || tilemap[xh][yh] || tilemap[xl][yh] )
        {
            moveok = false;
            if (xmove>=-2048 && xmove <=2048 && ymove>=-2048 && ymove <=2048)
            {
                ob->x = basex;
                ob->y = basey;
                return true;
            }
        }
        else
        {
            if (xmove>=-2048 && xmove <=2048 && ymove>=-2048 && ymove <=2048)
                return true;
            moveok = true;
        }
    } while (1);
}

/*
=============================================================================

                            PLAYER CONTROL

=============================================================================
*/

const statetype s_player = {0,0,&T_Player,&s_player};

void SpawnPlayer (s16 tilex, s16 tiley, s16 dir)
{
    player->obclass = playerobj;
    player->active = always;
    player->tilex = tilex;
    player->tiley = tiley;
    player->x = ((s32)tilex<<TILESHIFT)+TILEGLOBAL/2;
    player->y = ((s32)tiley<<TILESHIFT)+TILEGLOBAL/2;
    player->state = &s_player;
    player->size = MINDIST;
    CalcBounds(player);
    player->angle = (1-dir)*90;
    if (player->angle<0)
        player->angle += ANGLES;
}

void Thrust (s16 angle, u32 speed)
{
    s32 xmove,ymove;

    if (lasttimecount>>5 != ((lasttimecount-tics)>>5) )
    {
        if (lasttimecount&32)
            SD_PlaySound (WALK1SND);
        else
            SD_PlaySound (WALK2SND);
    }

    xmove = FixedByFrac(speed,costable[angle]);
    ymove = -FixedByFrac(speed,sintable[angle]);

    ClipXMove(player,xmove);
    ClipYMove(player,ymove);
    player->tilex = player->x >> TILESHIFT;
    player->tiley = player->y >> TILESHIFT;
}

void ControlMovement (objtype *ob)
{
    s16 angle;
    s32 speed;

    if (control.button1)
    {
        /* strafing */
        speed = 0;

        if (control.xaxis == -1)
        {
            speed += PLAYERSPEED*tics;
        }
        else if (control.xaxis == 1)
        {
            speed -= PLAYERSPEED*tics;
        }

        if (speed > 0)
        {
            if (speed >= TILEGLOBAL)
                speed = TILEGLOBAL-1;
            angle = ob->angle + ANGLES/4;
            if (angle >= ANGLES)
                angle -= ANGLES;
            Thrust (angle,speed);
        }
        else if (speed < 0)
        {
            if (speed <= -TILEGLOBAL)
                speed = -TILEGLOBAL+1;
            angle = ob->angle - ANGLES/4;
            if (angle < 0)
                angle += ANGLES;
            Thrust (angle,-speed);
        }
    }
    else
    {
        if (control.xaxis == 1)
        {
            ob->angle -= tics;

            if (running)
                ob->angle -= (tics<<1);
        }
        else if (control.xaxis == -1)
        {
            ob->angle+= tics;

            if (running)
                ob->angle += (tics<<1);
        }

        if (ob->angle >= ANGLES)
            ob->angle -= ANGLES;
        if (ob->angle < 0)
            ob->angle += ANGLES;
    }

    speed = 0;

    if (control.yaxis == -1)
    {
        speed += PLAYERSPEED*tics;
    }
    else if (control.yaxis == 1)
    {
        speed -= PLAYERSPEED*tics;
    }

    if (speed > 0)
    {
        if (speed >= TILEGLOBAL)
            speed = TILEGLOBAL-1;
        Thrust (ob->angle,speed);
    }
    else if (speed < 0)
    {
        if (speed <= -TILEGLOBAL)
            speed = -TILEGLOBAL+1;
        angle = ob->angle + ANGLES/2;
        if (angle >= ANGLES)
            angle -= ANGLES;
        Thrust (angle,-speed);
    }
}

void T_Player (objtype *ob)
{
    ControlMovement (ob);

    /* firing */
    if (boltsleft)
    {
        handheight+=(realtics<<2);
        if (handheight>MAXHANDHEIGHT)
            handheight = MAXHANDHEIGHT;

        ContinueBolt ();
        lasthand = lasttimecount;
    }
    else
    {
        if (control.button0)
        {
            handheight+=(realtics<<2);
            if (handheight>MAXHANDHEIGHT)
                handheight = MAXHANDHEIGHT;
            lasthand = lasttimecount;

            if (!button0down)
                Shoot();

            if (!autofire)
                button0down=true;
        }
        else
        {
            if (lasttimecount > lasthand+HANDPAUSE)
            {
                handheight-=(realtics<<1);
                if (handheight<0)
                    handheight = 0;
            }

            button0down = false;
        }
    }

    /* special actions */

    if (gbuttons.potion)
    {
        gbuttons.potion = false;
        if (gamestate.body != MAXBODY)
            DrinkPotion ();
    }

    if (gbuttons.zapper)
    {
        gbuttons.zapper = false;
        status_hint = S_ZAPPER;
        if (!boltsleft)
            CastBolt ();
    }

    if (gbuttons.nuke)
    {
        status_hint = S_XTER;
        if ((TimeCount-lastnuke > NUKETIME) || (autofire))
        {
            gbuttons.nuke = false;
            CastNuke ();
        }
        else if (!autofire)
            gbuttons.nuke = false;
    }

    if (gbuttons.scrolls)
    {
        s16 scroll;
        gbuttons.scrolls = false;
        scroll = MenuScroll();
        if ( scroll>=0 && scroll<NUMSCROLLS && gamestate.scrolls[scroll])
            ReadScroll (scroll);
    }

    DrawText(false);
    DrawHealth();
    if (FreezeTime)
        DrawFreezeTime();
    DrawRadar();
    DrawNSEWIcons();

    if (redraw_gems)
        DrawGems();
}

/*==========================================================================

                                EXPLOSION SPAWNING ROUTINES

===========================================================================*/

const statetype s_explode = {0,1,T_ExpThink,&s_explode};

void SpawnExplosion(fixed x, fixed y, s16 Delay)
{
    DSpawnNewObjFrac(x,y,&s_explode,PIXRADIUS*7);
    new->obclass = expobj;
    new->active = always;
    new->temp1 = Delay;
}

void T_ExpThink(objtype *obj)
{
    if (obj->temp1)
    {
        if ((obj->temp1-=realtics) <= 0)
            obj->temp1 = 0;
    }
    else
    {
        obj->state = &s_pshot_exp1;
        obj->ticcount = obj->state->tictime;
        SD_PlaySound(BOOMSND);
    }
}

/* Borland's random(n) is an int: with a 32 bit Range the result is
 * truncated to 16 bits, i.e. a random offset of up to half a tile. */
static fixed rndrange(fixed Range)
{
    (void)Range;
    return (s16)random();
}

void SpawnBigExplosion(fixed x, fixed y, s16 Delay, fixed Range)
{
    SpawnExplosion(x-rndrange(Range),y+rndrange(Range),rnd(Delay));
    SpawnExplosion(x+rndrange(Range),y-rndrange(Range),rnd(Delay));
    SpawnExplosion(x-rndrange(Range),y-rndrange(Range),rnd(Delay));
    SpawnExplosion(x+rndrange(Range),y+rndrange(Range),rnd(Delay));
}
