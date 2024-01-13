/*---------------------------------------------------------*/
/************************************************************
飞翔科技MC9S12XF512汽车电子开发板
E-mail: 2008f.d@163.com
淘宝店：http://fxfreefly.taobao.com
************************************************************/
/*---------------------------------------------------------*/
#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

#include "LCD.h"
#include "FlexRay_handler.h"

#include "FlexRay_handler.h"
#include "modbus_rtu.h"
#include "dflash.h"
#include "Fr_UNIFIED_types.h" 
#include "stdio.h"

extern T_DATA_BUFFER  g_recv_data_buf ;


extern T_MODBUS_REG gt_modbus_reg;
extern T_MB_Index mb_index[MAX_MB_INDEX_SIZE]; 
//默认的配置项目
extern  Fr_low_level_config_type Fr_low_level_cfg_set_00;
extern T_Recv_Data_Buf gt_recv_buf;
extern  Fr_buffer_info_type Fr_buffer_cfg_custom[MAX_MB_INDEX_SIZE]; 

#define  BUS_CLOCK		   32000000	   //总线频率
#define  OSC_CLOCK		   16000000	   //晶振频率

#define LEDCPU PORTD_PD0
//#define  OSC_CLOCK	        8000000	   // 小板子的时钟频率是8MHZ 

/*************************************************************/
/*                      初始化锁相环                         */
/*************************************************************/
void INIT_PLL(void) 
{
    PLLCTL_PLLON = 0;       //禁止锁相环

    //PLLCLK=2×OSCCLK×(SYNDIV+1)/(REFDIV+1), fbus=PLLCLK/2
    #if(BUS_CLOCK == 120000000) 
        SYNR = 0xcd;
    #elif(BUS_CLOCK == 104000000) 
      SYNR = 0xcc;
    #elif(BUS_CLOCK == 96000000) 
      SYNR = 0xcb;
    #elif(BUS_CLOCK == 88000000) 
      SYNR = 0xca;
    #elif(BUS_CLOCK == 80000000) 
      SYNR = 0xc9;
    #elif(BUS_CLOCK == 72000000) 
      SYNR = 0xc8;
    #elif(BUS_CLOCK == 64000000) 
      SYNR = 0xc7;
    #elif(BUS_CLOCK == 56000000) 
      SYNR = 0xc6;
    #elif(BUS_CLOCK == 48000000) 
      SYNR = 0xc5;
    #elif(BUS_CLOCK == 40000000) 
      SYNR = 0x44;
    #elif(BUS_CLOCK == 32000000)
      SYNR = 0x43;   
    #elif(BUS_CLOCK == 24000000)
      SYNR = 0x42;
    #elif(BUS_CLOCK == 16000000)
      SYNR = 0x01;
    #endif 
    
    
   
    
    
    // VCOFRQ[1:0]   SYNDIV[5:0]     2* OSC* (SYNDIV+1)/(REFDIV+1)
    //VCO 2 fOSC × ( ) SYNDIV 1 +
  
    #if(OSC_CLOCK == 16000000)
        REFDV= 0x81;    //REFDIV=1  
    #else //8MHZ  执行下面的程序
        REFDV= 0x80;
    #endif 
    
    
    //REFDV= 0x81;    //REFDIV=1        

    PLLCTL_PLLON = 1;   //使能锁相环
    while(!CRGFLG_LOCK);       //PLLCLK锁定
    CLKSEL_PLLSEL = 1;    //设置PLLCLK为系统时钟      
}


/*************************************************************/
/*                      初始化IPLL                           */
/*                   为FLEXRAY提供时钟   80MHZ                     */
/*************************************************************/
void init_IPLL(void)
{
    //Fvco = 2*OSCCLK*(SYNR+1)/(REFDV+1) = 32MHz*(4+1)/(3+1) = 40MHz/
    
    


    //SYNDIV[5:0]
    CGMSYNR = 0x49;  // 04   4   
    
#if(OSC_CLOCK == 16000000)
    CGMREFDV = 0x43; //REFDIV  
#else
    CGMREFDV = 0x41;    
#endif    


 
    //CGMREFDV = 0x43;
    
    CGMCTL_DIV2 = 0;    //IPLL = Fvco 
    CGMCTL_PLLON=1;     //使能IPLL
    while(!CGMFLG_LOCK); //等待IPLL锁定 
}

/************************************************************/
/*                    初始化ECT模块                         */
/************************************************************/
void initialize_ect(void){
  TSCR1_TFFCA = 1;  // 定时器标志位快速清除
  TSCR1_TEN = 1;    // 定时器使能位. 1=允许定时器正常工作; 0=使主定时器不起作用(包括计数器)
  TIOS  = 0xff;      //指定所有通道为输出比较方式
  TCTL1 = 0x00;	    // 后四个通道设置为定时器与输出引脚断开
  TCTL2 = 0x00;     // 前四个通道设置为定时器与输出引脚断开
  DLYCT = 0x00;	    // 延迟控制功能禁止
  ICOVW = 0x00;	    // 对应的寄存器允许被覆盖;  NOVWx = 1, 对应的寄存器不允许覆盖
  ICSYS = 0x00;	    // 禁止IC及PAC的保持寄存器
  TIE   = 0x00;     // 禁止所有通道定时中断
  TSCR2 = 0x07;	    // 预分频系数pr2-pr0:111,,时钟周期为4us,
  TFLG1 = 0xff;	    // 清除各IC/OC中断标志位
  TFLG2 = 0xff;     // 清除自由定时器中断标志位
}

/*************************************************************/
/*                      初始化外设                           */
/*************************************************************/
void init_Peripheral(void)
{  
    COPCTL = 0x00;     //关闭看门狗
    MMCCTL1 = 1;       //使能FLASH                  
    DIRECT = 0;        //直接页的地址为默认值            
    IVBR = 0xFF;       //中断向量地址为默认值        

    //TJA1080的使能端            
    DDRH_DDRH3 = 1;    
    DDRH_DDRH7 = 1;    
    RDRH = 0xFF;             
    PERH = 0;        
    PTH = 0x88; 
       
    
#if(OSC_CLOCK == 8000000)
    DDRJ_DDRJ3 = 1;  
    DDRJ_DDRJ4 = 1;    
    PTJ_PTJ3 = 1;  
    PTJ_PTJ4 = 1; 
#endif    



                                                  
}

/*************************************************************/
/*                    FLEXRAY中断函数                        */
/*************************************************************/
#pragma CODE_SEG __NEAR_SEG NON_BANKED
void interrupt FLEXRAY_ISR_TX(void)
{   
    //调用中断处理程序
    Fr_interrupt_handler(); 
}

void interrupt FLEXRAY_ISR_RX(void)
{          
    //调用中断处理程序
    Fr_interrupt_handler();  
}

void interrupt FLEXRAY_ISR_FIFO_A(void)
{                  
    //调用中断处理程序
    Fr_interrupt_handler();   
}

void interrupt FLEXRAY_ISR_PROTOCOL(void)
{      
    //调用中断处理程序
    Fr_interrupt_handler(); 
}

#pragma CODE_SEG DEFAULT


uint8_t set_default_parm(T_MODBUS_REG *reg){

    uint16_t i;
    gt_recv_buf.cur_used=0;
    reg->device_addr_reg=0x5a;
    reg->fr_conf=Fr_low_level_cfg_set_00;
    //为了程序的移植，最好多层封装
     reg->baudrate_reg=5;
     reg->stopbit_reg=1;
     reg->checkbit_reg=1;

    //先全部赋值为-1 表示未使用
   	for(i=0;i<MAX_SLOT_BUF_SIZE;i++){
	    reg->gt_slot_conf[i].frame_ID=0; //为0表示未使用	        
	}
    
 
	reg->gt_slot_conf[0].frame_ID=69;
    reg->gt_slot_conf[0].FrameType=0;	
    reg->gt_slot_conf[0].buffer_type=FR_RECEIVE_BUFFER;
    reg->gt_slot_conf[0].receive_channel=FR_CHANNEL_A;
    reg->gt_slot_conf[0].base_circle=0;
    reg->gt_slot_conf[0].base_circle_interval=0;
    reg->gt_slot_conf[0].payload_length=14;
    reg->gt_slot_conf[0].transmission_mode=FR_EVENT_TRANSMISSION_MODE;   //中断表示事件
    reg->gt_slot_conf[0].transmission_commit_mode=FR_STREAMING_COMMIT_MODE;  
    reg->gt_slot_conf[0].transmit_channel=FR_CHANNEL_AB;
    reg->gt_slot_conf[0].transmit_type=FR_SINGLE_TRANSMIT_BUFFER;
    
    reg->gt_slot_conf[0].data=&gt_recv_buf.data[gt_recv_buf.cur_used];
    gt_recv_buf.cur_used+=reg->gt_slot_conf[0].payload_length;


	reg->gt_slot_conf[1].frame_ID=70;
    reg->gt_slot_conf[1].FrameType=0;	
    reg->gt_slot_conf[1].buffer_type=FR_RECEIVE_BUFFER;
    reg->gt_slot_conf[1].receive_channel=FR_CHANNEL_A;
    reg->gt_slot_conf[1].base_circle=0;
    reg->gt_slot_conf[1].base_circle_interval=0;
    reg->gt_slot_conf[1].payload_length=14;
    reg->gt_slot_conf[1].transmission_mode=FR_EVENT_TRANSMISSION_MODE;
    reg->gt_slot_conf[1].transmission_commit_mode=FR_STREAMING_COMMIT_MODE;
    reg->gt_slot_conf[1].transmit_channel=FR_CHANNEL_AB;
    reg->gt_slot_conf[1].transmit_type=FR_SINGLE_TRANSMIT_BUFFER;
    
    reg->gt_slot_conf[1].data=&gt_recv_buf.data[gt_recv_buf.cur_used];
    gt_recv_buf.cur_used+=reg->gt_slot_conf[1].payload_length;
    
    //进行默认参数配置
    
    
    // Header CRC   可以通过
    reg->gt_slot_conf[2].frame_ID=1;
    reg->gt_slot_conf[2].FrameType=0;	
    reg->gt_slot_conf[2].buffer_type=FR_TRANSMIT_BUFFER;
    reg->gt_slot_conf[2].receive_channel=FR_NO_CHANNEL;
    reg->gt_slot_conf[2].base_circle=0;
    reg->gt_slot_conf[2].base_circle_interval=0;
    reg->gt_slot_conf[2].payload_length=14;
    reg->gt_slot_conf[2].transmission_mode=FR_STATE_TRANSMISSION_MODE;
    reg->gt_slot_conf[2].transmission_commit_mode=FR_STREAMING_COMMIT_MODE;
    reg->gt_slot_conf[2].transmit_channel=FR_CHANNEL_AB;
    
    reg->gt_slot_conf[2].transmit_type=FR_DOUBLE_TRANSMIT_BUFFER;	
    reg->gt_slot_conf[2].header_CRC=headcrc_clac(1,1,reg->gt_slot_conf[2].frame_ID,reg->gt_slot_conf[2].payload_length);
    reg->gt_slot_conf[2].data=&gt_recv_buf.data[gt_recv_buf.cur_used];
    gt_recv_buf.cur_used+=reg->gt_slot_conf[2].payload_length;
 
 
    if(gt_recv_buf.cur_used>=MAX_DATA_BUF)
    {
          //需要重新打乱重新计算。
    }

    if(calc_mb_index(reg))
    {
        
    }
             
}



uint8_t read_from_flash(T_MODBUS_REG *reg){
   uint16_t size;
   uint16_t ReadSize;
   uint8_t * addr;
   size= sizeof(T_MODBUS_REG);
   addr= (uint8_t *)reg;
   
   
   
   ReadSize=flash_read(addr, size);
   printf("Read Flash Size %d =%d\r\n",ReadSize,size);
   if(ReadSize<size)
   {
        return 0;
   } 
   else
   {
      printf("reg->reserved_id=%X\r\n",reg->reserved_id);
       // 表示读取成功
      if(reg->reserved_id==0x55)
      {
          return 1;
      } 
      else
      {
         return 0;
      }
   } 
}

uint8_t write_to_flash(T_MODBUS_REG *reg){
   uint16_t size;
   uint8_t * addr;
   size= sizeof(T_MODBUS_REG);
   addr= (uint8_t *)reg;
   DFlash_Erase(0);    
   DFlash_Erase(256);
   DFlash_Erase(512); 
   DFlash_Erase(800); 
   DFlash_Erase(1024);  
   DFlash_Erase(1300); 
   reg->reserved_id=0x55;
   if(flash_write(addr, size)<size){
        return 0;
   } else{
      return 1;
   } 
}



//配置好某项参数时，需要重新启动 

/*************************************************************/
/*                          主函数            
需要做条件编译,小板子和开发板晶振不太一样    */
/*************************************************************/
void main(void) 
{
   uint8_t  buf[1024];
   uint16_t i;
   uint8_t LEDFlag=0;
   
  uint16_t reset_num=0;
  DisableInterrupts;
  INIT_PLL();
  init_IPLL();
  
  DFlash_Init();
  uart2_init();
  printf("System Start\r\n");
  //FlashTest(); 
  
  if(!read_from_flash(&gt_modbus_reg))
  {
     printf("Flash Error\r\n");
    //如果读出来的数据，无效，就重新配置成默认参数 
      set_default_parm(&gt_modbus_reg);
  } 
  else 
  {
    calc_mb_index(&gt_modbus_reg);
  }
  
  /*
    headcrc_clac(0,0,reg->gt_slot_conf[2].frame_ID,reg->gt_slot_conf[2].payload_length);

    reg->gt_slot_conf[3].frame_ID=20;
    reg->gt_slot_conf[3].FrameType=0;	
    reg->gt_slot_conf[3].buffer_type=FR_TRANSMIT_BUFFER;
    reg->gt_slot_conf[3].receive_channel=FR_NO_CHANNEL;
    reg->gt_slot_conf[3].base_circle=0;
    reg->gt_slot_conf[3].base_circle_interval=0;
    reg->gt_slot_conf[3].payload_length=14;
    reg->gt_slot_conf[3].transmission_mode=FR_STATE_TRANSMISSION_MODE;
    reg->gt_slot_conf[3].transmission_commit_mode=FR_STREAMING_COMMIT_MODE;
    reg->gt_slot_conf[3].transmit_channel=FR_CHANNEL_AB;
    
    reg->gt_slot_conf[3].transmit_type=FR_DOUBLE_TRANSMIT_BUFFER;	
    reg->gt_slot_conf[3].header_CRC=headcrc_clac(0,0,reg->gt_slot_conf[3].frame_ID,reg->gt_slot_conf[3].payload_length);
    reg->gt_slot_conf[3].data=&gt_recv_buf.data[gt_recv_buf.cur_used];
    gt_recv_buf.cur_used+=reg->gt_slot_conf[3].payload_length;
    */
  

  //在这里做flash 的读取
  initialize_ect();
  init_Peripheral(); 
  DDRD_DDRD0=1;
  printf("modbusInit\r\n");  
  modbusInit();

  //delay1ms(500); //延迟
  printf("delay1ms\r\n");
  
  vfnFlexRay_Init();      //初始化FLEXRAY
  
  printf("vfnFlexRay_Init\r\n");
  
  //TIM_Init();
    
  COPCTL = 0x07;     //设置看门狗复位间隔为1.048576s 	

  EnableInterrupts;
  
  
  for(;;) 
  {
    uint8_t Lop=0;
    if(g_SecFlag==1) 
    {
        g_SecFlag=0;
        LEDCPU=LEDFlag;
        LEDFlag=1-LEDFlag;
        for(Lop=0;Lop<5;Lop++) 
        {
/*          if(gt_modbus_reg.gt_slot_conf[Lop].Circle&0X80) 
          {
              printf("Lop=%d Slot ID=%d,Circle:%d\r\n",
              Lop,gt_modbus_reg.gt_slot_conf[Lop].frame_ID,
              gt_modbus_reg.gt_slot_conf[Lop].Circle&0X7F);
              
              gt_modbus_reg.gt_slot_conf[Lop].Circle=0;
          }*/
        }
    }
    modbusPoll();
   // vfnFlexRay_Handler(); // 这个是轮询的方式
    //这里保存数据，然后复位   最好有延迟
    if(gt_modbus_reg.reserved_id==0x12)
    { 
         if(reset_num==200)
         {
             gt_modbus_reg.reserved_id=0x55;
             write_to_flash(&gt_modbus_reg);
             printf("write_to_flash config\r\n");
             delay1ms(3000); //之后复位
             reset_num=0;
         } 
         else if(reset_num==100)
         {
           
             DisableInterrupts;
             calc_mb_index(&gt_modbus_reg);
             vfnFlexRay_Init();
              EnableInterrupts;
             reset_num++; 
             _FEED_COP();
         }
         else
         {
             _FEED_COP();
              reset_num++;
         }
    } 
    else
    {
        _FEED_COP();  //喂狗函数 
        reset_num=0;  
    }
  } 
}
