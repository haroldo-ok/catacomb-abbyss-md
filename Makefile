# Catacomb Abyss for the Sega Genesis - SGDK project
#
#   make data   convert the original game data (tools/convert.py)
#   make        build out/rom.bin
#
GDK ?= /opt/sgdk
CATABYSS_DATA ?= /home/claude/data/zip
EXTRA_FLAGS += -Isrc

include $(GDK)/makefile.gen

# 68000-safe libgcc from SGDK, and no build-id note (it collides with the
# vector table when linking with a Linux-target binutils)
LIBGCC := $(GDK)/lib/libgcc.a -Wl,--build-id=none

.PHONY: data
data:
	CATABYSS_DATA=$(CATABYSS_DATA) python3 -I tools/convert.py .
