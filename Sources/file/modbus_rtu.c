#include "modbus_rtu.h"
#include "Fr_UNIFIED_cfg.h"
extern  Fr_low_level_config_type Fr_low_level_cfg_set_00;
T_Recv_Data_Buf gt_recv_buf;

/*
*********************************************************************************************************
*	�� �� ��: MODS_Poll
*	����˵��: �������ݰ�. �����������������á�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/

//������ڽ��յ�״̬�У��ǲ��ܷ������ݵ�

uint16_t need_update_data ;  //��Ҫ������Щ����

uint8_t need_save_reg_data ;  //��Ҫ��������


uint8_t start_recv_data ;  // ��ʾ���Խ�������
uint8_t recv_data_complete; //��ʾ�����������


T_MODBUS gt_modbus;
T_MODBUS_REG gt_modbus_reg;


T_MB_Index mb_index[MAX_MB_INDEX_SIZE];  //���32������
 
uint16_t  g_device_id=MODBUS_DEVICE_ID;


/*
�����Ҫ�ŵ��жϺ����� 
*/
void modbusReciveData(uint8_t byte){
	/*
		3.5���ַ���ʱ������ֻ������RTUģʽ���棬��ΪRTUģʽû�п�ʼ���ͽ�������
		�������ݰ�֮��ֻ�ܿ�ʱ���������֣�Modbus�����ڲ�ͬ�Ĳ������£����ʱ���ǲ�һ���ģ�
		���Ծ���3.5���ַ���ʱ�䣬�����ʸߣ����ʱ������С�������ʵͣ����ʱ������Ӧ�ʹ�

		4800  = 7.297ms
		9600  = 3.646ms
		19200  = 1.771ms
		38400  = 0.885ms
	*/
    if(start_recv_data>0){
        //��ʾ�Ѿ���ʱ,��Ҫ�жϵ�һ���ַ�
        if((gt_modbus_reg.device_addr_reg & 0xff)==byte){  //�Ǿ�˵��������Ч
            start_recv_data=0;//�������ʱ���
            gt_modbus.rx_count=0; //��ʼ��������
            recv_data_complete=0;//���ݽ��տ�ʼ
            gt_modbus.rx_buf[gt_modbus.rx_count++] = byte;
        }
    }
    else if(recv_data_complete==0){
        if (gt_modbus.rx_count < S_RX_BUF_SIZE){
            gt_modbus.rx_buf[gt_modbus.rx_count++] = byte;
        }
    }
	
    //��������Ҫ�������ö�ʱ��
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


// CRC ��λ�ֽ�ֵ��
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
// CRC ��λ�ֽ�ֵ��
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
*	�� �� ��: CRC16_Modbus
*	����˵��: ����CRC�� ����ModbusЭ�顣
*	��    ��: _pBuf : ����У�������
*			  _usLen : ���ݳ���
*	�� �� ֵ: 16λ����ֵ�� ����Modbus ���˽�����ֽ��ȴ��ͣ����ֽں��͡�
*
*   ���п��ܵ�CRCֵ����Ԥװ���������鵱�У������㱨������ʱ���Լ򵥵��������ɣ�
*   һ�����������16λCRC�������256�����ܵĸ�λ�ֽڣ���һ�����麬�е�λ�ֽڵ�ֵ��
*   ������������CRC�ķ�ʽ�ṩ�˱ȶԱ��Ļ�������ÿһ�����ַ��������µ�CRC����ķ�����
*
*  ע�⣺�˳����ڲ�ִ�и�/��CRC�ֽڵĽ������˺������ص����Ѿ�����������CRCֵ��Ҳ����˵���ú����ķ���ֵ����ֱ�ӷ���
*        �ڱ������ڷ��ͣ�
*********************************************************************************************************
*/
uint16_t CRC16_Modbus(uint8_t *_pBuf, uint16_t _usLen)
{
	uint8_t ucCRCHi = 0xFF; /* ��CRC�ֽڳ�ʼ�� */
	uint8_t ucCRCLo = 0xFF; /* ��CRC �ֽڳ�ʼ�� */
	uint16_t usIndex;  /* CRCѭ���е����� */

  while (_usLen--)
  {
      usIndex = ucCRCHi ^ *_pBuf++; /* ����CRC */
      ucCRCHi = ucCRCLo ^ s_CRCHi[usIndex];
      ucCRCLo = s_CRCLo[usIndex];
  }
  return ((uint16_t)ucCRCHi << 8 | ucCRCLo);
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_ReadRegValue
*	����˵��: ��ȡ���ּĴ�����ֵ
*	��    ��: reg_addr �Ĵ�����ַ
*			  reg_value ��żĴ������
*	�� �� ֵ: 1��ʾOK 0��ʾ����
*********************************************************************************************************
*/
static uint8_t modbusReadRegValue(uint16_t reg_addr, uint8_t *reg_value)
{
	uint16_t value;
    
    if(reg_addr<=0x72){
 
 /*       
        if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->temperature_reg)){
            //�����ʾ��Ҫ��ȡ�¶�,��ô��Ҫ�ɼ�һ��
            need_update_data|=UPDATE_TEMP_DATA;
        }
        
        if(reg_addr*2==(uint16_t)(&((T_MODBUS_REG *)0)->humidity_reg)){
            //�����ʾ��Ҫ��ȡ�¶�,��ô��Ҫ�ɼ�һ��
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
    }									/* ��ȡ�ɹ� */
}

/*
*********************************************************************************************************
*	�� �� ��: modbusWriteRegValue
*	����˵��: ��ȡ���ּĴ�����ֵ
*	��    ��: reg_addr �Ĵ�����ַ
*			  reg_value �Ĵ���ֵ
*	�� �� ֵ: 1��ʾOK 0��ʾ����
*********************************************************************************************************
*/
static uint8_t modbusWriteRegValue(uint16_t reg_addr, int16_t reg_value)
{
    //�����޸�
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
    else{  //�������0xff00 
       
       return 0;
    }
 
}




/*
*********************************************************************************************************
*	�� �� ��: BEBufToUint16
*	����˵��: ��2�ֽ�����(���Big Endian���򣬸��ֽ���ǰ)ת��Ϊ16λ����
*	��    ��: _pBuf : ����
*	�� �� ֵ: 16λ����ֵ
*
*   ���(Big Endian)��С��(Little Endian)
*********************************************************************************************************
*/
uint16_t BEBufToUint16(uint8_t *_pBuf)
{
    return (((uint16_t)_pBuf[0] << 8) | _pBuf[1]);
}



/*
*********************************************************************************************************
*	�� �� ��: MODS_SendWithCRC
*	����˵��: ����һ������, �Զ�׷��2�ֽ�CRC
*	��    ��: _pBuf ���ݣ�
*			  _ucLen ���ݳ��ȣ�����CRC��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void modbusSendWithCRC(uint8_t *_pBuf, uint8_t _ucLen){
	uint16_t crc;
	uint8_t buf[S_TX_BUF_SIZE];

	memcpy(buf, _pBuf, _ucLen);
	crc = CRC16_Modbus(_pBuf, _ucLen);
	buf[_ucLen++] = crc >> 8;
	buf[_ucLen++] = crc;
    
    //������ڲ�æ�Ļ����Ϳ��Է���
    send_data(buf,_ucLen);

}
/*
*********************************************************************************************************
*	�� �� ��: MODS_SendAckErr
*	����˵��: ���ʹ���Ӧ��
*	��    ��: _ucErrCode : �������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void modbusSendAckErr(uint8_t _ucErrCode)
{
	uint8_t txbuf[3];

	txbuf[0] = gt_modbus.rx_buf[0];					/* 485��ַ */
	txbuf[1] = gt_modbus.rx_buf[1] | 0x80;				/* �쳣�Ĺ����� */
	txbuf[2] = _ucErrCode;							/* �������(01,02,03,04) */

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

//����CC
static void config_cc_func(void){
   	uint16_t reg;
	uint16_t num;
	uint16_t i;
    uint8_t* dest_addr= (uint8_t *)&gt_modbus_reg.fr_conf;
	gt_modbus.rsp_code = RSP_OK;

	if (gt_modbus.rx_count != 80){// 								/* 03H���������8���ֽ� */
	
		gt_modbus.rsp_code = RSP_ERR_VALUE;					/* ����ֵ����� */
		goto err_ret;
	}
    cc_buf_convert_struct(&gt_modbus.rx_buf[4]);
err_ret:
	if (gt_modbus.rsp_code == RSP_OK){							/* ��ȷӦ�� */
		gt_modbus.tx_count = 0;
		gt_modbus.tx_buf[gt_modbus.tx_count++] = (gt_modbus_reg.device_addr_reg&0xff);
		gt_modbus.tx_buf[gt_modbus.tx_count++] = gt_modbus.rx_buf[1];
		gt_modbus.tx_buf[gt_modbus.tx_count++] = gt_modbus.rx_buf[2];			/* �����ֽ��� */
        gt_modbus.tx_buf[gt_modbus.tx_count++] = gt_modbus.rx_buf[3];

		modbusSendWithCRC(gt_modbus.tx_buf, gt_modbus.tx_count);	/* ������ȷӦ�� */
	}
	else
	{
		modbusSendAckErr(gt_modbus.rsp_code);					/* ���ʹ���Ӧ�� */
	}    
        
}

//��ȡCC     
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
		gt_modbus.rsp_code = RSP_ERR_VALUE;					/* ����ֵ����� */
		goto err_ret; 
    }
    
	if (gt_modbus.rx_count != 8){//								/* 03H���������8���ֽ� */
		gt_modbus.rsp_code = RSP_ERR_VALUE;					/* ����ֵ����� */
		goto err_ret;
	}
 
err_ret:
	if (gt_modbus.rsp_code == RSP_OK){							/* ��ȷӦ�� */
		gt_modbus.tx_count = 0;
		for(i=0;i<6;i++){
		   gt_modbus.tx_buf[gt_modbus.tx_count++] =gt_modbus.rx_buf[i]; 
		}
		cc_struct_convert_buf(&gt_modbus.tx_buf[gt_modbus.tx_count]);
		gt_modbus.tx_count+=send_size;
		modbusSendWithCRC(gt_modbus.tx_buf, gt_modbus.tx_count);	/* ������ȷӦ�� */
	}
	else{
		modbusSendAckErr(gt_modbus.rsp_code);					/* ���ʹ���Ӧ�� */
	}      
}



static void slot_buf_convert_struct(uint8_t *buf){
	
	//���ж���� 
	
	int i;
	int frame_id_index;
	//�����frame id ����Ҫռ�ã���Ҫ����,
	uint8 listNum = buf[0];

	uint16 frame_ID;
    //һ����Ŵ���һ�� 
    
    listNum = buf[0];
    frame_ID= swap_u16_from_u8(buf[1],buf[2]);	     

	frame_id_index=0;   
    //���жϸ���� 
    //�������ͬ��frame_id  �Ͳ�������
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
                  //��Ҫ���´������¼��㡣
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


 //���ص�ʹ�õ��ֽ�
void  slot_struct_convert_buf(uint8_t *buf,uint8_t num){
     int buf_index;
     uint8 listNum;
     //���жϸ����          
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



//���þ�̬�ۺͶ�̬�� 
static void config_slot_func(void){
   	uint16_t reg;
	uint16_t num;
	uint16_t i;
	
    uint8_t* dest_addr= (uint8_t *)&gt_modbus_reg.fr_conf;
	gt_modbus.rsp_code = RSP_OK;

	if (gt_modbus.rx_count != 24){//								/* 03H���������8���ֽ� */
	
		gt_modbus.rsp_code = RSP_ERR_VALUE;					/* ����ֵ����� */
		goto err_ret;
	}
    slot_buf_convert_struct(&gt_modbus.rx_buf[4]);
    


err_ret:
	if (gt_modbus.rsp_code == RSP_OK){							/* ��ȷӦ�� */
		gt_modbus.tx_count = 0;
		gt_modbus.tx_buf[gt_modbus.tx_count++] = (gt_modbus_reg.device_addr_reg&0xff);
		gt_modbus.tx_buf[gt_modbus.tx_count++] = gt_modbus.rx_buf[1];
		gt_modbus.tx_buf[gt_modbus.tx_count++] = gt_modbus.rx_buf[2];			/* �����ֽ��� */
        gt_modbus.tx_buf[gt_modbus.tx_count++] = gt_modbus.rx_buf[3];

		modbusSendWithCRC(gt_modbus.tx_buf, gt_modbus.tx_count);	/* ������ȷӦ�� */
	}
	else
	{
		modbusSendAckErr(gt_modbus.rsp_code);					/* ���ʹ���Ӧ�� */
	}      
}





//��ȡSLOT     
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

    
	if (gt_modbus.rx_count != 7){//								/* 03H���������8���ֽ� */
		gt_modbus.rsp_code = RSP_ERR_VALUE;					/* ����ֵ����� */
		goto err_ret;
	}
 
err_ret:
	if (gt_modbus.rsp_code == RSP_OK){							/* ��ȷӦ�� */
		gt_modbus.tx_count = 0;
		for(i=0;i<4;i++){
		   gt_modbus.tx_buf[gt_modbus.tx_count++] =gt_modbus.rx_buf[i]; 
		}
		slot_struct_convert_buf(&gt_modbus.tx_buf[gt_modbus.tx_count],read_td);
		gt_modbus.tx_count+=18;
		modbusSendWithCRC(gt_modbus.tx_buf, gt_modbus.tx_count);	/* ������ȷӦ�� */
	}
	else{
		modbusSendAckErr(gt_modbus.rsp_code);					/* ���ʹ���Ӧ�� */
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

	if (gt_modbus.rx_count != 6){//								/* 03H���������8���ֽ� */
		gt_modbus.rsp_code = RSP_ERR_VALUE;					/* ����ֵ����� */
		goto err_ret;
	}
 
err_ret:
	if (gt_modbus.rsp_code == RSP_OK){							/* ��ȷӦ�� */
		gt_modbus.tx_count = 0;
		for(i=0;i<4;i++){
		   gt_modbus.tx_buf[gt_modbus.tx_count++] =gt_modbus.rx_buf[i]; 
		}
    	gt_modbus.tx_count+=slot_read_data_buf(&gt_modbus.tx_buf[gt_modbus.tx_count],list_num);
		modbusSendWithCRC(gt_modbus.tx_buf, gt_modbus.tx_count);	/* ������ȷӦ�� */
	}
	else{
		modbusSendAckErr(gt_modbus.rsp_code);					/* ���ʹ���Ӧ�� */
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

    //�����С�Ȼ����Ū
    /*
	if (gt_modbus.rx_count != 6){//							
		gt_modbus.rsp_code = RSP_ERR_VALUE;				
		goto err_ret;
	}
	*/  
	send_size= send_slot_data(&gt_modbus.tx_buf[4],list_num);

err_ret:
	if (gt_modbus.rsp_code == RSP_OK){							/* ��ȷӦ�� */
		gt_modbus.tx_count = 0;
		for(i=0;i<4;i++){
		   gt_modbus.tx_buf[gt_modbus.tx_count++] =gt_modbus.rx_buf[i]; 
		}
		
		gt_modbus.tx_buf[gt_modbus.tx_count++]=send_size;
		modbusSendWithCRC(gt_modbus.tx_buf, gt_modbus.tx_count);	/* ������ȷӦ�� */
	}
	else{
		modbusSendAckErr(gt_modbus.rsp_code);					/* ���ʹ���Ӧ�� */
	}    
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_SendAckOk
*	����˵��: ������ȷ��Ӧ��.
*	��    ��: ��
*	�� �� ֵ: ��
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
*	�� �� ��: MODS_AnalyzeApp
*	����˵��: ����Ӧ�ò�Э��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void modbusAnalyzeApp(void){
    //��2��3�ֽ��Ǵε�ַ��
    uint8_t  op_code;
    uint16_t second_addr;
    second_addr=((gt_modbus.rx_buf[1]<<8)|gt_modbus.rx_buf[2]);
    op_code=gt_modbus.rx_buf[3];
    
    if(second_addr==1){
        if(op_code==2){         //����CC�Ĵ��� 
		    config_cc_func();  
		} else if(op_code==4){    //��ȡCC�Ĵ��� 
		    read_cc_func();  //��2���е��ͻ 
		} else if(op_code==3){
		    config_slot_func(); 
		}else if(op_code==5){
		    read_slot_func(); 
		}
    } else if(second_addr==3){
        read_slot_data_func(); //��ȡslot ������
    } else if(second_addr==4){  //�������� 
        send_slot_data_func();  
    }else{
		gt_modbus.rsp_code = RSP_ERR_CMD;
		modbusSendAckErr(gt_modbus.rsp_code);	/* ��������������� */
    }
        
        

}


void modbusInit()
{
    int i;
    need_save_reg_data=0;
    start_recv_data = 1;  // ��ʾ���Խ�������
    recv_data_complete=0; //��ʾ�����������
    gt_modbus_reg.device_addr_reg=0x5a;
    //Ϊ�˳������ֲ����ö���װ
    uart1_init(gt_modbus_reg.baudrate_reg,gt_modbus_reg.stopbit_reg,gt_modbus_reg.checkbit_reg);  //Ĭ��9600

    //��Ҫע��ص�����
    registerRecvFunc(modbusReciveData);
    need_update_data=0;

}


void modbusPoll(void)
{
	uint16_t addr;
	uint16_t crc1;
	/* ����3.5���ַ�ʱ���ִ��MODH_RxTimeOut()������ȫ�ֱ��� g_modbus_timeout = 1; ֪ͨ������ʼ���� */
	if (recv_data_complete == 0){ 	 //����û�н������  
		return;								/* û�г�ʱ���������ա���Ҫ���� g_tModS.RxCount */
	}
	
	if (gt_modbus.rx_count < 6){				/* ���յ�������С��4���ֽھ���Ϊ���� */
    	goto err_ret; ;
	}

	/* ����CRCУ��� */
	crc1 = CRC16_Modbus(gt_modbus.rx_buf, gt_modbus.rx_count);
	if (crc1 != 0){
		goto err_ret;
	}

	/* վ��ַ (1�ֽڣ� */
	addr = gt_modbus.rx_buf[0];				/* ��1�ֽ� վ�� */
    
	if (addr != (gt_modbus_reg.device_addr_reg&0xff)){	/* �ж��������͵������ַ�Ƿ���� */
		goto err_ret;
	}

	/* ����Ӧ�ò�Э�� */
	modbusAnalyzeApp();						
	
err_ret:
    //�м�����и�ͣ��
    start_recv_data=1;
    recv_data_complete=0;
	gt_modbus.rx_count = 0;					/* ��������������������´�֡ͬ�� */
}



#define CRC_HEADER_POLY 0xB85
#define CRC_HEADER_IV	0x1A
#define CRC_HEADER_LENGTH 11
#define CRC_HEADER_DATA_LENGTH	20

//����ͷCRC ��У��

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
    //���ǽ��գ�Ȼ���� 
    
   for(i=0;i<MAX_MB_INDEX_SIZE;i++){
        mb_index[i].mb_index=FR_LAST_MB;
        mb_index[i].slot=0xff;
  }
       //���� 
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