#include "modbus_rtu.h"
#include "Fr_UNIFIED_cfg.h"
extern  Fr_low_level_config_type Fr_low_level_cfg_set_00;
T_Recv_Data_Buf gt_recv_buf;

/*
*********************************************************************************************************
*	函 数 名: MODS_Poll
*	功能说明: 解析数据包. 在主程序中轮流调用。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/

//如果处于接收的状态中，是不能发送数据的

uint16_t need_update_data ;  //需要更新哪些数据

uint8_t need_save_reg_data ;  //需要保存数据


uint8_t start_recv_data ;  // 表示可以接收数据
uint8_t recv_data_complete; //表示接收数据完成


T_MODBUS gt_modbus;
T_MODBUS_REG gt_modbus_reg;


T_MB_Index mb_index[MAX_MB_INDEX_SIZE];  //最多32个配置
 
uint16_t  g_device_id=MODBUS_DEVICE_ID;


/*
这个需要放到中断函数中 
*/
void modbusReciveData(uint8_t byte){
	/*
		3.5个字符的时间间隔，只是用在RTU模式下面，因为RTU模式没有开始符和结束符，
		两个数据包之间只能靠时间间隔来区分，Modbus定义在不同的波特率下，间隔时间是不一样的，
		所以就是3.5个字符的时间，波特率高，这个时间间隔就小，波特率低，这个时间间隔相应就大

		4800  = 7.297ms
		9600  = 3.646ms
		19200  = 1.771ms
		38400  = 0.885ms
	*/
    if(start_recv_data>0){
        //表示已经超时,需要判断第一个字符
        if((gt_modbus_reg.device_addr_reg & 0xff)==byte){  //那就说明数据有效
            start_recv_data=0;//清除掉超时标记
            gt_modbus.rx_count=0; //开始接收数据
            recv_data_complete=0;//数据接收开始
            gt_modbus.rx_buf[gt_modbus.rx_count++] = byte;
        }
    }
    else if(recv_data_complete==0){
        if (gt_modbus.rx_count < S_RX_BUF_SIZE){
            gt_modbus.rx_buf[gt_modbus.rx_count++] = byte;
        }
    }
	
    //在这里需要重新设置定时器
}




uint16_t swap_u16(uint16_t v){
    uint16_t r=((v&0xff)<<8)|((v>>8)&0xff);
    return r;
}

uint32_t swap_u32(uint32_t v){
    uint32_t r=((v&0xff)<<24)  \
        |(((v>>8)&0xff)<<16) \
        |(((v>>16)&0xff)<<8) \
        |((v>>24)&0xff);

    return r;
}

uint16_t swap_u16_from_u8(uint8_t v1,uint8_t v2){
    uint16_t r=(v2<<8)|v1;
    return r;
}

uint32_t swap_u32_from_u8(uint8_t v1 , uint8_t v2 ,uint8_t v3,uint8_t v4){
    uint32_t r= (((uint32_t)v4<<24)|((uint32_t)v3<<16)|((uint32_t)v2<<8)|v1); 
    return r;
}

void  swap_u32_to_u8(uint32_t v,uint8_t *buf){
    buf[3]=(v>>24)&0xff;
    buf[2]=(v>>16)&0xff;
    buf[1]=(v>>8)&0xff;
    buf[0]= v&0xff;
}

void  swap_u16_to_u8(uint16_t v,uint8_t *buf){
    buf[1]=(v>>8)&0xff;
    buf[0]= v&0xff;
}


// CRC 高位字节值表
static const uint8_t s_CRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
// CRC 低位字节值表
const uint8_t s_CRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
	0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
	0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
	0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
	0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
	0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
	0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
	0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
	0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
	0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
	0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
	0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
	0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
	0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};
/*
*********************************************************************************************************
*	函 数 名: CRC16_Modbus
*	功能说明: 计算CRC。 用于Modbus协议。
*	形    参: _pBuf : 参与校验的数据
*			  _usLen : 数据长度
*	返 回 值: 16位整数值。 对于Modbus ，此结果高字节先传送，低字节后传送。
*
*   所有可能的CRC值都被预装在两个数组当中，当计算报文内容时可以简单的索引即可；
*   一个数组包含有16位CRC域的所有256个可能的高位字节，另一个数组含有低位字节的值；
*   这种索引访问CRC的方式提供了比对报文缓冲区的每一个新字符都计算新的CRC更快的方法；
*
*  注意：此程序内部执行高/低CRC字节的交换。此函数返回的是已经经过交换的CRC值；也就是说，该函数的返回值可以直接放置
*        于报文用于发送；
*********************************************************************************************************
*/
uint16_t CRC16_Modbus(uint8_t *_pBuf, uint16_t _usLen)
{
	uint8_t ucCRCHi = 0xFF; /* 高CRC字节初始化 */
	uint8_t ucCRCLo = 0xFF; /* 低CRC 字节初始化 */
	uint16_t usIndex;  /* CRC循环中的索引 */

  while (_usLen--)
  {
      usIndex = ucCRCHi ^ *_pBuf++; /* 计算CRC */
      ucCRCHi = ucCRCLo ^ s_CRCHi[usIndex];
      ucCRCLo = s_CRCLo[usIndex];
  }
  return ((uint16_t)ucCRCHi << 8 | ucCRCLo);
}

/*
*********************************************************************************************************
*	函 数 名: MODS_ReadRegValue
*	功能说明: 读取保持寄存器的值
*	形    参: reg_addr 寄存器地址
*			  reg_value 存放寄存器结果
*	返 回 值: 1表示OK 0表示错误
*********************************************************************************************************
*/
static uint8_t modbusReadRegValue(uint16_t reg_addr, uint8_t *reg_value)
{
	uint16_t value;
    
    if(reg_addr<=0x72){
 
 /*       
        if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->temperature_reg)){
            //这个表示需要读取温度,那么需要采集一下
            need_update_data|=UPDATE_TEMP_DATA;
        }
        
        if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->humidity_reg)){
            //这个表示需要读取温度,那么需要采集一下
            need_update_data|=UPDATE_HUMIDITY_DATA;
        }
        
        if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->gassPressure_reg)){
               need_update_data|=UPDATE_GASSPRESSURE_DATA;
        }
        
        if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->sunLight_reg)){
            need_update_data|=UPDATE_SUNLIGHT_DATA;
        }
            
       */
        uint16_t * temp_reg  =(uint16_t *)&gt_modbus_reg;
        //value = gt_modbus_reg.temperature_reg1;
        value = temp_reg[reg_addr];

        reg_value[0] = value >> 8;
        reg_value[1] = value;
        return 1;
    }
    else{
       return 0;
    }									/* 读取成功 */
}

/*
*********************************************************************************************************
*	函 数 名: modbusWriteRegValue
*	功能说明: 读取保持寄存器的值
*	形    参: reg_addr 寄存器地址
*			  reg_value 寄存器值
*	返 回 值: 1表示OK 0表示错误
*********************************************************************************************************
*/
static uint8_t modbusWriteRegValue(uint16_t reg_addr, int16_t reg_value)
{
    //不可修改
    /*
    if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->model_number_reg)){
        return 1;
    }
    if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->baudrate_reg)){
        if((reg_value>6)||(reg_value<1)){
            return 1;
        }
    }
    if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->checkbit_reg)){
        if((reg_value>3)||(reg_value<1)){
            return 1;
        }
    }
    
    if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->stopbit_reg)){
        if((reg_value>3)||(reg_value<1)){
            return 1;
        }
    }
    
    if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->temperature_correct_reg)){
        if((reg_value>1000)||(reg_value<-10000)){
            return 1;
        }
    }
    
     if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->humidity_correct_reg)){
        if((reg_value>1000)||(reg_value<-10000)){
            return 1;
        }
    }
     
    if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->gassPressure_correct_reg)){
        if((reg_value>1000)||(reg_value<-10000)){
            return 1;
        }
    }
    
   if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->sunLight_correct_reg)){
        if((reg_value>2000)||(reg_value<0)){
            return 1;
        }
    }
    */    
    if(reg_addr<=0x72){
        int16_t * temp_reg  =(int16_t *)&gt_modbus_reg;
        temp_reg[reg_addr]=reg_value;
        return 1;
    }
    else{  //如果大于0xff00 
       
       return 0;
    }
 
}




/*
*********************************************************************************************************
*	函 数 名: BEBufToUint16
*	功能说明: 将2字节数组(大端Big Endian次序，高字节在前)转换为16位整数
*	形    参: _pBuf : 数组
*	返 回 值: 16位整数值
*
*   大端(Big Endian)与小端(Little Endian)
*********************************************************************************************************
*/
uint16_t BEBufToUint16(uint8_t *_pBuf)
{
    return (((uint16_t)_pBuf[0] << 8) | _pBuf[1]);
}



/*
*********************************************************************************************************
*	函 数 名: MODS_SendWithCRC
*	功能说明: 发送一串数据, 自动追加2字节CRC
*	形    参: _pBuf 数据；
*			  _ucLen 数据长度（不带CRC）
*	返 回 值: 无
*********************************************************************************************************
*/
static void modbusSendWithCRC(uint8_t *_pBuf, uint8_t _ucLen){
	uint16_t crc;
	uint8_t buf[S_TX_BUF_SIZE];

	memcpy(buf, _pBuf, _ucLen);
	crc = CRC16_Modbus(_pBuf, _ucLen);
	buf[_ucLen++] = crc >> 8;
	buf[_ucLen++] = crc;
    
    //如果串口不忙的话，就可以发送
    send_data(buf,_ucLen);

}
/*
*********************************************************************************************************
*	函 数 名: MODS_SendAckErr
*	功能说明: 发送错误应答
*	形    参: _ucErrCode : 错误代码
*	返 回 值: 无
*********************************************************************************************************
*/
static void modbusSendAckErr(uint8_t _ucErrCode)
{
	uint8_t txbuf[3];

	txbuf[0] = gt_modbus.rx_buf[0];					/* 485地址 */
	txbuf[1] = gt_modbus.rx_buf[1] | 0x80;				/* 异常的功能码 */
	txbuf[2] = _ucErrCode;							/* 错误代码(01,02,03,04) */

	modbusSendWithCRC(txbuf, 3);
}




static void cc_buf_convert_struct(uint8_t *buf){
     gt_modbus_reg.fr_conf.G_COLD_START_ATTEMPTS=buf[0];
     gt_modbus_reg.fr_conf.GD_ACTION_POINT_OFFSET=buf[1];
     gt_modbus_reg.fr_conf.GD_CAS_RX_LOW_MAX=buf[2];
     gt_modbus_reg.fr_conf.GD_DYNAMIC_SLOT_IDLE_PHASE=buf[3];
     gt_modbus_reg.fr_conf.GD_MINISLOT=buf[4];
     gt_modbus_reg.fr_conf.GD_MINI_SLOT_ACTION_POINT_OFFSET=buf[5];
     gt_modbus_reg.fr_conf.GD_STATIC_SLOT=swap_u16_from_u8(buf[6],buf[7]);
     gt_modbus_reg.fr_conf.GD_SYMBOL_WINDOW=buf[8];
     gt_modbus_reg.fr_conf.GD_TSS_TRANSMITTER=buf[9];
     gt_modbus_reg.fr_conf.GD_WAKEUP_SYMBOL_RX_IDLE=buf[10];
     gt_modbus_reg.fr_conf.GD_WAKEUP_SYMBOL_RX_LOW=buf[11];
     
    gt_modbus_reg.fr_conf.GD_WAKEUP_SYMBOL_RX_WINDOW=swap_u16_from_u8(buf[12],buf[13]);
    gt_modbus_reg.fr_conf.GD_WAKEUP_SYMBOL_TX_IDLE=buf[14];
    gt_modbus_reg.fr_conf.GD_WAKEUP_SYMBOL_TX_LOW=buf[15];
    gt_modbus_reg.fr_conf.G_LISTEN_NOISE=buf[16];
    
    gt_modbus_reg.fr_conf.G_MACRO_PER_CYCLE=swap_u16_from_u8(buf[17],buf[18]);
    gt_modbus_reg.fr_conf.G_MAX_WITHOUT_CLOCK_CORRECTION_PASSIVE=buf[19];
    gt_modbus_reg.fr_conf.G_MAX_WITHOUT_CLOCK_CORRECTION_FATAL=buf[20];
    
    gt_modbus_reg.fr_conf.G_NUMBER_OF_MINISLOTS=swap_u16_from_u8(buf[21],buf[22]);
    gt_modbus_reg.fr_conf.G_NUMBER_OF_STATIC_SLOTS=swap_u16_from_u8(buf[23],buf[24]);
    gt_modbus_reg.fr_conf.G_OFFSET_CORRECTION_START=swap_u16_from_u8(buf[25],buf[26]);
    
    gt_modbus_reg.fr_conf.G_PAYLOAD_LENGTH_STATIC=buf[27];
    gt_modbus_reg.fr_conf.G_SYNC_NODE_MAX=buf[28];
    gt_modbus_reg.fr_conf.G_NETWORK_MANAGEMENT_VECTOR_LENGTH=buf[29];
    gt_modbus_reg.fr_conf.P_ALLOW_HALT_DUE_TO_CLOCK=buf[30];
    gt_modbus_reg.fr_conf.P_ALLOW_PASSIVE_TO_ACTIVE=buf[31];
    gt_modbus_reg.fr_conf.P_CHANNELS=swap_u16_from_u8(buf[32],buf[33]);
    gt_modbus_reg.fr_conf.PD_ACCEPTED_STARTUP_RANGE=swap_u16_from_u8(buf[34],buf[35]);
    gt_modbus_reg.fr_conf.P_CLUSTER_DRIFT_DAMPING=buf[36];
    gt_modbus_reg.fr_conf.P_DECODING_CORRECTION=buf[37];
    gt_modbus_reg.fr_conf.P_DELAY_COMPENSATION_CHA=buf[38];
    gt_modbus_reg.fr_conf.P_DELAY_COMPENSATION_CHB=buf[39];
    
    gt_modbus_reg.fr_conf.PD_LISTEN_TIMEOUT=swap_u32_from_u8(buf[40],buf[41],buf[42],buf[43]);
    gt_modbus_reg.fr_conf.PD_MAX_DRIFT=swap_u16_from_u8(buf[44],buf[45]);
    gt_modbus_reg.fr_conf.P_EXTERN_OFFSET_CORRECTION=buf[46];
    gt_modbus_reg.fr_conf.P_EXTERN_RATE_CORRECTION=buf[47];
    gt_modbus_reg.fr_conf.P_KEY_SLOT_ID=swap_u16_from_u8(buf[48],buf[49]);
    gt_modbus_reg.fr_conf.P_KEY_SLOT_USED_FOR_STARTUP=buf[50];
    gt_modbus_reg.fr_conf.P_KEY_SLOT_USED_FOR_SYNC=buf[51];
    gt_modbus_reg.fr_conf.P_KEY_SLOT_HEADER_CRC=swap_u16_from_u8(buf[52],buf[53]);
    gt_modbus_reg.fr_conf.P_LATEST_TX=swap_u16_from_u8(buf[54],buf[55]);
    gt_modbus_reg.fr_conf.P_MACRO_INITIAL_OFFSET_A=buf[56];
    gt_modbus_reg.fr_conf.P_MACRO_INITIAL_OFFSET_B=buf[57];
    gt_modbus_reg.fr_conf.P_MICRO_INITIAL_OFFSET_A=buf[58];
    gt_modbus_reg.fr_conf.P_MICRO_INITIAL_OFFSET_B=buf[59];
    gt_modbus_reg.fr_conf.P_MICRO_PER_CYCLE=swap_u32_from_u8(buf[60],buf[61],buf[62],buf[63]);
    gt_modbus_reg.fr_conf.P_OFFSET_CORRECTION_OUT=swap_u16_from_u8(buf[64],buf[65]);
    gt_modbus_reg.fr_conf.P_RATE_CORRECTION_OUT=swap_u16_from_u8(buf[66],buf[67]);
    gt_modbus_reg.fr_conf.P_SINGLE_SLOT_ENABLED=buf[68];
    gt_modbus_reg.fr_conf.P_WAKEUP_CHANNEL=swap_u16_from_u8(buf[69],buf[70]);
    gt_modbus_reg.fr_conf.P_WAKEUP_PATTERN=buf[71];
    gt_modbus_reg.fr_conf.P_MICRO_PER_MACRO_NOM=buf[72];
    gt_modbus_reg.fr_conf.P_PAYLOAD_LENGTH_DYN_MAX=buf[73];
    
}

static cc_struct_convert_buf(uint8_t *buf){
     buf[0]=gt_modbus_reg.fr_conf.G_COLD_START_ATTEMPTS;
     buf[1]=gt_modbus_reg.fr_conf.GD_ACTION_POINT_OFFSET;
     buf[2]=gt_modbus_reg.fr_conf.GD_CAS_RX_LOW_MAX;
     buf[3]=gt_modbus_reg.fr_conf.GD_DYNAMIC_SLOT_IDLE_PHASE;
     buf[4]=gt_modbus_reg.fr_conf.GD_MINISLOT;
     buf[5]=gt_modbus_reg.fr_conf.GD_MINI_SLOT_ACTION_POINT_OFFSET;
     

     swap_u16_to_u8(gt_modbus_reg.fr_conf.GD_STATIC_SLOT,&buf[6]);
     buf[8]=gt_modbus_reg.fr_conf.GD_SYMBOL_WINDOW;
     buf[9]=gt_modbus_reg.fr_conf.GD_TSS_TRANSMITTER;
     buf[10]=gt_modbus_reg.fr_conf.GD_WAKEUP_SYMBOL_RX_IDLE;
     buf[11]=gt_modbus_reg.fr_conf.GD_WAKEUP_SYMBOL_RX_LOW;
     
    swap_u16_to_u8(gt_modbus_reg.fr_conf.GD_WAKEUP_SYMBOL_RX_WINDOW,&buf[12]);
    buf[14]=gt_modbus_reg.fr_conf.GD_WAKEUP_SYMBOL_TX_IDLE;
    buf[15]=gt_modbus_reg.fr_conf.GD_WAKEUP_SYMBOL_TX_LOW;
    buf[16]=gt_modbus_reg.fr_conf.G_LISTEN_NOISE;
    
    swap_u16_to_u8(gt_modbus_reg.fr_conf.G_MACRO_PER_CYCLE,&buf[17]);
    buf[19]=gt_modbus_reg.fr_conf.G_MAX_WITHOUT_CLOCK_CORRECTION_PASSIVE;
    buf[20]=gt_modbus_reg.fr_conf.G_MAX_WITHOUT_CLOCK_CORRECTION_FATAL;
    
    swap_u16_to_u8(gt_modbus_reg.fr_conf.G_NUMBER_OF_MINISLOTS,&buf[21]);
    swap_u16_to_u8(gt_modbus_reg.fr_conf.G_NUMBER_OF_STATIC_SLOTS,&buf[23]);
    swap_u16_to_u8(gt_modbus_reg.fr_conf.G_OFFSET_CORRECTION_START,&buf[25]);
    
    buf[27]=gt_modbus_reg.fr_conf.G_PAYLOAD_LENGTH_STATIC;
    buf[28]=gt_modbus_reg.fr_conf.G_SYNC_NODE_MAX;
    buf[29]=gt_modbus_reg.fr_conf.G_NETWORK_MANAGEMENT_VECTOR_LENGTH;
    buf[30]=gt_modbus_reg.fr_conf.P_ALLOW_HALT_DUE_TO_CLOCK;
    buf[31]=gt_modbus_reg.fr_conf.P_ALLOW_PASSIVE_TO_ACTIVE;
    swap_u16_to_u8(gt_modbus_reg.fr_conf.P_CHANNELS,&buf[32]);
    swap_u16_to_u8(gt_modbus_reg.fr_conf.PD_ACCEPTED_STARTUP_RANGE,&buf[34]);
    buf[36]=gt_modbus_reg.fr_conf.P_CLUSTER_DRIFT_DAMPING;
    buf[37]=gt_modbus_reg.fr_conf.P_DECODING_CORRECTION;
    buf[38]=gt_modbus_reg.fr_conf.P_DELAY_COMPENSATION_CHA;
    buf[39]=gt_modbus_reg.fr_conf.P_DELAY_COMPENSATION_CHB;
    
    swap_u32_to_u8(gt_modbus_reg.fr_conf.PD_LISTEN_TIMEOUT,&buf[40]);
    swap_u16_to_u8(gt_modbus_reg.fr_conf.PD_MAX_DRIFT,&buf[44]);
    buf[46]=gt_modbus_reg.fr_conf.P_EXTERN_OFFSET_CORRECTION;
    buf[47]=gt_modbus_reg.fr_conf.P_EXTERN_RATE_CORRECTION;
    swap_u16_to_u8(gt_modbus_reg.fr_conf.P_KEY_SLOT_ID,&buf[48]);
    buf[50]=gt_modbus_reg.fr_conf.P_KEY_SLOT_USED_FOR_STARTUP;
    buf[51]=gt_modbus_reg.fr_conf.P_KEY_SLOT_USED_FOR_SYNC;
    swap_u16_to_u8(gt_modbus_reg.fr_conf.P_KEY_SLOT_HEADER_CRC,&buf[52]);
    swap_u16_to_u8(gt_modbus_reg.fr_conf.P_LATEST_TX,&buf[54]);
    buf[56]=gt_modbus_reg.fr_conf.P_MACRO_INITIAL_OFFSET_A;
    buf[57]=gt_modbus_reg.fr_conf.P_MACRO_INITIAL_OFFSET_B;
    buf[58]=gt_modbus_reg.fr_conf.P_MICRO_INITIAL_OFFSET_A;
    buf[59]=gt_modbus_reg.fr_conf.P_MICRO_INITIAL_OFFSET_B;
    swap_u32_to_u8(gt_modbus_reg.fr_conf.P_MICRO_PER_CYCLE,&buf[60]);
    swap_u16_to_u8(gt_modbus_reg.fr_conf.P_OFFSET_CORRECTION_OUT,&buf[64]);
    swap_u16_to_u8(gt_modbus_reg.fr_conf.P_RATE_CORRECTION_OUT,&buf[66]);
    buf[68]=gt_modbus_reg.fr_conf.P_SINGLE_SLOT_ENABLED;
    swap_u16_to_u8(gt_modbus_reg.fr_conf.P_WAKEUP_CHANNEL,&buf[69]);
    buf[71]=gt_modbus_reg.fr_conf.P_WAKEUP_PATTERN;
    buf[72]=gt_modbus_reg.fr_conf.P_MICRO_PER_MACRO_NOM;
    buf[73]=gt_modbus_reg.fr_conf.P_PAYLOAD_LENGTH_DYN_MAX;
}

//配置CC
static void config_cc_func(void){
   	uint16_t reg;
	uint16_t num;
	uint16_t i;
    uint8_t* dest_addr= (uint8_t *)&gt_modbus_reg.fr_conf;
	gt_modbus.rsp_code = RSP_OK;

	if (gt_modbus.rx_count != 80){// 								/* 03H命令必须是8个字节 */
	
		gt_modbus.rsp_code = RSP_ERR_VALUE;					/* 数据值域错误 */
		goto err_ret;
	}
    cc_buf_convert_struct(&gt_modbus.rx_buf[4]);
err_ret:
	if (gt_modbus.rsp_code == RSP_OK){							/* 正确应答 */
		gt_modbus.tx_count = 0;
		gt_modbus.tx_buf[gt_modbus.tx_count++] = (gt_modbus_reg.device_addr_reg&0xff);
		gt_modbus.tx_buf[gt_modbus.tx_count++] = gt_modbus.rx_buf[1];
		gt_modbus.tx_buf[gt_modbus.tx_count++] = gt_modbus.rx_buf[2];			/* 返回字节数 */
        gt_modbus.tx_buf[gt_modbus.tx_count++] = gt_modbus.rx_buf[3];

		modbusSendWithCRC(gt_modbus.tx_buf, gt_modbus.tx_count);	/* 发送正确应答 */
	}
	else
	{
		modbusSendAckErr(gt_modbus.rsp_code);					/* 发送错误应答 */
	}    
        
}

//读取CC     
static void read_cc_func(void){
   	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint16_t read_style;
	uint16_t send_size;
    uint8_t* dest_addr= (uint8_t *)&gt_modbus_reg.fr_conf;
	gt_modbus.rsp_code = RSP_OK;

    send_size=sizeof(Fr_low_level_config_type);
    
    read_style=((gt_modbus.rx_buf[4]<<8)|gt_modbus.rx_buf[5]);
    if(read_style!=1){
		gt_modbus.rsp_code = RSP_ERR_VALUE;					/* 数据值域错误 */
		goto err_ret; 
    }
    
	if (gt_modbus.rx_count != 8){//								/* 03H命令必须是8个字节 */
		gt_modbus.rsp_code = RSP_ERR_VALUE;					/* 数据值域错误 */
		goto err_ret;
	}
 
err_ret:
	if (gt_modbus.rsp_code == RSP_OK){							/* 正确应答 */
		gt_modbus.tx_count = 0;
		for(i=0;i<6;i++){
		   gt_modbus.tx_buf[gt_modbus.tx_count++] =gt_modbus.rx_buf[i]; 
		}
		cc_struct_convert_buf(&gt_modbus.tx_buf[gt_modbus.tx_count]);
		gt_modbus.tx_count+=send_size;
		modbusSendWithCRC(gt_modbus.tx_buf, gt_modbus.tx_count);	/* 发送正确应答 */
	}
	else{
		modbusSendAckErr(gt_modbus.rsp_code);					/* 发送错误应答 */
	}      
}



static void slot_buf_convert_struct(uint8_t *buf){
	
	//先判断序号 
	
	int i;
	int frame_id_index;
	//如果有frame id 和需要占用，都要返回,
	uint8 listNum = buf[0];

	uint16 frame_ID;
    //一个序号代表一个 
    
    listNum = buf[0];
    frame_ID= swap_u16_from_u8(buf[1],buf[2]);	     

	frame_id_index=0;   
    //先判断该序号 
    //如果有相同的frame_id  就不能配置
	for(i=0;i<MAX_SLOT_BUF_SIZE;i++){
	    if(frame_id_index==0){
	        if(gt_modbus_reg.gt_slot_conf[i].frame_ID==frame_ID){
	              frame_id_index=i;
	              break;  
	        }  
	    }
	}
	
    if((frame_id_index==0)||(frame_id_index==listNum)){  
         if((frame_id_index==listNum)&&(buf[10]<=gt_modbus_reg.gt_slot_conf[listNum].payload_length)) {
            
         } else{
             gt_modbus_reg.gt_slot_conf[listNum].data=&gt_recv_buf.data[gt_recv_buf.cur_used];
             gt_recv_buf.cur_used+=gt_modbus_reg.gt_slot_conf[listNum].payload_length; 
             
             if(gt_recv_buf.cur_used>=MAX_DATA_BUF){
                  //需要重新打乱重新计算。
             }
         }
   
	     gt_modbus_reg.gt_slot_conf[listNum].frame_ID=frame_ID;
	     gt_modbus_reg.gt_slot_conf[listNum].FrameType=buf[3];
	     gt_modbus_reg.gt_slot_conf[listNum].buffer_type=swap_u16_from_u8(buf[4],buf[5]);
	     gt_modbus_reg.gt_slot_conf[listNum].receive_channel=swap_u16_from_u8(buf[6],buf[7]);
	     gt_modbus_reg.gt_slot_conf[listNum].base_circle=buf[8];
	     gt_modbus_reg.gt_slot_conf[listNum].base_circle_interval=buf[9];
	     gt_modbus_reg.gt_slot_conf[listNum].payload_length=buf[10];
	     gt_modbus_reg.gt_slot_conf[listNum].transmission_mode=swap_u16_from_u8(buf[11],buf[12]);
	     gt_modbus_reg.gt_slot_conf[listNum].transmission_commit_mode=swap_u16_from_u8(buf[13],buf[14]);
	     gt_modbus_reg.gt_slot_conf[listNum].transmit_channel=swap_u16_from_u8(buf[15],buf[16]);
         
         if(gt_modbus_reg.gt_slot_conf[listNum].buffer_type==FR_TRANSMIT_BUFFER){
             gt_modbus_reg.gt_slot_conf[listNum].header_CRC=headcrc_clac(0,0,gt_modbus_reg.gt_slot_conf[listNum].frame_ID,gt_modbus_reg.gt_slot_conf[listNum].payload_length);
         }
         gt_modbus_reg.gt_slot_conf[listNum].transmit_type= FR_DOUBLE_TRANSMIT_BUFFER;
         gt_modbus_reg.reserved_id=0x12; 
    }
}


 //返回的使用的字节
void  slot_struct_convert_buf(uint8_t *buf,uint8_t num){
     int buf_index;
     uint8 listNum;
     //先判断该序号          
     buf_index=0;
     listNum = num;
        
     buf[buf_index]= num;
     swap_u16_to_u8(gt_modbus_reg.gt_slot_conf[listNum].frame_ID,&buf[buf_index+1]);
     buf[buf_index+3]=gt_modbus_reg.gt_slot_conf[listNum].FrameType;
     swap_u16_to_u8(gt_modbus_reg.gt_slot_conf[listNum].buffer_type,&buf[buf_index+4]);
     swap_u16_to_u8(gt_modbus_reg.gt_slot_conf[listNum].receive_channel,&buf[buf_index+6]);
     buf[buf_index+8]=gt_modbus_reg.gt_slot_conf[listNum].base_circle;
     buf[buf_index+9]=gt_modbus_reg.gt_slot_conf[listNum].base_circle_interval;
     buf[buf_index+10]=gt_modbus_reg.gt_slot_conf[listNum].payload_length;
     swap_u16_to_u8(gt_modbus_reg.gt_slot_conf[listNum].transmission_mode,&buf[buf_index+11]);
     swap_u16_to_u8(gt_modbus_reg.gt_slot_conf[listNum].transmission_commit_mode,&buf[buf_index+13]);
     swap_u16_to_u8(gt_modbus_reg.gt_slot_conf[listNum].transmit_channel,&buf[buf_index+14]);
     buf[buf_index+17]=0;
}



//配置静态槽和动态槽 
static void config_slot_func(void){
   	uint16_t reg;
	uint16_t num;
	uint16_t i;
	
    uint8_t* dest_addr= (uint8_t *)&gt_modbus_reg.fr_conf;
	gt_modbus.rsp_code = RSP_OK;

	if (gt_modbus.rx_count != 24){//								/* 03H命令必须是8个字节 */
	
		gt_modbus.rsp_code = RSP_ERR_VALUE;					/* 数据值域错误 */
		goto err_ret;
	}
    slot_buf_convert_struct(&gt_modbus.rx_buf[4]);
    


err_ret:
	if (gt_modbus.rsp_code == RSP_OK){							/* 正确应答 */
		gt_modbus.tx_count = 0;
		gt_modbus.tx_buf[gt_modbus.tx_count++] = (gt_modbus_reg.device_addr_reg&0xff);
		gt_modbus.tx_buf[gt_modbus.tx_count++] = gt_modbus.rx_buf[1];
		gt_modbus.tx_buf[gt_modbus.tx_count++] = gt_modbus.rx_buf[2];			/* 返回字节数 */
        gt_modbus.tx_buf[gt_modbus.tx_count++] = gt_modbus.rx_buf[3];

		modbusSendWithCRC(gt_modbus.tx_buf, gt_modbus.tx_count);	/* 发送正确应答 */
	}
	else
	{
		modbusSendAckErr(gt_modbus.rsp_code);					/* 发送错误应答 */
	}      
}





//读取SLOT     
static void read_slot_func(void){
   	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint8_t read_td;
	uint16_t send_size;
    uint8_t* dest_addr= (uint8_t *)&gt_modbus_reg.fr_conf;
	gt_modbus.rsp_code = RSP_OK;

    send_size=sizeof(Fr_low_level_config_type);
    read_td= gt_modbus.rx_buf[4];

    
	if (gt_modbus.rx_count != 7){//								/* 03H命令必须是8个字节 */
		gt_modbus.rsp_code = RSP_ERR_VALUE;					/* 数据值域错误 */
		goto err_ret;
	}
 
err_ret:
	if (gt_modbus.rsp_code == RSP_OK){							/* 正确应答 */
		gt_modbus.tx_count = 0;
		for(i=0;i<4;i++){
		   gt_modbus.tx_buf[gt_modbus.tx_count++] =gt_modbus.rx_buf[i]; 
		}
		slot_struct_convert_buf(&gt_modbus.tx_buf[gt_modbus.tx_count],read_td);
		gt_modbus.tx_count+=18;
		modbusSendWithCRC(gt_modbus.tx_buf, gt_modbus.tx_count);	/* 发送正确应答 */
	}
	else{
		modbusSendAckErr(gt_modbus.rsp_code);					/* 发送错误应答 */
	}    
        
}




uint8_t  slot_read_data_buf(uint8_t *buf,uint8_t num){

     uint16_t *data;
     int i;
     
     if(num>80 ||(gt_modbus_reg.gt_slot_conf[num].frame_ID==0)){
          for(i=0;i<14;i++){
              buf[i]=0;
          }
          return 14;
     } else{
 
         data=gt_modbus_reg.gt_slot_conf[num].data;
         
         for(i=0;i<gt_modbus_reg.gt_slot_conf[num].payload_length;i++){
              swap_u16_to_u8(data[i],&buf[i*2]);
         }
         return (gt_modbus_reg.gt_slot_conf[num].payload_length*2);
     }
}




static void read_slot_data_func(void){
   	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint8_t  list_num;
	uint16_t send_size;

	gt_modbus.rsp_code = RSP_OK;
    
    list_num= gt_modbus.rx_buf[3];

	if (gt_modbus.rx_count != 6){//								/* 03H命令必须是8个字节 */
		gt_modbus.rsp_code = RSP_ERR_VALUE;					/* 数据值域错误 */
		goto err_ret;
	}
 
err_ret:
	if (gt_modbus.rsp_code == RSP_OK){							/* 正确应答 */
		gt_modbus.tx_count = 0;
		for(i=0;i<4;i++){
		   gt_modbus.tx_buf[gt_modbus.tx_count++] =gt_modbus.rx_buf[i]; 
		}
    	gt_modbus.tx_count+=slot_read_data_buf(&gt_modbus.tx_buf[gt_modbus.tx_count],list_num);
		modbusSendWithCRC(gt_modbus.tx_buf, gt_modbus.tx_count);	/* 发送正确应答 */
	}
	else{
		modbusSendAckErr(gt_modbus.rsp_code);					/* 发送错误应答 */
	}    
}

uint8_t  send_slot_data(uint8_t *buf,uint8_t num) {
 
    int i=0;
    uint16_t *data; 
    if(num>80 ||(gt_modbus_reg.gt_slot_conf[num].frame_ID==0)){
          return 0;
     } else{
         data=gt_modbus_reg.gt_slot_conf[num].data;
         
         for(i=0;i<gt_modbus_reg.gt_slot_conf[num].payload_length;i++){
              data[i]=swap_u16_from_u8(buf[i*2],buf[i*2+1]);
         }
         return (gt_modbus_reg.gt_slot_conf[num].payload_length*2);
     }
 }


static void send_slot_data_func(void){
   	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint8_t  list_num;
	uint8_t send_size;

	gt_modbus.rsp_code = RSP_OK;
    
    list_num= gt_modbus.rx_buf[3];

    //这个大小等会儿在弄
    /*
	if (gt_modbus.rx_count != 6){//							
		gt_modbus.rsp_code = RSP_ERR_VALUE;				
		goto err_ret;
	}
	*/  
	send_size= send_slot_data(&gt_modbus.tx_buf[4],list_num);

err_ret:
	if (gt_modbus.rsp_code == RSP_OK){							/* 正确应答 */
		gt_modbus.tx_count = 0;
		for(i=0;i<4;i++){
		   gt_modbus.tx_buf[gt_modbus.tx_count++] =gt_modbus.rx_buf[i]; 
		}
		
		gt_modbus.tx_buf[gt_modbus.tx_count++]=send_size;
		modbusSendWithCRC(gt_modbus.tx_buf, gt_modbus.tx_count);	/* 发送正确应答 */
	}
	else{
		modbusSendAckErr(gt_modbus.rsp_code);					/* 发送错误应答 */
	}    
}

/*
*********************************************************************************************************
*	函 数 名: MODS_SendAckOk
*	功能说明: 发送正确的应答.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void modbusSendAckOk(void)
{
	uint8_t txbuf[6];
	uint8_t i;

	for (i = 0; i < 6; i++)
	{
		txbuf[i] = gt_modbus.rx_buf[i];
	}
	modbusSendWithCRC(txbuf, 6);
}

/*
*********************************************************************************************************
*	函 数 名: MODS_AnalyzeApp
*	功能说明: 分析应用层协议
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void modbusAnalyzeApp(void){
    //第2，3字节是次地址码
    uint8_t  op_code;
    uint16_t second_addr;
    second_addr=((gt_modbus.rx_buf[1]<<8)|gt_modbus.rx_buf[2]);
    op_code=gt_modbus.rx_buf[3];
    
    if(second_addr==1){
        if(op_code==2){         //配置CC寄存器 
		    config_cc_func();  
		} else if(op_code==4){    //读取CC寄存器 
		    read_cc_func();  //这2个有点冲突 
		} else if(op_code==3){
		    config_slot_func(); 
		}else if(op_code==5){
		    read_slot_func(); 
		}
    } else if(second_addr==3){
        read_slot_data_func(); //读取slot 的数据
    } else if(second_addr==4){  //发送数据 
        send_slot_data_func();  
    }else{
		gt_modbus.rsp_code = RSP_ERR_CMD;
		modbusSendAckErr(gt_modbus.rsp_code);	/* 告诉主机命令错误 */
    }
        
        

}


void modbusInit()
{
    int i;
    need_save_reg_data=0;
    start_recv_data = 1;  // 表示可以接收数据
    recv_data_complete=0; //表示接收数据完成
    gt_modbus_reg.device_addr_reg=0x5a;
    //为了程序的移植，最好多层封装
    uart1_init(gt_modbus_reg.baudrate_reg,gt_modbus_reg.stopbit_reg,gt_modbus_reg.checkbit_reg);  //默认9600

    //需要注册回调函数
    registerRecvFunc(modbusReciveData);
    need_update_data=0;

}


void modbusPoll(void)
{
	uint16_t addr;
	uint16_t crc1;
	/* 超过3.5个字符时间后执行MODH_RxTimeOut()函数。全局变量 g_modbus_timeout = 1; 通知主程序开始解码 */
	if (recv_data_complete == 0){ 	 //表明没有接收完成  
		return;								/* 没有超时，继续接收。不要清零 g_tModS.RxCount */
	}
	
	if (gt_modbus.rx_count < 6){				/* 接收到的数据小于4个字节就认为错误 */
    	goto err_ret; ;
	}

	/* 计算CRC校验和 */
	crc1 = CRC16_Modbus(gt_modbus.rx_buf, gt_modbus.rx_count);
	if (crc1 != 0){
		goto err_ret;
	}

	/* 站地址 (1字节） */
	addr = gt_modbus.rx_buf[0];				/* 第1字节 站号 */
    
	if (addr != (gt_modbus_reg.device_addr_reg&0xff)){	/* 判断主机发送的命令地址是否符合 */
		goto err_ret;
	}

	/* 分析应用层协议 */
	modbusAnalyzeApp();						
	
err_ret:
    //中间最好有个停顿
    start_recv_data=1;
    recv_data_complete=0;
	gt_modbus.rx_count = 0;					/* 必须清零计数器，方便下次帧同步 */
}



#define CRC_HEADER_POLY 0xB85
#define CRC_HEADER_IV	0x1A
#define CRC_HEADER_LENGTH 11
#define CRC_HEADER_DATA_LENGTH	20

//进行头CRC 的校验

uint16_t headcrc_clac(uint8_t sync_bit,uint8_t startup_bit,uint32_t frame_id,uint8_t payload_length){


    uint32_t need_crc_value;
    uint32_t shiftreg;
    uint16_t return_value;
    int16_t i;
    uint16_t bit;

    need_crc_value=(sync_bit << 19)|(startup_bit << 18)| payload_length | (frame_id << 7);

    shiftreg =CRC_HEADER_IV;



    for (i=CRC_HEADER_DATA_LENGTH-1; i>=0; i--){
        //qDebug("888--%d==%d",need_crc_value,i);
        bit = ((shiftreg >> (CRC_HEADER_LENGTH-1)) & 0x1) ^ ((need_crc_value >> i) & 0x1);


        shiftreg <<= 1;

        if (bit){
            shiftreg ^= CRC_HEADER_POLY;
        }

        shiftreg &= (1 << CRC_HEADER_LENGTH) - 1;
    }

    return_value=shiftreg;
    return return_value;
}


uint8_t calc_mb_index(T_MODBUS_REG *reg){
    uint16_t i;
    uint16_t index=0;
    
    uint16_t mb_used_index=0;
    T_slot_conf temp_slot_slot;
    //先是接收，然后发送 
    
   for(i=0;i<MAX_MB_INDEX_SIZE;i++){
        mb_index[i].mb_index=FR_LAST_MB;
        mb_index[i].slot=0xff;
  }
       //发送 
    for(i=0;i<MAX_SLOT_BUF_SIZE;i++){
        if(reg->gt_slot_conf[i].frame_ID!=0){
           if(reg->gt_slot_conf[i].buffer_type==FR_TRANSMIT_BUFFER) {

               if(reg->gt_slot_conf[i].transmit_type==FR_DOUBLE_TRANSMIT_BUFFER){
                    mb_index[index].mb_index=mb_used_index;
                    mb_index[index].slot=i; 
                    reg->gt_slot_conf[i].start_mb_index= mb_used_index;
                    mb_used_index+=2;
               } else{
                    mb_index[index].mb_index=mb_used_index;
                    mb_index[index].slot=i; 
                    reg->gt_slot_conf[i].start_mb_index= mb_used_index;
                    mb_used_index++; 
               }
               index++;

               if(mb_used_index>=MAX_MB_INDEX_SIZE){
                   return 0;
               }
           }
        }
    }

    for(i=0;i<MAX_SLOT_BUF_SIZE;i++){
        if(reg->gt_slot_conf[i].frame_ID!=0){
           if(reg->gt_slot_conf[i].buffer_type==FR_RECEIVE_BUFFER) {
               mb_index[index].mb_index=mb_used_index;
               mb_index[index].slot=i;
               reg->gt_slot_conf[i].start_mb_index= mb_used_index;
               index++;
               mb_used_index++; 
               if(mb_used_index>=MAX_MB_INDEX_SIZE){
                   return 0;
               }
           }
        }
    }

    
    mb_index[index].mb_index=FR_LAST_MB;
    mb_index[index].slot=0xff;
    
    return 1;
}            