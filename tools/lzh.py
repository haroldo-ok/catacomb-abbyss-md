"""Softdisk "CMP1" container + Okumura LZHUF decoder (F=30 variant used by
the Gamer's Edge shell/intro files: *.SHL, *.CAT, SHPxx.ABS, LAST.ABS ...).

Ported from LZHUF.C in the Catacomb Apocalypse GPL source release."""
import struct

N = 4096
F = 30
THRESHOLD = 2
N_CHAR = 256 - THRESHOLD + F
T = N_CHAR * 2 - 1
R = T - 1
MAX_FREQ = 0x8000

d_code = []
for v, n in [(0, 32), (1, 16), (2, 16), (3, 16), (4, 8), (5, 8), (6, 8), (7, 8),
             (8, 8), (9, 8), (10, 8), (11, 8), (12, 4), (13, 4), (14, 4), (15, 4),
             (16, 4), (17, 4), (18, 4), (19, 4), (20, 4), (21, 4), (22, 4), (23, 4)]:
    d_code += [v] * n
for v in range(24, 48):
    d_code += [v] * 2
for v in range(48, 64):
    d_code += [v]
assert len(d_code) == 256

d_len = [3] * 32 + [4] * 48 + [5] * 64 + [6] * 48 + [7] * 48 + [8] * 16
assert len(d_len) == 256


class _Dec:
    def __init__(self, src):
        self.src = src
        self.pos = 0
        self.getbuf = 0
        self.getlen = 0
        self.freq = [0] * (T + 1)
        self.prnt = [0] * (T + N_CHAR)
        self.son = [0] * T

    def _fill(self):
        while self.getlen <= 8:
            if self.pos < len(self.src):
                i = self.src[self.pos]
                self.pos += 1
            else:
                i = 0
            self.getbuf = (self.getbuf | (i << (8 - self.getlen))) & 0xFFFF
            self.getlen += 8

    def getbit(self):
        self._fill()
        i = self.getbuf
        self.getbuf = (self.getbuf << 1) & 0xFFFF
        self.getlen -= 1
        return 1 if i & 0x8000 else 0

    def getbyte(self):
        self._fill()
        i = self.getbuf
        self.getbuf = (self.getbuf << 8) & 0xFFFF
        self.getlen -= 8
        return i >> 8

    def start_huff(self):
        freq, son, prnt = self.freq, self.son, self.prnt
        for i in range(N_CHAR):
            freq[i] = 1
            son[i] = i + T
            prnt[i + T] = i
        i = 0
        j = N_CHAR
        while j <= R:
            freq[j] = freq[i] + freq[i + 1]
            son[j] = i
            prnt[i] = prnt[i + 1] = j
            i += 2
            j += 1
        freq[T] = 0xFFFF
        prnt[R] = 0

    def reconst(self):
        freq, son, prnt = self.freq, self.son, self.prnt
        j = 0
        for i in range(T):
            if son[i] >= T:
                freq[j] = (freq[i] + 1) // 2
                son[j] = son[i]
                j += 1
        i = 0
        j = N_CHAR
        while j < T:
            k = i + 1
            f = freq[j] = freq[i] + freq[k]
            k = j - 1
            while f < freq[k]:
                k -= 1
            k += 1
            freq[k + 1:j + 1] = freq[k:j]
            freq[k] = f
            son[k + 1:j + 1] = son[k:j]
            son[k] = i
            i += 2
            j += 1
        for i in range(T):
            k = son[i]
            if k >= T:
                prnt[k] = i
            else:
                prnt[k] = prnt[k + 1] = i

    def update(self, c):
        freq, son, prnt = self.freq, self.son, self.prnt
        if freq[R] == MAX_FREQ:
            self.reconst()
        c = prnt[c + T]
        while True:
            freq[c] += 1
            k = freq[c]
            l = c + 1
            if k > freq[l]:
                l += 1
                while k > freq[l]:
                    l += 1
                l -= 1
                freq[c] = freq[l]
                freq[l] = k
                i = son[c]
                prnt[i] = l
                if i < T:
                    prnt[i + 1] = l
                j = son[l]
                son[l] = i
                prnt[j] = c
                if j < T:
                    prnt[j + 1] = c
                son[c] = j
                c = l
            c = prnt[c]
            if c == 0:
                break

    def decode_char(self):
        c = self.son[R]
        while c < T:
            c += self.getbit()
            c = self.son[c]
        c -= T
        self.update(c)
        return c

    def decode_position(self):
        i = self.getbyte()
        c = d_code[i] << 6
        j = d_len[i] - 2
        while j:
            i = (i << 1) + self.getbit()
            j -= 1
        return c | (i & 0x3F)


def lzh_decompress(src, origlen):
    d = _Dec(src)
    d.start_huff()
    text_buf = bytearray(b' ' * (N + F - 1))
    r = N - F
    out = bytearray()
    while len(out) < origlen:
        c = d.decode_char()
        if c < 256:
            out.append(c)
            text_buf[r] = c
            r = (r + 1) & (N - 1)
        else:
            i = (r - d.decode_position() - 1) & (N - 1)
            j = c - 255 + THRESHOLD
            for k in range(j):
                c2 = text_buf[(i + k) & (N - 1)]
                out.append(c2)
                text_buf[r] = c2
                r = (r + 1) & (N - 1)
    return bytes(out[:origlen])


def lzw_decompress(src, origlen):
    """Softdisk 'LZW' (actually LZSS, N=4096, F=18, THRESHOLD=2)."""
    n, f, thr = 4096, 18, 2
    text_buf = bytearray(b' ' * (n + f - 1))
    r = n - f
    flags = 0
    out = bytearray()
    p = 0
    while len(out) < origlen and p < len(src):
        flags >>= 1
        if not (flags & 256):
            flags = src[p] | 0xFF00
            p += 1
        if flags & 1:
            c = src[p]; p += 1
            out.append(c)
            text_buf[r] = c
            r = (r + 1) & (n - 1)
        else:
            i = src[p]; j = src[p + 1]; p += 2
            i |= (j & 0xF0) << 4
            j = (j & 0x0F) + thr
            for k in range(j + 1):
                c = text_buf[(i + k) & (n - 1)]
                out.append(c)
                text_buf[r] = c
                r = (r + 1) & (n - 1)
    return bytes(out[:origlen])


def load(path):
    data = open(path, 'rb').read()
    if data[:4] == b'CMP1':
        ctype, orig, comp = struct.unpack_from('<HII', data, 4)
        body = data[14:14 + comp]
        if ctype == 0:
            return body[:orig]
        if ctype == 1:
            return lzw_decompress(body, orig)
        if ctype == 2:
            return lzh_decompress(body, orig)
        raise ValueError('unknown CMP1 type %d' % ctype)
    if data[:4] == b'COMP':
        orig = struct.unpack_from('<I', data, 4)[0]
        return lzw_decompress(data[8:], orig)
    return data


if __name__ == '__main__':
    import sys
    for p in sys.argv[1:]:
        d = load(p)
        print(p, len(d), d[:32])
