"""Readers for the id/Softdisk "Catacomb 3-D" engine data files:
EGAGRAPH.ABS (huffman, headers linked into the EXE -> ABSE*.OBJ),
GAMEMAPS.ABS (carmack + RLEW, MAPHEAD linked -> ABSMHEAD.OBJ),
AUDIO.ABS (huffman, ABSA*.OBJ)."""
import os
import struct

import omf

SRC = os.environ.get('CATABYSS_SRC', '/home/claude/src/CatacombAbyss')
DATA = os.environ.get('CATABYSS_DATA', '/home/claude/data/zip')

# chunk layout (GFXE_ABS.H)
NUMCHUNKS = 626
STRUCTPIC, STRUCTPICM, STRUCTSPRITE = 0, 1, 2
STARTFONT = 3
STARTPICS = 4
STARTPICM = 313
STARTSPRITES = 314
STARTTILE8 = 315
STARTTILE8M = 316
STARTTILE16 = 317
STARTTILE16M = 533
STARTEXTERNS = 605
NUMPICS = 309
NUMTILE8 = 104
NUMTILE8M = 12


def huff_expand(src, length, dict_nodes, pos=0):
    out = bytearray()
    head = 254
    node = head
    while len(out) < length:
        b = src[pos]
        pos += 1
        for bit in range(8):
            v = dict_nodes[node][(b >> bit) & 1]
            if v < 256:
                out.append(v)
                node = head
                if len(out) >= length:
                    break
            else:
                node = v - 256
    return bytes(out)


def _nodes(raw):
    return [struct.unpack_from('<HH', raw, i * 4) for i in range(255)]


_EXE = None


def exe_image():
    """Decompressed CATABYSS.EXE load image (the shareware v1.13 game binary).
    Its data segment holds the linked header tables that match the shipped
    data files (the GPL source release's .OBJ headers are a later build)."""
    global _EXE
    if _EXE is None:
        import unlzexe
        _EXE = unlzexe.unlzexe(open(os.path.join(DATA, 'CATABYSS.EXE'), 'rb').read())[0]
    return _EXE


def linked(name):
    exe = exe_image()
    size = {'EGAhead': (NUMCHUNKS + 1) * 3, 'EGAdict': 1024, 'audiohead': 104 * 4,
            'audiodict': 1024, 'maphead': 834}[name]
    if name == 'EGAhead':
        sz = os.path.getsize(os.path.join(DATA, 'EGAGRAPH.ABS'))
        end = exe.find(bytes([sz & 255, (sz >> 8) & 255, sz >> 16]))
        return exe[end + 3 - size:end + 3]
    if name == 'EGAdict':
        # dictionary: 255 nodes, last node (254) is the head; locate by the
        # property that every node value is < 256+255
        tail = omf.extract(os.path.join(SRC, 'ABSEDICT.OBJ'))[0][-32:]
        end = exe.find(tail) + 32
        return exe[end - size:end]
    if name == 'audiohead':
        sz = os.path.getsize(os.path.join(DATA, 'AUDIO.ABS'))
        end = exe.find(struct.pack('<I', sz)) + 4
        return exe[end - size:end]
    if name == 'audiodict':
        o = omf.extract(os.path.join(SRC, 'ABSADICT.OBJ'))[0]
        i = exe.find(o[:64])
        return exe[i:i + size]
    if name == 'maphead':
        o = omf.extract(os.path.join(SRC, 'ABSMHEAD.OBJ'))[0]
        i = exe.find(o[:6])
        return exe[i:i + size]


class Graphics:
    def __init__(self):
        self.dict = _nodes(linked('EGAdict'))
        head = linked('EGAhead')
        self.starts = []
        for i in range(NUMCHUNKS + 1):
            v = head[i * 3] | (head[i * 3 + 1] << 8) | (head[i * 3 + 2] << 16)
            self.starts.append(-1 if v == 0xFFFFFF else v)
        self.file = open(os.path.join(DATA, 'EGAGRAPH.ABS'), 'rb').read()
        pt = self.chunk(STRUCTPIC, explicit=True)
        self.pictable = [struct.unpack_from('<hh', pt, i * 4) for i in range(NUMPICS)]
        pm = self.chunk(STRUCTPICM, explicit=True)
        self.picmtable = [struct.unpack_from('<hh', pm, 0)]
        st = self.chunk(STRUCTSPRITE, explicit=True)
        self.spritetable = [struct.unpack_from('<9h', st, 0)]

    def raw(self, c):
        pos = self.starts[c]
        if pos < 0:
            return None
        n = c + 1
        while self.starts[n] == -1:
            n += 1
        return self.file[pos:self.starts[n]]

    def chunk(self, c, explicit=None, length=None):
        r = self.raw(c)
        if r is None:
            return None
        if length is None:
            if c >= STARTTILE8 and c < STARTEXTERNS:
                if c < STARTTILE8M:
                    length = 32 * NUMTILE8
                elif c < STARTTILE16:
                    length = 40 * NUMTILE8M
                elif c < STARTTILE16M:
                    length = 32 * 4
                else:
                    length = 40 * 4
                return huff_expand(r, length, self.dict)
            length = struct.unpack_from('<I', r, 0)[0]
            return huff_expand(r, length, self.dict, 4)
        return huff_expand(r, length, self.dict)

    def pic_size(self, c):
        w, h = self.pictable[c - STARTPICS]
        return w, h      # width in bytes


class Maps:
    def __init__(self):
        mh = linked('maphead')
        self.rlewtag = struct.unpack_from('<H', mh, 0)[0]
        self.offsets = struct.unpack_from('<100i', mh, 2)
        self.tinf = mh[402:]
        self.file = open(os.path.join(DATA, 'GAMEMAPS.ABS'), 'rb').read()

    @staticmethod
    def carmack(src, length):
        out = []
        words = length // 2
        p = 0
        while len(out) < words:
            ch = src[p] | (src[p + 1] << 8)
            p += 2
            hi = ch >> 8
            if hi in (0xA7, 0xA8):
                count = ch & 0xFF
                if count == 0:
                    ch |= src[p]
                    p += 1
                    out.append(ch)
                elif hi == 0xA7:
                    off = src[p]; p += 1
                    base = len(out) - off
                    for k in range(count):
                        out.append(out[base + k])
                else:
                    off = src[p] | (src[p + 1] << 8); p += 2
                    for k in range(count):
                        out.append(out[off + k])
            else:
                out.append(ch)
        return out

    def rlew(self, words, size):
        out = []
        i = 0
        while len(out) < size:
            w = words[i]; i += 1
            if w == self.rlewtag:
                cnt = words[i]; val = words[i + 1]; i += 2
                out.extend([val] * cnt)
            else:
                out.append(w)
        return out[:size]

    def level(self, n):
        pos = self.offsets[n]
        if pos <= 0:
            return None
        ps = struct.unpack_from('<3i', self.file, pos)
        pl = struct.unpack_from('<3H', self.file, pos + 12)
        w, h = struct.unpack_from('<HH', self.file, pos + 18)
        name = self.file[pos + 22:pos + 38].split(b'\0')[0].decode('latin1')
        planes = []
        for i in range(3):
            if not pl[i]:
                planes.append(None)
                continue
            src = self.file[ps[i]:ps[i] + pl[i]]
            expanded = struct.unpack_from('<H', src, 0)[0]
            cw = self.carmack(src[2:], expanded)
            planes.append(self.rlew(cw[1:], w * h))
        return dict(name=name, w=w, h=h, planes=planes)


class Audio:
    def __init__(self):
        self.dict = _nodes(linked('audiodict'))
        head = linked('audiohead')
        self.starts = struct.unpack_from('<%di' % (len(head) // 4), head)
        self.file = open(os.path.join(DATA, 'AUDIO.ABS'), 'rb').read()

    def chunk(self, c):
        a, b = self.starts[c], self.starts[c + 1]
        if b - a <= 0:
            return None
        r = self.file[a:b]
        length = struct.unpack_from('<I', r, 0)[0]
        return huff_expand(r, length, self.dict, 4)
