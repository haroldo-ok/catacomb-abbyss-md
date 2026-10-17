"""Full screen pictures of the Gamer's Edge front end (INTRO.EXE's SHPxx.ABS
screens, DEMOCAT's catalog pages, LOADSCN's text screens) converted to raw
320x200 4bpp buffers in the Genesis full screen surface layout (40 columns of
8 pixels x 200 rows, one long per 8 pixel row) that the game unpacks straight
into its screen buffer.

Texts that name PC keys are redrawn for the joypad."""
import os
import struct

import egaimg
import font4x8
import iddata
import lzh

W, H = 320, 200

# ------------------------------------------------------------------ fonts

# the 3x6 shadowed lettering of the title bar / status bar key hints
HINT = {
    'A': ['.#.', '#.#', '#.#', '###', '#.#', '#.#'],
    'B': ['##.', '#.#', '##.', '#.#', '#.#', '##.'],
    'C': ['.##', '#..', '#..', '#..', '#..', '.##'],
    'D': ['##.', '#.#', '#.#', '#.#', '#.#', '##.'],
    'E': ['###', '#..', '##.', '#..', '#..', '###'],
    'F': ['###', '#..', '##.', '#..', '#..', '#..'],
    'G': ['.##', '#..', '#..', '#.#', '#.#', '.##'],
    'I': ['#', '#', '#', '#', '#', '#'],
    'M': ['#...#', '##.##', '#.#.#', '#...#', '#...#', '#...#'],
    'N': ['#..#', '##.#', '#.##', '#..#', '#..#', '#..#'],
    'O': ['.#.', '#.#', '#.#', '#.#', '#.#', '.#.'],
    'R': ['##.', '#.#', '#.#', '##.', '#.#', '#.#'],
    'S': ['.##', '#..', '.#.', '..#', '..#', '##.'],
    'T': ['###', '.#.', '.#.', '.#.', '.#.', '.#.'],
    'U': ['#.#', '#.#', '#.#', '#.#', '#.#', '###'],
    'W': ['#...#', '#...#', '#...#', '#.#.#', '##.##', '#...#'],
    "'": ['#', '#'],
    '-': ['....', '....', '####', '....', '....', '....'],
    ' ': ['..', '..', '..', '..', '..', '..'],
}


def hint_width(parts):
    return sum(sum(len(HINT[ch][0]) + 1 for ch in t) for t, col in parts)


def draw_hint(pix, w, x, y, parts, shadow=(1, 1)):
    for t, col in parts:
        for ch in t:
            g = HINT[ch]
            for gy, row in enumerate(g):
                for gx, v in enumerate(row):
                    if v == '#':
                        pix[(y + gy + shadow[1]) * w + x + gx + shadow[0]] = 0
            for gy, row in enumerate(g):
                for gx, v in enumerate(row):
                    if v == '#':
                        pix[(y + gy) * w + x + gx] = col
            x += len(g[0]) + 1
    return x


class PropFont:
    """The game's proportional font (STARTFONT in EGAGRAPH)."""

    def __init__(self, g):
        f = g.chunk(iddata.STARTFONT)
        self.h = struct.unpack_from('<h', f, 0)[0]
        self.loc = struct.unpack_from('<256h', f, 2)
        self.wid = list(f[514:770])
        self.data = f

    def width(self, s):
        return sum(self.wid[ord(c)] for c in s)

    def draw(self, pix, w, x, y, s, col):
        for c in s:
            cw = self.wid[ord(c)]
            bpr = (cw + 7) >> 3
            off = self.loc[ord(c)]
            for r in range(self.h):
                for i in range(cw):
                    if self.data[off + r * bpr + (i >> 3)] & (0x80 >> (i & 7)):
                        pix[(y + r) * w + x + i] = col
            x += cw
        return x


# --------------------------------------------------------------- pictures

def load_ilbm(name):
    b = egaimg.decode_ilbm(lzh.load(os.path.join(iddata.DATA, name)))
    trans = b.get('trans', 0) if b['mask'] == 2 else None
    return b['w'], b['h'], list(b['pixels']), trans


def blank():
    return [0] * (W * H)


def blit(dst, src_name, x, y):
    """Opaque copy: the ILBM 'transparent colour' fields of these files are
    not used by the Gamer's Edge loaders (GRATE.CAT names dark grey)."""
    w, h, pix, trans = load_ilbm(src_name)
    for yy in range(h):
        for xx in range(w):
            p = pix[yy * w + xx]
            if 0 <= x + xx < W and 0 <= y + yy < H:
                dst[(y + yy) * W + x + xx] = p
    return w, h


def picture(name):
    dst = blank()
    blit(dst, name, 0, 0)
    return dst


def fill(pix, x0, y0, x1, y1, col):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            pix[y * W + x] = col


def patch_title(pix):
    """SHP02: 'ENTER-NEW GAME  F4-RESTORE GAME  F10-DEMO  ESC-EXIT'."""
    # inner bar: black frame line at row 186, text rows 187..193
    x0 = 6
    x1 = x0
    while pix[186 * W + x1 + 1] == 0:
        x1 += 1
    fill(pix, x0 + 1, 187, x1 - 1, 193, 8)
    items = [[('START-', 12), ('NEW GAME', 9)],
             [('B-', 12), ('RESTORE GAME', 9)],
             [('C-', 12), ('DEMO', 9)],
             [('A-', 12), ("USER'S GUIDE", 9)]]
    widths = [hint_width(p) for p in items]
    gap = (x1 - x0 - 8 - sum(widths)) // (len(items) - 1)
    x = x0 + 5
    for p, wd in zip(items, widths):
        draw_hint(pix, W, x, 187, p, shadow=(-1, 1))
        x += wd + gap


def patch_challenge(pix, font):
    """SHP07: '(PRESS N or W)'."""
    fill(pix, 0, 188, W - 1, 199, 0)
    parts = [('(', 9), ('B', 12), (' = Novice      ', 9), ('C', 12), (' = Warrior)', 9)]
    tw = sum(font.width(t) for t, c in parts)
    x = (W - tw) // 2
    for t, c in parts:
        x = font.draw(pix, W, x, 188, t, c)


def patch_trilogy(pix):
    """SHP11: 'Press F10 now or type DEMO at DOS' -> 'Press C now'.  The
    letters are cut from the picture itself; C is DEMO's O opened up."""
    y0, y1 = 172, 184
    cols = [any(pix[y * W + x] for y in range(y0, y1)) for x in range(W)]
    words = []
    x = 0
    while x < W:
        if cols[x]:
            s = x
            while x < W and any(cols[x:x + 3]):
                x += 1
            words.append((s, x))
        else:
            x += 1
    assert len(words) == 8, words

    def cut(a, b):
        return [[pix[y * W + xx] for xx in range(a, b)] for y in range(y0, y1)]

    press = cut(*words[0])
    now = cut(*words[2])
    o = cut(words[5][1] - 8, words[5][1])
    for r in range(3, 7):
        for c in range(5, 8):
            o[r][c] = 0
    o[7][6] = o[7][7] = 0
    space = 5
    line = [press, None, o, None, now]
    width = sum(len(p[0]) if p else space for p in line)
    fill(pix, 0, y0, W - 1, y1 - 1, 0)
    x = (W - width) // 2
    for p in line:
        if p is None:
            x += space
            continue
        for r, row in enumerate(p):
            for c, v in enumerate(row):
                pix[(y0 + r) * W + x + c] = v
        x += len(p[0])


def text_screen(name, patches=()):
    d = bytearray(lzh.load(os.path.join(iddata.DATA, name))[7:7 + 4000])
    for row, old, new in patches:
        line = bytes(d[row * 160 + i * 2] for i in range(80)).decode('latin1')
        i = line.index(old)
        new = new.center(len(old)) if len(new) < len(old) else new
        assert len(new) == len(old), (new, old)
        for k, ch in enumerate(new):
            d[row * 160 + (i + k) * 2] = ord(ch)
    return font4x8.render_text_screen(d)


def pack(pix):
    """320x200 indices -> 32000 bytes, 8 pixel columns of 200 rows."""
    out = bytearray()
    for bx in range(W // 8):
        for y in range(H):
            v = 0
            for x in range(8):
                v = (v << 4) | (pix[y * W + bx * 8 + x] & 15)
            out += struct.pack('>I', v)
    return bytes(out)


def build(g):
    """Returns [(symbol, pixels)] and the catalog page order."""
    font = PropFont(g)
    shots = []

    def add(sym, pix):
        shots.append((sym, pix))

    # INTRO.EXE
    p = blank()
    w, h, _, _ = load_ilbm('SHP05.ABS')
    blit(p, 'SHP05.ABS', (W - w) // 2, (H - h) // 2)
    add('scr_prepare', p)
    add('scr_softdisk', picture('SHP01.ABS'))
    p = picture('SHP02.ABS')
    patch_title(p)
    add('scr_title', p)
    add('scr_credits', picture('SHP03.ABS'))
    add('scr_story', picture('SHP04.ABS'))
    p = picture('SHP07.ABS')
    patch_challenge(p, font)
    add('scr_challenge', p)
    for sym, f in (('scr_novice', 'SHP08.ABS'), ('scr_warrior', 'SHP09.ABS'),
                   ('scr_backformore', 'SHP10.ABS')):
        p = picture('SHP06.ABS')
        w, h, _, _ = load_ilbm(f)
        blit(p, f, (W - w) // 2, 192)
        add(sym, p)
    p = picture('SHP11.ABS')
    patch_trilogy(p)
    add('scr_trilogy', p)
    add('scr_madgamers', picture('SHP12.ABS'))
    add('scr_armapc', picture('ARMAPC.ABS'))

    # DEMOCAT: SCRIPT.CAT order; 130 line pages replace the top of the page
    # before them (precomposed here, the catalog always shows them in order)
    script = lzh.load(os.path.join(iddata.DATA, 'SCRIPT.CAT')).decode('latin1')
    cats = [l[2:].strip() for l in script.replace('\r', '').split('\n') if l.startswith('^E')]
    prev = blank()
    order = []
    for c in cats:
        p = list(prev)
        blit(p, c, 0, 0)
        sym = 'cat_' + c.split('.')[0].lower()
        add(sym, p)
        order.append(sym)
        prev = p

    # LOADSCN text screens
    add('scr_last', text_screen('LAST.ABS', [
        (22, 'Type START, then [ENTER] to learn more!',
             'Press A at the title to learn more!')]))
    add('scr_lastcat', text_screen('LAST.CAT'))
    add('scr_shellbg', text_screen('BKGND.SHL'))
    return shots, order
