/* Catacomb Abyss Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * C4_DEBUG.C - the F10 debug keys (advertised on the cheat plaques in
 * "The Passage to the Surface") and the MIKE auto-bolt toggle, reached on
 * the Genesis by holding A+B+C and pressing START.
 */
#include "def.h"
#include "snd.h"
#include "vid.h"
#include "draw2d.h"
#include "input.h"
#include "menu.h"

static const char * const debug_items[] = {
    "G - God mode",
    "W - Warp to level",
    "I - Free items",
    "Z - Freeze time",
    "1-8 - Free scrolls",
    "MIKE - Auto-Bolt",
    "T - Tics",
    "V - Version",
};

void DebugKeys (void)
{
    s16 i, c;

    CenterWindow(20, 11);
    c = MenuWindow("Debug keys", 8, debug_items, 0);

    switch (c)
    {
    case 0:
        CenterWindow (12,2);
        if (godmode)
          US_PrintCentered ("God mode OFF");
        else
          US_PrintCentered ("God mode ON");
        vid_update();
        IN_Ack();
        godmode ^= 1;
        break;

    case 1:
        {
            s16 l = gamestate.mapon;
            for (;;)
            {
                char buf[48];
                CenterWindow(30, 4);
                US_CPrint("\nWarp to which level(0-18):\n");
                buf[0] = '<'; buf[1] = ' ';
                intToStr(l, buf + 2, 1);
                strcat(buf, "  ");
                strcat(buf, LevelName(l));
                strcat(buf, " >");
                US_CPrint(buf);
                vid_update();
                {
                    u16 p = IN_WaitPress();
                    if (p & (BUTTON_LEFT|BUTTON_DOWN)) l = l ? l - 1 : 18;
                    else if (p & (BUTTON_RIGHT|BUTTON_UP)) l = (l + 1) % 19;
                    else if (p & (BUTTON_B|BUTTON_START))
                    {
                        gamestate.mapon = l;
                        playstate = ex_warped;
                        lasttext = -1;
                        break;
                    }
                    else if (p & (BUTTON_C|BUTTON_A))
                        break;
                }
            }
        }
        break;

    case 2:
        CenterWindow (12,3);
        US_PrintCentered ("Free items!");
        vid_update();
        for (i=0;i<4;i++)
        {
            GiveBolt ();
            GiveNuke ();
            GivePotion ();
            GiveKey (i);
            gamestate.gems[i] = GEM_DELAY_TIME;
        }
        gamestate.gems[4] = GEM_DELAY_TIME;
        redraw_gems = true;
        for (i=0;i<8;i++)
            GiveScroll (i,false);
        IN_Ack ();
        break;

    case 3:
        if (FreezeTime)
          FreezeTime = 1;
        else
            StopTime();
        IN_Ack();
        break;

    case 4:
        for (i=0;i<8;i++)
            GiveScroll (i,false);
        break;

    case 5:
        CenterWindow (12,2);
        if (autofire)
          US_PrintCentered ("Auto-Bolt OFF");
        else
          US_PrintCentered ("Auto-Bolt ON");
        vid_update();
        IN_Ack();
        autofire ^= 1;
        break;

    case 6:
        CenterWindow (16,4);
        US_Print("Tics      :");
        US_PrintUnsigned (tics);
        US_Print("\nReal Tics :");
        US_PrintUnsigned(realtics);
        vid_update();
        IN_Ack ();
        break;

    case 7:
        CenterWindow (20,5);
        US_CPrint("\nThe Catacomb Abyss\n");
        US_CPrint("V1.13 (Genesis)\n");
        US_CPrint("QA[0]");
        vid_update();
        IN_Ack ();
        break;
    }
}
