



#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "dflash.h"
#define READword(address)     ((unsigned int)(*(volatile unsigned int *__near)(address)))

#define DFLASH_LOWEST_START_PAGE        0x00        //¶¨Òådata flashµÄÆğÊ¼Ò³
#define DFLASH_START                    0x00100000  //¶¨Òådata flashµÄÆğÊ¼µØÖ·
#define DFLASH_PAGE_SIZE                0x0400      //¶¨Òådata flashµÄ´óĞ¡Îª1K..
#define DFLASH_PAGE_WINDOW_START        0x0800      //¶¨Òådata flashÒ³Ãæ´°¿ÚµÄÆğÊ¼µØÖ·
#define LEDCPU PORTD_PD0               
#define LEDCPU_dir DDRD_DDRD0



#pragma  CODE_SEG DEFAULT_RAM
/*************************************************************/
/*                      åˆå§‹åŒ–DFLASH                         */
/*************************************************************/
void DFlash_Init(void)
{
   while(FSTAT_CCIF==0);            //µÈ´ıÕıÔÚ´¦ÀíµÄFLASH²Ù×÷Íê³É

#if(OSC_CLOCK == 16000000)
    FCLKDIV=0x0F;  
#else
    FCLKDIV=0x07;    
#endif    


  // FCLKDIV=0x0F;                    //Íâ²¿¾§ÕñÎª16M.FLASHÊ±ÖÓ²»³¬¹ı1M£¬¾ßÌå²ÎÕÕÊÖ²áŒ
   FCNFG=0x00;                      //½ûÖ¹ÖĞ¶Ï
   while(FCLKDIV_FDIVLD==0);         //µÈ´ıÊ±ÖÓÉèÖÃ³É¹¦
}

#define  DEFAULT_OP_FLASH_ADDR   0x0000
 //ÉÏÃæµÄ8×Ö½Ú¶ÔÆë°É
uint16_t flash_write(uint8_t* addr,uint16_t size)
{
    uint16_t i;
    uint16_t write_size;
    uint16_t start_addr=DEFAULT_OP_FLASH_ADDR;
    write_size=0;
    if(size<=0)
    {
        return 0;   
    }
     
    while(write_size<size)
    {
        while(FSTAT_CCIF==0); 
        if(FSTAT_ACCERR)           //ÅĞ¶Ï²¢Çå³ı±êÖ¾Î»£»
            FSTAT_ACCERR=1;
        if(FSTAT_FPVIOL)           //ÅĞ¶Ï²¢Çå³ı±êÖ¾Î»£»
            FSTAT_FPVIOL=1;
        FCCOBIX_CCOBIX=0x00; 
        FCCOB=0x1110;         //Ğ´ÈëÃüÁîºÍ¸ßÎ»µØÖ·
        FCCOBIX_CCOBIX=0x01;  //µØÖ·ºó16Î»
        FCCOB=start_addr;     //Ğ´ÈëµÍ16Î»µØÖ·
        FCCOBIX_CCOBIX=0x02;  //Ğ´ÈëµÚÒ»¸öÊı¾İ
        FCCOB=(addr[write_size]<<8)|addr[write_size+1];
        write_size+=2;
        FCCOBIX_CCOBIX=0x03;  //Ğ´ÈëµÚ¶ş¸öÊı¾İ
        FCCOB=(addr[write_size]<<8)|addr[write_size+1];
        write_size+=2;
        FCCOBIX_CCOBIX=0x04;  //Ğ´ÈëµÚÈı¸öÊı¾İ
        FCCOB=(addr[write_size]<<8)|addr[write_size+1];
        write_size+=2;
        FCCOBIX_CCOBIX=0x05;  //Ğ´ÈëµÚËÄ¸öÊı¾İ
        FCCOB=(addr[write_size]<<8)|addr[write_size+1];
        write_size+=2;
        FSTAT_CCIF=1;         //Ğ´ÈëÖ´ĞĞÃüÁî
        while(FSTAT_CCIF==0); //µÈ´ıÖ´ĞĞÍê±Ï
        start_addr=start_addr+8;
    }
    
    return write_size;

}

  /*

uint16_t flash_read (uint8_t* addr,uint16_t size){


    byte   lastepage;          //ÓÃÓÚ´æ´¢EPAGEµÄÖµ
    byte   epage;              //ÓÃÓÚ¼ÆËãEPAGEµÄÖµ
    uint16_t data;         //¶ÁÈ¡³öµÄÊı¾İ
    uint16_t i;
    uint16_t start_addr;
    uint16_t read_size;
    uint16_t rest_size;
    uint16_t read_data;
     
    if(size<=0)
    {
        return 0;   
    }

     start_addr=DEFAULT_OP_FLASH_ADDR;
    
     read_size=0;
     lastepage = EPAGE;   //±£´æEPAGEµÄÖµ
     //½øĞĞÒ»Ò³Ò»Ò³µÄ¶ÁÈ¡
    
     while(read_size<size)
     {
          // Ò»Ò³ 1K
        epage = (byte)((DFLASH_LOWEST_START_PAGE)+(start_addr >>10));   //¼ÆËãEPAGE
        EPAGE=epage; 
        rest_size=size-read_size;
        if(rest_size>DFLASH_PAGE_SIZE)
        {
            rest_size=DFLASH_PAGE_SIZE;
        }
        for(i=0;i<rest_size;i++)
        {
           //¶ÁÈ¡Ò³Ãæ´°¿ÚÖĞµÄÊı¾İ
           read_data=READword((start_addr & (DFLASH_PAGE_SIZE - 1)) + DFLASH_PAGE_WINDOW_START);
           
           addr[read_size++]=(read_data>>8)&0xff;
           addr[read_size++]=read_data&0xff;
           start_addr+=2;
        }
     }
    
    EPAGE= lastepage;      //»Ö¸´EPAGEµÄÖµ

    return read_size;
}
   */
uint16_t flash_read (uint8_t* addr,uint16_t size)
{
    byte   lastepage;          //ÓÃÓÚ´æ´¢EPAGEµÄÖµ
    byte   epage;              //ÓÃÓÚ¼ÆËãEPAGEµÄÖµ
    unsigned int data;         //¶ÁÈ¡³öµÄÊı¾İ
    int Lop=0;

    lastepage = EPAGE;   //±£´æEPAGEµÄÖµ

    epage = (byte)((DFLASH_LOWEST_START_PAGE)+(DEFAULT_OP_FLASH_ADDR >>10));   //¼ÆËãEPAGE
    EPAGE=epage;                                                     //¸øEPAGE¸³Öµ
 
    for(Lop=0;Lop<(size+1)/2;Lop++) 
    {
      
      data = READword((Lop*2 & (DFLASH_PAGE_SIZE - 1)) + DFLASH_PAGE_WINDOW_START);  //¶ÁÈ¡Ò³Ãæ´°¿ÚÖĞµÄÊı¾İ
      addr[Lop*2]=(data>>8)&0xff;
      addr[Lop*2+1]=data&0xff;
    }
    EPAGE= lastepage;       //»Ö¸´EPAGEµÄÖµ

    return size;
}
  
unsigned int    Buffer[]={0x1111,0x2222,0x3333,0x4444};//Êı¾İ»º´æÇø£¬Ö»ÄÜÒ»´ÎĞ´ÈëËÄ¸öÊı¾İ
unsigned int    data_Address=0x0000;
unsigned int    date_read[4];


  
//²âÊÔFlash
/*************************************************************/
/*                     ÏòDFLASHĞ´ÈëÊı¾İ                      */
/*************************************************************/
void DFlash_Write(word ADDR16)
{
    while(FSTAT_CCIF==0); 
    if(FSTAT_ACCERR)           //ÅĞ¶Ï²¢Çå³ı±êÖ¾Î»£»
        FSTAT_ACCERR=1;
    if(FSTAT_FPVIOL)           //ÅĞ¶Ï²¢Çå³ı±êÖ¾Î»£»
        FSTAT_FPVIOL=1;
    FCCOBIX_CCOBIX=0x00; 
    FCCOB=0x1110;         //Ğ´ÈëÃüÁîºÍ¸ßÎ»µØÖ·
    FCCOBIX_CCOBIX=0x01;  //µØÖ·ºó16Î»
    FCCOB=ADDR16;         //Ğ´ÈëµÍ16Î»µØÖ·
    FCCOBIX_CCOBIX=0x02;  //Ğ´ÈëµÚÒ»¸öÊı¾İ
    FCCOB=Buffer[0];
    FCCOBIX_CCOBIX=0x03;  //Ğ´ÈëµÚ¶ş¸öÊı¾İ
    FCCOB=Buffer[1];
    FCCOBIX_CCOBIX=0x04;  //Ğ´ÈëµÚÈı¸öÊı¾İ
    FCCOB=Buffer[2];
    FCCOBIX_CCOBIX=0x05;  //Ğ´ÈëµÚËÄ¸öÊı¾İ
    FCCOB=Buffer[3];  
      
    FSTAT_CCIF=1;         //Ğ´ÈëÖ´ĞĞÃüÁî
    while(FSTAT_CCIF==0); //µÈ´ıÖ´ĞĞÍê±Ï
} 

/*************************************************************/
/*                     ÓÉDFLASH¶ÁÈ¡Êı¾İ                      */
/*************************************************************/
word DFlash_Read (word destination)
{
    byte   lastepage;          //ÓÃÓÚ´æ´¢EPAGEµÄÖµ
    byte   epage;              //ÓÃÓÚ¼ÆËãEPAGEµÄÖµ
    unsigned int data;         //¶ÁÈ¡³öµÄÊı¾İ

    lastepage = EPAGE;   //±£´æEPAGEµÄÖµ

    epage = (byte)((DFLASH_LOWEST_START_PAGE)+(destination >>10));   //¼ÆËãEPAGE
    EPAGE=epage;                                                     //¸øEPAGE¸³Öµ
 
    data = READword((destination & (DFLASH_PAGE_SIZE - 1)) + DFLASH_PAGE_WINDOW_START);  //¶ÁÈ¡Ò³Ãæ´°¿ÚÖĞµÄÊı¾İ

    EPAGE= lastepage;       //»Ö¸´EPAGEµÄÖµ

    return(data);
}

void FlashTest(void) 
{

    date_read[0]=DFlash_Read(data_Address); 
    date_read[1]=DFlash_Read(data_Address+2); 
    date_read[2]=DFlash_Read(data_Address+4); 
    date_read[3]=DFlash_Read(data_Address+6);
    //if(date_read[0]!=0XFFFF) 
    {
      Buffer[0]=Buffer[0]+1;
      
      DFlash_Erase(data_Address);     //È·±£ÏÈ²Á³ıºóĞ´Èë
      DFlash_Write(data_Address);
    }
    printf("Read=%X\r\n",date_read[0]);
}

/*************************************************************/
/*                     ²Á³ıDFLASHµÄÒ»¸ö·ÖÇø                 */
/*************************************************************/
void DFlash_Erase(word ADDR16)
{   
  while(FSTAT_CCIF==0);
  if(FSTAT_ACCERR)           
      FSTAT_ACCERR=1;
  if(FSTAT_FPVIOL)           
      FSTAT_FPVIOL=1;
  
  FCCOBIX_CCOBIX=0x00;
  FCCOB=0x1210;           //Ğ´Èë²Á³ıÃüÁîºÍ¸ßÎ»µØÖ·    ¸ßÎ»ÊÇ0x10
  FCCOBIX_CCOBIX=0x01;   
  FCCOB=ADDR16;           //Ğ´ÈëµÍ16Î»µÄµØÖ·
  FSTAT_CCIF=1;           //Æô¶¯Ö´ĞĞÃüÁî
  while(FSTAT_CCIF==0);   //µÈ´ıÖ´ĞĞÍê³É
}

#pragma CODE_SEG DEFAULF

