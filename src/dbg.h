#ifndef DBG_H
#define DBG_H
#ifdef DBGMARK
#include <genesis.h>
static inline void dbg_mark(u16 i, u8 v) { SRAM_enable(); SRAM_writeByte(i, v); SRAM_disable(); }
#else
#define dbg_mark(i,v)
#endif
#endif
