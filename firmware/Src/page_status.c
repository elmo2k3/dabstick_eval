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
#include "dogm-graphic.h"
#include "font.h"
#include "page_misc.h"
#include "page_status.h"
#include "main.h"
#include "si46xx.h"


void page_status_draw_values(struct menuitem *self, uint8_t blink)
{
	lcd_set_font(FONT_PROP_8,NORMAL);

}

void page_status(struct menuitem *self)
{
	char str[20];
	struct dab_digrad_status_t dab_digrad_status;
	struct fm_rsq_status_t fm_status;

	si46xx_dab_digrad_status(&dab_digrad_status);
	si46xx_fm_rsq_status(&fm_status);
	lcd_moveto_xy(3,0);
	lcd_clear_area(6,102, NORMAL);
	lcd_set_font(FONT_FIXED_8,NORMAL);

	if(si46xx_status.mode == MODE_DAB){
		snprintf(str,20,"ACQ: %2d RSSI: %2d",dab_digrad_status.acq,
						     dab_digrad_status.rssi);
		lcd_putstr(str);
		lcd_moveto_xy(4,0);
		snprintf(str,20,"SNR: %2d CNR:  %2d",dab_digrad_status.snr,
						    dab_digrad_status.cnr);
		lcd_putstr(str);
	}else{

		snprintf(str,20,"FM:  %2d RSSI: %2d",fm_status.valid,
						     fm_status.rssi);
		lcd_putstr(str);
		lcd_moveto_xy(4,0);
		snprintf(str,20,"SNR: %2d OFF: %3d",fm_status.snr,
						    fm_status.freq_offset);
		lcd_putstr(str);
	}
}

void update_page_status(struct menuitem *self, uint8_t event)
{
	page_status(self);
}

uint8_t page_status_button_pressed(struct menuitem *self, uint8_t button)
{
	return 1; // fokus zurück zu main
}

void page_status_drehgeber(struct menuitem *self, int8_t steps)
{

}
