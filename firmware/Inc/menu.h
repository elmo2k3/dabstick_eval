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
#ifndef __MENU_H__
#define __MENU_H__

#include <stdint.h>

struct menuitem
{
	char *name;
	uint8_t num;
	void (*refresh_func)(struct menuitem *self, uint8_t event);
	void (*drehgeber_func)(struct menuitem *self, int8_t steps);
	uint8_t (*taster_func)(struct menuitem *self, uint8_t taster);
	void (*draw_func)(struct menuitem *self);
};

void menu_init(void);
void menu_tick(void);
void menu_task(void);

#endif

