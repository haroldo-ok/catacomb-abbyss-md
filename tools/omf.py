"""Minimal Intel OMF (.OBJ) reader: extracts the data bytes of each public
symbol's segment from the Borland-linked header objects that ship with the
Catacomb Abyss source (ABSEHEAD.OBJ, ABSEDICT.OBJ, ABSMHEAD.OBJ, ...).

The objects were produced by MAKEOBJ and contain a single segment whose
LEDATA records hold the raw bytes of the original header file."""
import struct
import sys


def read_omf(path):
    data = open(path, 'rb').read()
    pos = 0
    segs = {}        # segment index -> bytearray
    seg_names = []
    lnames = ['']
    pubs = []
    while pos < len(data):
        rtype = data[pos]
        rlen = struct.unpack_from('<H', data, pos + 1)[0]
        body = data[pos + 3:pos + 3 + rlen - 1]   # strip checksum
        pos += 3 + rlen
        if rtype == 0x96:     # LNAMES
            p = 0
            while p < len(body):
                n = body[p]
                lnames.append(body[p + 1:p + 1 + n].decode('latin1'))
                p += 1 + n
        elif rtype in (0x98, 0x99):   # SEGDEF
            seg_names.append(len(seg_names) + 1)
        elif rtype in (0x90, 0x91):   # PUBDEF
            p = 0
            grp = body[p]; p += 1
            seg = body[p]; p += 1
            if seg == 0:
                p += 2
            while p < len(body):
                n = body[p]
                name = body[p + 1:p + 1 + n].decode('latin1')
                p += 1 + n
                if rtype == 0x90:
                    off = struct.unpack_from('<H', body, p)[0]; p += 2
                else:
                    off = struct.unpack_from('<I', body, p)[0]; p += 4
                p += 1   # type index
                pubs.append((name, seg, off))
        elif rtype in (0xA0, 0xA1):   # LEDATA
            seg = body[0]
            if rtype == 0xA0:
                off = struct.unpack_from('<H', body, 1)[0]
                payload = body[3:]
            else:
                off = struct.unpack_from('<I', body, 1)[0]
                payload = body[5:]
            buf = segs.setdefault(seg, bytearray())
            if len(buf) < off + len(payload):
                buf.extend(b'\0' * (off + len(payload) - len(buf)))
            buf[off:off + len(payload)] = payload
        elif rtype == 0xA2:   # LIDATA (not expected)
            raise ValueError('LIDATA not supported in ' + path)
    return segs, pubs


def extract(path):
    segs, pubs = read_omf(path)
    if not segs:
        raise ValueError('no data in ' + path)
    # all header objects have a single data segment
    seg = sorted(segs)[0]
    return bytes(segs[seg]), pubs


if __name__ == '__main__':
    for p in sys.argv[1:]:
        d, pubs = extract(p)
        print(p, len(d), pubs)
