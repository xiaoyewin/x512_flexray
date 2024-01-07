#include <hidef.h>      /* common defines and macros */




#define CS_dir DDRM_DDRM6
#define PSB PTM_PTM7
#define PSB_dir DDRM_DDRM7
#define somenop(); asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");


void delay20us(unsigned int n);
void delay1ms(unsigned int n); 
 

