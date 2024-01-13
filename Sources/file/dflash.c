



#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "dflash.h"
#define READword(address)     ((unsigned int)(*(volatile unsigned int *__near)(address)))

#define DFLASH_LOWEST_START_PAGE        0x00        //����data flash����ʼҳ
#define DFLASH_START                    0x00100000  //����data flash����ʼ��ַ
#define DFLASH_PAGE_SIZE                0x0400      //����data flash�Ĵ�СΪ1K..
#define DFLASH_PAGE_WINDOW_START        0x0800      //����data flashҳ�洰�ڵ���ʼ��ַ
#define LEDCPU PORTD_PD0               
#define LEDCPU_dir DDRD_DDRD0



#pragma  CODE_SEG DEFAULT_RAM
/*************************************************************/
/*                      初始化DFLASH                         */
/*************************************************************/
void DFlash_Init(void)
{
   while(FSTAT_CCIF==0);            //�ȴ����ڴ����FLASH�������

#if(OSC_CLOCK == 16000000)
    FCLKDIV=0x0F;  
#else
    FCLKDIV=0x07;    
#endif    


  // FCLKDIV=0x0F;                    //�ⲿ����Ϊ16M.FLASHʱ�Ӳ�����1M����������ֲ�
   FCNFG=0x00;                      //��ֹ�ж�
   while(FCLKDIV_FDIVLD==0);         //�ȴ�ʱ�����óɹ�
}

#define  DEFAULT_OP_FLASH_ADDR   0x0000
 //�����8�ֽڶ����
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
        if(FSTAT_ACCERR)           //�жϲ������־λ��
            FSTAT_ACCERR=1;
        if(FSTAT_FPVIOL)           //�жϲ������־λ��
            FSTAT_FPVIOL=1;
        FCCOBIX_CCOBIX=0x00; 
        FCCOB=0x1110;         //д������͸�λ��ַ
        FCCOBIX_CCOBIX=0x01;  //��ַ��16λ
        FCCOB=start_addr;     //д���16λ��ַ
        FCCOBIX_CCOBIX=0x02;  //д���һ������
        FCCOB=(addr[write_size]<<8)|addr[write_size+1];
        write_size+=2;
        FCCOBIX_CCOBIX=0x03;  //д��ڶ�������
        FCCOB=(addr[write_size]<<8)|addr[write_size+1];
        write_size+=2;
        FCCOBIX_CCOBIX=0x04;  //д�����������
        FCCOB=(addr[write_size]<<8)|addr[write_size+1];
        write_size+=2;
        FCCOBIX_CCOBIX=0x05;  //д����ĸ�����
        FCCOB=(addr[write_size]<<8)|addr[write_size+1];
        write_size+=2;
        FSTAT_CCIF=1;         //д��ִ������
        while(FSTAT_CCIF==0); //�ȴ�ִ�����
        start_addr=start_addr+8;
    }
    
    return write_size;

}

  /*

uint16_t flash_read (uint8_t* addr,uint16_t size){


    byte   lastepage;          //���ڴ洢EPAGE��ֵ
    byte   epage;              //���ڼ���EPAGE��ֵ
    uint16_t data;         //��ȡ��������
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
     lastepage = EPAGE;   //����EPAGE��ֵ
     //����һҳһҳ�Ķ�ȡ
    
     while(read_size<size)
     {
          // һҳ 1K
        epage = (byte)((DFLASH_LOWEST_START_PAGE)+(start_addr >>10));   //����EPAGE
        EPAGE=epage; 
        rest_size=size-read_size;
        if(rest_size>DFLASH_PAGE_SIZE)
        {
            rest_size=DFLASH_PAGE_SIZE;
        }
        for(i=0;i<rest_size;i++)
        {
           //��ȡҳ�洰���е�����
           read_data=READword((start_addr & (DFLASH_PAGE_SIZE - 1)) + DFLASH_PAGE_WINDOW_START);
           
           addr[read_size++]=(read_data>>8)&0xff;
           addr[read_size++]=read_data&0xff;
           start_addr+=2;
        }
     }
    
    EPAGE= lastepage;      //�ָ�EPAGE��ֵ

    return read_size;
}
   */
uint16_t flash_read (uint8_t* addr,uint16_t size)
{
    byte   lastepage;          //���ڴ洢EPAGE��ֵ
    byte   epage;              //���ڼ���EPAGE��ֵ
    unsigned int data;         //��ȡ��������
    int Lop=0;

    lastepage = EPAGE;   //����EPAGE��ֵ

    epage = (byte)((DFLASH_LOWEST_START_PAGE)+(DEFAULT_OP_FLASH_ADDR >>10));   //����EPAGE
    EPAGE=epage;                                                     //��EPAGE��ֵ
 
    for(Lop=0;Lop<(size+1)/2;Lop++) 
    {
      
      data = READword((Lop*2 & (DFLASH_PAGE_SIZE - 1)) + DFLASH_PAGE_WINDOW_START);  //��ȡҳ�洰���е�����
      addr[Lop*2]=(data>>8)&0xff;
      addr[Lop*2+1]=data&0xff;
    }
    EPAGE= lastepage;       //�ָ�EPAGE��ֵ

    return size;
}
  
unsigned int    Buffer[]={0x1111,0x2222,0x3333,0x4444};//���ݻ�������ֻ��һ��д���ĸ�����
unsigned int    data_Address=0x0000;
unsigned int    date_read[4];


  
//����Flash
/*************************************************************/
/*                     ��DFLASHд������                      */
/*************************************************************/
void DFlash_Write(word ADDR16)
{
    while(FSTAT_CCIF==0); 
    if(FSTAT_ACCERR)           //�жϲ������־λ��
        FSTAT_ACCERR=1;
    if(FSTAT_FPVIOL)           //�жϲ������־λ��
        FSTAT_FPVIOL=1;
    FCCOBIX_CCOBIX=0x00; 
    FCCOB=0x1110;         //д������͸�λ��ַ
    FCCOBIX_CCOBIX=0x01;  //��ַ��16λ
    FCCOB=ADDR16;         //д���16λ��ַ
    FCCOBIX_CCOBIX=0x02;  //д���һ������
    FCCOB=Buffer[0];
    FCCOBIX_CCOBIX=0x03;  //д��ڶ�������
    FCCOB=Buffer[1];
    FCCOBIX_CCOBIX=0x04;  //д�����������
    FCCOB=Buffer[2];
    FCCOBIX_CCOBIX=0x05;  //д����ĸ�����
    FCCOB=Buffer[3];  
      
    FSTAT_CCIF=1;         //д��ִ������
    while(FSTAT_CCIF==0); //�ȴ�ִ�����
} 

/*************************************************************/
/*                     ��DFLASH��ȡ����                      */
/*************************************************************/
word DFlash_Read (word destination)
{
    byte   lastepage;          //���ڴ洢EPAGE��ֵ
    byte   epage;              //���ڼ���EPAGE��ֵ
    unsigned int data;         //��ȡ��������

    lastepage = EPAGE;   //����EPAGE��ֵ

    epage = (byte)((DFLASH_LOWEST_START_PAGE)+(destination >>10));   //����EPAGE
    EPAGE=epage;                                                     //��EPAGE��ֵ
 
    data = READword((destination & (DFLASH_PAGE_SIZE - 1)) + DFLASH_PAGE_WINDOW_START);  //��ȡҳ�洰���е�����

    EPAGE= lastepage;       //�ָ�EPAGE��ֵ

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
      
      DFlash_Erase(data_Address);     //ȷ���Ȳ�����д��
      DFlash_Write(data_Address);
    }
    printf("Read=%X\r\n",date_read[0]);
}

/*************************************************************/
/*                     ����DFLASH��һ������                 */
/*************************************************************/
void DFlash_Erase(word ADDR16)
{   
  while(FSTAT_CCIF==0);
  if(FSTAT_ACCERR)           
      FSTAT_ACCERR=1;
  if(FSTAT_FPVIOL)           
      FSTAT_FPVIOL=1;
  
  FCCOBIX_CCOBIX=0x00;
  FCCOB=0x1210;           //д���������͸�λ��ַ    ��λ��0x10
  FCCOBIX_CCOBIX=0x01;   
  FCCOB=ADDR16;           //д���16λ�ĵ�ַ
  FSTAT_CCIF=1;           //����ִ������
  while(FSTAT_CCIF==0);   //�ȴ�ִ�����
}

#pragma CODE_SEG DEFAULF

