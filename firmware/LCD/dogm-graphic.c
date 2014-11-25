/******************************************************************************
 * Display Library
 * for EA-DOGS102  GLCD (102px x 64px)
 *     EA-DOGM128  GLCD (128px x 64px)
 *     EA-DOGM132  GLCD (132px x 32px)
 *     EA-DOGL128  GLCD (128px x 64px)
 *     EA-DOGXL160 GLCD (160px x 104px) (under development)
 * 
 * Provides all basic functions to access the display
 * Since no graphics ram is used, the memory footprint is rather small but
 * also does not allow to change single pixels. Data written to the LCD can
 * not be read back!
 * Text can be displayed using the attached font generator containing several
 * character sets and font variants.
 * Thanks to Oliver Schwaneberg for adding several functions to this library!
 * 
 * Author:  Jan Michel (jan at mueschelsoft dot de)
 * License: GNU General Public License, version 3
 * Version: v0.94 October 2011
 * ****************************************************************************
 * New features in v0.94
 *   - corrected order of arguments in lcd_clear_area_xy()
 *   - added command definitions for DOGXL160
 *   - cleaned up dogm_graphic.h command area
 * New features in v0.93
 *   - modified initialization for DOGM128 and DOGS102
 * New features in v0.92
 *   - Added initialization for DOGS102 and DOGL128
 * New features in v0.91
 *   - Control of chip select pin
 *   - backlight & chip select are optional - can be turned of in header file
 *   - added function lcd_draw_image_P 
 *   - added function lcd_draw_image_xy_P
 *   - added function lcd_clear_area
 *   - added SPI initialization
 *****************************************************************************/

#include "dogm-graphic.h"

//=============================================================================
//keeping track of current position in ram - necessary for big fonts & bitmaps
//=============================================================================

uint8_t lcd_current_page = 0;
uint8_t lcd_current_column = 0;
uint8_t lcd_top_view = 0;

void lcd_set_backlight(uint16_t val)
{
	TIM_OC_InitTypeDef sConfigOC;

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = val;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
	HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}

/******************************************************************************
 * Changes the internal cursor by s pages
 * s             - number of pages to move
 */ 
uint8_t lcd_inc_page(int8_t s) {
	uint8_t p = lcd_current_page;
	p += s;
	p %= LCD_RAM_PAGES;    //all lcd have lcd_ram_pages which is power of two
	lcd_current_page = p;
	return p;
}

/******************************************************************************
 * Changes the internal cursor by s columns, including wrapping (if selected)
 * s             - number of columns to move
 */ 
uint8_t lcd_inc_column(int16_t s) {
	uint16_t c = lcd_current_column;
	c += s;
#if LCD_WRAP_AROUND == 1
	while (c >= LCD_WIDTH) {
		if (s > 0) lcd_inc_page(1);
		else       lcd_inc_page(-1);
		if (s > 0) c -= LCD_WIDTH;
		else       c += LCD_WIDTH;
	}
#endif
	lcd_current_column = c;
	return c;
}


/******************************************************************************
 * Moves the cursor to the given position
 * pages         - page to move to
 * columns       - column to move to
 */ 
void lcd_moveto_xy(uint8_t page, uint8_t column) {
	if(lcd_top_view){
		LCD_GOTO_ADDRESS(page,column+30); // BB geändert für TopView
	}else{
		LCD_GOTO_ADDRESS(page,column); // BB geändert für TopView
	}
	lcd_current_column = column;
	lcd_current_page = page;
}

/******************************************************************************
 * Moves the cursor relative to the current position
 * pages         - number of pages to move
 * columns       - number of columns to move
 */  
void lcd_move_xy(int8_t pages, int16_t columns) {
	lcd_moveto_xy(lcd_inc_page(pages),lcd_inc_column(columns));
}


//=============================================================================
//Basic Byte Access to Display
//=============================================================================

/******************************************************************************
 * Writes one data byte
 * data          - the data byte
 */
void lcd_data(uint8_t data) {
	LCD_SELECT();
	LCD_DRAM();
	spi_write(data);
	LCD_UNSELECT();
	lcd_inc_column(1);
}

/******************************************************************************
 * Writes one command byte
 * cmd           - the command byte
 */
void lcd_command(uint8_t cmd) {
	LCD_SELECT();
	LCD_CMD();
	spi_write(cmd);
	LCD_UNSELECT();
}


//=============================================================================
//Puts raw data from Flash to the Display
//=============================================================================
#if LCD_INCLUDE_GRAPHIC_FUNCTIONS >= 1
/******************************************************************************
 * This function draws a bitmap from the current position on the screen.
 * Parameters:
 * progmem_image - prog_uint8_t array of columns aka the bitmap image
 * pages         - height of image in pages
 * columns       - width of image in pixels (or columns)
 * style         - Bit2: sets inverse mode
 */  
void lcd_draw_image_P(PGM_P progmem_image, uint8_t pages, uint8_t columns, uint8_t style) {
	uint8_t i,j = 0;
	uint8_t inv = (style & INVERT_BIT);
	while(j<pages && (lcd_get_position_page() < LCD_RAM_PAGES)) {
		for (i=0; i<columns && (lcd_get_position_column() < LCD_WIDTH); i++) {
			uint8_t tmp = pgm_read_byte(progmem_image++);
			if(!inv)
				lcd_data(tmp);
			else
				lcd_data(~tmp);
		}
		if(++j != pages)
			lcd_move_xy(1,-columns);
	}
}


/******************************************************************************
 * This function draws a bitmap at any xy-position on the screen. 
 * Be aware that some pixels are deleted due to memory organization!
 * Parameters:
 * progmem_image - prog_uint8_t array of columns aka the bitmap image
 * x             - x start coordinate on the screen (in pixel)
 * y             - y start coordinate on the screen (in pixel)
 * pages         - height of image in pages
 * columns       - width of image in pixels
 * style         - Bit2: sets inverse mode
 */
void lcd_draw_image_xy_P(PGM_P progmem_image, uint8_t x, uint8_t y, uint8_t pages, uint8_t columns, uint8_t style) {
	uint16_t i,j;
	uint8_t data   = 0;
	uint8_t inv    = style & INVERT_BIT;
	uint8_t offset = y & 0x7; //Optimized modulo 8
	//If there is an offset, we must use an additional page
	if(offset)  
		pages++;
	//If there is not enough vertical space -> cut image
	if(pages > LCD_RAM_PAGES - lcd_get_position_page())   
		pages = LCD_RAM_PAGES - lcd_get_position_page();
	//Goto starting point and draw
	lcd_moveto_xy((y>>3), x);
	for (j=0; j<pages; j++) {
		for (i=0; i<columns && (lcd_get_position_column() < LCD_WIDTH); i++){
			data = 0;
			if (!offset || j+1 != pages)
				data = pgm_read_byte(&progmem_image[j*columns + i]) << offset;
			if(j > 0 && offset)
				data |= pgm_read_byte(&progmem_image[(j-1)*columns + i]) >> (8-offset);
			if(inv)	lcd_data(~data);
			else 		lcd_data(data);
		}
		if(j+1 != pages)
			lcd_move_xy(1,-columns);
	}
}
#endif


/******************************************************************************
 * This function clears an area of the screen
 * pages         - height of area in pages
 * columns       - width of area in pixels
 * style         - Bit2: sets inverse mode
 * Cursor is moved to start of area after clear
 */
void lcd_clear_area(uint8_t pages, uint8_t columns, uint8_t style) {
	uint8_t i,j,max;
	uint8_t inv = (style & INVERT_BIT)?0xFF:0;

	if(pages > (max = LCD_RAM_PAGES - lcd_get_position_page()))   
		pages = max;
	if(columns > (max = LCD_WIDTH - lcd_get_position_column()))   
		columns = max;

	for(j=0; j<pages; j++) {
		for(i=0; i<columns; i++) {
			lcd_data(inv);
		}
		lcd_move_xy((lcd_get_position_column()?1:0),-columns);
	}
	lcd_move_xy(-pages,0);
}

/******************************************************************************
 * This function clears an area of the screen starting at the given coordinates
 * pages         - height of area in pages
 * columns       - width of area in pixels
 * style         - style modifier
 * col           - column of upper left corner
 * page          - page of upper left corner
 * Cursor is moved to start of area after clear
 */
void lcd_clear_area_xy(uint8_t pages, uint8_t columns, uint8_t style, uint8_t page, uint8_t col) {
	lcd_moveto_xy(page,col);
	lcd_clear_area(pages,columns,style);
}


/******************************************************************************
 * Initializes the display in 6 o'clock mode, 4x booster for 2.4-3.3V supply voltage
 * scheme according to datasheet
 * Suitable for all DOGS, DOGM and DOGL displays.
 */
void lcd_init(uint8_t top_view) {
	//LCD_SET_PIN_DIRECTIONS();  //set outputs
	//LCD_INIT_SPI();            //Initialize SPI Interface
	LCD_RESET_OFF();               //Apply Reset to the Display Controller
	LCD_SEND_RESET();
	//Load settings
#if DISPLAY_TYPE == 160
	LCD_SET_COM_END(103);               //set last COM electrode
	LCD_SET_MAPPING_CTRL(0);            //set mapping control to "normal"
	LCD_SET_START_LINE(0);              //set scrolling to 0
	LCD_SET_PANEL_LOAD(3);              //set panel loading to 28-38nF
	LCD_SET_BIAS_RATIO(3);              //set bias ratio
	LCD_SET_VOLTAGE_BIAS(0x5F);         //set Vbias potentiometer for contrast
	LCD_SET_RAM_ADDR_CTRL(1);           //set auto-increment
#endif
#if DISPLAY_TYPE == 132
	LCD_SET_FIRST_LINE(0);              //first bit in RAM is on the first line of the LCD
	LCD_SET_BOTTOM_VIEW();              //6 o'clock mode, normal orientation
	LCD_ORIENTATION_NORMAL();
	LCD_SHOW_ALL_PIXELS_OFF();          //Normal Pixel mode
	LCD_SET_MODE_POSITIVE();            //positive display
	LCD_SET_BIAS_RATIO_1_9();           //bias 1/9
	LCD_SET_POWER_CONTROL(7);           //power control mode: all features on
	LCD_SET_BIAS_VOLTAGE(3);            //set voltage regulator R/R
	LCD_SET_VOLUME_MODE(0x1F);          //volume mode set
	LCD_SET_INDICATOR_OFF();            //switch indicator off, no blinking
#endif
#if DISPLAY_TYPE == 128
	LCD_SET_FIRST_LINE(0);              //first bit in RAM is on the first line of the LCD
	LCD_SET_BOTTOM_VIEW();              //6 o'clock mode, normal orientation
	LCD_ORIENTATION_NORMAL();
	LCD_SHOW_ALL_PIXELS_OFF();          //Normal Pixel mode
	LCD_SET_MODE_POSITIVE();            //positive display
	LCD_SET_BIAS_RATIO_1_7();           //bias 1/7
	LCD_SET_POWER_CONTROL(7);           //power control mode: all features on
	LCD_SET_BIAS_VOLTAGE(7);            //set voltage regulator R/R
	LCD_SET_VOLUME_MODE(0x06);          //volume mode set
	LCD_SET_INDICATOR_OFF();            //switch indicator off, no blinking
#endif
#if DISPLAY_TYPE == 102
	if(top_view){
		LCD_ORIENTATION_UPSIDEDOWN();
		LCD_SET_TOP_VIEW();              //12 o'clock mode, column now starts at 30 not 0
		lcd_top_view = 1;
	}else{
		LCD_SET_BOTTOM_VIEW();              //6 o'clock mode, normal orientation
		LCD_ORIENTATION_NORMAL();			//
		lcd_top_view = 0;
	}
	LCD_SET_FIRST_LINE(0);              //first bit in RAM is on the first line of the LCD
	LCD_SHOW_ALL_PIXELS_OFF();          //Normal Pixel mode
	LCD_SET_MODE_POSITIVE();            //positive display
	LCD_SET_BIAS_RATIO_1_9();           //bias 1/9
	LCD_SET_POWER_CONTROL(7);           //power control mode: all features on
	LCD_SET_BIAS_VOLTAGE(7);            //set voltage regulator R/R
	LCD_SET_VOLUME_MODE(0x9);           //volume mode set
	LCD_SET_ADV_PROG_CTRL(LCD_TEMPCOMP_HIGH);
#endif
	lcd_clear_area_xy(LCD_RAM_PAGES,LCD_WIDTH,NORMAL,0,0); //clear display content

	LCD_SWITCH_ON();                    //Switch display on
	return;
}
