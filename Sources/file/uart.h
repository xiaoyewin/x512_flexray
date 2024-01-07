#ifndef  _UART_H_
#define  _UART_H_

#include "Fr_UNIFIED_types.h"

typedef void (*uart_recv_data)(uint8_t );

//这边需要弄个环形结构体
#define MAX_BUF_SIZE  200   
typedef struct
{
    uint8_t data_buf[MAX_BUF_SIZE];
    uint8_t write_index;
    uint8_t read_index;
    uint8_t occupy;
    uint8_t start_recv_data ;  // 表示可以接收数据
    uint8_t recv_data_complete; //表示接收数据完成
    uint8_t has_occupy; //表示上一帧占用的
}T_DATA_BUFFER;


void uart1_init(uint16_t baud,uint16_t stop_bit,uint16_t check_bit); 
void send_string(unsigned char *putchar); 
void SCI_send(unsigned char data);
void send_data(unsigned char *putchar,int len); 
void data_buf_init(void);

//注册接收的回调函数
void registerRecvFunc(uart_recv_data func);
#endif

