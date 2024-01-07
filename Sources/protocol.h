#include <stdint.h>


typedef struct {
	/* 03H 读寄存器 */
	/* 06H 写寄存器 */
    int16_t temperature_reg; //0x0   //第一个值是识别好  为0x1234  //32768    氧气值
    int16_t humi_reg; //0x1   //   0-10000   
    int16_t smoke_reg; //0x2    继电器的状态，只读
    int16_t pm1_0_reg; //0x3      0
		int16_t smoke_alarm_state_reg; //0x  显示蜂鸣器有没有报警
    int16_t reserve_reg[95];  //0x2
    int16_t  model_number_reg;//0x64
    int16_t  testPoint_number_reg;//0x65 
    int16_t  device_addr_reg;//0x66
    int16_t  baudrate_reg;//0x67  //1:2400  2:4800 3:9600 4:19200  5:38400  6:115200
    int16_t  comm_mode_reg;//0x68    //0x1:RS485   OX2:主动上传 其他参数参数没有用
    int16_t  protocol_reg;//0x69
    int16_t  upload_time_reg;//0x6a   //1-3600  秒       、、
    int16_t  checkbit_reg;//0x6b
    int16_t  stopbit_reg;//0x6c
    int16_t  temperature_correct_reg;//0x6d
    int16_t  humi_correct_reg;//0x6e
    int16_t  smoke_correct_reg;//0x6f
    int16_t  pm1_0_correct_reg; //0x70  sunLight_correct_reg   （-1000~1000）
	  int16_t  nc_correct_reg; //0x71           
		int16_t  pm1_0_alarm_threshold_reg; //0x72   （0~5000）
		int16_t  ppm_threshold_reg; //0x73           （0~4095）
		int16_t  smoke_da_cur_reg; //0x74            （0~4095）    
		int16_t  smoke_da_max_reg; //0x75  固定      （0~4095）
	  int16_t  smoke_da_min_reg; //0x76            （0~4095）    
		int16_t  smoke_range_max_reg; //0x77         （0~5000） 
		int16_t  smoke_range_min_reg; //0x78         （0~5000）
	  int16_t  alarm_stop_reset_time_reg; //0x79   （0~3600）   单位分   最大 1440
		int16_t  alarm_stop_reg; //07a                   
		
    //uint16_t  reserve_reg_1;  //reserve 6f 
    
//    uint16_t  temperature_correct__80_reg;
//    uint16_t  temperature_correct__40_reg;
//    uint16_t  temperature_correct_0_reg;
//    uint16_t  temperature_correct_40_reg;
//    uint16_t  temperature_correct_80_reg;
//    uint16_t  temperature_correct_120_reg;
//    uint16_t  temperature_correct_160_reg;
//    uint16_t  temperature_correct_200_reg;
//    uint16_t  temperature_correct_240_reg;
//    uint16_t  temperature_correct_280_reg;
//    uint16_t  temperature_correct_320_reg;
//    uint16_t  temperature_correct_360_reg;
//    uint16_t  temperature_correct_400_reg;
    //.... 设置最大显示的温度
    
    //T_TEMP_CORRECT_VALUE temp_value[MAX_CORRECT_SIZE];//一般最大需要20组数据，进行从0到大排序即可
   
}T_MODBUS_REG;

void INIT_PORT(void); 
void write_byte(unsigned char A); 
void write_Data(unsigned char b); 
void write_command(unsigned char b); 
void lcd_clear(void);
void lcd_string(unsigned char row,unsigned char col,char *data1);
void delay20us(unsigned int n);
void delay1ms(unsigned int n); 
 

