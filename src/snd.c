/* snd.c - sound effects with the original priority rules (ID_SD.C) */
#include "def.h"
#include "snd.h"
#include "vid.h"
#include "res_assets.h"

SDMode SoundMode = sdm_AdLib;
boolean AdLibPresent = TRUE;

static volatile u16 SoundNumber, SoundPriority;
static volatile u32 SoundEnd;
static volatile boolean playing;

void SD_Startup(void)
{
    XGM2_loadDriver(TRUE);
    SoundNumber = SoundPriority = 0;
    playing = FALSE;
}

void SD_SetSoundMode(SDMode mode)
{
    SD_StopSound();
    SoundMode = mode;
}

void SD_StopSound(void)
{
    XGM2_stopPCM(SOUND_PCM_CH2);
    SoundNumber = SoundPriority = 0;
    playing = FALSE;
}

void SD_PlaySound(u16 sound)
{
    const u8 *data;
    u32 len;
    if (SoundMode == sdm_Off || sound >= NUMSOUNDS)
        return;
    if (sound_priority[sound] < SoundPriority)
        return;
    if (SoundMode == sdm_AdLib)
    {
        data = snd_al_data[sound];
        len = snd_al_len[sound];
    }
    else
    {
        data = snd_pc_data[sound];
        len = snd_pc_len[sound];
    }
    XGM2_playPCMEx(data, len, SOUND_PCM_CH2, 15, FALSE, FALSE);
    SoundNumber = sound;
    SoundPriority = sound_priority[sound];
    SoundEnd = TimeCount + sound_tics[sound];
    playing = TRUE;
}

void snd_tick(void)
{
    if (playing && (s32)(TimeCount - SoundEnd) >= 0)
    {
        playing = FALSE;
        SoundNumber = SoundPriority = 0;
    }
}

u16 SD_SoundPlaying(void)
{
    return playing ? SoundNumber + 1 : 0;
}

void SD_WaitSoundDone(void)
{
    while (playing)
        vid_frame();
}

void snd_beep(void)
{
    XGM2_playPCMEx(sfx_beep, sizeof(sfx_beep), SOUND_PCM_CH3, 15, FALSE, FALSE);
}
