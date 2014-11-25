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
#include "page_tuning.h"
#include "main.h"
#include "si46xx.h"
#include "buttons.h"
#include "dabstick.h"


uint8_t state;
static uint8_t toggle;

enum cursor_state{
	STATE_IDLE,
	STATE_MODE,
	STATE_ENSEMBLE,
	STATE_SERVICE
};
void page_tuning_draw_values(uint8_t blink)
{
	char *mode_str;
	char str[20];
	uint8_t i;
	/* struct dab_digrad_status_t dab_digrad_status; */
	/* struct fm_rsq_status_t fm_status; */

	/* si46xx_dab_digrad_status(&dab_digrad_status); */
	/* si46xx_fm_rsq_status(&fm_status); */
	lcd_set_font(FONT_FIXED_8,NORMAL);

	if(si46xx_status.mode == MODE_FM)
		mode_str = "FM";
	else
		mode_str = "DAB";
	lcd_moveto_xy(3,0);
	lcd_putstr("Modus:");
	lcd_moveto_xy(3,75);
	if(state == STATE_MODE && blink) lcd_putstr("       ");
	else lcd_putstr(mode_str);

	snprintf(str,20,"%2d",si46xx_status.dab_channel);
	lcd_moveto_xy(4,0);
	lcd_putstr("DAB Kanal:");
	lcd_moveto_xy(4,70);
	if(state == STATE_ENSEMBLE && blink) lcd_putstr("       ");
	else lcd_putstr(str);
	
	mode_str = dab_service_list.services[si46xx_status.
	                            dab_service_num].service_label;

	str[0] = '\0';
	for(i=0;i<((17-strlen(mode_str)))/2;i++){
		strcat(str," ");
	}
	strcat(str,mode_str);
	/* snprintf(str,20,"%16s",dab_service_list. */
	/*                      services[si46xx_status. */
	/* 		     dab_service_num].service_label); */
	lcd_moveto_xy(5,0);
	lcd_putstr("DAB Service:");
	lcd_moveto_xy(7,0);
	if(state == STATE_SERVICE && blink) lcd_putstr("                ");
	else lcd_putstr(str);
}

void page_tuning(struct menuitem *self)
{
	lcd_moveto_xy(3,0);
	lcd_clear_area(6,102, NORMAL);
	page_tuning_draw_values(0);
	cs43l22_on();
}

void update_page_tuning(struct menuitem *self, uint8_t event)
{
	if(++toggle>4){
		toggle = 0;
		page_tuning_draw_values(1);
	}else{
		page_tuning_draw_values(0);
	}
	//page_tuning(self);
}

uint8_t page_tuning_button_pressed(struct menuitem *self, uint8_t button)
{
	toggle = 4;
	if(button == KEY_ROT){
		if(++state > STATE_SERVICE){
			state = STATE_IDLE;
			return 1; // fokus zurück zu main
		}
	}
	return 0; // fokus bleibt hier
}

void page_tuning_drehgeber(struct menuitem *self, int8_t steps)
{
	switch(state){
	case STATE_MODE:
		if(steps % 2 != 0){ // odd number of increments
			if(si46xx_status.mode == MODE_FM){
				radio_dab();
			}else{
				radio_fm();
			}
		}
		break;
	case STATE_ENSEMBLE:
		change_in_range(&si46xx_status.dab_channel, steps,
		                si46xx_status.dab_num_channels-1,0);
		si46xx_dab_tune_freq(si46xx_status.dab_channel,0);
		break;
	case STATE_SERVICE:
		change_in_range(&si46xx_status.dab_service_num, steps,
		                dab_service_list.num_services-1,0);
		si46xx_dab_start_digital_service_num(si46xx_status.dab_service_num);
		break;
	default: break;
	}
	page_tuning_draw_values(0);
}

