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
#include "stm32f4xx_hal.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "user_diskio.h" /* defines USER_Driver as external */
#include "usb_device.h"

#include "main.h"
#include "si46xx.h"
#include "cs43l22.h"
#include "buttons.h"
#include "uart.h"
#include "dabstick.h"
#include "dogm-graphic.h"
#include "drehgeber.h"
#include "menu.h"

volatile uint8_t timer_100ms;
volatile uint8_t timer_1s;
FATFS RAMDISKFatFs;

#define I2S_BUF_SIZE 1024
/* static uint16_t i2s_tx_buf[4][I2S_BUF_SIZE]; */
static uint16_t i2s_tx_buf[I2S_BUF_SIZE];
static uint16_t i2s_tx_buf2[I2S_BUF_SIZE];
static volatile uint16_t *i2s_frontbuf;
static volatile uint16_t *i2s_backbuf;

int __io_putchar(int ch) //to make printf work
{
	HAL_UART_Transmit(&huart2, (uint8_t*)&ch,1,100);
	return 1;
}

void HAL_SYSTICK_Callback(void){

	static uint16_t cnt = 0;
	static uint16_t cnt2 = 0;

	buttons_every_1_ms();
	drehgeber_work();
	if(++cnt == 100){ // every 100ms
		timer_100ms = 1;
		cnt = 0;
	}
	if(++cnt2 == 1000){ // every 100ms
		timer_1s = 1;
		cnt2 = 0;
	}
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	static uint8_t first_run = 1;
	if(first_run){
		first_run = 0;
		HAL_I2S_Transmit_DMA(&hi2s3,i2s_backbuf,I2S_BUF_SIZE);
	}
}

volatile uint8_t lock = 0;
volatile uint8_t swapped = 0;
volatile uint8_t rx_cplt = 0;
volatile uint8_t tx_cplt = 0;

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	/* rx_cplt = 1; */
	static uint8_t first_run = 1;
	uint16_t *tmp;
	uint16_t timeout = 100;
	static uint8_t toggle = 0;

	if(toggle){
		GPIOB->BSRRL = (1<<13);
	}else{
		GPIOB->BSRRH = (1<<13);
	}
	toggle ^=1;

	lock = 1;
	tmp = i2s_frontbuf;
	i2s_frontbuf = i2s_backbuf;
	i2s_backbuf = tmp;
	lock = 0;
	swapped = 1;

	/* if(first_run){ */
	/* 	first_run = 0; */
	/* 	HAL_I2S_Transmit_DMA(&hi2s3,i2s_backbuf,I2S_BUF_SIZE); */
	/* } */
	/* HAL_I2S_Receive_DMA(&hi2s2,i2s_rx_ptr,I2S_BUF_SIZE); */
	HAL_I2S_Receive_DMA(&hi2s2,i2s_frontbuf,I2S_BUF_SIZE);
	/* while(HAL_I2S_GetState(&hi2s3) != HAL_I2S_STATE_READY && --timeout); */
	/* while(HAL_I2S_GetState(&hi2s3) != HAL_I2S_STATE_READY); */
	/* if(HAL_I2S_GetState(&hi2s3) == HAL_I2S_STATE_READY){ */
		/* HAL_I2S_Transmit_DMA(&hi2s3,i2s_backbuf,I2S_BUF_SIZE); */
	/* } */
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	static uint8_t toggle = 0;

	if(toggle){
		GPIOB->BSRRL = (1<<14);
	}else{
		GPIOB->BSRRH = (1<<14);
	}
	toggle ^=1;
	/* tx_cplt = 1; */
	HAL_I2S_Transmit_DMA(&hi2s3,i2s_backbuf,I2S_BUF_SIZE);
	swapped = 0;
}

void change_in_range(int8_t *old, int8_t inc, int8_t upper, int8_t lower)
{
	int8_t *ret = old;

	if(((int16_t)*ret + (int16_t)inc) > (int16_t)upper){
		*ret = upper;
	}else if(((int16_t)*ret + (int16_t)inc) < (int16_t)lower){
		*ret = lower;
	}else{
		*ret += inc;
	}
}

void dabstick_loop()
{
	char rxchar;
	uint16_t antcap = 0;
	int8_t volume = -10;
	FIL myfile;
	uint8_t toggle = 0;
	uint8_t toggle2 = 0;
	uint8_t station_num = 0;
	int8_t tmp;
	uint8_t first_run = 1;
	uint8_t lock = 0;
	uint8_t swapped = 0;

	cs43l22_init();
	drehgeber_init();

	i2s_frontbuf = i2s_tx_buf;
	i2s_backbuf = i2s_tx_buf2;
	if(HAL_I2S_Receive_DMA(&hi2s2,i2s_frontbuf,I2S_BUF_SIZE) != HAL_OK){
		printf("HAL_Status receive not OK\r\n");
	}

	lcd_init(0);
	lcd_set_backlight(DEFAULT_BACKLIGHT);
	menu_init();
	lcd_set_font(FONT_FIXED_8,NORMAL);

	/* if(f_mount(&RAMDISKFatFs,(TCHAR const*)USER_Path,0) != FR_OK){ */
	/*     printf("f_mount error\r\n"); */
	/* }else{ */
	/*     if(f_mkfs((TCHAR const*)USER_Path,512,0) != FR_OK){ */
	/*       printf("f_mkfs error\r\n"); */
	/*     } */
	/* } */
	/* f_open(&myfile, "WDR2", FA_CREATE_ALWAYS | FA_WRITE); */
	/* f_close(&myfile); */
	/* f_open(&myfile, "", FA_CREATE_ALWAYS | FA_WRITE); */
	/* f_close(&myfile); */
	/* HAL_Delay(200); */
	radio_dab();
	/* Infinite loop */
	while (1)
	{
		if(timer_100ms == 1){
			timer_100ms = 0;

			if(toggle){
				GPIOE->BSRRH |= LED_RED1;
			}else{
				GPIOE->BSRRL |= LED_RED1;
			}
			toggle ^= 1;
			menu_tick();
		}
		if(timer_1s == 1){
			timer_1s = 0;
			si46xx_dab_get_digital_service_list();
			si46xx_dab_get_subchannel_info();
		}
		if(get_key_press(KEY0) || get_key_press(KEY3)){ // KEY3 funtzt nicht?
			if(si46xx_status.mode == MODE_DAB){
				radio_fm();
			}else{
				radio_dab();
			}
		}
		if(get_key_press(KEY1)){
			si46xx_dab_get_digital_service_list();
			si46xx_dab_start_digital_service_num(2);
		}
		if(get_key_press(KEY2)){
			si46xx_dab_get_digital_service_list();
			si46xx_dab_start_digital_service_num(station_num);
			if(++station_num >= dab_service_list.num_services)
				station_num = 0;
		}
		uart_work();
		menu_task();
		if(si46xx_status.mode == MODE_FM){
			GPIOE->BSRRH |= LED_RED2;
		}else{
			GPIOE->BSRRL |= LED_RED2;
		}

		/* if(HAL_UART_Receive(&huart2, (uint8_t*)&rxchar,1,1) == HAL_OK){ */
		/*     switch(rxchar){ */
		/*         case 'a': radio_fm(); break; */
		/*         case 'b': radio_dab(); break; */
		/*         case 'c': si46xx_dab_digrad_status(&dab_digrad_status); */
		/*                   si46xx_dab_digrad_status_print(&dab_digrad_status); */
		/*                   break; */
		/*         case 'd': si46xx_dab_get_digital_service_list(); break; */
		/*         case 'e': si46xx_dab_print_service_list(); break; */
		/*         case 'f': si46xx_set_property(SI46XX_DAB_TUNE_FE_CFG,0x0000); break;// switch open */
		/*         case 'g': si46xx_set_property(SI46XX_DAB_TUNE_FE_CFG,0x0001); break;// switch closed */
		/*         case 'h': si46xx_dab_tune_freq(0,antcap); break; */
		/*         case 'i': si46xx_dab_start_digital_service(0x0000d392,0x00000007); break; */
		/*         case 'j': cs43l22_off(); break; // both off */
		/*         case 'k': cs43l22_on(); break; // both on */
		/*         case 'l': si46xx_fm_rsq_status(); break; */
		/*         case 'm': si46xx_fm_rds_status(); break; */
		/*         case 'o': si46xx_set_property(SI46XX_FM_RDS_CONFIG, 0x0001); break; */
		/*         case 'p': si46xx_set_property(SI46XX_FM_TUNE_FE_CFG,0x0001); break; // switch closed */
		/*         case 'q': si46xx_set_property(SI46XX_FM_TUNE_FE_CFG,0x0000); break; // switch closed */
		/*         case 'r': si46xx_fm_tune_freq(106700,0); break; */
		/*         case 's': si46xx_fm_tune_freq(98500,0); break; */
		/*         case 't': si46xx_fm_tune_freq(98500,127); break; */
		/*         case 'u': si46xx_dab_start_digital_service(0x0000d391,0x00000001); break; // einslive */
		/*         case '+': volume += 10; printf("volume: %d\r\n",volume);cs43l22_set_hp_volume(volume,volume); break; */
		/*         case '-': volume -= 10; printf("volume: %d\r\n",volume);cs43l22_set_hp_volume(volume,volume); break; */
		/*         default: printf("%c not known \r\n",rxchar); break; */
		/*     } */
		/* } */
	}
}
