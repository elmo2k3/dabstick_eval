/*
 * Copyright (C) 2014 Bjoern Biesenbach <bjoern at bjoern-b.de>
 *               2014 homerj00			<homerj00 at web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef PAGE_ETHERNET_CONFIG_H_
#define PAGE_ETHERNET_CONFIG_H_

#include "main.h"

void update_ethernet_config(struct menuitem *self, uint8_t event);
void page_ethernet_config(struct menuitem *self);
uint8_t ethernet_config_button_pressed(struct menuitem *self, uint8_t button);
void ethernet_config_drehgeber(struct menuitem *self, int8_t steps);

#endif /* PAGE_ETHERNET_CONFIG_H_ */
