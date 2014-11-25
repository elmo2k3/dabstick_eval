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
#include "page_volume.h"
#include "main.h"
#include "buttons.h"
#include "dabstick.h"
#include "cs43l22.h"


uint8_t state;
static uint8_t toggle;

enum cursor_state{
	STATE_IDLE,
	STATE_MASTER,
	STATE_HP,
	STATE_SPK
};
void page_volume_draw_values(uint8_t blink)
{
	char *mode_str;
	char str[20];
	uint8_t i;
	/* struct dab_digrad_status_t dab_digrad_status; */
	/* struct fm_rsq_status_t fm_status; */

	/* si46xx_dab_digrad_status(&dab_digrad_status); */
	/* si46xx_fm_rsq_status(&fm_status); */
	lcd_set_font(FONT_FIXED_8,NORMAL);

	snprintf(str,20,"%4d",audio_settings.master_volume);
	lcd_moveto_xy(3,0);
	lcd_putstr("Master:");
	lcd_moveto_xy(3,75);
	if(state == STATE_MASTER && blink) lcd_putstr("       ");
	else lcd_putstr(str);
	snprintf(str,20,"%4d",audio_settings.hp_volume);
	lcd_moveto_xy(4,0);
	lcd_putstr("Headphone:");
	lcd_moveto_xy(4,75);
	if(state == STATE_HP && blink) lcd_putstr("       ");
	else lcd_putstr(str);
	snprintf(str,20,"%4d",audio_settings.spka_volume);
	lcd_moveto_xy(5,0);
	lcd_putstr("Speaker:");
	lcd_moveto_xy(5,75);
	if(state == STATE_SPK && blink) lcd_putstr("       ");
	else lcd_putstr(str);
}

void page_volume(struct menuitem *self)
{
	lcd_moveto_xy(3,0);
	lcd_clear_area(6,102, NORMAL);
	page_volume_draw_values(0);
}

void update_page_volume(struct menuitem *self, uint8_t event)
{
	if(++toggle>4){
		toggle = 0;
		page_volume_draw_values(1);
	}else{
		page_volume_draw_values(0);
	}
	//page_volume(self);
}

uint8_t page_volume_button_pressed(struct menuitem *self, uint8_t button)
{
	toggle = 4;
	if(button == KEY_ROT){
		if(++state > STATE_SPK){
			state = STATE_IDLE;
			return 1; // fokus zurück zu main
		}
	}
	return 0; // fokus bleibt hier
}

void page_volume_drehgeber(struct menuitem *self, int8_t steps)
{
	switch(state){
	case STATE_MASTER:
		cs43l22_change_master_volume(steps);
		break;
	case STATE_HP:
		cs43l22_change_hp_volume(steps);
		break;
	case STATE_SPK:
		cs43l22_change_spk_volume(steps);
		break;
	default: break;
	}
	page_volume_draw_values(0);
}

