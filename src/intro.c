/* intro.c - the Gamer's Edge front end of THE CATACOMB ABYSS:
 *
 *   INTRO.EXE    title sequence, "Novice or Warrior?" challenge, story
 *   DEMOCAT.EXE  the electronic catalog of the Catacomb 3-D series
 *   START.EXE    the USER'S GUIDE shell (information, instructions,
 *                Gamer's Edge products, order form)
 *   LOADSCN.EXE  the closing text screens (LAST.ABS / LAST.CAT)
 *
 * The pictures are the original EGA screens unpacked into the full screen
 * surface; the shell's 80x25 text pages are drawn with a 4x8 font.
 *
 * Title screen buttons (PC key in brackets):
 *   START  new game [ENTER]      B  restore a game [F4]
 *   C      catalog demo [F10]    A  user's guide [START.EXE]
 */
#include "def.h"
#include "intro.h"
#include "snd.h"
#include "vid.h"
#include "input.h"
#include "res_assets.h"

#define TICKS_50MS(n)   ((n) * 7 / 2)       /* 70 Hz TimeCount ticks */

/* ------------------------------------------------------------- screens */

static void show_screen(const u8 *packed)
{
    vid_wait_idle();
    if (!vid_isfull())
        vid_fullscreen(TRUE);
    vid_set_palette_black();
    vid_frame();
    aplib_unpack((u8 *)packed, screenbuf);
    surf_markall(&fullsurf);
    vid_flush(&fullsurf);
    vid_wait_idle();
    vid_set_palette_normal();
}

/* waits up to 'ticks' (70 Hz) for a button; returns the buttons pressed */
static u16 wait_buttons(u32 ticks)
{
    u32 start = TimeCount;
    IN_PollPress();
    for (;;)
    {
        u16 p;
        vid_frame();
        p = IN_PollPress();
        if (p)
            return p;
        if (ticks && TimeCount - start >= ticks)
            return 0;
    }
}

/* ----------------------------------------------------------- text mode */

static const u16 nib_expand[16] = {
    0x0000, 0x000F, 0x00F0, 0x00FF, 0x0F00, 0x0F0F, 0x0FF0, 0x0FFF,
    0xF000, 0xF00F, 0xF0F0, 0xF0FF, 0xFF00, 0xFF0F, 0xFFF0, 0xFFFF };

/* one 4x8 character cell of the 80x25 text screen */
static void tm_cell(u16 cx, u16 cy, u8 ch, u8 attr)
{
    u16 *d = (u16 *)(screenbuf + (cx >> 1) * 800 + (cx & 1) * 2 + cy * 32);
    const u8 *g = font4x8 + ch * 8;
    u16 fg = (attr & 15) * 0x1111;
    u16 bg = ((attr >> 4) & 7) * 0x1111;
    u16 r;
    for (r = 0; r < 8; r++)
    {
        u16 m = nib_expand[g[r] & 15];
        *d = (fg & m) | (bg & ~m);
        d += 2;
    }
}

static void tm_text(u16 cx, u16 cy, const char *s, u8 attr)
{
    while (*s && cx < 80)
        tm_cell(cx++, cy, (u8)*s++, attr);
}

static void tm_center(u16 cy, const char *s, u8 attr)
{
    u16 i;
    for (i = 0; i < 80; i++)
        tm_cell(i, cy, ' ', attr);
    tm_text((80 - strlen(s)) / 2, cy, s, attr);
}

/* a page of the shell window; row 'hilite' of the window drawn inverted */
static void shell_page(u16 page, s16 hilite)
{
    const u8 *p = shell_page_data + page * (SHELL_WIN_W * SHELL_WIN_H * 2);
    u16 r, c;
    for (r = 0; r < SHELL_WIN_H; r++)
        for (c = 0; c < SHELL_WIN_W; c++, p += 2)
        {
            u8 a = p[1];
            if ((s16)r == hilite)
                a = 0x1F;           /* bright white on blue */
            tm_cell(SHELL_WIN_X + c, SHELL_WIN_Y + r, p[0], a);
        }
}

static void shell_show(u16 page, s16 hilite, const char *prompt)
{
    vid_wait_idle();
    if (!vid_isfull())
        vid_fullscreen(TRUE);
    aplib_unpack((u8 *)scr_shellbg, screenbuf);
    shell_page(page, hilite);
    if (prompt)
        tm_center(24, prompt, 0x0E);
    surf_markall(&fullsurf);
    vid_flush(&fullsurf);
    vid_wait_idle();
    if (screenfaded)
        vid_set_palette_normal();
}

/* -------------------------------------------------------- the catalog */

static void Catalog(void)
{
    s16 page = 0;
    for (;;)
    {
        u16 p;
        show_screen(catalog_pages[page]);
        p = wait_buttons(0);
        if (p & (BUTTON_START | BUTTON_A))
            break;
        if (p & (BUTTON_LEFT | BUTTON_UP))
        {
            if (page > 0) page--;
        }
        else if (++page >= CATALOG_COUNT)
            break;
    }
    /* DEMOCAT's closing screen */
    show_screen(scr_lastcat);
    wait_buttons(0);
}

/* ------------------------------------------------------------ the shell */

static void Document(u16 first, u16 count)
{
    s16 page = 0;
    for (;;)
    {
        char prompt[80];
        u16 p;
        strcpy(prompt, "LEFT/RIGHT: turn the page    A: back to the menu    Page ");
        intToStr(page + 1, prompt + strlen(prompt), 1);
        strcat(prompt, " of ");
        intToStr(count, prompt + strlen(prompt), 1);
        shell_show(first + page, -1, prompt);
        p = wait_buttons(0);
        if (p & (BUTTON_A | BUTTON_START))
            return;
        if (p & (BUTTON_LEFT | BUTTON_UP))
        {
            if (page > 0) page--;
        }
        else if (++page >= (s16)count)
            return;
    }
}

/* PRINTFIL.TXT: 80x24 pages on the whole screen */
static void PrintFile(void)
{
    s16 page = 0;
    for (;;)
    {
        char prompt[80];
        const u8 *p = printfil_page_data + page * (80 * 24 * 2);
        u16 r, c, b;
        vid_wait_idle();
        if (!vid_isfull())
            vid_fullscreen(TRUE);
        for (r = 0; r < 24; r++)
            for (c = 0; c < 80; c++, p += 2)
                tm_cell(c, r, p[0], p[1]);
        strcpy(prompt, "LEFT/RIGHT: turn the page    A: back to the menu    Page ");
        intToStr(page + 1, prompt + strlen(prompt), 1);
        strcat(prompt, " of ");
        intToStr(PRINTFIL_PAGES, prompt + strlen(prompt), 1);
        tm_center(24, prompt, 0x0E);
        surf_markall(&fullsurf);
        vid_flush(&fullsurf);
        vid_wait_idle();
        b = wait_buttons(0);
        if (b & (BUTTON_A | BUTTON_START))
            return;
        if (b & (BUTTON_LEFT | BUTTON_UP))
        {
            if (page > 0) page--;
        }
        else if (++page >= PRINTFIL_PAGES)
            return;
    }
}

static boolean NewGameFlow(void);

/* the START.EXE menu; returns TRUE when a game was started */
static boolean UsersGuide(void)
{
    s16 sel = 0;
    for (;;)
    {
        u16 p;
        shell_show(DOC_MENU_FIRST, shell_menu_rows[sel],
            "UP/DOWN: choose    B or START: select    A: back to the title");
        p = wait_buttons(0);
        if (p & BUTTON_UP)
            sel = sel ? sel - 1 : 5;
        else if (p & BUTTON_DOWN)
            sel = (sel + 1) % 6;
        else if (p & BUTTON_A)
            return FALSE;
        else if (p & (BUTTON_B | BUTTON_C | BUTTON_START))
        {
            switch (sel)
            {
            case 0: Document(DOC_QUICK_FIRST, DOC_QUICK_PAGES); break;
            case 1: Document(DOC_INSTRUCT_FIRST, DOC_INSTRUCT_PAGES); break;
            case 2: Document(DOC_PRODUCTS_FIRST, DOC_PRODUCTS_PAGES); break;
            case 3: PrintFile(); break;
            case 4: Catalog(); break;
            case 5:
                if (NewGameFlow())
                    return TRUE;
                break;
            }
        }
    }
}

/* ------------------------------------------------------------ the intro */

/* "Dare you challenge the great Nemesis as Novice or Warrior?" */
static boolean NewGameFlow(void)
{
    u16 p;
    for (;;)
    {
        show_screen(scr_challenge);
        p = wait_buttons(0);
        if (p & BUTTON_B)
        {
            EASYMODEON = TRUE;
            show_screen(scr_novice);
        }
        else if (p & BUTTON_C)
        {
            EASYMODEON = FALSE;
            show_screen(scr_warrior);
        }
        else if (p & BUTTON_A)
            return FALSE;           /* [ESC] */
        else
            continue;
        snd_beep();
        if (wait_buttons(0) & BUTTON_A)
            return FALSE;
        show_screen(scr_story);
        wait_buttons(0);
        return TRUE;
    }
}

void IntroProgram(void)
{
    static const u8 * const attract[5] = {
        scr_madgamers, scr_softdisk, scr_title, scr_credits, scr_trilogy };
    u16 i = 0;

    InitGame();
    restore_requested = FALSE;
    starting_level = 0;
#ifdef START_LEVEL
    starting_level = START_LEVEL;
#endif

    /* "Prepare to enter the CATACOMB..." while INTRO.EXE loads */
    show_screen(scr_prepare);
    wait_buttons(TICKS_50MS(30));

    for (;;)
    {
        u16 p;
        show_screen(attract[i]);
        snd_beep();
        p = wait_buttons(TICKS_50MS(i == 2 ? 150 : 100));
        if (p & BUTTON_START)
        {
            if (NewGameFlow())
                break;
        }
        else if (p & BUTTON_B)
        {
            /* "Good!  Back for more I see!" */
            show_screen(scr_backformore);
            snd_beep();
            wait_buttons(TICKS_50MS(30));
            restore_requested = TRUE;
            break;
        }
        else if (p & BUTTON_C)
            Catalog();
        else if (p & BUTTON_A)
        {
            if (UsersGuide())
                break;
        }
        else
        {
            i = (i + 1) % 5;
            continue;
        }
        i = 2;                      /* back to the title */
    }
    IN_ClearKeysDown();
    VW_FadeOut();
}

/* LOADSCN: the closing screens after quitting the game (the Armageddon /
 * Apocalypse picture, then the LAST.ABS text screen) */
void EndScreens(void)
{
    show_screen(scr_armapc);
    wait_buttons(TICKS_50MS(200));
    show_screen(scr_last);
    wait_buttons(0);
    VW_FadeOut();
}
