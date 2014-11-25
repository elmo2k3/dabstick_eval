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
#include "main.h"
#include "dabstick.h"
#include "cs43l22.h"

static void settings_default_values(void)
{
	/* audio_settings.treble_corner = TREBLE_CORNER_5KHZ; */
	audio_settings.master_volume = 0;
	audio_settings.spka_volume = 0;
	audio_settings.hp_volume = 0;
}

static void audio_set_register(uint8_t addr, uint8_t data)
{
	uint8_t tmp[2];

	tmp[0] = addr;
	tmp[1] = data;
	HAL_I2C_Master_Transmit(&hi2c1, 0x94,tmp,2,1000);
}

void cs43l22_change_master_volume(int8_t vol)
{
	change_in_range(&audio_settings.master_volume, vol, 24, -102);
	cs43l22_set_master_volume(audio_settings.master_volume,
	                          audio_settings.master_volume);
}

void cs43l22_change_spk_volume(int8_t vol)
{
	change_in_range(&audio_settings.spka_volume, vol, 0, -127);
	cs43l22_set_spk_volume(audio_settings.spka_volume,
	                          audio_settings.spka_volume);
}

void cs43l22_change_hp_volume(int8_t vol)
{
	change_in_range(&audio_settings.hp_volume, vol, 0, -127);
	cs43l22_set_hp_volume(audio_settings.hp_volume,
	                          audio_settings.hp_volume);
}

void cs43l22_set_master_volume(int8_t left, int8_t right){
	audio_set_register(0x20,left);
	audio_set_register(0x21,right);
}

void cs43l22_set_hp_volume(int8_t left, int8_t right){
	audio_set_register(0x22,left);
	audio_set_register(0x23,right);
}

void cs43l22_set_spk_volume(int8_t left, int8_t right){
	audio_set_register(0x24,left);
	audio_set_register(0x25,right);
}

void cs43l22_init(){
	settings_default_values();
	GPIOD->BSRRH = GPIO_PIN_4;
	HAL_Delay(100);
	GPIOD->BSRRL = GPIO_PIN_4;
	HAL_Delay(100);
	audio_set_register(0x02,0x01);
	//  audio_set_register(0x04,0xaf); // only hp
	audio_set_register(0x04,0xaa); // spk hp both on
	//audio_set_register(0x04,0xff); // both off
	audio_set_register(0x34,0xf0); // charge pump frequency
	audio_set_register(0x05,0x81);
	audio_set_register(0x06,0x04);
	audio_set_register(0x02,0x9e);
}

void cs43l22_on(){
	audio_set_register(0x04,0xaa); // both on
}

void cs43l22_only_hp(){
	audio_set_register(0x04,0xaf); // only hp
}

void cs43l22_off(){
	audio_set_register(0x04,0xff); // both off
}
