"""Mechanical first pass used to port the C4_*.C sources (output is then
edited by hand)."""
import re, sys
src = open(sys.argv[1], 'rb').read().decode('latin1').replace('\r', '')
s = src
s = re.sub(r'\bstatetype\s+s_', 'const statetype s_', s)
s = s.replace('extern\tstatetype', 'extern const statetype').replace('extern statetype', 'extern const statetype')
s = re.sub(r'\bextern\s+const statetype\s+const statetype', 'extern const statetype', s)
s = re.sub(r'\brandom\s*\(', 'rnd(', s)
s = re.sub(r'\blong\b', 's32', s)
s = re.sub(r'\bshort\b', 's16', s)
s = re.sub(r'\bunsigned\b', 'u16', s)
s = s.replace('#include "DEF.H"', '#include "def.h"\n#include "snd.h"\n#include "vid.h"\n#include "draw2d.h"\n#include "input.h"')
s = s.replace('#pragma hdrstop\n', '')
open(sys.argv[2], 'w').write(s)
