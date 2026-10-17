# The Catacomb Abyss for the Sega Genesis / Mega Drive

A port of Softdisk's *The Catacomb Abyss* (Gamer's Edge shareware v1.13,
1992) to the Sega Genesis / Mega Drive, built with SGDK 2.11 from the GPL
source release (CatacombGames/CatacombAbyss) and the original shareware
data files.

Everything in the shareware package is converted from the original files:

* **The game** (CATABYSS.EXE): all 19 levels, every monster, item, scroll,
  level description text, the in-game help (HELP.TXT), the F10 debug keys,
  and all sound effects in both their AdLib and PC speaker versions.
* **The title sequence** (INTRO.EXE): all twelve SHPxx.ABS screens with the
  original order, timings and beeps, the "Novice or Warrior?" challenge and
  the story screen.
* **The electronic catalog** (DEMOCAT.EXE): the 23 pages of SCRIPT.CAT and
  the LAST.CAT closing screen.
* **The user's guide shell** (START.EXE): MENU, QUICK, INSTRUCT and PRODUCTS
  .SHL documents in the original 80x25 text layout inside BKGND.SHL's frame,
  plus PRINTFIL.TXT (the catalog's written information and order form).
* **The closing screens** (LOADSCN.EXE): ARMAPC.ABS and LAST.ABS.

Text that describes PC keys, DOS, disks, memory or printers was rewritten
for the joypad and the cartridge (including the key hints drawn into the
status bar and title screen pictures); everything else is kept as written.

## Controls

**Title screen**

| Button | Action |
|---|---|
| START | New game ("Novice or Warrior?": B = Novice, C = Warrior, A = back) |
| B | Restore a saved game |
| C | The electronic catalog (LEFT/RIGHT turn pages, START or A leave) |
| A | The user's guide (UP/DOWN choose, B or START select, A back) |

**In the game, 3 button pad**

| Button | Action |
|---|---|
| D-pad | Move forward/backward, turn left/right |
| B | Shoot a Magick Missile |
| hold C + LEFT/RIGHT | Move sideways |
| hold A + LEFT/RIGHT | Quick Turn |
| A (tap) | Zapper |
| hold A + press B | Xterminator |
| hold A + press C | Cure potion |
| START | Options: Help, Sound, Scrolls, Save, Restore, New, Quit |

**6 button pad extras:** X Xterminator, Y Cure potion, Z Quick Turn,
MODE read a scroll.

Hold A+B+C and press START for the original debug keys (god mode, warp,
free items, freeze time, ...).  Three games are kept in battery backed SRAM.

## Building

Requirements:

* SGDK 2.11 with an m68k GCC toolchain (`GDK` points at SGDK; the Makefile
  links SGDK's 68000-safe libgcc and passes `--build-id=none`, so a Linux
  hosted `m68k-linux-gnu` GCC works as well as `m68k-elf`)
* Python 3 and a host C compiler (only for `make data`)

The generated sources (`src/gen/`) and resources (`res/`) are included, so
the ROM can be built straight away:

    export GDK=/path/to/sgdk
    make            # -> out/rom.bin

To regenerate them from the original files:

    make data CATABYSS_DATA=/path/to/shareware/files

`tools/convert.py` reads the shareware v1.13 files (EGAGRAPH.ABS,
GAMEMAPS.ABS, AUDIO.ABS, CATABYSS.EXE, the .SHL/.CAT/.ABS front end files,
HELP.TXT, PRINTFIL.TXT) from `CATABYSS_DATA`, a few dictionary objects of
the GPL source tree from `CATABYSS_SRC`, and renders the AdLib sounds with
Nuked-OPL3 found in `NUKED_OPL3`.

Build options (`make EXTRA_FLAGS="-Isrc ..."`, then `make clean`):
`-DPROFILE` shows a renderer timing HUD in scanlines, `-DSTART_LEVEL=n`
starts new games on level n.

## Source layout

| Path | Contents |
|---|---|
| `src/c4_*.c`, `states.c` | Game logic of the GPL release, adapted (C4_ACT1, C4_WIZ, C4_PLAY, C4_GAME, C4_MAIN, C4_STATE, C4_DEBUG) |
| `src/render.c`, `src/render_a.s` | 3-D view: ray caster, wall and sprite scaling |
| `src/vid.c`, `src/draw2d.c` | Screen buffer, DMA queue, page flip, palettes, fizzle fade, 2-D drawing and the proportional font |
| `src/intro.c` | Front end: INTRO, DEMOCAT, START shell, LOADSCN |
| `src/menu.c` | Joypad menus, help viewer, save/restore slot menus |
| `src/input.c`, `src/snd.c`, `src/save.c` | Joypad, sound (XGM2 PCM), SRAM |
| `src/gen/` | Generated: graphics, maps, wall scalers, tables, sounds, texts |
| `res/` | Generated rescomp inputs: sound effects, packed screens |
| `tools/` | Converters: `convert.py` (main), `iddata.py` (EGAGRAPH/GAMEMAPS/AUDIO), `lzh.py` (Softdisk LZHUF), `egaimg.py` (ILBM/EGA), `screens.py` (front end pictures), `docs.py` (documents), `font4x8.py`, `unlzexe.py`, `omf.py`, `adlib_render.c` |

## How it works

* **Renderer.** The view is 320x120 rendered as 160 two-pixel columns into
  a 4bpp RAM buffer laid out like VDP tiles.  An assembly DDA casts rays
  from the original's camera point (FOCALLENGTH behind the player) for
  every 4th column and refines only where the wall face changes; columns
  on one face are interpolated.  Walls are drawn by compiled 68000 scalers
  generated from the original BuildCompScale() row mapping, in byte (one
  column) and word (two columns) versions; close walls are paired.
  Sprites use the original TransformActor()/ScaleShape() maths with a
  column z-buffer.  Speed is about 11-14 frames per second.
* **Display.** The buffer is DMAed to VRAM in vertical blank and double
  buffered; the page flip is a single vertical scroll write on a 64x64
  plane that maps both view buffers.  The status bar and full screen
  pictures use the same tile layout.
* **Sound.** AdLib effects are rendered offline through an OPL3 emulator,
  PC speaker effects as square waves, both played as XGM2 PCM with the
  original priority rules.
* **Front end.** The EGA pictures are stored as APLIB packed screen dumps
  unpacked straight into the screen buffer; the text mode screens use a
  4x8 code page 437 font (80 columns fill the 320 pixel screen).
