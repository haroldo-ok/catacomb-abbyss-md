/* Catacomb Abyss Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * C4_STATE.C - Genesis port
 */
#include "def.h"
#include "snd.h"
#include "vid.h"

const dirtype opposite[9] =
    {south,west,north,east,southwest,northwest,northeast,southeast,nodir};

#define SIZE_TEST 0

void Internal_SpawnNewObj (u16 x, u16 y, const statetype *state, u32 size,boolean UseDummy)
{
    GetNewObj(UseDummy);
    new->size = size;
    new->state = state;
    new->ticcount = rnd (state->tictime)+1;

    new->tilex = x;
    new->tiley = y;
    new->x = ((s32)x<<TILESHIFT)+TILEGLOBAL/2;
    new->y = ((s32)y<<TILESHIFT)+TILEGLOBAL/2;
    CalcBounds(new);
    new->dir = nodir;
    new->active = noalways;

    if (new != &dummyobj)
        actorat[new->tilex][new->tiley] = ACTOROBJ(new);
}

void Internal_SpawnNewObjFrac (s32 x, s32 y, const statetype *state, u32 size,boolean UseDummy)
{
    GetNewObj(UseDummy);
    new->size = size;
    new->state = state;
    new->ticcount = rnd (state->tictime)+1;
    new->active = noalways;

    new->x = x;
    new->y = y;
    new->tilex = x>>TILESHIFT;
    new->tiley = y>>TILESHIFT;
    CalcBounds(new);
    new->distance = 100;
    new->dir = nodir;
}

boolean CheckHandAttack (objtype *ob)
{
    s32 deltax,deltay,size;

    size = (s32)ob->size + player->size + ob->speed*tics + SIZE_TEST;
    deltax = ob->x - player->x;
    deltay = ob->y - player->y;

    if (deltax > size || deltax < -size || deltay > size || deltay < -size)
        return false;

    return true;
}

void T_DoDamage (objtype *ob)
{
    s16 points;

    if (CheckHandAttack(ob) && (!(ob->flags & of_damagedone)))
    {
        points = 0;

        switch (ob->obclass)
        {
        case orcobj:
            points = 4;
            break;
        case zombieobj:
        case trollobj:
            points = 8;
            break;
        case reddemonobj:
        case demonobj:
            points = 15;
            break;
        case spookobj:
            points = 2;
            break;
        case skeletonobj:
            points = 6;
            break;
        case wetobj:
            points = 7;
            break;
        }
        TakeDamage (EasyDoDamage(points));

        ob->flags |= of_damagedone;
    }
}

boolean Walk (objtype *ob)
{
    switch (ob->dir)
    {
    case north:
        if (actorat[ob->tilex][ob->tiley-1])
            return false;
        ob->tiley--;
        ob->distance = TILEGLOBAL;
        return true;

    case northeast:
        if (actorat[ob->tilex+1][ob->tiley-1])
            return false;
        ob->tilex++;
        ob->tiley--;
        ob->distance = TILEGLOBAL;
        return true;

    case east:
        if (actorat[ob->tilex+1][ob->tiley])
            return false;
        ob->tilex++;
        ob->distance = TILEGLOBAL;
        return true;

    case southeast:
        if (actorat[ob->tilex+1][ob->tiley+1])
            return false;
        ob->tilex++;
        ob->tiley++;
        ob->distance = TILEGLOBAL;
        return true;

    case south:
        if (actorat[ob->tilex][ob->tiley+1])
            return false;
        ob->tiley++;
        ob->distance = TILEGLOBAL;
        return true;

    case southwest:
        if (actorat[ob->tilex-1][ob->tiley+1])
            return false;
        ob->tilex--;
        ob->tiley++;
        ob->distance = TILEGLOBAL;
        return true;

    case west:
        if (actorat[ob->tilex-1][ob->tiley])
            return false;
        ob->tilex--;
        ob->distance = TILEGLOBAL;
        return true;

    case northwest:
        if (actorat[ob->tilex-1][ob->tiley-1])
            return false;
        ob->tilex--;
        ob->tiley--;
        ob->distance = TILEGLOBAL;
        return true;

    case nodir:
        return false;
    }

    Quit ("Walk: Bad dir");
    return false;
}

void ChaseThink (objtype *obj, boolean diagonal)
{
    s16 deltax,deltay;
    dirtype d[3];
    s16 tdir, olddir, turnaround;

    olddir=obj->dir;
    turnaround=opposite[olddir];

    deltax=player->tilex - obj->tilex;
    deltay=player->tiley - obj->tiley;

    d[1]=nodir;
    d[2]=nodir;

    if (deltax>0)
        d[1]= east;
    if (deltax<0)
        d[1]= west;
    if (deltay>0)
        d[2]=south;
    if (deltay<0)
        d[2]=north;

    if (abs(deltay)>abs(deltax))
    {
        tdir=d[1];
        d[1]=d[2];
        d[2]=tdir;
    }

    if (d[1]==turnaround)
        d[1]=nodir;
    if (d[2]==turnaround)
        d[2]=nodir;

    if (diagonal)
    {
        if (d[1]!=nodir)
        {
            obj->dir=d[1];
            if (Walk(obj))
                return;
        }

        if (d[2]!=nodir)
        {
            obj->dir=d[2];
            if (Walk(obj))
                return;
        }
    }
    else
    {
        if (d[2]!=nodir)
        {
            obj->dir=d[2];
            if (Walk(obj))
                return;
        }

        if (d[1]!=nodir)
        {
            obj->dir=d[1];
            if (Walk(obj))
                return;
        }
    }

    obj->dir=olddir;
    if (Walk(obj))
        return;

    if (US_RndT()>128)
    {
        for (tdir=north;tdir<=west;tdir++)
        {
            if (tdir!=turnaround)
            {
                obj->dir=tdir;
                if (Walk(obj))
                    return;
            }
        }
    }
    else
    {
        for (tdir=west;tdir>=north;tdir--)
        {
            if (tdir!=turnaround)
            {
              obj->dir=tdir;
              if (Walk(obj))
                return;
            }
        }
    }

    obj->dir=turnaround;
    Walk(obj);
}

void MoveObj (objtype *ob, s32 move)
{
    ob->distance -=move;

    switch (ob->dir)
    {
    case north:
        ob->y -= move;
        return;
    case northeast:
        ob->x += move;
        ob->y -= move;
        return;
    case east:
        ob->x += move;
        return;
    case southeast:
        ob->x += move;
        ob->y += move;
        return;
    case south:
        ob->y += move;
        return;
    case southwest:
        ob->x -= move;
        ob->y += move;
        return;
    case west:
        ob->x -= move;
        return;
    case northwest:
        ob->x -= move;
        ob->y -= move;
        return;

    case nodir:
        return;
    }
}

boolean Chase (objtype *ob, boolean diagonal)
{
    s32 move;
    s32 deltax,deltay,size;

    ob->flags &= ~of_damagedone;

    move = ob->speed*tics;
    size = (s32)ob->size + player->size + move + SIZE_TEST;

    while (move)
    {
        deltax = ob->x - player->x;
        deltay = ob->y - player->y;

        if (deltax <= size && deltax >= -size
        && deltay <= size && deltay >= -size)
        {
            CalcBounds (ob);
            return true;
        }

        if (move < ob->distance)
        {
            MoveObj (ob,move);
            break;
        }
        actorat[ob->tilex][ob->tiley] = 0;
        if (ob->dir == nodir)
            ob->dir = north;

        ob->x = ((s32)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
        ob->y = ((s32)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;
        move -= ob->distance;

        ChaseThink (ob,diagonal);
        if (!ob->distance)
            break;
        actorat[ob->tilex][ob->tiley] = ACTOROBJ(ob);
    }
    CalcBounds (ob);
    return false;
}

void ShootActor (objtype *ob, u16 damage)
{
    ob->hitpoints -= damage;
    if (ob->hitpoints<=0)
    {
        switch (ob->obclass)
        {
        case reddemonobj:
            ob->state = &s_red_demondie1;
            break;
        case orcobj:
            ob->state = &s_orcdie1;
            break;
        case trollobj:
            ob->state = &s_trolldie1;
            break;
        case demonobj:
            ob->state = &s_demondie1;
            break;
        case mageobj:
            ob->state = &s_magedie1;
            break;
        case batobj:
            ob->state = &s_batdie1;
            break;
        case grelmobj:
            ob->state = &s_greldie1;
            break;

        case zombieobj:
            ob->state = &s_zombie_death1;
        break;

        case skeletonobj:
            ob->state = &s_skel_die1;
        break;

        case spookobj:
            ob->state = &s_spookdie;
        break;

        case wetobj:
            ob->state = &s_wet_die1;
        break;

        case eyeobj:
            ob->state = &s_eye_die1;
        break;

        case eshotobj:
        case mshotobj:
            ob->state = &s_bonus_die;
        break;

        case bonusobj:
        case freezeobj:
            switch (ob->temp1)
            {
                case B_POTION:
                case B_CHEST:
                case B_NUKE:
                case B_BOLT:
                    ob->state = &s_pshot_exp1;
                    ob->obclass = expobj;
                    ob->ticcount = ob->state->tictime;
                    SpawnBigExplosion(ob->x,ob->y,12,(16l<<16L));
                    bordertime = FLASHTICS<<2;
                    bcolor = 14;
                    VW_ColorBorder(14 | 56);
                    DisplaySMsg("Item destroyed", NULL);
                    status_flag  = S_NONE;
                    status_delay = 80;
                break;
            }
        break;

        }

        if (ob->obclass != solidobj)
        {
            ob->obclass = inertobj;
            ob->flags &= ~of_shootable;
            actorat[ob->tilex][ob->tiley] = 0;
        }
    }
    else
    {
        switch (ob->obclass)
        {
        case reddemonobj:
            ob->state = &s_red_demonouch;
            break;
        case orcobj:
            ob->state = &s_orcouch;
            break;
        case trollobj:
            ob->state = &s_trollouch;
            break;
        case demonobj:
            ob->state = &s_demonouch;
            break;
        case mageobj:
            ob->state = &s_mageouch;
            break;

        case grelmobj:
            ob->state = &s_grelouch;
            break;

        case zombieobj:
            ob->state = &s_zombie_ouch;
        break;

        case spookobj:
            ob->state = &s_spookouch;
            break;

        case skeletonobj:
            ob->state = &s_skel_ouch;
            break;

        case wetobj:
            ob->state = &s_wet_ouch;
            break;

        case eyeobj:
            ob->state = &s_eye_ouch;
        break;

        }
    }
    ob->ticcount = ob->state->tictime;
}
