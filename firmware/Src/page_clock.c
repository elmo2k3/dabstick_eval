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
#include "page_clock.h"
#include "main.h"
#include "si46xx.h"
#include "buttons.h"
#include "dabstick.h"
#include "clock.h"


void page_clock_draw_values(struct menuitem *self, uint8_t update)
{
	struct tm timeinfo;
	char str[20];

	si46xx_dab_get_time(&timeinfo);
	lcd_moveto_xy(0,0);
	lcd_clear_area(8,102, NORMAL);
	lcd_set_font(FONT_DIGITS_24,SPACING );//| DOUBLE_HEIGHT);

	lcd_moveto_xy(1,7);
	snprintf(str,20,"%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
	lcd_putstr(str);
	if(!update){
		cs43l22_off();
	}
}

void page_clock(struct menuitem *self)
{
	page_clock_draw_values(self,0);
}

void update_page_clock(struct menuitem *self, uint8_t event)
{
	static uint16_t count = 0;
	if(++count == 10) {
		count = 0;
		page_clock_draw_values(self,1);
	}
}

uint8_t page_clock_button_pressed(struct menuitem *self, uint8_t button)
{
	static uint8_t state = 0;
	if(button == KEY_ROT){
		if(++state > 1){
			state = 0;
			return 1; // fokus zurück zu main
		}
	}
	return 0; // fokus bleibt hier
}

void page_clock_drehgeber(struct menuitem *self, int8_t steps)
{
	static int16_t backlight = DEFAULT_BACKLIGHT;
	
	backlight += steps;
	if(backlight <0) backlight = 0;
	if(backlight > 1024) backlight = 1024;
	lcd_set_backlight(backlight);
}
