#ifndef _MODBUS_RTU_H_
#define _MODBUS_RTU_H_

#include "Fr_UNIFIED_types.h"
#include "Fr_UNIFIED.h"

#include "uart.h"  //����ײ㴮�ڵĽӿ�


#define MODBUS_DEVICE_ID 0x01 //�豸��IdΪ0x01

//�ͺŵ�ID
#define MODBUS_MODEL  1002 //�豸��IdΪ�̶� 

#define MODBUS_DEVICE_QUERY 0xfa //�豸��IdΪ0x01


#define S_RX_BUF_SIZE		   128
#define S_TX_BUF_SIZE		   128


/* RTU Ӧ����� */
#define RSP_OK				       0		/* �ɹ� */
#define RSP_ERR_CMD			  0x01	/* ��֧�ֵĹ����� */
#define RSP_ERR_REG_ADDR	0x02	/* �Ĵ�����ַ���� */
#define RSP_ERR_VALUE		  0x03	/* ����ֵ����� */
#define RSP_ERR_WRITE		  0x04	/* д��ʧ�� */




#define MAX_SLOT_BUF_SIZE        32
#define MAX_MB_INDEX_SIZE        32 

#pragma pack(1) // ������ܼӣ�һ�Ӿͳ���

typedef struct {
    uint8_t reserved_id;     //0x0  δ��
	uint8_t device_addr_reg;//0x1  �豸��ַ
	uint8_t model_number_reg;//0x2 ��Ʒ�ͺ�
	
	uint8_t  baudrate_reg;//0x3  //1:2400  2:4800 3:9600 4:19200  5:38400  6:115200  ������
	uint8_t  checkbit_reg;//0x4  // У��λ   1:��У�� 2����У��  3��żУ��
  uint8_t  stopbit_reg; //0x5   // ֹͣλ  1:1λ 2��1.5λ   3��2λ
    
  Fr_low_level_config_type  fr_conf;
  T_slot_conf gt_slot_conf[MAX_SLOT_BUF_SIZE];  //���50��    ������Ϊ-1 ��ʾδʹ��
  
}T_MODBUS_REG;

#pragma pack()

typedef struct
{
	uint8_t rx_buf[S_RX_BUF_SIZE];
	uint8_t rx_count;
	uint8_t rx_status;
	uint8_t rx_new_flag;

	uint8_t rsp_code;

	uint8_t tx_buf[S_TX_BUF_SIZE];
	uint8_t tx_count;
}T_MODBUS;

void modbusInit();
void modbusPoll(void);

uint16_t headcrc_clac(uint8_t sync_bit,uint8_t startup_bit,uint32_t frame_id,uint8_t payload_length);
uint8_t calc_mb_index(T_MODBUS_REG *reg);
#endif
