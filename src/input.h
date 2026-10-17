/* input.h - joypad replacement for ID_IN (keyboard/mouse/joystick).
 *
 * Game controls (3 button pad):
 *   D-pad        Up/Down advance/retreat, Left/Right turn
 *   B            Magick Missile
 *   C (hold)     Sidestep (Left/Right strafe)
 *   A            Zapper (on release)
 *   A + B        Xterminator
 *   A + C        Cure potion
 *   A + Left/Right  Quick turn
 *   START        Options (help, sound, scrolls, save, restore, new, quit)
 * 6 button pad extras: X Xterminator, Y Cure potion, Z quick turn,
 *   MODE read scrolls.
 * Hold A+B+C and press START for the original F10 debug/cheat keys.
 */
#ifndef INPUT_H
#define INPUT_H

#include "def.h"

/* edge-triggered game actions, consumed by the game code */
typedef struct
{
    boolean fire;           /* B held */
    boolean firepressed;
    boolean strafe;         /* C held */
    boolean quickturn;
    boolean zapper;         /* one-shot */
    boolean nuke;
    boolean potion;
    boolean options;
    boolean scrolls;
    boolean debug;
} GameButtons;

extern GameButtons gbuttons;
extern u16 pad_now, pad_last;

void IN_ReadControl(ControlInfo *info);
void IN_PollGameButtons(void);
void IN_ClearKeysDown(void);
boolean in_anykey(void);            /* any button newly pressed */
void IN_Ack(void);                  /* wait for a button press */
u16  IN_WaitPress(void);            /* returns the newly pressed buttons */
u16  IN_PollPress(void);            /* newly pressed buttons, no wait */
u16  pad_read(void);

#define PAD_OK      (BUTTON_B|BUTTON_START)
#define PAD_CANCEL  (BUTTON_C)

#endif
