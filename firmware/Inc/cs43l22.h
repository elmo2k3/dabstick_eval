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
#ifndef __CS43L22_H__
#define __CS43L22_H__

struct audio_settings_t{
	uint8_t treble_corner;
	uint8_t bass_corner;
	uint8_t tone_control_enabled;
	uint8_t treble_gain;
	uint8_t bass_gain;
	int8_t master_volume;
	int8_t hp_volume;
	int8_t spka_volume;
	int8_t spkb_volume;
}audio_settings;

void cs43l22_init(void);
void cs43l22_on(void);
void cs43l22_only_hp(void);
void cs43l22_off(void);

void cs43l22_change_master_volume(int8_t vol);
void cs43l22_change_spk_volume(int8_t vol);
void cs43l22_change_hp_volume(int8_t vol);

void cs43l22_set_master_volume(int8_t left, int8_t right);
void cs43l22_set_hp_volume(int8_t left, int8_t right);
void cs43l22_set_spk_volume(int8_t left, int8_t right);
#endif
