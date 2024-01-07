#ifndef  _DFLASH_H_
#define  _DFLASH_H_


#include "Fr_UNIFIED_types.h"     

void DFlash_Init(void);


void DFlash_Erase(word ADDR16);

uint16_t flash_read(uint8_t *addr, uint16_t size);
uint16_t flash_write(uint8_t *addr, uint16_t size);


#endif

