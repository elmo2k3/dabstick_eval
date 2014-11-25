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
#include <string.h>
#include "main.h"
#include "si46xx.h"
#include "cs43l22.h"

static void cmd_help(uint32_t *params);

static void cmd_dab(uint32_t *params)
{
	radio_dab();
}

static void cmd_fm(uint32_t *params)
{
	radio_fm();
}

static void cmd_dab_status(uint32_t *params)
{
	struct dab_digrad_status_t dab_digrad_status;

	si46xx_dab_digrad_status(&dab_digrad_status);
	si46xx_dab_digrad_status_print(&dab_digrad_status);
}

static void cmd_fm_status(uint32_t *params)
{
	/* si46xx_fm_rsq_status(); */
}

static void cmd_fm_rds(uint32_t *params)
{
	si46xx_fm_rds_status();
}

static void cmd_fm_frequency(uint32_t *params)
{
	si46xx_fm_tune_freq(params[0],0);
}

static void cmd_dab_channel(uint32_t *params)
{
	si46xx_dab_tune_freq(params[0],0);
}

static void cmd_dab_service_list(uint32_t *params)
{
	si46xx_dab_get_digital_service_list();
	si46xx_dab_print_service_list();
}

static void cmd_dab_start_service(uint32_t *params)
{
	si46xx_dab_start_digital_service_num(params[0]);
}

static void cmd_volume_hp(uint32_t *params)
{
	cs43l22_set_hp_volume(params[0],params[0]);
}

static void cmd_volume_master(uint32_t *params)
{
	cs43l22_set_master_volume(params[0],params[0]);
}

static void cmd_volume_spk(uint32_t *params)
{
	cs43l22_set_spk_volume(params[0],params[0]);
}

static void cmd_audio_hp_only(uint32_t *params)
{
	cs43l22_only_hp();
}

static void cmd_audio_on(uint32_t *params)
{
	cs43l22_on();
}

static void cmd_audio_off(uint32_t *params)
{
	cs43l22_off();
}

struct cmd{
	char *cmd_string;
	uint8_t num_params;
	void (*callback)(uint32_t *params);
};

static struct cmd cmds[] = {
	{"audio_hp",0,cmd_audio_hp_only},
	{"audio_off",0,cmd_audio_off},
	{"audio_on",0,cmd_audio_on},
	{"dab_c",1,cmd_dab_channel},
	{"dab_sl",0,cmd_dab_service_list},
	{"dab_start",1,cmd_dab_start_service},
	{"dab_status",0,cmd_dab_status},
	{"dab",0,cmd_dab},
	{"fm_f",1,cmd_fm_frequency},
	{"fm_rds",0,cmd_fm_rds},
	{"fm_status",0,cmd_fm_status},
	{"fm",0,cmd_fm},
	{"help",0,cmd_help},
	{"vol_hp",1,cmd_volume_hp},
	{"vol_master",1,cmd_volume_master},
	{"vol_spk",1,cmd_volume_spk},
};
static const uint8_t CMD_NUM = sizeof(cmds) / sizeof(cmds[0]);


static void cmd_help(uint32_t *params)
{
	uint8_t i;

	for(i=0;i<CMD_NUM;i++){
		printf("Command:%13s   Params:%d\r\n",cmds[i].cmd_string,cmds[i].num_params);
	}
}

void cmd_parse(char *recv_string)
{
	uint8_t i;
	uint8_t p;
	char *cmd_start_pos;
	char *temp_str;
	uint32_t params[5];
	uint8_t enough_params = 1;
	uint8_t cmd_found = 0;

	for(i=0;i<CMD_NUM;i++) {
		if((cmd_start_pos = strstr(recv_string,cmds[i].cmd_string)) != 0) {
			if(cmds[i].num_params) {
				temp_str = strtok(cmd_start_pos, " ");
				if(temp_str != NULL) {
					for(p=0;p<cmds[i].num_params;p++)
					{
						temp_str = strtok(NULL," ");
						if(temp_str != NULL){
							params[p] = atol(temp_str);
						}else{
							enough_params = 0;
							break;
						}
					}
				}
			}
			if(enough_params && cmds[i].callback){
				cmds[i].callback(params);
			}else{
				printf("Too few parameters\r\n");
			}
			cmd_found = 1;
			break;
		}
	}
	if(!cmd_found){
		printf("Command not found\r\n");
	}
}
