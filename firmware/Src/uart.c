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
#include "main.h"
#include "uart.h"
#include "command.h"

static char recv_string[20];

void uart_work()
{
	char recv_char;
	static uint8_t count = 0;

	if(HAL_UART_Receive(&huart2, (uint8_t*)&recv_char,1,1) == HAL_OK){
		if(recv_char == '\r' || count == 18)
		{
			recv_string[count] = '\0'; // definitiv null terminierter string
			cmd_parse(recv_string);
			count = 0;
		}
		else
		{
			recv_string[count++] = recv_char;
		}
	}
}
