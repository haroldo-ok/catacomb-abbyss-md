/* adlib_render.c - renders one Catacomb Abyss AdLib sound effect chunk to
 * raw signed 16-bit mono PCM (49716 Hz) on stdout, using Nuked-OPL3 in OPL2
 * mode and reproducing ID_SD.C exactly:
 *   SDL_DetectAdLib(): zero regs 1..0xF5, reg1=0x20 (WSE), reg8=0
 *   SDL_ALPlaySound(): set channel 0 modulator/carrier from the instrument
 *   SDL_ALSoundService() at 140 Hz: freqL = sample, freqH = block|keyon
 * After the data ends the note is keyed off and the release is rendered. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "opl3.h"

#define RATE 49716

static opl3_chip chip;

static void w(uint16_t reg, uint8_t v) { OPL3_WriteReg(&chip, reg, v); }

static void render(double secs)
{
    long n = (long)(secs * RATE + 0.5);
    int16_t buf[2];
    for (long i = 0; i < n; i++) {
        OPL3_GenerateResampled(&chip, buf);
        int v = buf[0] * 2;
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;
        int16_t s = (int16_t)v;
        fwrite(&s, 2, 1, stdout);
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) return 1;
    FILE *f = fopen(argv[1], "rb");
    if (!f) return 1;
    static uint8_t d[65536];
    size_t len = fread(d, 1, sizeof d, f);
    fclose(f);
    if (len < 23) return 1;
    uint32_t length = d[0] | d[1] << 8 | d[2] << 16 | (uint32_t)d[3] << 24;
    const uint8_t *inst = d + 6;
    uint8_t block = d[22];
    const uint8_t *data = d + 23;

    OPL3_Reset(&chip, RATE);
    for (int i = 1; i <= 0xF5; i++) w(i, 0);
    w(1, 0x20);
    w(8, 0);
    w(0xBD, 0);

    /* modifiers[0] = 0, carriers[0] = 3 */
    w(0x20 + 0, inst[0]);  w(0x40 + 0, inst[2]);  w(0x60 + 0, inst[4]);
    w(0x80 + 0, inst[6]);  w(0xE0 + 0, inst[8]);
    w(0x20 + 3, inst[1]);  w(0x40 + 3, inst[3]);  w(0x60 + 3, inst[5]);
    w(0x80 + 3, inst[7]);  w(0xE0 + 3, inst[9]);

    uint8_t alBlock = ((block & 7) << 2) | 0x20;
    for (uint32_t i = 0; i < length && 23 + i < len; i++) {
        uint8_t s = data[i];
        if (!s)
            w(0xB0, 0);
        else {
            w(0xA0, s);
            w(0xB0, alBlock);
        }
        render(1.0 / 140.0);
    }
    w(0xB0, 0);
    render(0.25);       /* release tail */
    return 0;
}
