#include "uart.h"
#include "derivative.h"      /* derivative-specific definitions */

#include "modbus_rtu.h"
extern uint8_t start_recv_data ;  // ��ʾ���Խ�������
extern uint8_t recv_data_complete; //��ʾ�����������
extern T_MODBUS gt_modbus;
#define  BUS_CLOCK		   32000000	   //����Ƶ�� 
#define  BAUD		       9600

uart_recv_data recvdata;
/* USART��ʼ�� */
static  int16_t TIMER_LOAD_VALUE=0;
void SCI_send(unsigned char data) 
{
  while(!SCI0SR1_TDRE);       //�ȴ��������ݼĴ�������������Ϊ��
    SCI0DRL = data;
}


/*************************************************************/
/*                        ��ʼ��SCI                          */
/*************************************************************/
void INIT_SCI(void) {

  SCI0BD = BUS_CLOCK/16/BAUD;   //����SCI0������Ϊ9600
  SCI0CR1 = 0x00;        //����SCI0Ϊ����ģʽ����λ����λ������żУ��
  SCI0CR2 = 0x2c;        //������պͷ������ݣ���������жϹ��� 
}

unsigned char data_receive;
unsigned char SCI_receive(void) 
{
  while(!SCI0SR1_RDRF);          //�ȴ��������ݼĴ�����
  return(SCI0DRL);
}

/*************************************************************/
/*                     �����жϽ��պ���                      */
/*************************************************************/
#pragma CODE_SEG __NEAR_SEG NON_BANKED


/*************************************************************/
/*                      ��ʱ�жϺ���                         */
/*************************************************************/
interrupt void PIT_INTER(void)
{
  if(TFLG1_C0F == 1){
    TFLG1_C0F = 1;
    PITCE_PCE0=0;     //PIT��0ͨ����������
    TIE   = 0x00;  
    start_recv_data=1;
    recv_data_complete=1;
  }
  /*
  if(PITTF_PTF0 == 1){
    
    PITTF_PTF0 = 1; //��ʾһ֡���ݴ������
    PITCE_PCE0=0;     //PIT��0ͨ����������
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
    TC0 = TCNT + 30000;         //��������Ƚ�ʱ��Ϊ5ms 
    TIE   = 0x01;  
     
     //������˻� ����������÷�һ����ʱ�� ���������жϡ�
}
#pragma CODE_SEG DEFAULT



void send_string(unsigned char *putchar) 
{
  while(*putchar!=0x00)       //�ж��ַ����Ƿ������
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

//�����ж�һ֡�����Ķ�ʱ��
void TIM_Init(void){

   PITMUX_PMUX0=0;   //��0ͨ��ʹ��΢������0
   PITCFLMT=0x80;    //ʹ�������ж϶�ʱ��
   TFLG1_C0F = 1;
   TCNT=0; 
   TC0 = TCNT + 30000; //��������Ƚ�ʱ��Ϊ5ms 
   PITCE_PCE0=0;     //PIT��0ͨ������������ 
   PITINTE_PINTE0=1; //PIT0ͨ����ʱ����ʱ�жϱ�ʹ��
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
  SCI0BD = BUS_CLOCK/16/temp_baud;   //����SCI0������Ϊ9600
  SCI0CR1 = 0x00;        //����SCI0Ϊ����ģʽ����λ����λ������żУ��
  SCI0CR2 = 0x2c;        //������պͷ������ݣ���������жϹ��� 
  
  
}

void registerRecvFunc(uart_recv_data func){
    recvdata=func;
}
