/* save.h - battery backed SRAM storage for saved games (replaces the
 * .SAV files of GE_SaveGame/GE_LoadGame). */
#ifndef SAVE_H
#define SAVE_H

#include <genesis.h>

#define SAVE_SLOTS      3
#define SAVE_SLOT_SIZE  0x2400

void save_init(void);
u32  save_slot_base(u16 slot);
void save_begin(void);
void save_end(void);
u32  save_write(u32 addr, const void *data, u16 len);
u32  save_read(u32 addr, void *data, u16 len);
/* slot description (level name) or NULL when empty */
boolean save_slot_info(u16 slot, char *desc, u16 len);

#endif
