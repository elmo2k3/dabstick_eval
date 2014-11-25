#ifndef COMPAT_AVR_H_
#define COMPAT_AVR_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define PGM_P const uint8_t *
#define PGM_VOID_P const void *
#define pgm_read_byte(addr) (*(addr))
#define pgm_read_word(addr) (*(addr))
#define PROGMEM   __attribute__(())
#define memcpy_P memcpy


#endif /* COMPAT_AVR_H_ */
