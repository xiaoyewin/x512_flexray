/*---------------------------------------------------------*/
/************************************************************
����Ƽ�MC9S12XF512�������ӿ�����
E-mail: 2008f.d@163.com
�Ա��꣺http://fxfreefly.taobao.com
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
//Ĭ�ϵ�������Ŀ
extern  Fr_low_level_config_type Fr_low_level_cfg_set_00;
extern T_Recv_Data_Buf gt_recv_buf;
extern  Fr_buffer_info_type Fr_buffer_cfg_custom[MAX_MB_INDEX_SIZE]; 

#define  BUS_CLOCK		   32000000	   //����Ƶ��
#define  OSC_CLOCK		   16000000	   //����Ƶ��

#define LEDCPU PORTD_PD0
//#define  OSC_CLOCK	        8000000	   // С���ӵ�ʱ��Ƶ����8MHZ 

/*************************************************************/
/*                      ��ʼ�����໷                         */
/*************************************************************/
void INIT_PLL(void) 
{
    PLLCTL_PLLON = 0;       //��ֹ���໷

    //PLLCLK=2��OSCCLK��(SYNDIV+1)/(REFDIV+1), fbus=PLLCLK/2
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
    //VCO 2 fOSC �� ( ) SYNDIV 1 +
  
    #if(OSC_CLOCK == 16000000)
        REFDV= 0x81;    //REFDIV=1  
    #else //8MHZ  ִ������ĳ���
        REFDV= 0x80;
    #endif 
    
    
    //REFDV= 0x81;    //REFDIV=1        

    PLLCTL_PLLON = 1;   //ʹ�����໷
    while(!CRGFLG_LOCK);       //PLLCLK����
    CLKSEL_PLLSEL = 1;    //����PLLCLKΪϵͳʱ��      
}


/*************************************************************/
/*                      ��ʼ��IPLL                           */
/*                   ΪFLEXRAY�ṩʱ��   80MHZ                     */
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
    CGMCTL_PLLON=1;     //ʹ��IPLL
    while(!CGMFLG_LOCK); //�ȴ�IPLL���� 
}

/************************************************************/
/*                    ��ʼ��ECTģ��                         */
/************************************************************/
void initialize_ect(void){
  TSCR1_TFFCA = 1;  // ��ʱ����־λ�������
  TSCR1_TEN = 1;    // ��ʱ��ʹ��λ. 1=����ʱ����������; 0=ʹ����ʱ����������(����������)
  TIOS  = 0xff;      //ָ������ͨ��Ϊ����ȽϷ�ʽ
  TCTL1 = 0x00;	    // ���ĸ�ͨ������Ϊ��ʱ����������ŶϿ�
  TCTL2 = 0x00;     // ǰ�ĸ�ͨ������Ϊ��ʱ����������ŶϿ�
  DLYCT = 0x00;	    // �ӳٿ��ƹ��ܽ�ֹ
  ICOVW = 0x00;	    // ��Ӧ�ļĴ�����������;  NOVWx = 1, ��Ӧ�ļĴ�����������
  ICSYS = 0x00;	    // ��ֹIC��PAC�ı��ּĴ���
  TIE   = 0x00;     // ��ֹ����ͨ����ʱ�ж�
  TSCR2 = 0x07;	    // Ԥ��Ƶϵ��pr2-pr0:111,,ʱ������Ϊ4us,
  TFLG1 = 0xff;	    // �����IC/OC�жϱ�־λ
  TFLG2 = 0xff;     // ������ɶ�ʱ���жϱ�־λ
}

/*************************************************************/
/*                      ��ʼ������                           */
/*************************************************************/
void init_Peripheral(void)
{  
    COPCTL = 0x00;     //�رտ��Ź�
    MMCCTL1 = 1;       //ʹ��FLASH                  
    DIRECT = 0;        //ֱ��ҳ�ĵ�ַΪĬ��ֵ            
    IVBR = 0xFF;       //�ж�������ַΪĬ��ֵ        

    //TJA1080��ʹ�ܶ�            
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
/*                    FLEXRAY�жϺ���                        */
/*************************************************************/
#pragma CODE_SEG __NEAR_SEG NON_BANKED
void interrupt FLEXRAY_ISR_TX(void)
{   
    //�����жϴ������
    Fr_interrupt_handler(); 
}

void interrupt FLEXRAY_ISR_RX(void)
{          
    //�����жϴ������
    Fr_interrupt_handler();  
}

void interrupt FLEXRAY_ISR_FIFO_A(void)
{                  
    //�����жϴ������
    Fr_interrupt_handler();   
}

void interrupt FLEXRAY_ISR_PROTOCOL(void)
{      
    //�����жϴ������
    Fr_interrupt_handler(); 
}

#pragma CODE_SEG DEFAULT


uint8_t set_default_parm(T_MODBUS_REG *reg){

    uint16_t i;
    gt_recv_buf.cur_used=0;
    reg->device_addr_reg=0x5a;
    reg->fr_conf=Fr_low_level_cfg_set_00;
    //Ϊ�˳������ֲ����ö���װ
     reg->baudrate_reg=5;
     reg->stopbit_reg=1;
     reg->checkbit_reg=1;

    //��ȫ����ֵΪ-1 ��ʾδʹ��
   	for(i=0;i<MAX_SLOT_BUF_SIZE;i++){
	    reg->gt_slot_conf[i].frame_ID=0; //Ϊ0��ʾδʹ��	        
	}
    
 
	reg->gt_slot_conf[0].frame_ID=69;
    reg->gt_slot_conf[0].FrameType=0;	
    reg->gt_slot_conf[0].buffer_type=FR_RECEIVE_BUFFER;
    reg->gt_slot_conf[0].receive_channel=FR_CHANNEL_A;
    reg->gt_slot_conf[0].base_circle=0;
    reg->gt_slot_conf[0].base_circle_interval=0;
    reg->gt_slot_conf[0].payload_length=14;
    reg->gt_slot_conf[0].transmission_mode=FR_EVENT_TRANSMISSION_MODE;   //�жϱ�ʾ�¼�
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
    
    //����Ĭ�ϲ�������
    
    
    // Header CRC   ����ͨ��
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
          //��Ҫ���´������¼��㡣
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
       // ��ʾ��ȡ�ɹ�
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



//���ú�ĳ�����ʱ����Ҫ�������� 

/*************************************************************/
/*                          ������            
��Ҫ����������,С���ӺͿ����徧��̫һ��    */
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
    //��������������ݣ���Ч�����������ó�Ĭ�ϲ��� 
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
  

  //��������flash �Ķ�ȡ
  initialize_ect();
  init_Peripheral(); 
  DDRD_DDRD0=1;
  printf("modbusInit\r\n");  
  modbusInit();

  //delay1ms(500); //�ӳ�
  printf("delay1ms\r\n");
  
  vfnFlexRay_Init();      //��ʼ��FLEXRAY
  
  printf("vfnFlexRay_Init\r\n");
  
  //TIM_Init();
    
  COPCTL = 0x07;     //���ÿ��Ź���λ���Ϊ1.048576s 	

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
   // vfnFlexRay_Handler(); // �������ѯ�ķ�ʽ
    //���ﱣ�����ݣ�Ȼ��λ   ������ӳ�
    if(gt_modbus_reg.reserved_id==0x12)
    { 
         if(reset_num==200)
         {
             gt_modbus_reg.reserved_id=0x55;
             write_to_flash(&gt_modbus_reg);
             printf("write_to_flash config\r\n");
             delay1ms(3000); //֮��λ
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
        _FEED_COP();  //ι������ 
        reset_num=0;  
    }
  } 
}
