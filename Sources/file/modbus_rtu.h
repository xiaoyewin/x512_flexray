#ifndef _MODBUS_RTU_H_
#define _MODBUS_RTU_H_

#include "Fr_UNIFIED_types.h"
#include "Fr_UNIFIED.h"

#include "uart.h"  //这个底层串口的接口


#define MODBUS_DEVICE_ID 0x01 //设备的Id为0x01

//型号的ID
#define MODBUS_MODEL  1002 //设备的Id为固定 

#define MODBUS_DEVICE_QUERY 0xfa //设备的Id为0x01


#define S_RX_BUF_SIZE		   128
#define S_TX_BUF_SIZE		   128


/* RTU 应答代码 */
#define RSP_OK				       0		/* 成功 */
#define RSP_ERR_CMD			  0x01	/* 不支持的功能码 */
#define RSP_ERR_REG_ADDR	0x02	/* 寄存器地址错误 */
#define RSP_ERR_VALUE		  0x03	/* 数据值域错误 */
#define RSP_ERR_WRITE		  0x04	/* 写入失败 */




#define MAX_SLOT_BUF_SIZE        32
#define MAX_MB_INDEX_SIZE        32 

#pragma pack(1) // 这个不能加，一加就出错

typedef struct {
    uint8_t reserved_id;     //0x0  未用
	uint8_t device_addr_reg;//0x1  设备地址
	uint8_t model_number_reg;//0x2 产品型号
	
	uint8_t  baudrate_reg;//0x3  //1:2400  2:4800 3:9600 4:19200  5:38400  6:115200  波特率
	uint8_t  checkbit_reg;//0x4  // 校验位   1:无校验 2：奇校验  3：偶校验
  uint8_t  stopbit_reg; //0x5   // 停止位  1:1位 2：1.5位   3：2位
    
  Fr_low_level_config_type  fr_conf;
  T_slot_conf gt_slot_conf[MAX_SLOT_BUF_SIZE];  //最大50个    如果序号为-1 表示未使用
  
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
