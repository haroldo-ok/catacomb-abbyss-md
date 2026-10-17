/* Catacomb Abyss Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * C4_PLAY.C - Genesis port
 */
#include "def.h"
#include "dbg.h"
#include "snd.h"
#include "vid.h"
#include "draw2d.h"
#include "input.h"
#include "menu.h"

byte bcolor;
s16 skytimer=-1,skytimer_reset;
s16 groundtimer=-1,groundtimer_reset;
const u16 *skycolor,*groundcolor;
byte BGFLAGS,
      bgflag;

static const u16 sky_daytonight[]={0x0909,0x0101,0x0808,0x0000,0xFFFF};
static const u16 sky_lightning[]={0x0101,0x0909,0x0f0f,0x0808,0x0000,0xFFFF};

const u16 sky_colors[NUMLEVELS]={0x0000,0x0000,0x0000,0x0000,0x0808,
                                          0x0404,0x0000,0x0000,0x0000,0x0000,
                                          0x0000,0x0000,0x0000,0x0000,0x0606,
                                          0x0000,0x0000,0x0000,0x0000,0x0000,
                                          0x0000};
const u16 gnd_colors[NUMLEVELS]={0x0202,0x0202,0x0606,0x0202,0x0707,
                                          0x0505,0x0808,0x0606,0x0101,0x0808,
                                          0x0606,0x0404,0x0808,0x0c0c,0x0e0e,
                                          0x0808,0x0808,0x0c0c,0x0000,0x0707,
                                          0x0808};

ControlInfo control;
boolean     running=false;

s16         bordertime;
objtype objlist[MAXACTORS],*new,*obj,*player,*lastobj,*objfreelist;

boolean     godmode;
status_flags status_flag;
s16         status_delay;

u16         mapwidth,mapheight,tics,realtics;

s16         BeepTime = 0;
status_flags status_hint;

MapArrays   mapa;
u8          actorat[MAPSIZE][MAPSIZEY];
byte        mapplane0[MAPSIZE*MAPSIZEY];
byte        mapplane2[MAPSIZE*MAPSIZEY];

objtype dummyobj;
s16 objectcount;

#define MAXX    320
#define MAXY    120

void CenterWindow(u16 w,u16 h)
{
    vid_wait_idle();
    cursurf = &viewsurf;
    US_DrawWindow(((MAXX / 8) - w) / 2,((MAXY / 8) - h) / 2,w,h);
}

/* the F2 sound menu */
static void SoundMenu(void)
{
    s16 c;
    CenterWindow(22,7);
    c = MenuWindow("\nSelect sound:\n", 3,
        (const char * const[]){"NO SOUND", "PC  AUDIO", "ADLIB AUDIO"},
        SoundMode == sdm_Off ? 0 : SoundMode == sdm_PC ? 1 : 2);
    if (c == 0) SD_SetSoundMode(sdm_Off);
    else if (c == 1) SD_SetSoundMode(sdm_PC);
    else if (c == 2) SD_SetSoundMode(sdm_AdLib);
}

/* F10 debug keys of C4_DEBUG.C, through a menu */
void DebugKeys(void);

void CheckKeys (void)
{
    if (screenfaded)
        return;

    if (gbuttons.debug)
    {
        gbuttons.debug = false;
        DebugKeys();
        lasttimecount = TimeCount;
        tics = realtics = 1;
        IN_ClearKeysDown();
    }

deadloop:;
    if (gbuttons.options || (Flags & FL_DEAD))
    {
        char ch;
        gbuttons.options = false;

        DisplaySMsg("Options", NULL);
        if ((status_flag != S_TIMESTOP) || (Flags & FL_DEAD))
            status_flag = S_NONE;

        if (Flags & FL_DEAD)
            ch = DisplayMsg("",  "RWQ");
        else
            ch = DisplayMsg("", "HOLSRWQ");
        DrawText(true);

        switch (ch)
        {
            case 'H':
                HelpScreens();
                break;

            case 'O':
                SoundMenu();
                break;

            case 'L':
                {
                    s16 scroll = MenuScroll();
                    if (scroll >= 0 && gamestate.scrolls[scroll])
                        ReadScroll(scroll);
                }
                break;

            case 'S':
                if (!(Flags & FL_DEAD))
                {
                    PreFullDisplay();
                    GE_SaveGame();
                    PostFullDisplay(true);
                }
            break;

            case 'R':
                PreFullDisplay();
                if (GE_LoadGame())
                {
                    loadedgame = true;
                    playstate = ex_loadedgame;
                    Flags &= ~FL_DEAD;
                    PostFullDisplay(false);
                }
                else
                if (playstate == ex_victorious)
                {
                    PostFullDisplay(false);
                    Victory(false);
                }
                else
                    PostFullDisplay(true);
            break;

            case 'W':
                DisplaySMsg("Starting anew", NULL);
                VW_WaitVBL(60);
                playstate = ex_resetgame;
                Flags &= ~FL_DEAD;
                status_flag = S_NONE;
            break;

            case 'Q':
                DisplaySMsg("FARE THEE WELL!", NULL);
                status_flag = S_NONE;
                VW_WaitVBL(120);
                VW_FadeOut();
                Flags &= ~FL_DEAD;
                playstate = ex_abort;
                abortgame = true;
            break;
        }
        tics = realtics = 1;
        lasttimecount = TimeCount;
        IN_ClearKeysDown();

        if (status_flag == S_TIMESTOP)
            DisplaySMsg("Time Stopped:     ",NULL);
    }

    if ((Flags & FL_DEAD) && playstate != ex_abort)
        goto deadloop;
}

void PreFullDisplay(void)
{
    VW_FadeOut();
    vid_fullscreen(true);
    cursurf = &fullsurf;
}

void PostFullDisplay(boolean draw_view)
{
    vid_fullscreen(false);
    cursurf = &statsurf;
    VW_DrawPic (0,0,STATUSPIC);
    RedrawStatusWindow();
    if (draw_view)
    {
        ThreeDRefresh();
        VW_FadeIn();
    }
}

/*
#############################################################################

                  The objlist data structure

#############################################################################
*/

void InitObjList (void)
{
    s16 i;

    for (i=0;i<MAXACTORS;i++)
    {
        objlist[i].prev = &objlist[i+1];
        objlist[i].next = NULL;
    }

    objlist[MAXACTORS-1].prev = NULL;

    objfreelist = &objlist[0];
    lastobj = NULL;

    objectcount = 0;

    GetNewObj (false);
    player = new;
}

void GetNewObj (boolean usedummy)
{
    if (!objfreelist)
    {
        if (usedummy)
        {
            new = &dummyobj;
            return;
        }
        Quit ("GetNewObj: No free spots in objlist!");
    }

    new = objfreelist;
    objfreelist = new->prev;
    memset (new,0,sizeof(*new));

    if (lastobj)
        lastobj->next = new;
    new->prev = lastobj;

    new->active = false;
    lastobj = new;

    objectcount++;
}

void RemoveObj (objtype *gone)
{
    if (gone == player)
        Quit ("RemoveObj: Tried to remove the player!");

    if (gone == lastobj)
        lastobj = (objtype *)gone->prev;
    else
        gone->next->prev = gone->prev;

    gone->prev->next = gone->next;

    gone->prev = objfreelist;
    objfreelist = gone;
}

void PollControls (void)
{
    IN_ReadControl(&control);
    IN_PollGameButtons();
    running = gbuttons.quickturn;
}

/*
===================
=
= PlayLoop
=
===================
*/

void PlayLoop (void)
{
    static const s16 shot_color[3] = {4,9,14};

    static const s16 allgems[5]={GEM_DELAY_TIME,GEM_DELAY_TIME,GEM_DELAY_TIME,
                        GEM_DELAY_TIME,GEM_DELAY_TIME};

    s32 dx,dy,psin=0,pcos=0,newx,newy;
    s16 objnum;
    s32 ox,oy,xl=0,xh=0,yl=0,yh=0,px=0,py=0,norm_dx,norm_dy;
    s16 o_radius;
    static u16 framepage;

    thinkfn think;

    ingame = true;
    playstate = TimeCount = 0;
    gamestate.shotpower = handheight = 0;

    switch (gamestate.mapon)
    {
        case 0:
            if (!(BGFLAGS & BGF_NIGHT))
            {
                InitBgChange(3*60,sky_daytonight,-1,NULL,BGF_NIGHT);
                groundcolor = &gnd_colors[0];
            }
            else
            {
                skycolor = &sky_colors[0];
                groundcolor = &gnd_colors[0];
            }
        break;

        default:
            skycolor = &sky_colors[gamestate.mapon];
            groundcolor = &gnd_colors[gamestate.mapon];
            skytimer = groundtimer = -1;
        break;
    }

    RedrawStatusWindow();
    ThreeDRefresh();
    dbg_mark(5,1);
    if (screenfaded)
        VW_FadeIn();

    fizzlein = true;
    TimeCount = lasttimecount = lastnuke = 0;

    PollControls ();
    do
    {
        PollControls();

        objnum=0;
        for (obj = player;obj;obj = obj->next)
        {
            if ((obj->active >= yes) && (!(FreezeTime && (obj!=player))))
            {
                if (obj->ticcount)
                {
                    obj->ticcount-=tics;

                    while ( obj->ticcount <= 0)
                    {
                        think = obj->state->think;
                        if (think)
                        {
                            const statetype *oldstate=obj->state;

                            think (obj);
                            if (!obj->state)
                            {
                                RemoveObj (obj);
                                goto nextactor;
                            }

                            if (obj->state != oldstate)
                                break;
                        }

                        obj->state = obj->state->next;
                        if (!obj->state)
                        {
                            RemoveObj (obj);
                            goto nextactor;
                        }

                        if (!obj->state->tictime)
                        {
                            obj->ticcount = 0;
                            goto nextactor;
                        }

                        if (obj->state->tictime>0)
                            obj->ticcount += obj->state->tictime;
                    }
                }

                think = obj->state->think;

                if (think)
                {
                    think (obj);
                    if (!obj->state)
                        RemoveObj (obj);
                }
nextactor:;
            }

            if (playstate == ex_abort)
                break;

            /* keep a list of objects around the player for radar updates */
            if (obj == player)
            {
                px = player->x;
                py = player->y;
                psin = sintable[player->angle];
                pcos = costable[player->angle];
                xl = px-((s32)RADAR_WIDTH<<TILESHIFT)/2;
                xh = px+((s32)RADAR_WIDTH<<TILESHIFT)/2-1;
                yl = py-((s32)RADAR_HEIGHT<<TILESHIFT)/2;
                yh = py+((s32)RADAR_HEIGHT<<TILESHIFT)/2;
            }

            if (objnum > MAX_RADAR_BLIPS-2)
                objnum = MAX_RADAR_BLIPS-2;

            ox = obj->x;
            oy = obj->y;

            if ((ox >= xl) && (ox <= xh) && (oy >= yl) && (oy <= yh))
            {
                norm_dx = (dx = px-ox)>>TILESHIFT;
                norm_dy = (dy = oy-py)>>TILESHIFT;

                o_radius = IntSqrt((norm_dx * norm_dx) + (norm_dy * norm_dy));

                if (o_radius < RADAR_RADIUS)
                {
                    newx = FixedByFrac(dy,pcos)-FixedByFrac(dx,psin);
                    newy = FixedByFrac(dy,psin)+FixedByFrac(dx,pcos);

                    RadarXY[objnum][0]=newx>>TILESHIFT;
                    RadarXY[objnum][1]=newy>>TILESHIFT;

                    switch (obj->obclass)
                    {
                            case playerobj:
                                RadarXY[objnum++][2]=15;
                            break;

                            case pshotobj:
                            case bigpshotobj:
                                RadarXY[objnum++][2]=shot_color[framepage];
                            break;

                            case batobj:
                                if (obj->active == always)
                                    RadarXY[objnum++][2]=8;
                            break;

                            case eyeobj:
                            case reddemonobj:
                                if (gamestate.gems[B_RGEM-B_RGEM])
                                    if (obj->active == always)
                                        RadarXY[objnum++][2]=4;
                            break;

                            case mageobj:
                                if (gamestate.gems[B_RGEM-B_RGEM])
                                    if (obj->active == always)
                                        RadarXY[objnum++][2]=12;
                            break;

                            case wetobj:
                                if (gamestate.gems[B_BGEM-B_RGEM])
                                    if (obj->active == always)
                                        RadarXY[objnum++][2]=9;
                            break;

                            case demonobj:
                                if (gamestate.gems[B_BGEM-B_RGEM])
                                    if (obj->active == always)
                                        RadarXY[objnum++][2]=1;
                            break;

                            case trollobj:
                                if (gamestate.gems[B_GGEM-B_RGEM])
                                    if (obj->active == always)
                                        RadarXY[objnum++][2]=10;
                            break;

                            case orcobj:
                                if (gamestate.gems[B_GGEM-B_RGEM])
                                    if (obj->active == always)
                                        RadarXY[objnum++][2]=2;
                            break;

                            case spookobj:
                                if (gamestate.gems[B_YGEM-B_RGEM])
                                    if (obj->active == always)
                                        RadarXY[objnum++][2]=6;
                            break;

                            case skeletonobj:
                                if (gamestate.gems[B_YGEM-B_RGEM])
                                    if (obj->active == always)
                                        RadarXY[objnum++][2]=14;
                            break;

                            case zombieobj:
                                if (gamestate.gems[B_PGEM-B_RGEM])
                                    if (obj->active == always)
                                        RadarXY[objnum++][2]=13;
                            break;

                            case grelmobj:
                                if (!memcmp(gamestate.gems,allgems,sizeof(gamestate.gems)))
                                    if (obj->active == always)
                                        RadarXY[objnum++][2]=15;
                            break;
                    }
                }
            }
        }
        RadarXY[objnum][2]=-1;
        if (playstate == ex_abort)
            break;

        if (bordertime)
        {
            bordertime -= realtics;
            if (bordertime<=0)
            {
                bordertime = 0;
                VW_ColorBorder(0);
            }
        }

        /* random lightning? */
        if (BGFLAGS & (BGF_NIGHT|BGF_NOT_LIGHTNING))
            switch (gamestate.mapon)
            {
                case 0:
                case 1:
                case 3:
                    if (!rnd(120-realtics))
                    {
                        BGFLAGS &= ~BGF_NOT_LIGHTNING;
                        InitBgChange(1,sky_lightning,-1,NULL,BGF_NOT_LIGHTNING);
                    }
                break;
            }

        /* handle sky/ground color changes */
        if (skytimer != -1)
        {
            skytimer -= realtics;
            if (skytimer < 0)
            {
                skycolor++;
                if (*skycolor == 0xffff)
                {
                    skytimer = -1;
                    skycolor--;
                    if (groundtimer == -1)
                        BGFLAGS |= bgflag;
                }
                else
                    skytimer = skytimer_reset;
            }
        }

        if (groundtimer != -1)
        {
            groundtimer -= realtics;
            if (groundtimer < 0)
            {
                groundcolor++;
                if (*groundcolor == 0xffff)
                {
                    groundtimer = -1;
                    groundcolor--;
                    if (skytimer == -1)
                        BGFLAGS |= bgflag;
                }
                else
                    groundtimer = groundtimer_reset;
            }
        }

        /* Handle FreezeTime counter.. */
        if (FreezeTime)
        {
            if (FreezeTime<20*30)
                if ((BeepTime+=realtics)>=60)
                {
                    BeepTime -= 60;
                    SD_PlaySound(TICKSND);
                }

            if ((FreezeTime-=realtics)<=0)
            {
                FreezeTime=0;
                SD_PlaySound(TIMERETURNSND);
                DisplaySMsg(NULL,NULL);
                status_flag = S_NONE;
            }
        }

        /* refresh all */
        if (++framepage == 3)
            framepage = 0;
        ThreeDRefresh ();
        dbg_mark(6,(u8)vid_frames);

        if (Flags & FL_DEAD)
        {
            SD_PlaySound (GAMEOVERSND);
            DisplaySMsg("DEAD",NULL);
            DrawHealth();
            if (gamestate.potions)
            {
                 CenterWindow(35,3);
                 US_CPrint("\nYou should use your Cure Potions wisely\n");
                 IN_Ack();
            }
        }

        if (playstate == ex_victorious)
        {
            Victory(true);
            Flags |= FL_DEAD;
        }

        DisplayStatus(&status_flag);
        CheckKeys();

    }while (!playstate);

    ingame = false;
    if (bordertime)
    {
        bordertime = 0;
        VW_ColorBorder(0);
    }

    if (abortgame)
        abortgame = false;
}

/* IntSqrt() - by Master Programmer, George Leritte! (integer Newton) */
s16 IntSqrt(s32 va)
{
    u32 v = va, r, b;
    if (!v) return 0;
    r = 0;
    b = 1ul << 30;
    while (b > v) b >>= 2;
    while (b)
    {
        if (v >= r + b)
        {
            v -= r + b;
            r = (r >> 1) + b;
        }
        else
            r >>= 1;
        b >>= 2;
    }
    return (s16)r;
}

void InitBgChange(s16 stimer, const u16 *scolors, s16 gtimer, const u16 *gcolors, byte flag)
{
    skytimer_reset = skytimer = stimer;
    if (scolors)
        skycolor = scolors;

    groundtimer_reset = groundtimer = gtimer;
    if (gcolors)
        groundcolor = gcolors;

    bgflag = flag;
}

#define MESSAGEDELAY  25
void DisplayStatus (status_flags *stat_flag)
{
    status_flags temp_status;

    if (*stat_flag == S_TIMESTOP)
      return;

    if (status_delay > 0)
    {
        status_delay -= realtics;
        return;
    }
    else
        status_delay = 0;

    temp_status = S_VIEWING;

    if (control.button0)
        temp_status = S_MISSLE;

    if (status_hint)
        temp_status = status_hint;
    status_hint = S_NONE;

    if (control.x)
        temp_status = S_TURN;

    if (running && control.x)
        temp_status = S_QTURN;

    if (control.button1 && control.x)
        temp_status = S_SIDESTEP;

    if (control.y < 0)
        temp_status = S_ADVANCE;

    if (control.y > 0)
        temp_status = S_RETREAT;

    if (temp_status != *stat_flag)
    {
        *stat_flag = temp_status;

        switch (*stat_flag)
        {
            case S_MISSLE:
                DisplaySMsg("Magick Missile", NULL);
                status_delay = MESSAGEDELAY;
            break;

            case S_ZAPPER:
                if (gamestate.bolts)
                {
                    DisplaySMsg("Zapper", NULL);
                    status_delay = MESSAGEDELAY+10;
                }
            break;

            case S_XTER:
                if (gamestate.nukes)
                {
                    DisplaySMsg("Xterminator", NULL);
                    status_delay = MESSAGEDELAY+5;
                }
            break;

            case S_TURN:
                DisplaySMsg("Turning", NULL);
                status_delay = MESSAGEDELAY;
            break;

            case S_QTURN:
                DisplaySMsg("Quick Turning", NULL);
                status_delay = MESSAGEDELAY;
            break;

            case S_SIDESTEP:
                DisplaySMsg("Sidestepping", NULL);
                status_delay = MESSAGEDELAY;
            break;

            case S_ADVANCE:
                DisplaySMsg("Advancing", NULL);
                status_delay = MESSAGEDELAY;
            break;

            case S_RETREAT:
                DisplaySMsg("Retreating", NULL);
                status_delay = MESSAGEDELAY;
            break;

            case S_VIEWING:
                DisplaySMsg("Viewing", NULL);
            break;

            default:
            break;
        }
    }
}
