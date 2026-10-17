"""Unpacker for LZEXE 0.91 compressed DOS executables (used by the Gamer's
Edge shell programs).  Returns the decompressed load image (no header)."""
import struct
import sys


def unlzexe(data):
    hdr = struct.unpack_from('<14H', data, 0)
    hsize = hdr[4] * 16
    cs = hdr[11]
    ip = hdr[10]
    loader = hsize + cs * 16
    lip, lcs, lsp, lss, csize, inc, dsize, chk = struct.unpack_from('<8H', data, loader)
    src = hsize
    out = bytearray()
    bitbuf = struct.unpack_from('<H', data, src)[0]; src += 2
    count = 16

    def getbit():
        nonlocal bitbuf, count, src
        b = bitbuf & 1
        bitbuf >>= 1
        count -= 1
        if count == 0:
            bitbuf = struct.unpack_from('<H', data, src)[0]
            src += 2
            count = 16
        return b

    while True:
        if getbit():
            out.append(data[src]); src += 1
            continue
        if not getbit():
            ln = getbit() << 1
            ln |= getbit()
            ln += 2
            span = data[src] | 0xFF00; src += 1
        else:
            span = data[src]; src += 1
            ln = data[src]; src += 1
            span |= ((ln & ~0x07) << 5) | 0xE000
            ln = (ln & 0x07) + 2
            if ln == 2:
                ln = data[src]; src += 1
                if ln == 0:
                    break
                if ln == 1:
                    continue
                ln += 1
        span -= 0x10000
        for _ in range(ln):
            out.append(out[len(out) + span])
    return bytes(out), dict(ip=lip, cs=lcs, sp=lsp, ss=lss)


if __name__ == '__main__':
    img, regs = unlzexe(open(sys.argv[1], 'rb').read())
    open(sys.argv[2], 'wb').write(img)
    print(len(img), regs)
