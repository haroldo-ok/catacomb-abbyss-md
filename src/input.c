/* input.c - joypad handling (replaces ID_IN.C) */
#include "def.h"
#include "input.h"
#include "vid.h"

GameButtons gbuttons;
u16 pad_now, pad_last;

static boolean aheld, acombo;
static u16 ack_last;

u16 pad_read(void)
{
    return JOY_readJoypad(JOY_1);
}

void IN_ReadControl(ControlInfo *info)
{
    u16 p = pad_read();
    info->xaxis = (p & BUTTON_LEFT) ? -1 : (p & BUTTON_RIGHT) ? 1 : 0;
    info->yaxis = (p & BUTTON_UP) ? -1 : (p & BUTTON_DOWN) ? 1 : 0;
    info->x = info->xaxis * 127;
    info->y = info->yaxis * 127;
    info->button0 = (p & BUTTON_B) && !(p & BUTTON_A);
    info->button1 = (p & BUTTON_C) && !(p & BUTTON_A);
    info->dir = 0;
}

void IN_PollGameButtons(void)
{
    u16 p = pad_read();
    u16 pressed = p & ~pad_last;
    pad_last = pad_now = p;

    gbuttons.fire = (p & BUTTON_B) && !(p & BUTTON_A);
    gbuttons.firepressed = (pressed & BUTTON_B) && !(p & BUTTON_A);
    gbuttons.strafe = (p & BUTTON_C) && !(p & BUTTON_A);
    gbuttons.quickturn = (p & BUTTON_Z) ? TRUE : FALSE;

    if (pressed & BUTTON_A)
    {
        aheld = TRUE;
        acombo = FALSE;
    }
    if (aheld)
    {
        if (p & BUTTON_A)
        {
            if (pressed & BUTTON_B) { gbuttons.nuke = TRUE; acombo = TRUE; }
            if (pressed & BUTTON_C) { gbuttons.potion = TRUE; acombo = TRUE; }
            if (p & (BUTTON_LEFT | BUTTON_RIGHT)) { gbuttons.quickturn = TRUE; acombo = TRUE; }
        }
        else
        {
            if (!acombo) gbuttons.zapper = TRUE;
            aheld = FALSE;
        }
    }
    if (pressed & BUTTON_X) gbuttons.nuke = TRUE;
    if (pressed & BUTTON_Y) gbuttons.potion = TRUE;
    if (pressed & BUTTON_MODE) gbuttons.scrolls = TRUE;
    if (pressed & BUTTON_START)
    {
        if ((p & (BUTTON_A | BUTTON_B | BUTTON_C)) == (BUTTON_A | BUTTON_B | BUTTON_C))
        {
            gbuttons.debug = TRUE;
            aheld = FALSE;
        }
        else
            gbuttons.options = TRUE;
    }
}

void IN_ClearKeysDown(void)
{
    pad_last = pad_now = pad_read();
    ack_last = pad_now;
    aheld = acombo = FALSE;
    memset(&gbuttons, 0, sizeof(gbuttons));
}

boolean in_anykey(void)
{
    u16 p = pad_read();
    u16 pressed = p & ~ack_last;
    ack_last = p;
    return pressed ? TRUE : FALSE;
}

u16 IN_PollPress(void)
{
    u16 p = pad_read();
    u16 pressed = p & ~ack_last;
    ack_last = p;
    return pressed;
}

u16 IN_WaitPress(void)
{
    u16 p, pressed;
    ack_last = pad_read();
    for (;;)
    {
        vid_flush(&statsurf);
        vid_frame();
        p = pad_read();
        pressed = p & ~ack_last;
        ack_last = p;
        if (pressed)
            break;
    }
    pad_last = pad_now = p;
    return pressed;
}

void IN_Ack(void)
{
    vid_update();
    IN_WaitPress();
    IN_ClearKeysDown();
}
