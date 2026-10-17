/* main.c - Catacomb Abyss for the Sega Genesis */
#include "def.h"
#include "dbg.h"
#include "vid.h"
#include "snd.h"
#include "input.h"
#include "intro.h"

int main(bool hardReset)
{
    JOY_init();
    dbg_mark(0,1);
    for (;;)
    {
        IntroProgram();         /* INTRO.EXE: title, difficulty choice */
        if (DemoLoop())         /* CATABYSS.EXE: the game */
            EndScreens();       /* LOADSCN: LAST.ABS order screen */
    }
    return 0;
}
