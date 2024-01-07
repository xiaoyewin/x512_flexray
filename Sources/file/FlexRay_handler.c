#include "LCD.h"
#include <stdio.h>
#include "derivative.h"      /* derivative-specific definitions */

/** Standard and driver types */
#include "Fr_UNIFIED_types.h"  

/** UNIFIED driver implementation */         
#include "Fr_UNIFIED.h"        
         
/** Configuration data for the FlexRay node */
#include "Fr_UNIFIED_cfg.h"
#include "modbus_rtu.h"

extern T_MODBUS_REG gt_modbus_reg;
extern T_MB_Index mb_index[MAX_MB_INDEX_SIZE]; 				
/** Double transmit MB 0 - commit side, slot 1 */
#define TX_SLOT_1                   0  
/** Double transmit MB 1 - transmit side, slot 1 */
#define TX_SLOT_1_TRANSMIT_SIDE     1   
/** Receive MB 3, slot 4 */
 

#define RX_SLOT_69                   2
#define RX_SLOT_70                   3    
/** Return values */
Fr_return_type return_value;      
/** Current protocol state */      
Fr_POC_state_type protocol_state;       
/** Current wakeup status */
Fr_wakeup_state_type wakeup_status;     
/** Current cycle value */
uint8 current_cycle;                    
/** Current macrotick value */
uint16 current_macrotick;               


/** Definition of the variables used for updating of the transmit MBs */
/** Data array - static segment, slot 1 */
uint16 tx_data_1[16] = {0};             
/** Transmission return values */
Fr_tx_MB_status_type tx_return_value;   

/** Variables used for storing data and status from the receive MBs */
/** Data array - static segment, slot 4 */
uint16 rx_data_4[16] = {0};             
/** Received data length */
uint8 rx_data_length = 0;               
/** Received frame status */
uint16 rx_status_slot = 0;              
/** Reception return values */
Fr_rx_MB_status_type rx_return_value;    

/** Variables used for storing data and status of the FIFO A */
/** Data array */
uint16 fifo_data[8] = {0};              
/** Received data length */
uint8 fifo_data_length = 0;             
/** Received frame status */
uint16 fifo_status_slot = 0;            
/** Received frame index */
uint16 fifo_slot_idx = 0;               
/** Return values */
Fr_FIFO_status_type fifo_return_value;  

/******************************************************************************/
/**
* \brief    Error function for debugging 
* \author   Jaime Orozco
* \param    u8number: error code
* \return   void
*/
void Failed(uint8 u8number)               
{    
    while(u8number);       /* Function only for debugging, CC should be restarted  */
}




//需要做一个通用函数 

void CC_interrupt_slot_recv_func(uint8 buffer_idx) {
    //根据这个函数算出  
          //复制接收到的数据到缓冲区
    int i=0;
    
    uint8_t list_num;
    uint16 *Fr_data_ptr; 
    uint8 *Fr_data_length_ptr;
 
    
    for(i=0;i<MAX_MB_INDEX_SIZE;i++){
         if(mb_index[i].mb_index==buffer_idx){
             break;
         }
    }
    list_num=255;
    if(i!=MAX_MB_INDEX_SIZE){
       list_num=mb_index[i].slot;   
    }
    Fr_get_global_time(&current_cycle, &current_macrotick); 
    //这个得到当前的循环周期
   // circle_num = (uint8_t)(Fr_CC_reg_ptr[FrCYCTR]);  
	                    
	if(list_num==255){  //那表示没有
	   	 Fr_data_length_ptr= &rx_data_length;
	     Fr_data_ptr=&rx_data_4[0]; 
	}
	else{
	      Fr_data_ptr= gt_modbus_reg.gt_slot_conf[list_num].data ;
	      Fr_data_length_ptr=&gt_modbus_reg.gt_slot_conf[list_num].payload_length;
          rx_return_value = Fr_receive_data(buffer_idx,Fr_data_ptr,Fr_data_length_ptr
                                  , &rx_status_slot);
	}                              
}



//发射的中断           
void CC_interrupt_slot_send_func(uint8 buffer_idx) {
    
    
    //发射需要考虑到双内存   
    int i=0; 
    uint8_t list_num;
    for(i=0;i<MAX_MB_INDEX_SIZE;i++){
         if(mb_index[i].mb_index==buffer_idx){
           //表示找到了
             break;
         }
         else if((mb_index[i].mb_index+1)==buffer_idx){
            if(gt_modbus_reg.gt_slot_conf[mb_index[i].slot].transmit_type==FR_DOUBLE_TRANSMIT_BUFFER){
                break;
            }
         }
    }
    list_num=255;
    if(i!=MAX_MB_INDEX_SIZE){
       list_num=mb_index[i].slot;   
    }
    Fr_get_global_time(&current_cycle, &current_macrotick); 
    //这个得到当前的循环周期
   // circle_num = (uint8_t)(Fr_CC_reg_ptr[FrCYCTR]);  
	                    
	if(list_num==255){  //那表示没有

	}
	else{
	
     	tx_return_value = Fr_transmit_data(mb_index[i].mb_index, &gt_modbus_reg.gt_slot_conf[list_num].data, gt_modbus_reg.gt_slot_conf[list_num].payload_length);
     	if(mb_index[i].mb_index!=buffer_idx){
     	     Fr_clear_MB_interrupt_flag(TX_SLOT_1_TRANSMIT_SIDE);
     	}
	}     
}
/*******************************************************************************/
/**
* \brief    Function for transmission on Slot 1
* \author   R62779
* \param    buffer_idx: Message Buffer identifier
* \return   void
*/


void CC_interrupt_slot_1(uint8 buffer_idx)
{
    /* Update double transmit MB with new data (commit side) */
    tx_return_value = Fr_transmit_data(TX_SLOT_1, &tx_data_1[0], 14);
    
    /* It is necessary to clear the flag at tranmsit side */
    Fr_clear_MB_interrupt_flag(TX_SLOT_1_TRANSMIT_SIDE);
    
    if(tx_return_value == FR_TXMB_UPDATED)
    {
        tx_data_1[0]++;     /* Increment variable */
    }    
}

/*******************************************************************************/
/**
* \brief    Function for reception on Slot 4  
* \author   R62779
* \param    buffer_idx: Message Buffer identifier
* \return   void
*/
void CC_interrupt_slot_70(uint8 buffer_idx)
{
    //复制接收到的数据到缓冲区
    int i=0;
    
    uint8_t list_num;
    	uint16 *Fr_data_ptr; 
    uint8 *Fr_data_length_ptr;
    list_num =255;
        
    for(i=0;i<MAX_SLOT_BUF_SIZE;i++){
        if(gt_modbus_reg.gt_slot_conf[i].frame_ID==69){
              list_num=i;
              break;
        }  
	}
	                    
	if(list_num==255){  //那表示没有
	    
	   	 Fr_data_length_ptr= &rx_data_length;
	     Fr_data_ptr=&rx_data_4[0]; 
	}
	else{
	      Fr_data_ptr= gt_modbus_reg.gt_slot_conf[list_num].data ;
	      Fr_data_length_ptr=&gt_modbus_reg.gt_slot_conf[list_num].payload_length;
	}
        
    rx_return_value = Fr_receive_data(buffer_idx,Fr_data_ptr,Fr_data_length_ptr
                                      , &rx_status_slot);
             
    tx_data_1[1] = rx_status_slot;  //存储状态slot
    

    
    
}

void CC_interrupt_slot_69(uint8 buffer_idx)
{
    //复制接收到的数据到缓冲区
    int i=0;
    
    uint8_t list_num;
    	uint16 *Fr_data_ptr; 
    uint8 *Fr_data_length_ptr;
    list_num =255;
        
    for(i=0;i<MAX_SLOT_BUF_SIZE;i++){
        if(gt_modbus_reg.gt_slot_conf[i].frame_ID==69){
              list_num=i;
              break;
        }  
	}
	                    
	if(list_num==255){  //那表示没有
	    
	   	 Fr_data_length_ptr= &rx_data_length;
	     Fr_data_ptr=&rx_data_4[0]; 
	}
	else{
	      Fr_data_ptr= gt_modbus_reg.gt_slot_conf[list_num].data ;
	      Fr_data_length_ptr=&gt_modbus_reg.gt_slot_conf[list_num].payload_length;
	}
        
    rx_return_value = Fr_receive_data(buffer_idx,Fr_data_ptr,Fr_data_length_ptr
                                      , &rx_status_slot);
             
    tx_data_1[1] = rx_status_slot;  //存储状态slot
    
    //slot 1中的数据递增
    tx_data_1[2]++;
    
    //显示接收到的数据
}
/*******************************************************************************/
/**
* \brief    Function for FlexRay Timer 1 
* \author   R62779
* \param    void
* \return   void
*/
void CC_interrupt_timer_1(void)
{
    /* Get the global time */
    Fr_get_global_time(&current_cycle, &current_macrotick);     
    tx_data_1[14] = current_macrotick;      /* Store current macrotick value */
    tx_data_1[15] = (uint16) (current_cycle);   /* Store current cycle value */
}

/*******************************************************************************/
/**
* \brief    Function for FlexRay Timer 2 
* \author   R62779
* \param    void
* \return   void
*/
void CC_interrupt_timer_2(void)
{
    /* Get the global time */
    Fr_get_global_time(&current_cycle, &current_macrotick);
    tx_data_1[12] = current_macrotick;      /* Store current macrotick value */
    tx_data_1[13] = (uint16) (current_cycle);   /* Store current cycle value */
}

/*******************************************************************************/
/**

//表示每个CIRCLR开始的循环    
*/

void CC_interrupt_cycle_start(void){
    /* Get the global time */
    Fr_get_global_time(&current_cycle, &current_macrotick); 
}

/*******************************************************************************/
/**
* \brief    Function for FlexRay FIFO A interrupt
* \author   R62779
* \param    header_idx: Header index
* \return   void
*/
void CC_interrupt_FIFO_A(uint16 header_idx)
{
    /* Copy received data into given array  */
    fifo_return_value = Fr_receive_fifo_data(header_idx, &fifo_data[0],
                        &fifo_data_length, &fifo_slot_idx, &fifo_status_slot);
    
    if(fifo_return_value == FR_FIFO_RECEIVED)   /* Data has been received? */
    {                                  
        /* Store header index into data array - position 8 */
        tx_data_1[3] = header_idx;     
        /* Store value of the first field into data array - position 9 */     
        tx_data_1[4] = fifo_data[0];        
        
        /* Update appropriate variables in case that a frame has been received */
        switch(fifo_slot_idx)
        {
        case 4:          
            /* Slot 4,fault:
            Store 0xF0F3 value into data array - position 5 */
            tx_data_1[5] = 0xF0F3;   
            break;  
            
        case 5:                             
            /* Slot 5, fault:
            Store 0xF0F2 value into data array - position 6 */
            tx_data_1[6] = 0xF0F2;          
            break;
            
        case 62:                            
            /* Slot 62: Increment variable in case that a frame
            has been received - position 7 */
            tx_data_1[7]++;  
            break;
            
        case 63:   
            /* Slot 63: Increment variable in case that a frame
            has been received - position 8 */     
            tx_data_1[8]++;                            
            break;
            
        default:                            
            /* Another frame - fault */
            /* Store 0xF0F0 value into data array - position 9 */
            tx_data_1[9] = 0xF0F0;          
            break;
        }
    }
    
    /* If data has not been received */
    else if(fifo_return_value == FR_FIFO_NOT_RECEIVED)  
    {
        /* Store 0xFFFF value to the transmit data array, position 10 */
        tx_data_1[10] = 0xFFFF;              
    }

}

/*************************************************************/
/*                   FLEXRAY初始化函数     这里面的需要支持自定义                   */
/*************************************************************/
void vfnFlexRay_Init(){     
 
    uint16_t i;


    //使能FlexRay CC并进入FR_POCSTATE_CONFIG
    return_value = Fr_init(&Fr_HW_cfg_00, &gt_modbus_reg.fr_conf);
    //如果不成功，则等待 
    //if(return_value == FR_NOT_SUCCESS)
    //        Failed(1);  

    //初始化参数
    Fr_set_configuration(&Fr_HW_cfg_00, &gt_modbus_reg.fr_conf);
    
    //初始化所有BUFFER
    Fr_buffers_init_custom(gt_modbus_reg.gt_slot_conf,mb_index);
   // Fr_buffers_init(&Fr_buffer_cfg_00[0], &Fr_buffer_cfg_set_00[0]);
    


    //设置MB 0发生中断的调用函数   中断服务函数 
   // Fr_set_MB_callback(&CC_interrupt_slot_1, TX_SLOT_1);  
   // Fr_set_MB_callback(&CC_interrupt_slot_1, TX_SLOT_1_TRANSMIT_SIDE);  


    for(i=0;i<MAX_SLOT_BUF_SIZE;i++){
        if(gt_modbus_reg.gt_slot_conf[i].buffer_type==FR_TRANSMIT_BUFFER){
            if(gt_modbus_reg.gt_slot_conf[i].transmit_type==FR_DOUBLE_TRANSMIT_BUFFER){
                Fr_set_MB_callback(&CC_interrupt_slot_send_func, gt_modbus_reg.gt_slot_conf[i].start_mb_index+1);    
            }
            Fr_set_MB_callback(&CC_interrupt_slot_send_func, gt_modbus_reg.gt_slot_conf[i].start_mb_index);  
        }
        if(gt_modbus_reg.gt_slot_conf[i].buffer_type==FR_RECEIVE_BUFFER){
            Fr_set_MB_callback(&CC_interrupt_slot_recv_func, gt_modbus_reg.gt_slot_conf[i].start_mb_index);  
        }	        
	}


    //设置MB 3发生中断的调用函数
    //进行循环设置MB 对应的中断
   // Fr_set_MB_callback(&CC_interrupt_slot_recv_func, RX_SLOT_69);
   // Fr_set_MB_callback(&CC_interrupt_slot_recv_func, RX_SLOT_70);
    

    //初始化定时器
    Fr_timers_init(&Fr_timers_cfg_00_ptr[0]);

    //设置定时器1产生中断时的调用函数
    Fr_set_protocol_0_IRQ_callback(&CC_interrupt_timer_1, FR_TIMER_1_EXPIRED_IRQ);

    //设置定时器2产生中断时的调用函数
    Fr_set_protocol_0_IRQ_callback(&CC_interrupt_timer_2, FR_TIMER_2_EXPIRED_IRQ);

    //设置cycle_start中断时的调用函数
    Fr_set_protocol_0_IRQ_callback(&CC_interrupt_cycle_start, FR_CYCLE_START_IRQ);

    //设置IRQ中断的调用函数
    Fr_set_fifo_IRQ_callback(&CC_interrupt_FIFO_A, FR_FIFO_A_IRQ);    
        
    
    //脱离FR_POCSTATE_CONFIG状态
    return_value = Fr_leave_configuration_mode();
    
    if(return_value == FR_NOT_SUCCESS)
            Failed(2);   
 
    //恢复wakeup状态
    wakeup_status = Fr_get_wakeup_state();  
    
    //检查是否进入wakeup状态
    if(wakeup_status == FR_WAKEUPSTATE_UNDEFINED)
    {   /* No wakeup pattern has been received, initiate wakeup procedure */
        return_value = Fr_send_wakeup();
            if(return_value == FR_NOT_SUCCESS)
                Failed(3);   /* Call debug function in case of any error */
    }
   
    protocol_state = Fr_get_POC_state();    //载入当前的 POC状态
    
    
    //等待直到FR CC不处于FR_POCSTATE_READY 状态
        while(Fr_get_POC_state() != FR_POCSTATE_READY)  
            protocol_state = Fr_get_POC_state(); //载入当前的 POC状态
        
    return_value = Fr_start_communication();    //初始化startup 
    
        if(return_value == FR_NOT_SUCCESS)
            Failed(4);  
    
    
    protocol_state = Fr_get_POC_state();   //载入当前的 POC状态
    
    //等待直到FR CC不处于FR_POCSTATE_NORMAL_ACTIVE 状态
    while(Fr_get_POC_state() != FR_POCSTATE_NORMAL_ACTIVE)
        protocol_state = Fr_get_POC_state();//载入当前的 POC状态     
  
    protocol_state = Fr_get_POC_state();    //载入当前的 POC状态
    
    //第一次初始化buffer 0
    for(i=0;i<MAX_SLOT_BUF_SIZE;i++){
        if(gt_modbus_reg.gt_slot_conf[i].buffer_type==FR_TRANSMIT_BUFFER){
            tx_return_value = Fr_transmit_data(gt_modbus_reg.gt_slot_conf[i].start_mb_index, gt_modbus_reg.gt_slot_conf[i].data, gt_modbus_reg.gt_slot_conf[i].payload_length);
            if(tx_return_value == FR_TXMB_NO_ACCESS)
               Failed(4);   
        }	        
	}
	

    wakeup_status = Fr_get_wakeup_state(); //载入当前wakeup状态    

    //使能中断 
    Fr_enable_interrupts((FR_PROTOCOL_IRQ | FR_FIFO_A_IRQ | FR_RECEIVE_IRQ | FR_TRANSMIT_IRQ), 
                    (FR_TIMER_1_EXPIRED_IRQ | FR_TIMER_2_EXPIRED_IRQ | FR_CYCLE_START_IRQ), 0);
                        
    Fr_start_timer(FR_TIMER_T1);    //启动定时器T1
    Fr_start_timer(FR_TIMER_T2);    //启动定时器T2
                        
} 



//进行发送的处理
void vfnFlexRay_Handler(void){

    uint16_t i;
    Fr_tx_status_type tx_status;    
    boolean cycle_starts = FALSE;  
    //检查传输循环是否已经开始
    cycle_starts = Fr_check_cycle_start(&current_cycle);
        
        if(cycle_starts){    //是否开始
        
            for(i=0;i<MAX_SLOT_BUF_SIZE;i++){
                if(gt_modbus_reg.gt_slot_conf[i].buffer_type==FR_TRANSMIT_BUFFER){
                    tx_status = Fr_check_tx_status(gt_modbus_reg.gt_slot_conf[i].start_mb_index);
                    //检查数据是否发送
                    if(tx_status == FR_TRANSMITTED){
                        //更新MB数据
                         Fr_transmit_data(gt_modbus_reg.gt_slot_conf[i].start_mb_index, gt_modbus_reg.gt_slot_conf[i].data, gt_modbus_reg.gt_slot_conf[i].payload_length);  
                     } 
                }	        
            }
        }
            //检查接收状态
}
