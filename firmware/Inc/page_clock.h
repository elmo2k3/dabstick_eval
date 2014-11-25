#ifndef PAGE_clock_H_
#define PAGE_clock_H_

#include "main.h"

void update_page_clock(struct menuitem *self, uint8_t event);
void page_clock(struct menuitem *self);
uint8_t page_clock_button_pressed(struct menuitem *self, uint8_t button);
void page_clock_drehgeber(struct menuitem *self, int8_t steps);



#endif /* PAGE_clock_H_ */
