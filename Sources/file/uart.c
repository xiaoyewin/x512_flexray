#include "uart.h"
#include "derivative.h"      /* derivative-specific definitions */

#include "modbus_rtu.h"
extern uint8_t start_recv_data ;  // 表示可以接收数据
extern uint8_t recv_data_complete; //表示接收数据完成
extern T_MODBUS gt_modbus;
#define  BUS_CLOCK		   32000000	   //总线频率 
#define  BAUD		       9600

uart_recv_data recvdata;
/* USART初始化 */
static  int16_t TIMER_LOAD_VALUE=0;
void SCI_send(unsigned char data) 
{
  while(!SCI0SR1_TDRE);       //等待发送数据寄存器（缓冲器）为空
    SCI0DRL = data;
}


/*************************************************************/
/*                        初始化SCI                          */
/*************************************************************/
void INIT_SCI(void) {

  SCI0BD = BUS_CLOCK/16/BAUD;   //设置SCI0波特率为9600
  SCI0CR1 = 0x00;        //设置SCI0为正常模式，八位数据位，无奇偶校验
  SCI0CR2 = 0x2c;        //允许接收和发送数据，允许接收中断功能 
}

unsigned char data_receive;
unsigned char SCI_receive(void) 
{
  while(!SCI0SR1_RDRF);          //等待发送数据寄存器满
  return(SCI0DRL);
}

/*************************************************************/
/*                     串口中断接收函数                      */
/*************************************************************/
#pragma CODE_SEG __NEAR_SEG NON_BANKED


/*************************************************************/
/*                      计时中断函数                         */
/*************************************************************/
interrupt void PIT_INTER(void)
{
  if(TFLG1_C0F == 1){
    TFLG1_C0F = 1;
    PITCE_PCE0=0;     //PIT第0通道计数器工
    TIE   = 0x00;  
    start_recv_data=1;
    recv_data_complete=1;
  }
  /*
  if(PITTF_PTF0 == 1){
    
    PITTF_PTF0 = 1; //表示一帧数据处理完成
    PITCE_PCE0=0;     //PIT第0通道计数器工
    start_recv_data=1;
    recv_data_complete=1;
    SCI_send(gt_modbus.rx_count);
  }
  
  */
}


interrupt void receivedata(void) {
    
    TIE   = 0x00;  
    data_receive = SCI_receive(); 
    recvdata(data_receive);
    TCNT=0;
    TC0 = TCNT + 30000;         //设置输出比较时间为5ms 
    TIE   = 0x01;  
     
     //如果满了话 就舍弃，最好放一个定时器 ，来进行判断。
}
#pragma CODE_SEG DEFAULT



void send_string(unsigned char *putchar) 
{
  while(*putchar!=0x00)       //判断字符串是否发送完毕
  {
   SCI_send(*putchar++);  
  }
}

void send_data(unsigned char *putchar,int len){
    int i;
    if(len<=0){
        return;   
    }
    
    for(i=0;i<len;i++){
        SCI_send(*putchar++);  
    }
}

//用于判断一帧结束的定时器
void TIM_Init(void){

   PITMUX_PMUX0=0;   //第0通道使用微计数器0
   PITCFLMT=0x80;    //使能周期中断定时器
   TFLG1_C0F = 1;
   TCNT=0; 
   TC0 = TCNT + 30000; //设置输出比较时间为5ms 
   PITCE_PCE0=0;     //PIT第0通道计数器工作 
   PITINTE_PINTE0=1; //PIT0通道定时器定时中断被使能
   TIE   = 0x00;  
}



void uart1_init(uint16_t baud,uint16_t stop_bit,uint16_t check_bit){


    uint32_t temp_baud;
    uint32_t temp_stopbit;
    uint32_t temp_checkbit;
    TIM_Init();
    switch(baud)
    {

        case 1:
            temp_baud=2400;
				TIMER_LOAD_VALUE=0;
            break;
        case 2:
               temp_baud=4800;
				TIMER_LOAD_VALUE=20;
            break;
        case 3:
            temp_baud=9600;
				TIMER_LOAD_VALUE=60;
            break;
        case 4:
            temp_baud=19200;
						TIMER_LOAD_VALUE=80;
            break;
        case 5:
            temp_baud=38400;
						TIMER_LOAD_VALUE=90;
            break;
        case 6:
            temp_baud=115200;
						TIMER_LOAD_VALUE=98;
            break;
        deafult:
            temp_baud=9600;
					  TIMER_LOAD_VALUE=0;
            break;
    }
    
    switch(stop_bit)
    {
        case 1:
            temp_stopbit=1;
            break;
        case 2:
            temp_stopbit=2;
            break;
        case 3:
            temp_stopbit=3;
            break;
        default:
           temp_stopbit=4;
           break;        
    }

    switch(check_bit)
    {
        case 1:
            temp_checkbit=1;
            break;
        case 2:
            temp_checkbit=1;
            break;
        case 3:
            temp_checkbit=2;
            break;
        default:
            temp_checkbit=3;
            break;        
    }
  //  INIT_SCI();
  temp_baud=115200;
  SCI0BD = BUS_CLOCK/16/temp_baud;   //设置SCI0波特率为9600
  SCI0CR1 = 0x00;        //设置SCI0为正常模式，八位数据位，无奇偶校验
  SCI0CR2 = 0x2c;        //允许接收和发送数据，允许接收中断功能 
  
  
}

void registerRecvFunc(uart_recv_data func){
    recvdata=func;
}
