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
#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include <stdint.h>
#include "stm32f4xx_hal.h"

//code taken from http://www.mikrocontroller.net/articles/Entprellung

#define SW2 ((GPIOE->IDR & (1<<14))>>14)
#define SW3 ((GPIOE->IDR & (1<<15))>>15)
#define SW4 ((GPIOB->IDR & (1<<11))>>11)
#define SW_ROT ((GPIOB->IDR & (1<<0)))
#define SW5 ((GPIOB->IDR & (1<<2))>>2)
#define SW6 ((GPIOE->IDR & (1<<7))>>7)

#define KEY0 (1)
#define KEY1 (1<<1)
#define KEY2 (1<<2)
#define KEY_ROT (1<<3)
#define KEY3 (1<<4)
#define KEY4 (1<<5)

#define REPEAT_MASK 0
#define REPEAT_START 50 // 500ms
#define REPEAT_NEXT 20 // 200ms

void buttons_every_1_ms(void);
uint8_t get_key_press(uint8_t key_mask);
uint8_t get_key_rpt(uint8_t key_mask);

#endif

