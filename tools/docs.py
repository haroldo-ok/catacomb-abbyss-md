"""The documentation of THE CATACOMB ABYSS (HELP.TXT for the in-game help,
the START shell's .SHL files) adapted to the Genesis joypad, and the shell's
text mode page layout.

Only what describes PC keys, DOS, disks and printers is rewritten; all other
text is kept as written."""

# ------------------------------------------------------------ joypad text
# (HELP.TXT pages: '^' centres a line, a TAB starts the second column)

HELP_FOOTER = '^- B: play   LEFT/RIGHT: page   UP/DOWN: scroll -'

HELP_GETTING_STARTED = """^GETTING STARTED
^---------------
*  The Catacomb Abyss for the Sega Genesis / Mega Drive runs directly from the cartridge.

*  Up to three games can be saved in the cartridge's battery backed memory.  Press START during play and choose Save or Restore.

*  Both the 3 button and the 6 button control pads can be used.  The 6 button pad adds single buttons for the Xterminators, Cure Potions, Quick Turn and scrolls.

*  The sound effects can be played as AdLib or PC Speaker sounds, or turned off: press START during play and choose Sound.

""" + HELP_FOOTER

HELP_JOYPAD = """^JOYPAD CONTROLS
^---------------

Button...\tResult...
UP / DOWN\tMove Forward/Backward
LEFT / RIGHT\tTurn to the Left/Right
hold C + L/R\tMove sideways
hold A + L/R\tTurn quickly (Quick Turn)

B\tShoot a Magick Missile
A (tap)\tZapper: stream of missiles
hold A + B\tXterminator: missile circle
hold A + C\tCure yourself
START, Scrolls\tRead a scroll

^(The 6 button pad is on the next screen)

""" + HELP_FOOTER

HELP_SIXBUTTON = """^6 BUTTON CONTROL PAD
^--------------------


Button...\tResult...
X\tXterminator: missile circle
Y\tCure yourself
Z + L/R\tTurn quickly (Quick Turn)
MODE\tRead a scroll

^All the 3 button controls work
^on the 6 button pad as well.


""" + HELP_FOOTER

HELP_SYSTEM = """^SYSTEM FUNCTION CONTROLS
^------------------------

START opens the options menu.  LEFT/RIGHT chooses, B or START confirms, C cancels.

Option...\tResult...
Help\tHelp (these pages)
Sound\tNo Sound, PC Speaker or AdLib
Scrolls\tRead one of your scrolls
Save\tSave current game position
Restore\tRestore a saved game
New\tStart a new game
Quit\tQuit to the title screen

""" + HELP_FOOTER

HELP_SAVING = """^TIPS ABOUT SAVING GAMES
^-----------------------
*  Save your play status as often as you wish.  Selecting "Restore" after pressing START will let you return to any saved position which you had saved.  This can be one of the most useful techniques you can employ during play, allowing you to "experiment" with different play strategies.

*  The Save and Restore screens show the three save slots of the cartridge with the name of the level each game was saved in.

*  When all three slots are in use, saving over a slot replaces the game that was saved there; you will be asked to confirm.

""" + HELP_FOOTER

FOOTER_OLD = ['^-  ESC to return to play, or ARROW KEYS to page through more Help  -',
              '^-  ESC to return to play, or ARROW KEYS to page through Help  -']
FOOTER_NEW = HELP_FOOTER

PHRASES = [
    ('^Keyboard Controls\n\n^Mouse and Joystick Controls',
     '^Joypad Controls\n\n^6 Button Control Pad'),
    ('^< You can use a text editor to load and print this file, named HELP.TXT >',
     '^< These pages are also in the USER\'S GUIDE of the title screen >'),
    ('by pressing the appropriate number key related to the scroll.',
     'by pressing START and choosing Scrolls (or MODE on a 6 button pad).'),
    ('by pressing the CTRL key.', 'by pressing the B button.'),
    ('(press TAB or V while turning)', '(hold A, or Z on a 6 button pad, while turning)'),
    ('You can press ESC at any time to pause', 'You can press START at any time to pause'),
]


def help_pages(t):
    """HELP.TXT text -> list of pages adapted for the Genesis."""
    for o, n in PHRASES:
        assert o in t, o
        t = t.replace(o, n)
    for f in FOOTER_OLD:
        t = t.replace(f, FOOTER_NEW)
    pages = [p.strip('\n') for p in t.split('@')]
    pages = [p for p in pages if p.strip()]
    out = []
    for p in pages:
        title = p.strip('\n').split('\n')[0]
        if title.startswith('^GETTING STARTED'):
            out.append(HELP_GETTING_STARTED)
        elif title.startswith('^KEYBOARD CONTROLS'):
            out.append(HELP_JOYPAD)
        elif title.startswith('^MOUSE AND JOYSTICK'):
            out.append(HELP_SIXBUTTON)
        elif title.startswith('^SYSTEM FUNCTION'):
            out.append(HELP_SYSTEM)
        elif title.startswith('^TIPS ABOUT SAVING'):
            out.append(HELP_SAVING)
        else:
            out.append(p)
    return out


# -------------------------------------------------------- shell documents

SHELL_HEAD = '^BG4^FGe^CE*%s*\n^FG1^BG7\n'


def shell_title(t):
    return SHELL_HEAD % t.center(66)


SHELL_GETTING_STARTED = shell_title('GETTING STARTED') + """

^FG4THE CARTRIDGE
^FG1*  The Catacomb Abyss for the Sega Genesis / Mega Drive runs directly from the cartridge.  No installation is needed.


^FG4SAVED GAMES
^FG1*  Three games can be saved in the cartridge's battery backed memory.  Press START during play and choose Save or Restore.


^FG4CONTROL PADS
^FG1*  Plug a 3 button or a 6 button control pad into port 1.



^CE^FG8...more...
^EP
^FG4SOUND
^FG1*  The game's sound effects were made for the AdLib music card and for the PC speaker.  Both versions are on the cartridge: press START during play and choose Sound to select No Sound, PC Speaker or AdLib.


^FG4THE START MENU
^FG1*  During play, START pauses the game and opens the options menu in the message box at the bottom of the screen.  Choose an option with LEFT and RIGHT, confirm it with B or START, or go back to the game with C.
^EP
"""

SHELL_JOYPAD = shell_title('JOYPAD CONTROLS') + """


^FG4Buttons to press...       Result...
-------------------       ---------
^FG1UP/DOWN                   Move Forward/Backward
LEFT/RIGHT                Turn to the Left/Right
hold C + LEFT/RIGHT       Move sideways
hold A + LEFT/RIGHT       Turn quickly (Quick Turn)

B                         Shoot a single Magick Missile
A (press and release)     Zappers:  shoot a stream of missiles
hold A + press B          Xterminators:  shoot in a circular pattern
hold A + press C          Cure yourself
START, then Scrolls       Read one of your scrolls
^EP
"""

SHELL_SIXBUTTON = shell_title('6 BUTTON CONTROL PAD') + """


^FG4Button to press...        Result...
------------------        ---------
^FG1X                         Xterminators:  shoot in a circular pattern
Y                         Cure yourself
Z + LEFT/RIGHT            Turn quickly (Quick Turn)
MODE                      Read one of your scrolls




^FG4^CEAll the 3 button controls work on the 6 button pad as well.
^EP
"""

SHELL_SYSTEM = shell_title('SYSTEM FUNCTION CONTROLS') + """


^FG4START menu option...  Result...
--------------------  ---------
^FG1    Help              Help (the instructions)

    Sound             Select No Sound, PC Speaker, or AdLib

    Scrolls           Read one of your scrolls

    Save              Save current game position

    Restore           Restore a saved game position

    New               Start a new game

    Quit              Quit to the title screen
^EP
"""

SHELL_SYSCONFIG = shell_title('Recommended system configuration') + """
^TB5*  A Sega Genesis or Mega Drive console (NTSC or PAL)

^TB5*  A television or monitor; the game shows the 16 colors of the original EGA graphics

^FG1^TB5*  Sound effects of the AdLib and PC speaker versions

^TB5*  Game controlled by a 3 button or 6 button control pad


^FG8^CEFree Technical Support for this software
^CEis available by calling 1-318-221-8311.
^EP
"""

SHELL_PHRASES = {
    'INSTRUCT.SHL': [
        ('^TBa*  Keyboard Controls\n\n^TBa*  Mouse and Joystick controls',
         '^TBa*  Joypad Controls\n\n^TBa*  6 Button Control Pad'),
        ('^CEby pressing ^FG4[F1] ^FG8during actual game play.',
         '^CEby pressing ^FG4START^FG8 and choosing Help during actual game play.'),
        ('by pressing the appropriate number key related to the scroll.',
         'by pressing START and choosing Scrolls (or MODE on a 6 button pad).'),
        ('by pressing the CTRL key.', 'by pressing the B button.'),
        ('(press TAB or V while turning)', '(hold A, or Z on a 6 button pad, while turning)'),
        ('Selecting "Restore" after pressing ESC', 'Selecting "Restore" after pressing START'),
        ('You can press ESC at any time to pause', 'You can press START at any time to pause'),
    ],
    'QUICK.SHL': [],
    'PRODUCTS.SHL': [],
    'MENU.SHL': [
        ('^TBd  4. Printing options', '^TBd  4. Catalog information and order form'),
    ],
    'ORDER.SHL': [],
}


def printfil(t):
    """PRINTFIL.TXT (the electronic catalog's printable 'written
    information' with the order form) as shell markup: '^' lines are page
    breaks, '@' ends the text."""
    out = []
    for line in t.split('\n'):
        if line.strip() == '@':
            break
        if line.startswith('^') and not line[1:].strip():
            out.append('^EP')
            continue
        out.append(line.replace('%%', '%').rstrip())
    return '\n'.join(out)


def shell_pages_raw(t):
    return t.split('^EP')


def adapt_shell(name, t):
    for o, n in SHELL_PHRASES[name]:
        assert o in t, (name, o)
        t = t.replace(o, n)
    if name == 'INSTRUCT.SHL':
        pages = t.split('^EP')
        out = []
        skip = 0
        for i, p in enumerate(pages):
            if skip:
                skip -= 1
                continue
            head = p.lstrip('\n').split('\n')[0]
            if 'GETTING STARTED' in head:
                out.append(SHELL_GETTING_STARTED.rsplit('^EP', 1)[0])
                skip = 1            # the MEMORY page belongs to it
            elif 'KEYBOARD CONTROLS' in head:
                out.append(SHELL_JOYPAD.rsplit('^EP', 1)[0])
            elif 'MOUSE AND JOYSTICK' in head:
                out.append(SHELL_SIXBUTTON.rsplit('^EP', 1)[0])
            elif 'SYSTEM FUNCTION' in head:
                out.append(SHELL_SYSTEM.rsplit('^EP', 1)[0])
            elif 'TIPS ABOUT SAVING' in p:
                out.append(p.replace(
                    '*  If your disk becomes too full due to many saved games, you can delete the older ones to make room for more. The files end with ".SAV".',
                    '*  The cartridge keeps three saved games.  Saving over a slot that is in use replaces the game that was saved there; you will be asked to confirm.').replace(
                    '*  If you are playing on floppy disk instead of on a hard drive, you may wish to create a special empty "save game disk" for allowing a maximum number of saved games on disk.',
                    '').replace(
                    '*  The Save and Restore screens display your most recently saved game names.  Saved games older than the most recent ones are still available for Restoring by entering the appropriate name.',
                    '*  The Save and Restore screens show the three save slots with the name of the level each game was saved in.'))
            else:
                out.append(p)
        t = '^EP'.join(out)
    elif name == 'QUICK.SHL':
        pages = t.split('^EP')
        for i, p in enumerate(pages):
            head = p.lstrip('\n').split('\n')[0]
            if 'Recommended system configuration' in head and head.startswith('^BG4'):
                    pages[i] = SHELL_SYSCONFIG.rsplit('^EP', 1)[0]
        t = '^EP'.join(pages)
    return t


# ------------------------------------------------------ text mode layout

WIN_X, WIN_Y, WIN_W, WIN_H = 4, 2, 72, 20


def layout(t, fg=0, bg=7, WIN_W=WIN_W, WIN_H=WIN_H):
    """Lays the shell markup out in the 72x20 window of BKGND.SHL.
    Returns pages, each a list of rows of (char, attr) and the rows where
    option lines '<n>.' start (for the menu)."""
    pages = []
    cur = []
    marks = {}

    def newrow():
        cur.append([None] * WIN_W)

    state = {'fg': fg, 'bg': bg}

    def flush_page():
        nonlocal cur
        pages.append(cur)
        cur = []

    def emit_line(cells, center):
        # cells: list of (ch, attr); wrap at WIN_W
        line = cells
        rows = []
        while True:
            if len(line) <= WIN_W:
                rows.append(line)
                break
            cut = WIN_W
            while cut > 0 and line[cut][0] != ' ':
                cut -= 1
            if cut == 0:
                cut = WIN_W
            rows.append(line[:cut])
            rest = line[cut:]
            while rest and rest[0][0] == ' ':
                rest = rest[1:]
            line = rest
        for r in rows:
            if center:
                text = r
                while text and text[-1][0] == ' ' and text[-1][1] >> 4 == bg:
                    text = text[:-1]
                pad = max(0, (WIN_W - len(text)) // 2)
                r = [None] * pad + text
            if len(cur) >= WIN_H:
                flush_page()
            cur.append(r)

    for para in t.split('\n'):
        if para.startswith('^EP') or para == '^EP':
            flush_page()
            para = para[3:]
            if not para:
                continue
        cells = []
        center = False
        i = 0
        while i < len(para):
            c = para[i]
            if c == '^' and i + 2 < len(para) and para[i + 1:i + 3] in ('BG', 'FG', 'TB'):
                code = para[i + 1:i + 3]
                v = int(para[i + 3], 16)
                if code == 'BG':
                    state['bg'] = v & 7
                elif code == 'FG':
                    state['fg'] = v
                else:
                    while len(cells) < v:
                        cells.append(None)
                i += 4
                continue
            if c == '^' and para[i + 1:i + 3] == 'CE':
                center = True
                i += 3
                continue
            if c == '^' and para[i + 1:i + 3] == 'EP':
                i += 3
                continue
            cells.append((c, (state['bg'] << 4) | state['fg']))
            i += 1
        if para and not any(c is not None for c in cells):
            continue            # a line of colour / tab codes only
        stripped = ''.join(ch[0] if ch else ' ' for ch in cells).strip()
        if len(stripped) > 1 and stripped[0] in '123456' and stripped[1] == '.':
            marks[int(stripped[0])] = (len(pages), len(cur))
        emit_line(cells, center)
    if cur:
        flush_page()
    # drop empty pages
    out = []
    for p in pages:
        if any(any(c is not None and c[0] != ' ' for c in r) for r in p):
            out.append(p)
    return out, marks


def page_bytes(page, fg=0, bg=7, WIN_W=WIN_W, WIN_H=WIN_H):
    """72x20 (char, attr) cells; empty cells are spaces in the window colour."""
    blank = (ord(' '), (bg << 4) | fg)
    data = bytearray()
    for r in range(WIN_H):
        row = page[r] if r < len(page) else []
        for c in range(WIN_W):
            cell = row[c] if c < len(row) else None
            if cell is None:
                data += bytes(blank)
            else:
                data += bytes((ord(cell[0]) & 0xFF, cell[1]))
    return bytes(data)
