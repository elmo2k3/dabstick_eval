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
#include "buttons.h"

static volatile uint16_t key_state;
static volatile uint16_t key_press;
static volatile uint16_t key_rpt;

void buttons_every_1_ms(){
	static uint16_t ct0, ct1, rpt;
	uint16_t i;

	i = key_state ^ ~((SW6<<5) | (SW5<<4) |(SW_ROT<<3)|(SW4<<2)|(SW3<<1)|(SW2));
	ct0 = ~(ct0 & i);
	ct1 = ct0 ^(ct1 & i);
	i &= ct0 & ct1;
	key_state ^= i;
	key_press |= key_state & i;

	if((key_state & REPEAT_MASK) == 0){
		rpt = REPEAT_START;
	}
	if(--rpt == 0){
		rpt = REPEAT_NEXT;
		key_rpt |= key_state & REPEAT_MASK;
	}
}

///////////////////////////////////////////////////////////////////
//
// check if a key has been pressed. Each pressed key is reported
// only once
//
uint8_t get_key_press( uint8_t key_mask )
{                                       // read and clear atomic !
	key_mask &= key_press;                          // read key(s)
	key_press ^= key_mask;                          // clear key(s)
	return key_mask;
}
///////////////////////////////////////////////////////////////////
//
// check if a key has been pressed long enough such that the
// key repeat functionality kicks in. After a small setup delay
// the key is reported beeing pressed in subsequent calls
// to this function. This simulates the user repeatedly
// pressing and releasing the key.
//
uint8_t get_key_rpt( uint8_t key_mask )
{                                       // read and clear atomic !
	key_mask &= key_rpt;                            // read key(s)
	key_rpt ^= key_mask;                            // clear key(s)
	return key_mask;
}

