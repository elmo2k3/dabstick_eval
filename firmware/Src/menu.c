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
#include <stdio.h>
#include "buttons.h"
#include "drehgeber.h"
#include "menu.h"
#include "page_misc.h"
#include "page_tuning.h"
#include "page_clock.h"
#include "page_status.h"
#include "page_volume.h"

#define SEC_HALF 0

volatile uint8_t refreshFlags;

int8_t drehgeber_delta;
int8_t menu_position;
int8_t old_menu_position;
uint8_t focus_here;

static struct menuitem menu[] = {
	//    Page Name | Number | Periodic | Drehgeber | Button | Init page
	{NULL, 0, update_page_clock, page_clock_drehgeber, page_clock_button_pressed, page_clock},
	{NULL, 0, update_page_tuning, page_tuning_drehgeber, page_tuning_button_pressed, page_tuning},
	{NULL, 0, update_page_status, page_status_drehgeber, page_status_button_pressed, page_status},
	{NULL, 0, update_page_volume, page_volume_drehgeber, page_volume_button_pressed, page_volume}
};
static const uint8_t NUM_PAGES = sizeof(menu) / sizeof(menu[0]);

void menu_init()
{
	//NUM_PAGES = sizeof(struct menuitem) * sizeof(menu);
	menu[0].name = "Clock";
	menu[1].name = "DAB/FM Radio";
	menu[2].name = "Status";
	menu[3].name = "Volumes";

	draw_page_header(&menu[menu_position]);
	menu[menu_position].draw_func(&menu[menu_position]);
	focus_here = 1;

}

void menu_tick()
{
	refreshFlags |= (1 << SEC_HALF);
}

void menu_task()
{
	if ((drehgeber_delta = drehgeber_read())) {     // Drehgeber in action
		if (focus_here) {
			menu_position += drehgeber_delta;
			if (menu_position > (NUM_PAGES - 1))
				menu_position = NUM_PAGES - 1;
			if (menu_position < 0)
				menu_position = 0;
			if (menu_position != old_menu_position) {
				if (menu[menu_position].draw_func) {
					menu[menu_position].draw_func(&menu
							[menu_position]);
				}
				draw_page_header(&menu[menu_position]);
				old_menu_position = menu_position;
			}
		} else {
			if (menu[menu_position].drehgeber_func) {
				menu[menu_position].drehgeber_func(&menu
						[menu_position],
						drehgeber_delta);
			}
		}
	}
	/* if (get_key_press(1 << KEY0)) { //button left */
	/* 	if (menu[menu_position].taster_func) { */
	/* 		focus_here = */
	/* 			menu[menu_position].taster_func(&menu[menu_position], */
	/* 					0); */
	/* 	} */
	/* 	settings.output_active ^= 1; */
	/* 	dds_power(settings.output_active); */
	/* } */
	/* if (get_key_press(1 << KEY1)) { //button left +1 */
	/* 	if (menu[menu_position].taster_func) { */
	/* 		focus_here = */
	/* 			menu[menu_position].taster_func(&menu[menu_position], */
	/* 					1); */
	/* 	} */
	/* } */
	/* if (get_key_press(1 << KEY2)) { //button right -1 */
	/* 	if (menu[menu_position].taster_func) { */
	/* 		focus_here = */
	/* 			menu[menu_position].taster_func(&menu[menu_position], */
	/* 					2); */
	/* 	} */
	/* } */
	/* if (get_key_press(1 << KEY3)) { //button right */
	/* 	wdt_enable(WDTO_15MS); */
	/* 	while (1); */
	/* 	if (menu[menu_position].taster_func) { */
	/* 		focus_here = */
	/* 			menu[menu_position].taster_func(&menu[menu_position], */
	/* 					3); */
	/* 	} */
	/* } */
	if (get_key_press(KEY_ROT)) { //button encoder
		if (menu[menu_position].taster_func) {
			focus_here =
				menu[menu_position].taster_func(&menu[menu_position],
						KEY_ROT);
		}
	}
	if (refreshFlags & (1 << SEC_HALF)) {
		if (menu[menu_position].refresh_func) {
			menu[menu_position].refresh_func(&menu[menu_position],
					SEC_HALF);
		}
		refreshFlags &= ~(1 << SEC_HALF);
	}
}
