/* menu.h - joypad driven replacements for the keyboard prompts, the help
 * text viewer (DisplayText) and the save/restore dialogs (GE_SaveGame). */
#ifndef MENU_H
#define MENU_H

#include "def.h"

/* Choice on the status bar message line.  choices is a string of codes:
 * Y yes, N no, S save, R restore, W new, Q quit, H help, O sound,
 * L scrolls.  Returns the chosen code or 0 when cancelled. */
char MenuChoice(const char *prompt, const char *choices);

/* Vertical list in the current window, returns index or -1 */
s16 MenuWindow(const char *title, u16 count, const char * const *items, s16 sel);

/* Pick one of the scrolls the player owns, -1 when cancelled */
s16 MenuScroll(void);

void HelpScreens(void);
void PreFullDisplay(void);
void PostFullDisplay(boolean draw_view);
void GE_SaveGame(void);
boolean GE_LoadGame(void);
boolean GE_LoadGameMenu(void);      /* from the intro (no game running) */

#endif
