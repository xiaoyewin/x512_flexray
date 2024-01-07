#ifndef  _UART_H_
#define  _UART_H_

#include "Fr_UNIFIED_types.h"

typedef void (*uart_recv_data)(uint8_t );

//�����ҪŪ�����νṹ��
#define MAX_BUF_SIZE  200   
typedef struct
{
    uint8_t data_buf[MAX_BUF_SIZE];
    uint8_t write_index;
    uint8_t read_index;
    uint8_t occupy;
    uint8_t start_recv_data ;  // ��ʾ���Խ�������
    uint8_t recv_data_complete; //��ʾ�����������
    uint8_t has_occupy; //��ʾ��һ֡ռ�õ�
}T_DATA_BUFFER;


void uart1_init(uint16_t baud,uint16_t stop_bit,uint16_t check_bit); 
void send_string(unsigned char *putchar); 
void SCI_send(unsigned char data);
void send_data(unsigned char *putchar,int len); 
void data_buf_init(void);

//ע����յĻص�����
void registerRecvFunc(uart_recv_data func);
#endif

