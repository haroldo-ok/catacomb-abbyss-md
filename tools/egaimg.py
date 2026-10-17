"""Image helpers: EGA palette, ILBM decoding, planar EGA chunk decoding and a
tiny dependency-free PNG writer (indexed, 16 colour EGA palette)."""
import struct
import zlib

# Default EGA 16-colour palette (8-bit RGB)
EGA_RGB = [
    (0x00, 0x00, 0x00), (0x00, 0x00, 0xAA), (0x00, 0xAA, 0x00), (0x00, 0xAA, 0xAA),
    (0xAA, 0x00, 0x00), (0xAA, 0x00, 0xAA), (0xAA, 0x55, 0x00), (0xAA, 0xAA, 0xAA),
    (0x55, 0x55, 0x55), (0x55, 0x55, 0xFF), (0x55, 0xFF, 0x55), (0x55, 0xFF, 0xFF),
    (0xFF, 0x55, 0x55), (0xFF, 0x55, 0xFF), (0xFF, 0xFF, 0x55), (0xFF, 0xFF, 0xFF),
]


def write_png(path, w, h, pix, pal=EGA_RGB):
    """pix: list/bytes of palette indices, row-major w*h."""
    raw = bytearray()
    for y in range(h):
        raw.append(0)
        raw.extend(bytes(pix[y * w:(y + 1) * w]))

    def chunk(t, d):
        c = struct.pack('>I', len(d)) + t + d
        return c + struct.pack('>I', zlib.crc32(t + d) & 0xFFFFFFFF)
    plte = b''.join(bytes(c) for c in pal)
    png = b'\x89PNG\r\n\x1a\n'
    png += chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 3, 0, 0, 0))
    png += chunk(b'PLTE', plte)
    png += chunk(b'IDAT', zlib.compress(bytes(raw), 9))
    png += chunk(b'IEND', b'')
    open(path, 'wb').write(png)


def decode_ilbm(data):
    """Return dict(w,h,x,y,depth,pixels,cmap) from an EGA ILBM as loaded by
    the Gamer's Edge LoadShape()/UnpackEGAShapeToScreen() code."""
    assert data[:4] == b'FORM' and data[8:12] == b'ILBM'
    p = 12
    bm = None
    cmap = None
    body = None
    while p + 8 <= len(data):
        cid = data[p:p + 4]
        ln = struct.unpack_from('>I', data, p + 4)[0]
        cd = data[p + 8:p + 8 + ln]
        if cid == b'BMHD':
            w, h, x, y, d, mask, comp, pad, trans = struct.unpack_from('>HHhhBBBBH', cd)
            bm = dict(w=w, h=h, x=x, y=y, depth=d, mask=mask, comp=comp, trans=trans)
        elif cid == b'CMAP':
            cmap = [tuple(cd[i:i + 3]) for i in range(0, len(cd) - 2, 3)]
        elif cid == b'BODY':
            body = cd
            break
        p += 8 + ((ln + 1) & ~1)
    w, h, d = bm['w'], bm['h'], bm['depth']
    bpr = (w + 7) >> 3
    wbpr = ((bpr + 1) >> 1) << 1          # rows are word aligned in the file
    pix = bytearray(w * h)
    src = 0
    for y in range(h):
        for plane in range(d):
            row = bytearray()
            if bm['comp']:
                while len(row) < wbpr:
                    n = body[src]; src += 1
                    if n > 127:
                        n -= 256
                    if n >= 0:
                        row.extend(body[src:src + n + 1]); src += n + 1
                    elif n != -128:
                        row.extend(bytes([body[src]]) * (1 - n)); src += 1
            else:
                row = body[src:src + wbpr]; src += wbpr
            for x in range(w):
                if row[x >> 3] & (0x80 >> (x & 7)):
                    pix[y * w + x] |= 1 << plane
    bm['pixels'] = pix
    bm['cmap'] = cmap
    return bm


def deplane(data, width_bytes, height, planes=4, offset=0):
    """EGA chunk with whole planes stored sequentially (plane0 = blue bit)."""
    w = width_bytes * 8
    psize = width_bytes * height
    pix = bytearray(w * height)
    for pl in range(planes):
        base = offset + pl * psize
        for y in range(height):
            for xb in range(width_bytes):
                b = data[base + y * width_bytes + xb]
                if not b:
                    continue
                for bit in range(8):
                    if b & (0x80 >> bit):
                        pix[y * w + xb * 8 + bit] |= 1 << pl
    return pix
