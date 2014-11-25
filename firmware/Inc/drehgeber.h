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
#ifndef __DREHGEBER_H__
#define __DREHGEBER_H__

#include <stdint.h>
#include "stm32f4xx_hal.h"
// inits timer0

#define DREHGEBER_A PINA7
#define DREHGEBER_B PINA6

#define PHASE_A ((GPIOC->IDR & (1<<4)))
#define PHASE_B ((GPIOC->IDR & (1<<5)))

void drehgeber_init(void);
void drehgeber_work(void);      //call every 1ms
int8_t drehgeber_read(void);    // read four step encoders

#endif
