/* 
 * dabstick_eval - experimental board for si4688 fm/dab+ receiver with
 * stm32f4 and audio codec
 * Copyright (C) 2014  Bjoern Biesenbach <bjoern@bjoern-b.de>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
// code taken from http://www.mikrocontroller.net/articles/Drehgeber
#include "drehgeber.h"

#define DYNAMIK 20

static volatile int16_t enc_delta;      // -128 ... 127
static volatile int8_t last;

void drehgeber_init(void)
{
    int8_t new;

    new = 0;
    if (PHASE_A)
        new = 3;
    if (PHASE_B)
        new ^= 1;               // convert gray to binary
    last = new;                 // power on state
    enc_delta = 0;
}

void drehgeber_work()           //call every 1ms
{
    static char enc_state = 0x01;
    static uint16_t enc_accel = 0;
    signed short ed;
    char i = 0;

    if (enc_accel > 0)
        enc_accel--;

    if (PHASE_A)
        i = 1;
    if (PHASE_B)
        i ^= 3;                 // convert gray to binary
    i -= enc_state;             // difference new - last
    if (i & 1) {                // bit 0 = value (1)
        ed = enc_delta;
        if (enc_accel < (255 - DYNAMIK))
            enc_accel += DYNAMIK;
        enc_state += i;         // store new as next last
        if (i & 2)
            ed += 1 + (enc_accel >> 6); // bit 1 = direction (+/-)
        else
            ed -= 1 + (enc_accel >> 6);
        enc_delta = ed;
    }
}

int8_t drehgeber_read(void)     // read four step encoders
{
    int16_t val;

    val = enc_delta;
    enc_delta = val & 3;
    return (int8_t) (val >> 2);
}
