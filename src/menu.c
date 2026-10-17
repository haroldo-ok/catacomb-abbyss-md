/* menu.c - joypad menus replacing the keyboard prompts of the original:
 * GetKeyChoice(), the F2 sound menu, the scroll keys 1-8, DisplayText()
 * (HELP.TXT viewer) and GE_SaveGame()/GE_LoadGame(). */
#include "def.h"
#include "snd.h"
#include "vid.h"
#include "draw2d.h"
#include "input.h"
#include "menu.h"
#include "save.h"

#define STATUSCOLOR 8
#define TEXTCOLOR   14

static const char *choice_label(char c)
{
    switch (c)
    {
    case 'Y': return "Yes";
    case 'N': return "No";
    case 'S': return "Save";
    case 'R': return "Restore";
    case 'W': return "New";
    case 'Q': return "Quit";
    case 'H': return "Help";
    case 'O': return "Sound";
    case 'L': return "Scrolls";
    }
    return "?";
}

static void text_at(s16 x, s16 y, const char *s, u16 color)
{
    boolean ox = fontxor;
    byte oc = fontcolor;
    fontxor = FALSE;
    fontcolor = color;
    px = x;
    py = y;
    VW_DrawPropString(s);
    fontxor = ox;
    fontcolor = oc;
}

static u16 text_w(const char *s)
{
    u16 w, h;
    VW_MeasurePropString(s, &w, &h);
    return w;
}

char MenuChoice(const char *prompt, const char *choices)
{
    u16 n = strlen(choices), i, sel = 0, total, gap;
    s16 x0;
    const u16 bx = 20, bw = 270;

    IN_ClearKeysDown();
    for (;;)
    {
        cursurf = &statsurf;
        VW_Bar(bx, 2, bw, 8, STATUSCOLOR);
        total = prompt ? text_w(prompt) : 0;
        for (i = 0; i < n; i++)
            total += text_w(choice_label(choices[i]));
        gap = n ? (bw - 8 - total) / (n + (prompt && *prompt ? 1 : 0)) : 0;
        if (gap > 24) gap = 24;
        total += gap * (n - 1 + (prompt && *prompt ? 1 : 0));
        x0 = bx + (bw - total) / 2;
        if (prompt && *prompt)
        {
            text_at(x0, 1, prompt, TEXTCOLOR);
            x0 += text_w(prompt) + gap;
        }
        for (i = 0; i < n; i++)
        {
            const char *l = choice_label(choices[i]);
            u16 w = text_w(l);
            if (i == sel)
            {
                VW_Bar(x0 - 2, 2, w + 4, 8, TEXTCOLOR);
                text_at(x0, 1, l, 0);
            }
            else
                text_at(x0, 1, l, 7);
            x0 += w + gap;
        }
        vid_flush(&statsurf);

        {
            u16 p = IN_WaitPress();
            if (p & (BUTTON_LEFT | BUTTON_UP))
                sel = sel ? sel - 1 : n - 1;
            else if (p & (BUTTON_RIGHT | BUTTON_DOWN))
                sel = (sel + 1) % n;
            else if (p & (BUTTON_B | BUTTON_START))
            {
                IN_ClearKeysDown();
                return choices[sel];
            }
            else if (p & (BUTTON_C | BUTTON_A))
            {
                IN_ClearKeysDown();
                return 0;
            }
        }
    }
}

s16 MenuWindow(const char *title, u16 count, const char * const *items, s16 sel)
{
    u16 i, top;
    if (sel < 0) sel = 0;
    PrintY = WindowY;
    if (title)
        US_CPrint(title);
    top = PrintY;
    IN_ClearKeysDown();
    for (;;)
    {
        for (i = 0; i < count; i++)
        {
            u16 w = text_w(items[i]);
            s16 x = WindowX + (WindowW - w) / 2;
            s16 y = top + i * 10;
            VW_Bar(WindowX, y + 1, WindowW, 10, LT_GREY);
            if (i == (u16)sel)
            {
                VW_Bar(x - 3, y + 1, w + 6, 10, 4);
                text_at(x, y, items[i], 15);
            }
            else
                text_at(x, y, items[i], 8);
        }
        vid_update();
        if (screenfaded)
            VW_FadeIn();
        {
            u16 p = IN_WaitPress();
            if (p & BUTTON_UP)
                sel = sel ? sel - 1 : count - 1;
            else if (p & BUTTON_DOWN)
                sel = (sel + 1) % count;
            else if (p & (BUTTON_B | BUTTON_START))
            {
                IN_ClearKeysDown();
                return sel;
            }
            else if (p & (BUTTON_C | BUTTON_A))
            {
                IN_ClearKeysDown();
                return -1;
            }
        }
    }
}

s16 MenuScroll(void)
{
    const char *items[8];
    s16 idx[8];
    static char names[8][12];
    u16 n = 0, i;
    s16 r;
    for (i = 0; i < 8; i++)
        if (gamestate.scrolls[i])
        {
            strcpy(names[n], "Scroll  ");
            names[n][7] = '1' + i;
            items[n] = names[n];
            idx[n] = i;
            n++;
        }
    if (!n)
    {
        SD_PlaySound(NOITEMSND);
        return -1;
    }
    CenterWindow(14, n + 2);
    r = MenuWindow("Read which scroll?", n, items, 0);
    lasttext = -1;
    if (r < 0)
        return -1;
    return idx[r];
}

/* ------------------------------------------------------------ help text */

/* Word wrapping identical to PrintPropText(); calls out() for each line */
static void wrap_text(const char *text, s16 left, s16 right,
                      void (*out)(const char *line, boolean center, void *ctx), void *ctx)
{
    char pb[200];
    s16 length, maxend, maxx, curx;
    boolean centerit;

    while (*text)
    {
        if (*text == '^')
            centerit = TRUE, text++;
        else
            centerit = FALSE;

        curx = left;
        length = 0;
        {
            /* table lines (with a TAB) are never wrapped */
            const char *e = text;
            while (*e && *e != '\n' && *e != '\t') e++;
            if (*e == '\t')
            {
                while (*e && *e != '\n') e++;
                length = e - text;
                if (*e == '\n') length++;
                goto haveline;
            }
        }
        while ((curx + font_widths[(u8)text[length + 1]] < right) &&
               (text[length]) && (text[length] != '\n'))
            curx += font_widths[(u8)text[length++]];

        if ((text[length]) && (text[length] != '\n'))
        {
            maxx = curx;
            maxend = length;
            while ((length) && (text[length] != ' '))
                curx -= font_widths[(u8)text[length--]];
            if (!length)
                length = maxend, curx = maxx;
        }

        if ((text[length] == ' ') || (text[length] == '\n'))
            length++;
haveline:

        if (length > 199) length = 199;
        memcpy(pb, text, length);
        pb[length] = 0;
        if (length && pb[length - 1] == '\n')
            pb[length - 1] = 0;
        out(pb, centerit, ctx);
        text += length;
        if (!length) break;
    }
}

typedef struct { s16 line, first, count; } HelpCtx;

static void help_out(const char *s, boolean center, void *vctx)
{
    HelpCtx *c = vctx;
    if (c->line >= c->first && c->line < c->first + c->count)
    {
        const char *tab = strchr(s, '\t');
        if (tab)
        {
            /* two column table line */
            char left[100];
            u16 n = tab - s;
            if (n > 99) n = 99;
            memcpy(left, s, n);
            left[n] = 0;
            PrintX = PPT_LeftEdge + 4;
            US_Print(left);
            PrintX = PPT_LeftEdge + 112;
            US_Print(tab + 1);
            US_Print("\n");
        }
        else if (center)
            US_CPrintLine(s);
        else
        {
            PrintX = PPT_LeftEdge;
            US_Print(s);
            US_Print("\n");
        }
    }
    c->line++;
}

static void count_out(const char *s, boolean center, void *vctx)
{
    ((HelpCtx *)vctx)->line++;
}

void HelpScreens(void)
{
    s16 page = 0, first = 0, lines;
    const s16 visible = 17;
    HelpCtx ctx;
    boolean redraw = TRUE;

    VW_FadeOut();
    vid_fullscreen(true);
    cursurf = &fullsurf;
    VW_ClearSurface(0);
    PPT_LeftEdge = 16;
    PPT_RightEdge = 304;

    for (;;)
    {
        if (redraw)
        {
            ctx.line = 0;
            wrap_text(help_pages[page], PPT_LeftEdge, PPT_RightEdge, count_out, &ctx);
            lines = ctx.line;
            US_DrawWindow(1, 1, 38, 23);
            PrintY = 12;
            ctx.line = 0;
            ctx.first = first;
            ctx.count = visible;
            wrap_text(help_pages[page], PPT_LeftEdge, PPT_RightEdge, help_out, &ctx);
            {
                char buf[24];
                strcpy(buf, "Page ");
                intToStr(page + 1, buf + 5, 1);
                strcat(buf, " of ");
                intToStr(HELP_PAGES, buf + strlen(buf), 1);
                PrintY = 184;
                US_CPrintLine(buf);
            }
            vid_update();
            if (screenfaded)
                VW_FadeIn();
            redraw = FALSE;
        }
        {
            u16 p = IN_WaitPress();
            if (p & (BUTTON_B | BUTTON_C | BUTTON_START))
                break;
            if ((p & BUTTON_LEFT) && page > 0) { page--; first = 0; redraw = TRUE; }
            if ((p & BUTTON_RIGHT) && page < HELP_PAGES - 1) { page++; first = 0; redraw = TRUE; }
            if ((p & BUTTON_UP) && first > 0) { first -= 8; if (first < 0) first = 0; redraw = TRUE; }
            if ((p & BUTTON_DOWN) && first + visible < lines) { first += 8; redraw = TRUE; }
        }
    }
    IN_ClearKeysDown();
    VW_FadeOut();
    vid_fullscreen(false);
    cursurf = &statsurf;
    if (ingame)
    {
        VW_DrawPic (0,0,STATUSPIC);
        RedrawStatusWindow();
        ThreeDRefresh();
        VW_FadeIn();
    }
    tics = realtics = 1;
}

/* --------------------------------------------------------- save / load */

static s16 SlotMenu(const char *title)
{
    static char names[SAVE_SLOTS][40];
    const char *items[SAVE_SLOTS];
    u16 i;
    for (i = 0; i < SAVE_SLOTS; i++)
    {
        char desc[32];
        strcpy(names[i], "Slot ");
        names[i][5] = '1' + i;
        names[i][6] = 0;
        strcat(names[i], ":  ");
        if (save_slot_info(i, desc, sizeof(desc)))
            strcat(names[i], desc);
        else
            strcat(names[i], "- empty -");
        items[i] = names[i];
    }
    VW_ClearSurface(0);
    US_CenterWindow(34, 3 + SAVE_SLOTS + 2);
    return MenuWindow(title, SAVE_SLOTS, items, 0);
}

static void MessageBox(const char *l1, const char *l2)
{
    US_CenterWindow(28, 4);
    PrintY += 4;
    US_CPrintLine(l1);
    if (l2) US_CPrintLine(l2);
    vid_update();
    if (screenfaded)
        VW_FadeIn();
    IN_Ack();
}

void GE_SaveGame(void)
{
    s16 slot;
    char desc[32];

    for (;;)
    {
        cursurf = &fullsurf;
        slot = SlotMenu("\nChoose a slot to SAVE this game:\n");
        if (slot < 0)
            break;
        if (save_slot_info(slot, desc, sizeof(desc)))
        {
            static const char * const yn[2] = {"Yes", "No"};
            US_CenterWindow(24, 6);
            US_CPrintLine("That slot is in use...");
            if (MenuWindow("\nOverwrite it ????\n", 2, yn, 1) != 0)
                continue;
        }
        if (!SaveTheGame(slot))
            Quit("Save game error");
        MessageBox("Game saved.", NULL);
        break;
    }
    IN_ClearKeysDown();
    if (!screenfaded)
        VW_FadeOut();
}

boolean GE_LoadGame(void)
{
    s16 slot;
    char desc[32];
    boolean rt = false;

    IN_ClearKeysDown();
    for (;;)
    {
        cursurf = &fullsurf;
        slot = SlotMenu("\nChoose the game to RESTORE:\n");
        if (slot < 0)
            break;
        if (!save_slot_info(slot, desc, sizeof(desc)))
        {
            MessageBox("That slot is empty....", "Press a button to try again.");
            continue;
        }
        if (!LoadTheGame(slot))
            Quit("Load game error.");
        rt = true;
        break;
    }
    IN_ClearKeysDown();
    if (!screenfaded)
        VW_FadeOut();
    return rt;
}
