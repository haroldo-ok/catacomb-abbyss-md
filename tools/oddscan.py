"""Scan the linked ELF for word/long memory accesses with an odd constant
displacement (68000 address error risk)."""
import re, subprocess, sys
elf = sys.argv[1]
nm = subprocess.check_output(['m68k-linux-gnu-nm', '-S', '--defined-only', elf]).decode()
funcs = []
for l in nm.splitlines():
    p = l.split()
    if len(p) == 4 and p[2] in 'tT':
        funcs.append((int(p[0], 16), int(p[1], 16), p[3]))
pat = re.compile(r'\s([0-9a-f]+):\s+(?:[0-9a-f]{4} )+\s*(\w+)[wl]\s+(.*)$')
bad = 0
for a, sz, name in funcs:
    out = subprocess.check_output(['m68k-linux-gnu-objdump', '-d', elf,
        '--start-address=0x%x' % a, '--stop-address=0x%x' % (a + sz)]).decode()
    for line in out.splitlines():
        m = re.search(r'^\s*([0-9a-f]+):\s+(?:[0-9a-f]{4} )+\s*(\w+?)([wl])\s+(.*)$', line)
        if not m:
            continue
        ops = m.group(4)
        if m.group(2) in ('lea', 'pea', 'jsr', 'jmp', 'link'):
            continue
        for d in re.findall(r'@\((-?\d+)[,)]', ops):
            if int(d) % 2:
                print(name, line.strip())
                bad += 1
print('odd accesses:', bad)
