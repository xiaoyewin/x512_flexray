



#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "dflash.h"
#define READword(address)     ((unsigned int)(*(volatile unsigned int *__near)(address)))

#define DFLASH_LOWEST_START_PAGE        0x00        //定义data flash的起始页
#define DFLASH_START                    0x00100000  //定义data flash的起始地址
#define DFLASH_PAGE_SIZE                0x0400      //定义data flash的大小为1K..
#define DFLASH_PAGE_WINDOW_START        0x0800      //定义data flash页面窗口的起始地址
#define LEDCPU PORTD_PD0               
#define LEDCPU_dir DDRD_DDRD0




/*************************************************************/
/*                      鍒濆鍖朌FLASH                         */
/*************************************************************/
void DFlash_Init(void){
   while(FSTAT_CCIF==0);            //等待正在处理的FLASH操作完成

#if(OSC_CLOCK == 16000000)
    FCLKDIV=0x0F;  
#else
    FCLKDIV=0x07;    
#endif    


  // FCLKDIV=0x0F;                    //外部晶振为16M.FLASH时钟不超过1M，具体参照手册�
   FCNFG=0x00;                      //禁止中断
   while(FCLKDIV_FDIVLD==0);         //等待时钟设置成功
}

  #define  DEFAULT_OP_FLASH_ADDR   0x0000
 //上面的8字节对齐吧
uint16_t flash_write(uint8_t* addr,uint16_t size)
{
    uint16_t i;
    uint16_t write_size;
    uint16_t start_addr=DEFAULT_OP_FLASH_ADDR;
    write_size=0;
    if(size<=0){
        return 0;   
    }
     
    while(write_size<size){
        while(FSTAT_CCIF==0); 
        if(FSTAT_ACCERR)           //判断并清除标志位；
            FSTAT_ACCERR=1;
        if(FSTAT_FPVIOL)           //判断并清除标志位；
            FSTAT_FPVIOL=1;
        FCCOBIX_CCOBIX=0x00; 
        FCCOB=0x1110;         //写入命令和高位地址
        FCCOBIX_CCOBIX=0x01;  //地址后16位
        FCCOB=start_addr;     //写入低16位地址
        FCCOBIX_CCOBIX=0x02;  //写入第一个数据
        FCCOB=(addr[write_size]<<8)|addr[write_size+1];
        write_size+=2;
        FCCOBIX_CCOBIX=0x03;  //写入第二个数据
        FCCOB=(addr[write_size]<<8)|addr[write_size+1];
        write_size+=2;
        FCCOBIX_CCOBIX=0x04;  //写入第三个数据
        FCCOB=(addr[write_size]<<8)|addr[write_size+1];
        write_size+=2;
        FCCOBIX_CCOBIX=0x05;  //写入第四个数据
        FCCOB=(addr[write_size]<<8)|addr[write_size+1];
        write_size+=2;
        FSTAT_CCIF=1;         //写入执行命令
        while(FSTAT_CCIF==0); //等待执行完毕
        start_addr=start_addr+8;
    }
    
    return write_size;

}



uint16_t flash_read (uint8_t* addr,uint16_t size){


    byte   lastepage;          //用于存储EPAGE的值
    byte   epage;              //用于计算EPAGE的值
    uint16_t data;         //读取出的数据
    uint16_t i;
    uint16_t start_addr;
    uint16_t read_size;
    uint16_t rest_size;
    uint16_t read_data;
     
    if(size<=0){
        return 0;   
    }

     start_addr=DEFAULT_OP_FLASH_ADDR;
    
     read_size=0;
     lastepage = EPAGE;   //保存EPAGE的值
     //进行一页一页的读取
    
     while(read_size<size){
          // 一页 1K
        epage = (byte)((DFLASH_LOWEST_START_PAGE)+(start_addr >>10));   //计算EPAGE
        EPAGE=epage; 
        rest_size=size-read_size;
        if(rest_size>DFLASH_PAGE_SIZE){
            rest_size=DFLASH_PAGE_SIZE;
        }
        for(i=0;i<rest_size;i++){
           //读取页面窗口中的数据
           read_data=READword((start_addr & (DFLASH_PAGE_SIZE - 1)) + DFLASH_PAGE_WINDOW_START);
           
           addr[read_size++]=(read_data>>8)&0xff;
           addr[read_size++]=read_data&0xff;
           start_addr+=2;
        }
     }
    
    EPAGE= lastepage;      //恢复EPAGE的值

    return read_size;
}

/*************************************************************/
/*                     擦除DFLASH的一个分区                 */
/*************************************************************/
void DFlash_Erase(word ADDR16){   
  while(FSTAT_CCIF==0);
  if(FSTAT_ACCERR)           
      FSTAT_ACCERR=1;
  if(FSTAT_FPVIOL)           
      FSTAT_FPVIOL=1;
  
  FCCOBIX_CCOBIX=0x00;
  FCCOB=0x1210;           //写入擦除命令和高位地址    高位是0x10
  FCCOBIX_CCOBIX=0x01;   
  FCCOB=ADDR16;           //写入低16位的地址
  FSTAT_CCIF=1;           //启动执行命令
  while(FSTAT_CCIF==0);   //等待执行完成
}



