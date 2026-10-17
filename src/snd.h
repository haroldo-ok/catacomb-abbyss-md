/* snd.h - ID_SD replacement: the original AdLib and PC speaker effects,
 * pre-rendered to PCM by tools/convert.py and played by the XGM2 driver. */
#ifndef SND_H
#define SND_H

#include <genesis.h>

typedef enum { sdm_Off, sdm_PC, sdm_AdLib } SDMode;

extern SDMode SoundMode;
extern boolean AdLibPresent;

void SD_Startup(void);
void SD_SetSoundMode(SDMode mode);
void SD_PlaySound(u16 sound);
void SD_StopSound(void);
void SD_WaitSoundDone(void);
u16  SD_SoundPlaying(void);
void snd_tick(void);                /* called from V-Int */
void snd_beep(void);                /* intro screen beep */

#endif
