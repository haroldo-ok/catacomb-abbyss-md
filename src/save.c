/* save.c - SRAM access (odd bytes of $200001..) */
#include "def.h"
#include "save.h"

#define SAVE_MAGIC  0x43413133

void save_init(void)
{
}

u32 save_slot_base(u16 slot)
{
    return 0x40 + (u32)slot * SAVE_SLOT_SIZE;
}

void save_begin(void)
{
    SRAM_enable();
}

void save_end(void)
{
    SRAM_disable();
}

u32 save_write(u32 addr, const void *data, u16 len)
{
    const u8 *p = data;
    while (len--)
        SRAM_writeByte(addr++, *p++);
    return addr;
}

u32 save_read(u32 addr, void *data, u16 len)
{
    u8 *p = data;
    while (len--)
        *p++ = SRAM_readByte(addr++);
    return addr;
}

boolean save_slot_info(u16 slot, char *desc, u16 len)
{
    u32 a = save_slot_base(slot);
    u32 magic;
    u16 used, mapon, i;
    SRAM_enableRO();
    a = save_read(a, &magic, 4);
    a = save_read(a, &used, 2);
    a = save_read(a, &mapon, 2);
    for (i = 0; i < len - 1 && i < 31; i++)
        desc[i] = SRAM_readByte(a + i);
    desc[i] = 0;
    SRAM_disable();
    return magic == SAVE_MAGIC && used;
}
