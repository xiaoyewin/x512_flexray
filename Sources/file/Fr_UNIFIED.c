/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2005 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************//*!
*
* @file      Fr_UNIFIED.c
*
* @author    R62779
* 
* @version   1.0.74.0
* 
* @date      Apr-23-2007
* 
* @brief     FlexRay UNIFIED Driver implementation
*
******************************************************************************/

/******************************************************************************
* Includes
******************************************************************************/

#include "Fr_UNIFIED.h"
#include "Fr_UNIFIED_cfg.h"

/******************************************************************************
* Constants
******************************************************************************/

/******************************************************************************
* Local variables
******************************************************************************/

// This variable is used for base address of the FlexRay module
volatile uint16 * FR_REG_FAR Fr_CC_reg_ptr;
volatile uint32 * Fr_CC_reg_32_ptr;             // 32-bit access to the registers, for the MPC5567

// This variable is used for determine which type of FlexRay hardware is used
static Fr_connected_HW_type Fr_connected_HW;

/* This internal array contains message buffer configuration information (except shadow message buffers and FIFO storage).*/
#ifdef FR_NUMBER_TXRX_MB
/* The FR_NUMBER_TXRX_MB parameter is defined in the Fr_UNIFIED_cfg.h file 
   - memory consumption optimization is applied */
    // The UNIFIED Driver allocates following array with limited number of elements
    // - the number is given by host in the Fr_UNIFIED_cfg.h file
    static Fr_MB_information_internal_type Fr_MB_information_internal[FR_NUMBER_TXRX_MB + 1];
#else
/* The FR_NUMBER_TXRX_MB parameter is not defined in the Fr_UNIFIED_cfg.h file 
   - no memory consumption optimization is applied */
    // The UNIFIED Driver allocates following array with the maximal number of elements (for maximum number of possible MB)
    static Fr_MB_information_internal_type Fr_MB_information_internal[FR_MAX_NUMBER_TXRX_MB];
#endif

/* This internal array contains slot status configuration information */
static Fr_slot_status_internal_type Fr_slot_status_information_internal;

/* This internal structure contains names of callback functions */
static Fr_callback_functions_type Fr_callback_functions;

// Internal pointers to configuration data, they are initialized in the Fr_set_configuration function
static const Fr_HW_config_type *Fr_HW_config_ptr;
static const Fr_low_level_config_type *Fr_low_level_config_ptr;
static const Fr_buffer_info_type *Fr_buffers_config_ptr;
static const uint8 *Fr_buffer_config_set_ptr;

// Internal pointers to configuration data
static  const Fr_timer_config_type *Fr_timers_config_ptr;
static const Fr_slot_status_counter_config_type *Fr_slot_status_counter_config_ptr;

static Fr_FIFO_info_type Fr_FIFO_info;

/******************************************************************************
* Global functions
******************************************************************************/

/************************************************************************************
* Function name:    Fr_init
* Description:      This API call stores the base address of the FlexRay 
*                   module and available memory, configures the channels,
*                   resets FlexRay CC and forces the CC into FR_POCSTATE_CONFIG state
*
* @author           r62779
* @version          9/11/2006
* Function arguments:
*                   Fr_HW_config_temp_ptr	Reference to the hardware configuration 
*                                       parameters for the FlexRay driver
*                   Fr_low_level_config_temp_ptr	Reference to the low level 
*                                       configuration parameters
*
* Return value:
*                   FR_SUCCESS	        API call has been successful
*                   FR_NOT_SUCCESS	    Error occured during transition 
*                                       into FR_POCSTATE_CONFIG
*************************************************************************************/
Fr_return_type Fr_init(const Fr_HW_config_type *Fr_HW_config_temp_ptr, 
                       const Fr_low_level_config_type *Fr_low_level_config_temp_ptr)
{
    volatile uint16 temp_value; // Temporary variable for bit operations
    uint8 i = 0;                // Temporary counter

    // Initialize the base address
    Fr_CC_reg_ptr = (volatile uint16 * FR_REG_FAR) Fr_HW_config_temp_ptr->CC_base_address;
   
    // Initilaze connected HW
    Fr_connected_HW = Fr_HW_config_temp_ptr->connected_HW;
    
    // Initialize the FlexRay Memory Start address
    if((Fr_connected_HW != FR_MFR4300) && (Fr_connected_HW != FR_MFR4310))  // Not necessary for the MFR4300 and MFR4310
    {
        Fr_CC_reg_ptr[FrSYMBADHR] = (uint16)(Fr_HW_config_temp_ptr->CC_FlexRay_memory_base_address >> 16);
        Fr_CC_reg_ptr[FrSYMBADLR] = (uint16)(Fr_HW_config_temp_ptr->CC_FlexRay_memory_base_address & 0xffff);
    }
    
    /* Configuration of the SYMATOR register */
    // Not implemented for MFR4300, MPC5567 (rev 0), MC9S12XFR128 and MFR4310
    if((Fr_connected_HW != FR_MFR4300) && (Fr_connected_HW != FR_MFR4310) && \
       (Fr_connected_HW != FR_MPC5567) && (Fr_connected_HW != FR_MC9S12XFR128))
    {
        if(Fr_HW_config_temp_ptr->timeout != 0)     // Stored only if the value is not equal to 0
        {
            // Store the value of the TIMEOUT bit field in the SYMATOR register
            Fr_CC_reg_ptr[FrSYMATOR] = (uint16)(Fr_HW_config_temp_ptr->timeout & 0x1F);
        }
    }
    
    /* Configuration of the MCR register */
    temp_value = 0x0000;        // Clear temporary variable
    // Configuration of the device operation mode
    if(Fr_HW_config_temp_ptr->single_channel_mode == FR_SINGLE_CHANNEL_MODE)
    {
        temp_value = FrMCR_SCM;     // Single chip mode
    }
    // FlexRay channel configuration, see data sheet for detailed description of the single and dual channel mode
    if(Fr_low_level_config_temp_ptr->P_CHANNELS == FR_CHANNEL_AB)
    {
       temp_value = FrMCR_CHA | FrMCR_CHB;  // Dual mode - both ports enabled, single mode - no ports enabled
    } 
    else if(Fr_low_level_config_temp_ptr->P_CHANNELS == FR_CHANNEL_B)
    {
        // Dual mode - port B enabled, single mode - port A enabled, internal channel A behaves as channel B
        temp_value |= FrMCR_CHB;
    }
    else if(Fr_low_level_config_temp_ptr->P_CHANNELS == FR_CHANNEL_A)
    {
        // Dual mode - port B enabled, single mode - port A enabled, internal channel A behaves as channel B
        temp_value |= FrMCR_CHA;
    }
    
    if(Fr_HW_config_temp_ptr->synchronization_filtering_enable)
    {
        temp_value |= FrMCR_SFFE;   // Synchronization frame filter enabled
    }
    
    // Not used for the MFR4300 and MFR4310
    if(((Fr_connected_HW != FR_MFR4300) && (Fr_connected_HW != FR_MFR4310)) && \
                (Fr_HW_config_temp_ptr->clock_source == FR_INTERNAL_SYSTEM_BUS_CLOCK))
    {
        temp_value |= FrMCR_CLKSEL;     // Use internal system bus clock as PE clock source
    }
    
    // PRESCALE or BITRATE bit field settings in the MCR register - a little different meaning of these bits
    if(Fr_connected_HW != FR_MFR4300)  // Not necessary for the MFR4300
    {
       // The factor by which the clock of the protocol engine will be scale down on required FlexRay bitrate
       temp_value |=  (uint16)((Fr_HW_config_temp_ptr->prescaler_value & 0x07) << 1);
    }
    
    Fr_CC_reg_ptr[FrMCR] = temp_value;  // Write to MCR reg. before enabling the FR module
    
    temp_value |= FrMCR_MEN;
    Fr_CC_reg_ptr[FrMCR] = temp_value;  // Enable FlexRay module

    /* Force the FlexRay CC into FR_POCSTATE_CONFIG */
    i = 0;      // Clear temporary counter
    while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
    {
        i++;        // Increment temporary counter
    }
    if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error
    
    Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_CONFIG | FrPOCR_WME);    // Transition to FR_POCSTATE_CONFIG
    
    i = 0;      // Clear temporary counter
    while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
    {
        i++;        // Increment temporary counter
    }
    if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error

    // Wait till FlexRay CC is not in FR_POCSTATE_CONFIG
    i = 0;      // Clear temporary counter
    while((i < FR_MAX_WAIT_CYCLES) && ( (Fr_CC_reg_ptr[FrPSR0] & 0x0700) != FrPSR0_PROTSTATE_CONFIG))
    {
        i++;        // Increment temporary counter
    }
    if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error
    
    return FR_SUCCESS;                                      // API call has been successful
}

/************************************************************************************
* Function name:    Fr_set_configuration
* Description:      This API call initializes the FlexRay CC. The following action 
*                   are performed by this function:
*                   - initialization of the data structures of the FlexRay driver
*                   - configuration of FlexRay configuration parameters (e.g. cluster configuration)
*                   - disabling all FlexRay CC interrupts
*
* @author           r62779
* @version          9/11/2006
* Function arguments:
*                   Fr_HW_config_temp_ptr   Reference to the hardware configuration 
*                                       parameters for the FlexRay driver
*                   Fr_low_level_config_temp_ptr Reference to the low level 
*                                       configuration parameters
*
* Return value:
*                   None
*************************************************************************************/
void Fr_set_configuration(const Fr_HW_config_type *Fr_HW_config_temp_ptr, 
                          const Fr_low_level_config_type *Fr_low_level_config_temp_ptr)
{
    volatile uint16 temp_value_1, temp_value_3;     // Temporary variables intended for bit operations
    volatile uint32 temp_value_2;                   // Temporary variable intended for bit operations
    uint8 Fr_p;                                     // Temporary counter
    
    // Initialize the base address for the second time - in case that this function is called more than once 
    // with different configuration
    Fr_CC_reg_ptr = (volatile uint16 * FR_REG_FAR) Fr_HW_config_temp_ptr->CC_base_address;
    Fr_CC_reg_32_ptr = (volatile uint32 *) Fr_HW_config_temp_ptr->CC_base_address;

    // Initialization of the internal pointers, which will be used by driver later on
    Fr_HW_config_ptr = Fr_HW_config_temp_ptr;
    Fr_low_level_config_ptr = Fr_low_level_config_temp_ptr;
    
    // Initialization of the internal MB information structure
    #ifdef FR_NUMBER_TXRX_MB
    /* The FR_NUMBER_TXRX_MB parameter is defined in the Fr_UNIFIED_cfg.h file 
    - memory consumption optimization is applied */
        for(Fr_p = 0; Fr_p < (FR_NUMBER_TXRX_MB + 1); Fr_p++) // Configure limited number of items
        {
            Fr_MB_information_internal[Fr_p].Fr_MB_function_ptr = NULL; // Set callback function to NULL
            Fr_MB_information_internal[Fr_p].slot_number = 0;           // Clear slot number item
            // Not necessary for the current version of the UNIFIED Driver
            //Fr_MB_information_internal[Fr_p].buffer_type = 0;           // Clear buffer type item
            Fr_MB_information_internal[Fr_p].transmission_mode = FR_SINGLE_TRANSMIT_BUFFER;     // Clear transmission mode item
            Fr_MB_information_internal[Fr_p].transmission_type = FR_EVENT_TRANSMISSION_MODE;    // Clear transmission type item
        }
    #else
    /* The FR_NUMBER_TXRX_MB parameter is not defined in the Fr_UNIFIED_cfg.h file 
       - no memory consumption optimization is applied */
        for(Fr_p = 0; Fr_p < FR_MAX_NUMBER_TXRX_MB; Fr_p++) // Configure maximum number of items
        {
            Fr_MB_information_internal[Fr_p].Fr_MB_function_ptr = NULL; // Set callback function to NULL
            Fr_MB_information_internal[Fr_p].slot_number = 0;           // Clear slot number item
            // Not necessary for the current version of the UNIFIED Driver
            //Fr_MB_information_internal[Fr_p].buffer_type = 0;           // Clear buffer type item
            Fr_MB_information_internal[Fr_p].transmission_mode = FR_SINGLE_TRANSMIT_BUFFER;     // Clear transmission mode item
            Fr_MB_information_internal[Fr_p].transmission_type = FR_EVENT_TRANSMISSION_MODE;    // Clear transmission type item
        }
    #endif

    // Initialization of the internal slot status information structure
    Fr_slot_status_information_internal.registers_used = FALSE;     // Slot status registers are not configured yet
    
    // PCR0
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->GD_ACTION_POINT_OFFSET - 1;
    temp_value_1 = (temp_value_1 << 10) | Fr_low_level_config_ptr->GD_STATIC_SLOT;
    Fr_CC_reg_ptr[FrPCR0] = temp_value_1;

    // PCR1 
    temp_value_1 = 0x0000;
    temp_value_1 = (temp_value_1 << 14) | (Fr_low_level_config_ptr->G_MACRO_PER_CYCLE - Fr_low_level_config_ptr->GD_STATIC_SLOT);
    Fr_CC_reg_ptr[FrPCR1] = temp_value_1;
    
    // PCR2 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->GD_MINISLOT - Fr_low_level_config_ptr->GD_MINI_SLOT_ACTION_POINT_OFFSET - 1;
    temp_value_1 = (temp_value_1 << 10) | Fr_low_level_config_ptr->G_NUMBER_OF_STATIC_SLOTS;
    Fr_CC_reg_ptr[FrPCR2] = temp_value_1;

    // PCR3 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->GD_WAKEUP_SYMBOL_RX_LOW;
    temp_value_1 = (temp_value_1 << 5) | (Fr_low_level_config_ptr->GD_MINI_SLOT_ACTION_POINT_OFFSET - 1);
    temp_value_1 = (temp_value_1 << 5) | Fr_low_level_config_ptr->G_COLD_START_ATTEMPTS;
    Fr_CC_reg_ptr[FrPCR3] = temp_value_1;

    // PCR4 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->GD_CAS_RX_LOW_MAX - 1;
    temp_value_1 = (temp_value_1 << 9) | Fr_low_level_config_ptr->GD_WAKEUP_SYMBOL_RX_WINDOW;
    Fr_CC_reg_ptr[FrPCR4] = temp_value_1;

    // PCR5 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->GD_TSS_TRANSMITTER;
    temp_value_1 = (temp_value_1 << 6) | Fr_low_level_config_ptr->GD_WAKEUP_SYMBOL_TX_LOW;
    temp_value_1 = (temp_value_1 << 6) | Fr_low_level_config_ptr->GD_WAKEUP_SYMBOL_RX_IDLE;
    Fr_CC_reg_ptr[FrPCR5] = temp_value_1;

    // PCR6 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->GD_SYMBOL_WINDOW - Fr_low_level_config_ptr->GD_ACTION_POINT_OFFSET - 1;
    temp_value_1 = (temp_value_1 << 7) | Fr_low_level_config_ptr->P_MACRO_INITIAL_OFFSET_A;
    Fr_CC_reg_ptr[FrPCR6] = temp_value_1;

    // PCR7 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->P_DECODING_CORRECTION + Fr_low_level_config_ptr->P_DELAY_COMPENSATION_CHB + 2;
    temp_value_1 = (temp_value_1 << 7) | (Fr_low_level_config_ptr->P_MICRO_PER_MACRO_NOM / 2);
    Fr_CC_reg_ptr[FrPCR7] = temp_value_1;

    // PCR8 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->G_MAX_WITHOUT_CLOCK_CORRECTION_FATAL;
    temp_value_1 = ((temp_value_1 << 4) | Fr_low_level_config_ptr->G_MAX_WITHOUT_CLOCK_CORRECTION_PASSIVE);
    temp_value_1 = (temp_value_1 << 8) | Fr_low_level_config_ptr->GD_WAKEUP_SYMBOL_TX_IDLE;
    Fr_CC_reg_ptr[FrPCR8] = temp_value_1;
    
    // PCR9 
    temp_value_1 = 0x0000;
    if(Fr_low_level_config_ptr->G_NUMBER_OF_MINISLOTS != 0)
    {
      temp_value_1 = TRUE;
    }
    else                                                  
    {
      temp_value_1 = FALSE;
    }

    if(Fr_low_level_config_ptr->GD_SYMBOL_WINDOW != 0)
    {
      temp_value_3 = TRUE;
    } 
    else
    {
      temp_value_3 = FALSE; 
    }
    temp_value_1 = (temp_value_1 << 1) | temp_value_3;
    temp_value_1 = (temp_value_1 << 14) | Fr_low_level_config_ptr->P_OFFSET_CORRECTION_OUT;
    Fr_CC_reg_ptr[FrPCR9] = temp_value_1;

    // PCR10 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->P_SINGLE_SLOT_ENABLED;
    temp_value_1 = (temp_value_1 << 1) | Fr_low_level_config_ptr->P_WAKEUP_CHANNEL;
    temp_value_1 = (temp_value_1 << 14) | Fr_low_level_config_ptr->G_MACRO_PER_CYCLE;
    Fr_CC_reg_ptr[FrPCR10] = temp_value_1;
      
    // PCR11 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->P_KEY_SLOT_USED_FOR_STARTUP;
    temp_value_1 = (temp_value_1 << 1) | Fr_low_level_config_ptr->P_KEY_SLOT_USED_FOR_SYNC;
    temp_value_1 = (temp_value_1 << 14) | Fr_low_level_config_ptr->G_OFFSET_CORRECTION_START;
    Fr_CC_reg_ptr[FrPCR11] = temp_value_1;

    // PCR12 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->P_ALLOW_PASSIVE_TO_ACTIVE;
    temp_value_1 = (temp_value_1 << 11) | Fr_low_level_config_ptr->P_KEY_SLOT_HEADER_CRC;
    Fr_CC_reg_ptr[FrPCR12] = temp_value_1;
    
    // PCR13 
    temp_value_1 = 0x0000;
    if(Fr_low_level_config_ptr->GD_ACTION_POINT_OFFSET >= Fr_low_level_config_ptr->GD_MINI_SLOT_ACTION_POINT_OFFSET)
    {
      temp_value_1 = Fr_low_level_config_ptr->GD_ACTION_POINT_OFFSET - 1;
    }
    else
    {
      temp_value_1 = Fr_low_level_config_ptr->GD_MINI_SLOT_ACTION_POINT_OFFSET - 1;
    }  
    temp_value_1 = (temp_value_1 << 10);
    temp_value_1 |= (Fr_low_level_config_ptr->GD_STATIC_SLOT -  Fr_low_level_config_ptr->GD_ACTION_POINT_OFFSET - 1);
    Fr_CC_reg_ptr[FrPCR13] = temp_value_1;
    
    // PCR14 
    temp_value_1 = 0x0000;
    temp_value_2 = 0x00000000;
    temp_value_2 = (Fr_low_level_config_ptr->PD_LISTEN_TIMEOUT - 1) & 0x001F0000;
    temp_value_1 = Fr_low_level_config_ptr->P_RATE_CORRECTION_OUT;
    temp_value_1 = (temp_value_1 <<5) | ((uint16)(temp_value_2 >> 16));
    Fr_CC_reg_ptr[FrPCR14] = temp_value_1;
    
    // PCR15 
    temp_value_1 = 0x0000;
    temp_value_2 = 0x00000000;
    temp_value_2 = (Fr_low_level_config_ptr->PD_LISTEN_TIMEOUT - 1) & 0x0000FFFF;
    temp_value_1 = (uint16)temp_value_2;
    Fr_CC_reg_ptr[FrPCR15] = temp_value_1;
    
    // PCR16 
    temp_value_1 = 0x0000;
    temp_value_2 = 0x00000000;
    temp_value_1 = Fr_low_level_config_ptr->P_MACRO_INITIAL_OFFSET_B;
    temp_value_2 = ((Fr_low_level_config_ptr->G_LISTEN_NOISE*Fr_low_level_config_ptr->PD_LISTEN_TIMEOUT) - 1) & 0x01FF0000;
    temp_value_1 = (temp_value_1 << 9 ) | ((uint16)(temp_value_2 >> 16));
    Fr_CC_reg_ptr[FrPCR16] = temp_value_1;
    
    // PCR17 
    temp_value_1 = 0x0000;
    temp_value_2 = 0x00000000;
    temp_value_2 = ((Fr_low_level_config_ptr->G_LISTEN_NOISE*Fr_low_level_config_ptr->PD_LISTEN_TIMEOUT) - 1) & 0x0000FFFF;
    temp_value_1 = (uint16)temp_value_2;
    Fr_CC_reg_ptr[FrPCR17] = temp_value_1;
    
    // PCR18 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->P_WAKEUP_PATTERN;
    temp_value_1 = (temp_value_1 << 10) | Fr_low_level_config_ptr->P_KEY_SLOT_ID;
    Fr_CC_reg_ptr[FrPCR18] = temp_value_1;
    
    // PCR19 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->P_DECODING_CORRECTION + Fr_low_level_config_ptr->P_DELAY_COMPENSATION_CHA + 2;
    temp_value_1 = (temp_value_1 << 7) | Fr_low_level_config_ptr->G_PAYLOAD_LENGTH_STATIC;
    Fr_CC_reg_ptr[FrPCR19] = temp_value_1;

    // PCR20 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->P_MICRO_INITIAL_OFFSET_B;
    temp_value_1 = (temp_value_1 << 8) | Fr_low_level_config_ptr->P_MICRO_INITIAL_OFFSET_A;
    Fr_CC_reg_ptr[FrPCR20] = temp_value_1;
        
    // PCR21 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->P_EXTERN_RATE_CORRECTION;
    temp_value_1 = (temp_value_1 << 13) | (Fr_low_level_config_ptr->G_NUMBER_OF_MINISLOTS - Fr_low_level_config_ptr->P_LATEST_TX);
    Fr_CC_reg_ptr[FrPCR21] = temp_value_1;
    
    // PCR22 
    temp_value_1 = 0x0000;
    temp_value_2 = 0x00000000;
    temp_value_1 = Fr_low_level_config_ptr->PD_ACCEPTED_STARTUP_RANGE - Fr_low_level_config_ptr->P_DELAY_COMPENSATION_CHA;
    temp_value_2 = (Fr_low_level_config_ptr->P_MICRO_PER_CYCLE) & 0x000F0000;
    temp_value_1 = (temp_value_1 << 4) | ((uint16)(temp_value_2 >> 16));
    Fr_CC_reg_ptr[FrPCR22] = temp_value_1;
    
    // PCR23 
    temp_value_1 = 0x0000;
    temp_value_2 = 0x00000000;
    temp_value_2 = Fr_low_level_config_ptr->P_MICRO_PER_CYCLE & 0x0000FFFF;
    temp_value_1 = (uint16)temp_value_2;
    Fr_CC_reg_ptr[FrPCR23] = temp_value_1;
    
    // PCR24 
    temp_value_1 = 0x0000;
    temp_value_2 = 0x00000000;
    temp_value_1 = Fr_low_level_config_ptr->P_CLUSTER_DRIFT_DAMPING;
    temp_value_1 = (temp_value_1 << 7) | Fr_low_level_config_ptr->P_PAYLOAD_LENGTH_DYN_MAX;
    temp_value_2 = (Fr_low_level_config_ptr->P_MICRO_PER_CYCLE - Fr_low_level_config_ptr->PD_MAX_DRIFT) & 0x000F0000;
    temp_value_1 = (temp_value_1 << 4) | ((uint16)(temp_value_2 >> 16));
    Fr_CC_reg_ptr[FrPCR24] = temp_value_1;

    // PCR25 
    temp_value_1 = 0x0000;
    temp_value_2 = 0x00000000;
    temp_value_2 = (Fr_low_level_config_ptr->P_MICRO_PER_CYCLE - Fr_low_level_config_ptr->PD_MAX_DRIFT) & 0x0000FFFF;
    temp_value_1 = (uint16)temp_value_2;
    Fr_CC_reg_ptr[FrPCR25] = temp_value_1;

    // PCR26 
    temp_value_1 = 0x0000;
    temp_value_2 = 0x00000000;
    temp_value_1 = Fr_low_level_config_ptr->P_ALLOW_HALT_DUE_TO_CLOCK;
    temp_value_1 = (temp_value_1 << 11);
    temp_value_1 |= (Fr_low_level_config_ptr->PD_ACCEPTED_STARTUP_RANGE - Fr_low_level_config_ptr->P_DELAY_COMPENSATION_CHB);
    temp_value_2 = (Fr_low_level_config_ptr->P_MICRO_PER_CYCLE + Fr_low_level_config_ptr->PD_MAX_DRIFT) & 0x000F0000;
    temp_value_1 = (temp_value_1 << 4) | ((uint16)(temp_value_2 >> 16));
    Fr_CC_reg_ptr[FrPCR26] = temp_value_1;
    
    // PCR27 
    temp_value_1 = 0x0000;
    temp_value_2 = 0x00000000;
    temp_value_2 = (Fr_low_level_config_ptr->P_MICRO_PER_CYCLE + Fr_low_level_config_ptr->PD_MAX_DRIFT) & 0x0000FFFF;
    temp_value_1 = (uint16)temp_value_2;
    Fr_CC_reg_ptr[FrPCR27] = temp_value_1;
    
    // PCR28 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->GD_DYNAMIC_SLOT_IDLE_PHASE;
    temp_value_1 = (temp_value_1 << 14);
    temp_value_1 |= (Fr_low_level_config_ptr->G_MACRO_PER_CYCLE - Fr_low_level_config_ptr->G_OFFSET_CORRECTION_START);
    Fr_CC_reg_ptr[FrPCR28] = temp_value_1;
    
    // PCR29 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->P_EXTERN_OFFSET_CORRECTION;
    temp_value_1 = (temp_value_1 << 13) | (Fr_low_level_config_ptr->G_NUMBER_OF_MINISLOTS - 1);
    Fr_CC_reg_ptr[FrPCR29] = temp_value_1;

    // PCR30 
    temp_value_1 = 0x0000;
    temp_value_1 = Fr_low_level_config_ptr->G_SYNC_NODE_MAX;
    Fr_CC_reg_ptr[FrPCR30] = temp_value_1;


    // NMVLR
    Fr_CC_reg_ptr[FrNMVLR] = Fr_low_level_config_ptr->G_NETWORK_MANAGEMENT_VECTOR_LENGTH;
    
    // Message Buffer Data Size Register initialization
    temp_value_1 = (uint16)(Fr_HW_config_ptr->MB_segment_2_data_size);
    temp_value_1 = temp_value_1 << 8;
    temp_value_1 = temp_value_1 | ((uint16)(Fr_HW_config_ptr->MB_segment_1_data_size));

    // Message Buffer Segment Size and Utilization Register initialization
    temp_value_3 = (uint16)(Fr_HW_config_ptr->last_MB_seg_1);
    temp_value_3 = temp_value_3 << 8;
    temp_value_3 = temp_value_3 | (uint16)(Fr_HW_config_ptr->last_MB_util);

    if(Fr_connected_HW != FR_MPC5567)             // Normal configuration of the MBSSUTR and MBDSR registers
    {
        // Message Buffer Data Size Register initialization
        Fr_CC_reg_ptr[FrMBDSR] = temp_value_1;    // MBSEG2DS, MBSEG1DS
        
        // Message Buffer Segment Size and Utilization Register initialization
        Fr_CC_reg_ptr[FrMBSSUTR] = temp_value_3;  // LAST_MB_SEG1, LAST_MB_UTIL
    }
    else                                          // Modification for the MPC5567 (only for Rev 0) - errata no. MUCts02955
    {
        // 32-bit access to the MBDSR register
        temp_value_2 = (uint32) temp_value_3;     // Copy value of MBSSUTR register to 32-bit variable
        temp_value_2 = (temp_value_2 << 16);      // Move bits to higher part of 32-bit variable
        temp_value_2 |= (uint32) temp_value_3;    // Copy value of the MBSSUTR register to lower part of 32-bit variable
        Fr_CC_reg_32_ptr[FrMBDSR / 2] = temp_value_2; // Write the MBSSUTR configuration into MBDSR register, bug MUCts02955
        
        // 16-bit access to the MBDSR register
        Fr_CC_reg_ptr[FrMBDSR] = temp_value_1;    // MBSEG2DS, MBSEG1DS
    }

    // SFTOR
    if(Fr_HW_config_temp_ptr->sync_frame_table_offset != 0)     // Stored only if the value is not equal to 0
    {
        // Store the value of the SFT_OFFSET bit field in the SFTOR register
        Fr_CC_reg_ptr[FrSFTOR] = Fr_HW_config_temp_ptr->sync_frame_table_offset;
    }
    
    // Disable all FlexRay interrupts
    Fr_CC_reg_ptr[FrGIFER] = 0x0000;
    Fr_CC_reg_ptr[FrPIER0] = 0x0000;
    Fr_CC_reg_ptr[FrPIER1] = 0x0000;
    
    // CRSR
    if((Fr_connected_HW == FR_MFR4300) || (Fr_connected_HW == FR_MFR4310))  // Necessary only for the MFR4300 and MFR4310
    {
        // Clear Clock and Reset Status Register
        Fr_CC_reg_ptr[FrCRSR] = 0x000F;
    }
}

/************************************************************************************
* Function name:    Fr_buffers_init
* Description:      This API call initializes the FlexRay CC message buffers, 
*                   the receive shadow buffers and receive FIFO storages
*
* @author           r62779
* @version          9/11/2006
* Function arguments:
*                   Fr_buffers_config_temp_ptr - Pointer to structure with message 
*                               buffers configuration data
*                   Fr_buffer_config_set_temp_ptr - Reference to the array with 
*                               information which message buffers will be used 
*                               from MB configuration structure (referenced by 
*                               Fr_buffers_config_temp_ptr pointer) 
*
* Return value:
*                   FR_SUCCESS	        API call has been successful
*                   FR_NOT_SUCCESS	    The FR_NUMBER_TXRX_MB parameter is not correctly set 
*                                       in the Fr_UNIFIED_cfg.h file
*************************************************************************************/
Fr_return_type Fr_buffers_init(const Fr_buffer_info_type *Fr_buffers_config_temp_ptr, 
                     const Fr_index_selector_type *Fr_buffer_config_set_temp_ptr)
{
    volatile uint8 temp_index;                  // Temporary index used for indexing of the buffer config set structure
    volatile uint8 highest_txrx_mb_number = 0;  // The highest number of configured transmit or receive MB index -for verification
                              // whether the FR_NUMBER_TXRX_MB parameter is correctly set in the Fr_UNIFIED_cfg.h file
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation
    volatile uint16 temp_value_2;               // Temporary variable used for bit operation
    volatile uint16 temp_value_3;               // Temporary variable used for bit operation - for double buffered MB
    volatile uint16 Fr_buffer_info_set_index;   // Item index in array referenced by Fr_buffer_config_set_temp_ptr pointer
    volatile uint16 Fr_MB_registers_offset_add_temp; // Temporary offset adress of message buffer registers
    Fr_receive_buffer_config_type * Fr_receive_buffer_config_ptr;   // Temporary pointer to receive configuration data
    Fr_transmit_buffer_config_type * Fr_transmit_buffer_config_ptr; // Temporary pointer to transmit configuration data
    Fr_receive_shadow_buffers_config_type * Fr_shadow_buffer_config_ptr; // Temporary pointer to receive shadow 
                                                                                // configuration data
    Fr_FIFO_config_type * Fr_FIFO_config_ptr;   // Temporary pointer to receive FIFO configuration data
    volatile uint16 * FR_DATA_FAR header_MB_ptr; // Message buffer header pointer
    uint16 Fr_p;                                // Temporary counter used for FIFO configuration
    
    // Initialization of the internal pointers, which will be used by driver later on
    Fr_buffers_config_ptr = Fr_buffers_config_temp_ptr;
    Fr_buffer_config_set_ptr = Fr_buffer_config_set_temp_ptr;
    
    // Initialization of the internal FIFO information structure
    Fr_FIFO_info.FIFO_1_used = FALSE;
    Fr_FIFO_info.FIFO_1_channel = FR_NO_CHANNEL;
    Fr_FIFO_info.FIFO_1_depth = 0;
    Fr_FIFO_info.FIFO_1_entry_size = 0;
    Fr_FIFO_info.FIFO_2_used = FALSE;
    Fr_FIFO_info.FIFO_2_channel = FR_NO_CHANNEL;
    Fr_FIFO_info.FIFO_2_depth = 0;
    Fr_FIFO_info.FIFO_2_entry_size = 0;

    // Initialization of the internal calback function structure - mainly used in Fr_interrupt_handler() function
    Fr_callback_functions.Fr_module_ptr = NULL;
    Fr_callback_functions.Fr_protocol_ptr = NULL;
    Fr_callback_functions.Fr_chi_ptr = NULL;
    Fr_callback_functions.Fr_wakeup_ptr = NULL;
    Fr_callback_functions.Fr_fifo_B_ptr = NULL;
    Fr_callback_functions.Fr_fifo_A_ptr = NULL;
    Fr_callback_functions.Fr_fatal_protocol_error_ptr = NULL;
    Fr_callback_functions.Fr_internal_protocol_error_ptr = NULL;
    Fr_callback_functions.Fr_illegal_protocol_conf_ptr = NULL; 
    Fr_callback_functions.Fr_coldstart_abort_ptr = NULL;
    Fr_callback_functions.Fr_missing_rate_corr_ptr = NULL;
    Fr_callback_functions.Fr_missing_offset_corr_ptr = NULL;
    Fr_callback_functions.Fr_clock_corr_limit_ptr = NULL;
    Fr_callback_functions.Fr_max_sync_frames_ptr = NULL;
    Fr_callback_functions.Fr_mts_received_ptr = NULL;  
    Fr_callback_functions.Fr_violation_B_ptr = NULL;   
    Fr_callback_functions.Fr_violation_A_ptr = NULL;  
    Fr_callback_functions.Fr_trans_across_boundary_B_ptr = NULL;
    Fr_callback_functions.Fr_trans_across_boundary_A_ptr = NULL;
    Fr_callback_functions.Fr_timer_2_expired_ptr = NULL;
    Fr_callback_functions.Fr_timer_1_expired_ptr = NULL;
    Fr_callback_functions.Fr_cycle_start_ptr = NULL;  
    Fr_callback_functions.Fr_error_mode_changed_ptr = NULL;  
    Fr_callback_functions.Fr_illegal_protocol_command_ptr = NULL;   
    Fr_callback_functions.Fr_protocol_engine_comm_failure_ptr = NULL;  
    Fr_callback_functions.Fr_protocol_state_changed_ptr = NULL;
    Fr_callback_functions.Fr_slot_status_counter_3_inc_ptr = NULL;
    Fr_callback_functions.Fr_slot_status_counter_2_inc_ptr = NULL;
    Fr_callback_functions.Fr_slot_status_counter_1_inc_ptr = NULL;
    Fr_callback_functions.Fr_slot_status_counter_0_inc_ptr = NULL;  
    Fr_callback_functions.Fr_even_cycle_table_written_ptr = NULL;
    Fr_callback_functions.Fr_odd_cycle_table_written_ptr = NULL;

    temp_index = 0;     // Clear index
    while(Fr_buffer_config_set_ptr[temp_index] != FR_LAST_MB)   // Configure all required MBs
    {
        Fr_buffer_info_set_index = Fr_buffer_config_set_ptr[temp_index];    // Store configuration index
        // Temporary offset address of MB registers
        Fr_MB_registers_offset_add_temp = Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init * 4;
        
        switch(Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_type)
        {
        case FR_RECEIVE_BUFFER:         // Configure receive MB
            Fr_receive_buffer_config_ptr = ((Fr_receive_buffer_config_type*)
                                            (Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_config_ptr));
            
            // Disable appropriate MB
            if(Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] & FrMBCCSR_EDS)
            {
                temp_value_1 = FrMBCCSR_EDT;
                Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
            }
            
            // Configure MBCCSRn register
            temp_value_2 = 0x0000;      // Clear variable
            if(Fr_receive_buffer_config_ptr->rx_MB_interrupt_enable)
            {
                temp_value_2 = FrMBCCSR_MBIE;            // Interrupt generation enabled
            }
            // Configure MB as Receive MB and enable interrupt if required
            Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_2;
            
            // Configure MBCCFRn register
            temp_value_1 = 0x0000;      // Clear variable
            if(Fr_receive_buffer_config_ptr->receive_channel_enable == FR_CHANNEL_A)
            {
                temp_value_1 = FrMBCCFR_CHA;    // Channel assigment - ch. A
            }
            else if(Fr_receive_buffer_config_ptr->receive_channel_enable == FR_CHANNEL_B)
            {
                temp_value_1 = FrMBCCFR_CHB;    // Channel assigment - ch. B
            }
            else if(Fr_receive_buffer_config_ptr->receive_channel_enable == FR_CHANNEL_AB)
            {
                temp_value_1 = (FrMBCCFR_CHA | FrMBCCFR_CHB);    // Channel assigment - ch. A
            }
            // Cycle counter filter settings
            if(Fr_receive_buffer_config_ptr->rx_cycle_counter_filter_enable)    // Should be the cycle counter filter enabled?
            {
                temp_value_2 = 0x0000;
                // Cycle counter filter value
                temp_value_1 |= (uint16)(Fr_receive_buffer_config_ptr->rx_cycle_counter_filter_value & 0x3F); 
                // Cycle counter filter mask
                temp_value_2 =  (uint16)(Fr_receive_buffer_config_ptr->rx_cycle_counter_filter_mask & 0x3F);
                temp_value_1 |= (temp_value_2 << 6);
                temp_value_1 |= FrMBCCFR_CCFE;  // Cycle counter filter enabled
            }
            Fr_CC_reg_ptr[FrMBCCFR0 + Fr_MB_registers_offset_add_temp] = temp_value_1; // Store configuration into MBCCFRn register
            
            // Configure MBFIDRn register
            Fr_CC_reg_ptr[FrMBFIDR0 + Fr_MB_registers_offset_add_temp] = Fr_receive_buffer_config_ptr->receive_frame_ID;
            
            // Store receive MB frame ID into internal MB information structure
            Fr_MB_information_internal[Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init].
                                        slot_number = Fr_receive_buffer_config_ptr->receive_frame_ID;

            
            // Calculate and configure data field offset 
            // According to a FlexRay Memory Layout descibed in FlexRay module documentation
            
            // Calculate the message buffer header 
            header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                            Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init * 5);
            
            // Check if the message buffer is in segment 1 or segment 2
            if(Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init < (Fr_HW_config_ptr->last_MB_seg_1 + 1))
            {   // Payload data of MB are in segment 1
                // Calculate valid Data Field Offset
                // (total_MB_number * 10) + (buffer_index_init * MB_segment_1_data_size * 2)
                temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                               (Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init * \
                               Fr_HW_config_ptr->MB_segment_1_data_size * 2);
            }
            else // Payload data of MB are in segment 2
            {
                // Calculate valid Data Field Offset
                // (total_MB_number * 10) + ((last_MB_seg_1 + 1) * MB_segment_1_data_size * 2) + \
                // ((buffer_index_init - (last_MB_seg_1 + 1)) * MB_segment_2_data_size * 2)
                temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                               ((Fr_HW_config_ptr->last_MB_seg_1 + 1) * Fr_HW_config_ptr->MB_segment_1_data_size * 2) + \
                               ((Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init - \
                               (Fr_HW_config_ptr->last_MB_seg_1 + 1)) * Fr_HW_config_ptr->MB_segment_2_data_size * 2);
            }
            header_MB_ptr[3] = temp_value_2;    // Store Data Field Offset to Frame header register
            
            // Message Buffer Index Registers initialization
            temp_value_1 = Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init;
            Fr_CC_reg_ptr[FrMBIDXR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
            
            // Store the highest number of message buffer index into variable for optimization verification
            if(highest_txrx_mb_number < temp_value_1){
                highest_txrx_mb_number = temp_value_1;      // Update variable with the highest index number
            }
            
            // Store MB type into internal MB information structure
            // Not necessary for the current version of the UNIFIED Driver
            //Fr_MB_information_internal[temp_value_1].buffer_type =  FR_RECEIVE_BUFFER;
            
            // Enable message buffer
            temp_value_1 = Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] | FrMBCCSR_EDT;
            Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
            
            break;
        case FR_TRANSMIT_BUFFER:        // Configure transmit MB
            Fr_transmit_buffer_config_ptr = ((Fr_transmit_buffer_config_type*)
                                            (Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_config_ptr));
            temp_value_1 = FrMBCCSR_EDT;    // Store trigger enable/disbale bit into variable
           
            // Disable appropriate MB
            if(Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] & FrMBCCSR_EDS)
            {
                Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
            }
            // Disable transmit side of the double buffered MB
            if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Disable appropriate MB
                if(Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp + 4] & FrMBCCSR_EDS)
                {
                    Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp + 4] = temp_value_1;
                }
            }
            
            // Configure MBCCSRn register
            temp_value_2 = FrMBCCSR_MTD;    // Configure MB as Transmit
            temp_value_3 = 0x0000;          // Clear temporary variable

            // Double buffered TX MB configured?
            if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                temp_value_2 |= FrMBCCSR_MBT;           // Double buffered transmit MB
                // Immediate commit mode configured? (only for double buffered TX MB
                if(Fr_transmit_buffer_config_ptr->transmission_commit_mode == FR_IMMEDIATE_COMMIT_MODE)
                {
                    temp_value_2 |= FrMBCCSR_MCM;           // Immediate commit mode
                }
                // Store transmit MB type into internal MB information structure
                Fr_MB_information_internal[Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init].
                                            transmission_mode =  FR_DOUBLE_TRANSMIT_BUFFER;
                Fr_MB_information_internal[Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init + 1].
                                            transmission_mode =  FR_DOUBLE_TRANSMIT_BUFFER;
            }
            else
            {
                // Store transmit MB type into internal MB information structure
                Fr_MB_information_internal[Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init].
                                            transmission_mode =  FR_SINGLE_TRANSMIT_BUFFER;
            }
            // Copy configuration into temp_value_3 variable            
            temp_value_3 = temp_value_2;    // Interrupt flag can be set for the commit or transmit side of the double buf.MB
            
            if(Fr_transmit_buffer_config_ptr->tx_MB_interrupt_enable)   // Interrupt generation configured?
            {
                if((Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER) && \
                                        Fr_transmit_buffer_config_ptr->tx_MB_interrupt_transmit_side_enable)
                {
                    // Interrupt flag is set for the transmit side of the double buf.MB, not for commit side
                    temp_value_2 |= FrMBCCSR_MBIE;           // Interrupt generation enabled
                }
                else
                {
                    // Interrupt flag is not set for the transmit side of the double buf.MB
                    temp_value_3 |= FrMBCCSR_MBIE;           // Interrupt generation enabled
                }
            }

            // Store configuration into the MBCCSRn register
            Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_3;
            // Store configuration into transmit side of the double buffered MB
            if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the MBCCSR[2n+1] register
                Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp + 4] = temp_value_2;
            }
            
            // Configure MBCCFRn register
            temp_value_1 = 0x0000;      // Clear variable
            if(Fr_transmit_buffer_config_ptr->transmit_channel_enable == FR_CHANNEL_A)
            {
                temp_value_1 = FrMBCCFR_CHA;    // Channel assigment - ch. A
            }
            else if(Fr_transmit_buffer_config_ptr->transmit_channel_enable == FR_CHANNEL_B)
            {
                temp_value_1 = FrMBCCFR_CHB;    // Channel assigment - ch. B
            }
            else if(Fr_transmit_buffer_config_ptr->transmit_channel_enable == FR_CHANNEL_AB)
            {
                temp_value_1 = (FrMBCCFR_CHA | FrMBCCFR_CHB);    // Channel assigment - ch. A
            }
            // Cycle counter filter settings
            if(Fr_transmit_buffer_config_ptr->tx_cycle_counter_filter_enable)    // Should be the cycle counter filter enabled?
            {
                temp_value_2 = 0x0000;
                // Cycle counter filter value
                temp_value_1 |= (uint16)(Fr_transmit_buffer_config_ptr->tx_cycle_counter_filter_value & 0x3F); 
                // Cycle counter filter mask
                temp_value_2 =  (uint16)(Fr_transmit_buffer_config_ptr->tx_cycle_counter_filter_mask & 0x3F);
                temp_value_1 |= (temp_value_2 << 6);
                temp_value_1 |= FrMBCCFR_CCFE;  // Cycle counter filter enabled
            }
            if(Fr_transmit_buffer_config_ptr->transmission_mode == FR_STATE_TRANSMISSION_MODE) // Which transmission mode?
            {
                temp_value_1 |= FrMBCCFR_MTM;   // State transmission mode is enabled
                // Store information into internal structure
                Fr_MB_information_internal[Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init].
                                            transmission_type =  FR_STATE_TRANSMISSION_MODE;
                if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
                {
                    // Store information also for transmit side of the double buffered MB
                    Fr_MB_information_internal[Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init + 1].
                                            transmission_type =  FR_STATE_TRANSMISSION_MODE;
                }                            
            }
            else
            {
               // Store information into internal structure
               Fr_MB_information_internal[Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init].
                                            transmission_type =  FR_EVENT_TRANSMISSION_MODE;
            }
            Fr_CC_reg_ptr[FrMBCCFR0 + Fr_MB_registers_offset_add_temp] = temp_value_1; // Store configuration into MBCCFRn register
            // Store configuration into transmit side of the double buffered MB
            if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the MBCCFR[2n+1] register
                Fr_CC_reg_ptr[FrMBCCFR0 + Fr_MB_registers_offset_add_temp + 4] = temp_value_1;
            }

            
            // Configure MBFIDRn register
            Fr_CC_reg_ptr[FrMBFIDR0 + Fr_MB_registers_offset_add_temp] = Fr_transmit_buffer_config_ptr->transmit_frame_ID;
            // Store configuration into transmit side of the double buffered MB
            if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the MBFIDR[2n+1] register
                Fr_CC_reg_ptr[FrMBFIDR0 + Fr_MB_registers_offset_add_temp + 4] = Fr_transmit_buffer_config_ptr->transmit_frame_ID;
            }

            
            // Store transmit MB frame ID into internal MB information structure
            Fr_MB_information_internal[Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init].
                                        slot_number = Fr_transmit_buffer_config_ptr->transmit_frame_ID;

            // Initialize the second part of double buffer if double buffered TX MB configured
            if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                Fr_MB_information_internal[Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init + 1].
                                        slot_number = Fr_transmit_buffer_config_ptr->transmit_frame_ID;
            }
          
            // Calculate and configure data field offset 
            // According to a FlexRay Memory Layout descibed in FlexRay module documentation
            
            // Calculate the message buffer header 
            header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                            Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init * 5);
    
            // Configure Frame Header registers
            temp_value_1 = 0x0000;          // Clear
            temp_value_1 |= (Fr_transmit_buffer_config_ptr->transmit_frame_ID & 0x07FF);    // Frame ID
            if(Fr_transmit_buffer_config_ptr->payload_preamble == TRUE)         // Payload preamble indicator configured?
            {
                temp_value_1 = FrF_HEADER_PPI;                                  // Configure Payload preamble indicator
            }
            header_MB_ptr[0] = temp_value_1;        // Store the first word of Frame Header Section
            // Store configuration into transmit side of the double buffered MB
            if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the first word of Frame Header Section
                header_MB_ptr[0+5] = temp_value_1;        // Store the first word of Frame Header Section
            }

            temp_value_2 = 0x0000;          // Clear
            temp_value_2 |= (Fr_transmit_buffer_config_ptr->payload_length & 0x007F);    // Payload length
            header_MB_ptr[1] = temp_value_2;        // Store the second word of Frame Header Section
            // Store configuration into transmit side of the double buffered MB
            if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the second word of Frame Header Section
                header_MB_ptr[1+5] = temp_value_2;        // Store the second word of Frame Header Section
            }


            temp_value_1 = 0x0000;          // Clear
            temp_value_1 |= (Fr_transmit_buffer_config_ptr->header_CRC & 0x07FF);       // Header CRC
            header_MB_ptr[2] = temp_value_1;        // Store the third word of Frame Header Section
            // Store configuration into transmit side of the double buffered MB
            if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the third word of Frame Header Section
                header_MB_ptr[2+5] = temp_value_1;        // Store the third word of Frame Header Section
            }

          
            // Check if the message buffer is in segment 1 or segment 2
            if(Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init < (Fr_HW_config_ptr->last_MB_seg_1 + 1))
            {   // Payload data of MB are in segment 1
                // Calculate valid Data Field Offset
                // (total_MB_number * 10) + (buffer_index_init * MB_segment_1_data_size * 2)
                temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                               (Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init * \
                               Fr_HW_config_ptr->MB_segment_1_data_size * 2);
            
                // Calculate the Data Field Offset for the transmit side of the double MB
                if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
                {
                    // Calculate valid Data Field Offset
                    // (total_MB_number * 10) + (buffer_index_init * MB_segment_1_data_size * 2) + 1 * MB_segment_1_data_size * 2
                    temp_value_1 = temp_value_2 + Fr_HW_config_ptr->MB_segment_1_data_size * 2;
                }
            }
            else // Payload data of MB are in segment 2
            {
                // Calculate valid Data Field Offset
                // (total_MB_number * 10) + ((last_MB_seg_1 + 1) * MB_segment_1_data_size * 2) + \
                // ((buffer_index_init - (last_MB_seg_1 + 1)) * MB_segment_2_data_size * 2)
                temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                               ((Fr_HW_config_ptr->last_MB_seg_1 + 1) * Fr_HW_config_ptr->MB_segment_1_data_size * 2) + \
                               ((Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init - \
                               (Fr_HW_config_ptr->last_MB_seg_1 + 1)) * Fr_HW_config_ptr->MB_segment_2_data_size * 2);
                // Calculate the Data Field Offset for the transmit side of the double MB
                if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
                {
                    // Calculate valid Data Field Offset
                    // (total_MB_number * 10) + ((last_MB_seg_1 + 1) * MB_segment_1_data_size * 2) + \
                    // ((buffer_index_init - (last_MB_seg_1 + 1)) * MB_segment_2_data_size * 2) + 1 * MB_segment_2_data_size * 2
                    temp_value_1 = temp_value_2 + Fr_HW_config_ptr->MB_segment_2_data_size * 2;
                }
            }
            header_MB_ptr[3] = temp_value_2;    // Store Data Field Offset to Frame header register
            // Store configuration into transmit side of the double buffered MB
            if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store the Data Field Offset to Frame Header Section
                header_MB_ptr[3+5] = temp_value_1;        // Store Data Field Offset to Frame header register
            }

 
            // Message Buffer Index Registers initialization
            temp_value_1 = Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init;
            Fr_CC_reg_ptr[FrMBIDXR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
            // Store configuration into transmit side of the double buffered MB

            // Store the highest number of message buffer index into variable for optimization verification
            if(highest_txrx_mb_number < temp_value_1)
            {
                highest_txrx_mb_number = temp_value_1;      // Update variable with the highest index number
            }

            if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the MBIDXR[2n+1] register
                Fr_CC_reg_ptr[FrMBIDXR0 + Fr_MB_registers_offset_add_temp + 4] = temp_value_1 + 1;
                
                // Store the highest number of message buffer index into variable for optimization verification
                if(highest_txrx_mb_number < (temp_value_1 + 1))
                {
                    highest_txrx_mb_number = (temp_value_1 + 1);      // Update variable with the highest index number
                }
            }
            
            // Store MB type into internal MB information structure
            // Not necessary for the current version of the UNIFIED Driver
            //Fr_MB_information_internal[temp_value_1].buffer_type =  FR_TRANSMIT_BUFFER;

            // Enable message buffer
            temp_value_1 = Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] | FrMBCCSR_EDT;
          
            // Initialize the second part of double buffer if double buffered TX MB configured
            if(Fr_transmit_buffer_config_ptr->transmit_MB_buffering == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Enable the MB - only in the transmit side of the double buffered - the MBCCSR[2n+1] register
                Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp + 4] = temp_value_1;
            }
            else
            {
                // Enable the MB in the MBCCSRn register
                Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
            }
            break;

        case FR_RECEIVE_FIFO:           // Configure receive FIFO storage
            Fr_FIFO_config_ptr = ((Fr_FIFO_config_type*)
                                 (Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_config_ptr));
            
            if(Fr_FIFO_config_ptr->FIFO_channel == FR_CHANNEL_A)    // FIFO A or FIFO B?
            {   // FIFO A
                Fr_CC_reg_ptr[FrRFSR] = 0x0000;                     // Set channel A in Receive FIFO Selection reg.

                // Calculate data field offset and configure MB Header Field reg.
                temp_value_1 = 0x0000;      // Clear
                // Calculate data field offset - for the first FIFO item
                if(Fr_FIFO_info.FIFO_1_used == FALSE)   // Has a FIFO storage already been configured?
                {                                       // No FIFO storage has been configured yet
                    // (total_MB_number * 10) + ((last_MB_seg_1 + 1) * MB_segment_1_data_size * 2) + \
                    // ((last_MB_util -last_MB_seg_1) * MB_segment_2_data_size * 2)
                    temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                                   ((Fr_HW_config_ptr->last_MB_seg_1 + 1) * Fr_HW_config_ptr->MB_segment_1_data_size * 2) + \
                                   ((Fr_HW_config_ptr->last_MB_util - Fr_HW_config_ptr->last_MB_seg_1) * \
                                   Fr_HW_config_ptr->MB_segment_2_data_size * 2);

                    // Update FIFO info structure
                    Fr_FIFO_info.FIFO_1_used = TRUE;
                    Fr_FIFO_info.FIFO_1_channel = FR_CHANNEL_A;
                    Fr_FIFO_info.FIFO_1_depth = Fr_FIFO_config_ptr->FIFO_depth;
                    Fr_FIFO_info.FIFO_1_entry_size = Fr_FIFO_config_ptr->FIFO_entry_size;
                }
                else                                    // One FIFO storage has already been configured
                {
                    // (total_MB_number * 10) + ((last_MB_seg_1 + 1) * MB_segment_1_data_size * 2) + \
                    // ((last_MB_util -last_MB_seg_1) * MB_segment_2_data_size * 2) + (FIFO_1_depth * FIFO_1_entry_size * 2)
                    temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                                   ((Fr_HW_config_ptr->last_MB_seg_1 + 1) * Fr_HW_config_ptr->MB_segment_1_data_size * 2) + \
                                   ((Fr_HW_config_ptr->last_MB_util - Fr_HW_config_ptr->last_MB_seg_1) * \
                                   Fr_HW_config_ptr->MB_segment_2_data_size * 2) + (Fr_FIFO_info.FIFO_1_depth * \
                                   Fr_FIFO_info.FIFO_1_entry_size * 2);

                    // Update FIFO info structure
                    Fr_FIFO_info.FIFO_2_used = TRUE;
                    Fr_FIFO_info.FIFO_2_channel = FR_CHANNEL_A;
                    Fr_FIFO_info.FIFO_2_depth = Fr_FIFO_config_ptr->FIFO_depth;
                    Fr_FIFO_info.FIFO_2_entry_size = Fr_FIFO_config_ptr->FIFO_entry_size;

                }
                
                
                // Configure Data Field Offset in all MB Header Fields intended for Receive FIFO buffers
                for(Fr_p = 0; Fr_p < Fr_FIFO_config_ptr->FIFO_depth; Fr_p++)   // Configure all MB Header Fields
                {
                    // Calculate Data Field Offset for current configured buffer
                    temp_value_1 = temp_value_2 + (Fr_p * Fr_FIFO_config_ptr->FIFO_entry_size * 2);
                    // Calculate the message buffer header 
                    header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                            ((Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init + Fr_p) * 5));
                    header_MB_ptr[3] = temp_value_1;    // Store Data Field Offset to Frame header register
                }
            }
            else
            {   // FIFO B
                Fr_CC_reg_ptr[FrRFSR] = 0x0001;                     // Set channel B in Receive FIFO Selection reg.

                // Calculate data field offset and configure MB Header Field reg.
                temp_value_1 = 0x0000;      // Clear

                // Calculate data field offset - for the first FIFO item
                if(Fr_FIFO_info.FIFO_1_used == FALSE)   // Has a FIFO storage already been configured?
                {                                       // No FIFO storage has been configured yet
                    // (total_MB_number * 10) + ((last_MB_seg_1 + 1) * MB_segment_1_data_size * 2) + \
                    // ((last_MB_util -last_MB_seg_1) * MB_segment_2_data_size * 2)
                    temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                                   ((Fr_HW_config_ptr->last_MB_seg_1 + 1) * Fr_HW_config_ptr->MB_segment_1_data_size * 2) + \
                                   ((Fr_HW_config_ptr->last_MB_util - Fr_HW_config_ptr->last_MB_seg_1) * \
                                   Fr_HW_config_ptr->MB_segment_2_data_size * 2);

                    // Update FIFO info structure
                    Fr_FIFO_info.FIFO_1_used = TRUE;
                    Fr_FIFO_info.FIFO_1_channel = FR_CHANNEL_B;
                    Fr_FIFO_info.FIFO_1_depth = Fr_FIFO_config_ptr->FIFO_depth;
                    Fr_FIFO_info.FIFO_1_entry_size = Fr_FIFO_config_ptr->FIFO_entry_size;
                }
                else                                    // One FIFO storage has already been configured
                {
                    // (total_MB_number * 10) + ((last_MB_seg_1 + 1) * MB_segment_1_data_size * 2) + \
                    // ((last_MB_util -last_MB_seg_1) * MB_segment_2_data_size * 2) + (FIFO_1_depth * FIFO_1_entry_size * 2)
                    temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                                   ((Fr_HW_config_ptr->last_MB_seg_1 + 1) * Fr_HW_config_ptr->MB_segment_1_data_size * 2) + \
                                   ((Fr_HW_config_ptr->last_MB_util - Fr_HW_config_ptr->last_MB_seg_1) * \
                                   Fr_HW_config_ptr->MB_segment_2_data_size * 2) + (Fr_FIFO_info.FIFO_1_depth * \
                                   Fr_FIFO_info.FIFO_1_entry_size * 2);

                    // Update FIFO info structure
                    Fr_FIFO_info.FIFO_2_used = TRUE;
                    Fr_FIFO_info.FIFO_2_channel = FR_CHANNEL_B;
                    Fr_FIFO_info.FIFO_2_depth = Fr_FIFO_config_ptr->FIFO_depth;
                    Fr_FIFO_info.FIFO_2_entry_size = Fr_FIFO_config_ptr->FIFO_entry_size;
                }
                
                // Configure Data Field Offset in all MB Header Fields intended for Receive FIFO buffers
                for(Fr_p = 0; Fr_p < Fr_FIFO_config_ptr->FIFO_depth; Fr_p++)   // Configure all MB Header Fields
                {
                    // Calculate Data Field Offset for current configured buffer
                    temp_value_1 = temp_value_2 + (Fr_p * Fr_FIFO_config_ptr->FIFO_entry_size * 2);
                    // Calculate the message buffer header 
                    header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                            ((Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init + Fr_p) * 5));
                    header_MB_ptr[3] = temp_value_1;    // Store Data Field Offset to Frame header register
                }
            }
            
            // Configure Receive FIFO start index reg.
            temp_value_2 = 0x0000;      // Clear
            temp_value_2 |= (Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init & 0x03FF); // Load start index
            Fr_CC_reg_ptr[FrRFSIR] = temp_value_2;  // Store start index
            
            // Configure FIFO Depth and Size reg.
            temp_value_1 = 0x0000;      // Clear variable
            temp_value_1 |= (Fr_FIFO_config_ptr->FIFO_entry_size & 0x7F);   // Entry size [Words]
            temp_value_1 |= (Fr_FIFO_config_ptr->FIFO_depth << 8);          // FIFO Depth
            Fr_CC_reg_ptr[FrRFDSR] = temp_value_1;                          // Store FIFO entry size and depth
            
            // Configure FIFO Message ID Acceptance Filter Value
            Fr_CC_reg_ptr[FrRFMIDAFVR] = Fr_FIFO_config_ptr->FIFO_message_ID_acceptance_filter_value;   // Store filter value
            
            // Configure FIFO Message ID Acceptance Filter Mask
            Fr_CC_reg_ptr[FrRFMIAFMR] = Fr_FIFO_config_ptr->FIFO_message_ID_acceptance_filter_mask;     // Store filter mask
            
            // Configure FIFO Frame ID Rejection Filter Value
            temp_value_2 = 0x0000;      // Clear
            temp_value_2 |= (Fr_FIFO_config_ptr->FIFO_frame_ID_rejection_filter_value & 0x07FF);    // Rejection filter value
            Fr_CC_reg_ptr[FrRFFIDRFVR] = temp_value_2;                      // Store filter value
            
            // Configure FIFO Frame ID Rejection Filter Mask
            temp_value_1 = 0x0000;      // Clear
            temp_value_1 |= (Fr_FIFO_config_ptr->FIFO_frame_ID_rejection_filter_mask & 0x07FF);    // Rejection filter mask
            Fr_CC_reg_ptr[FrRFFIDRFMR] = temp_value_1;                      // Store filter mask
            
            // Configure FIFO Interrupt
            if(Fr_FIFO_config_ptr->FIFO_interrupt_enable)                   // FIFO interrupt should be configured?
            {
                if(Fr_FIFO_config_ptr->FIFO_channel == FR_CHANNEL_A)        // Which FIFO channel
                {
                    // Enable interrupt for FIFO channel A in the GIFER reg.
                    temp_value_2 = 0x0000;                  // Clear
                    temp_value_2 |= Fr_CC_reg_ptr[FrGIFER]; // Load content of the GIFER reg.
                    temp_value_2 |= FrGIFER_FNEAIE;         // FNEAIE bit - enable interrupt line
                    Fr_CC_reg_ptr[FrGIFER] = temp_value_2;  // Store settings
                }
                else
                {
                    // Enable interrupt for FIFO channel B in the GIFER reg.
                    temp_value_2 = 0x0000;                  // Clear
                    temp_value_2 |= Fr_CC_reg_ptr[FrGIFER]; // Load content of the GIFER reg.
                    temp_value_2 |= FrGIFER_FNEBIE;         // FNEBIE bit - enable interrupt line
                    Fr_CC_reg_ptr[FrGIFER] = temp_value_2;  // Store settings
                }
            }

            // Configure Range Filters
            // Range filter 0
            if(Fr_FIFO_config_ptr->FIFO_range_filters_config[0].range_filter_enable)  // Range filter 0 enabled?
            {
                temp_value_1 = 0x0000;                      // Clear + set filter Selector - filter 0
                // Set Lower Interval Boundary Slot ID
                temp_value_1 |= (Fr_FIFO_config_ptr->FIFO_range_filters_config[0].range_filter_lower_interval & 0x07FF);
                Fr_CC_reg_ptr[FrRFRFCFR] = temp_value_1;    // Store Range filter 0 settings
            
                temp_value_2 = 0x0000;                      // Clear + set filter Selector - filter 0
                temp_value_2 |= FrRFRFCFR_IBD;              // Program upper interval boundary
                // Set Upper Interval Boundary Slot ID
                temp_value_2 |= (Fr_FIFO_config_ptr->FIFO_range_filters_config[0].range_filter_upper_interval & 0x07FF);  
                Fr_CC_reg_ptr[FrRFRFCFR] = temp_value_2;    // Store Range filter 0 settings
            }
        
            // Range filter 1
            if(Fr_FIFO_config_ptr->FIFO_range_filters_config[1].range_filter_enable)  // Range filter 1 enabled?
            {
                temp_value_1 = 0x0000;                      // Clear
                temp_value_1 |= 0x1000;                     // Set Filter SELector - filter 1
                // Set Lower Interval Boundary Slot ID
                temp_value_1 |= (Fr_FIFO_config_ptr->FIFO_range_filters_config[1].range_filter_lower_interval & 0x07FF);
                Fr_CC_reg_ptr[FrRFRFCFR] = temp_value_1;    // Store Range filter 1 settings
            
                temp_value_2 = 0x0000;                      // Clear
                temp_value_2 |= 0x1000;                     // Set Filter SELector - filter 1
                temp_value_2 |= FrRFRFCFR_IBD;              // Program upper interval boundary
                // Set Upper Interval Boundary Slot ID
                temp_value_2 |= (Fr_FIFO_config_ptr->FIFO_range_filters_config[1].range_filter_upper_interval & 0x07FF);
                Fr_CC_reg_ptr[FrRFRFCFR] = temp_value_2;    // Store Range filter 1 settings
            }
            
            // Range filter 2
            if(Fr_FIFO_config_ptr->FIFO_range_filters_config[2].range_filter_enable)  // Range filter 2 enabled?
            {
                temp_value_1 = 0x0000;                      // Clear
                temp_value_1 |= 0x2000;                     // Set Filter SELector - filter 2
                // Set Lower Interval Boundary Slot ID
                temp_value_1 |= (Fr_FIFO_config_ptr->FIFO_range_filters_config[2].range_filter_lower_interval & 0x07FF);
                Fr_CC_reg_ptr[FrRFRFCFR] = temp_value_1;    // Store Range filter 2 settings
            
                temp_value_2 = 0x0000;                      // Clear
                temp_value_2 |= 0x2000;                     // Set Filter SELector - filter 2
                temp_value_2 |= FrRFRFCFR_IBD;              // Program upper interval boundary
                // Set Upper Interval Boundary Slot ID
                temp_value_2 |= (Fr_FIFO_config_ptr->FIFO_range_filters_config[2].range_filter_upper_interval & 0x07FF);
                Fr_CC_reg_ptr[FrRFRFCFR] = temp_value_2;    // Store Range filter 2 settings
            }

            // Range filter 3
            if(Fr_FIFO_config_ptr->FIFO_range_filters_config[3].range_filter_enable)  // Range filter 3 enabled?
            {
                temp_value_1 = 0x0000;                      // Clear
                temp_value_1 |= 0x3000;                     // Set Filter SELector - filter 3
                // Set Lower Interval Boundary Slot ID
                temp_value_1 |= (Fr_FIFO_config_ptr->FIFO_range_filters_config[3].range_filter_lower_interval & 0x07FF);
                Fr_CC_reg_ptr[FrRFRFCFR] = temp_value_1;    // Store Range filter 3 settings
            
                temp_value_2 = 0x0000;                      // Clear
                temp_value_2 |= 0x3000;                     // Set Filter SELector - filter 3
                temp_value_2 |= FrRFRFCFR_IBD;              // Program upper interval boundary
                // Set Upper Interval Boundary Slot ID
                temp_value_2 |= (Fr_FIFO_config_ptr->FIFO_range_filters_config[3].range_filter_upper_interval & 0x07FF);
                Fr_CC_reg_ptr[FrRFRFCFR] = temp_value_2;    // Store Range filter 3 settings
            }

            // Configure the FIFO Range Filter Control reg.
            temp_value_1 = 0x0000;                          // Clear
            if(Fr_FIFO_config_ptr->FIFO_range_filters_config[0].range_filter_enable)  // Range filter 0 enabled?
            {
                temp_value_1 |= FrRFRFCTR_F0EN;             // Enable Range Filter 0
            }
            if(Fr_FIFO_config_ptr->FIFO_range_filters_config[1].range_filter_enable)  // Range filter 1 enabled?
            {
                temp_value_1 |= FrRFRFCTR_F1EN;             // Enable Range Filter 1
            }
            if(Fr_FIFO_config_ptr->FIFO_range_filters_config[2].range_filter_enable)  // Range filter 2 enabled?
            {
                temp_value_1 |= FrRFRFCTR_F2EN;             // Enable Range Filter 2
            }
            if(Fr_FIFO_config_ptr->FIFO_range_filters_config[3].range_filter_enable)  // Range filter 3 enabled?
            {
                temp_value_1 |= FrRFRFCTR_F3EN;             // Enable Range Filter 3
            }
            
            if(Fr_FIFO_config_ptr->FIFO_range_filters_config[0].range_filter_mode == FR_REJECTION)  // Filter 0 as rejection?
            {
                temp_value_1 |= FrRFRFCTR_F0MD;             // Range filter 0 as rejection filter
            }
            if(Fr_FIFO_config_ptr->FIFO_range_filters_config[1].range_filter_mode == FR_REJECTION)  // Filter 1 as rejection?
            {
                temp_value_1 |= FrRFRFCTR_F1MD;             // Range filter 1 as rejection filter
            }
            if(Fr_FIFO_config_ptr->FIFO_range_filters_config[2].range_filter_mode == FR_REJECTION)  // Filter 2 as rejection?
            {
                temp_value_1 |= FrRFRFCTR_F2MD;             // Range filter 2 as rejection filter
            }
            if(Fr_FIFO_config_ptr->FIFO_range_filters_config[3].range_filter_mode == FR_REJECTION)  // Filter 3 as rejection?
            {
                temp_value_1 |= FrRFRFCTR_F3MD;             // Range filter 3 as rejection filter
            }
            // Write configuration settings to the FIFO Range Filter Control reg.
            Fr_CC_reg_ptr[FrRFRFCTR] = temp_value_1;

            break;
           
        case FR_RECEIVE_SHADOW:         // Configure receive shadow buffers
            Fr_shadow_buffer_config_ptr = ((Fr_receive_shadow_buffers_config_type*)
                                (Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_config_ptr));

            // Configuration of receive shadow buffer for channel A and segment 1
            temp_value_1 = 0x0000;      // Clear
            if(Fr_shadow_buffer_config_ptr->RSBIR_A1_enable)    // Shadow buffer for cha A and seg 1 enabled?
            {   // Enabled
                // Configure RSBIR reg.
                temp_value_1 = (uint16)(Fr_shadow_buffer_config_ptr->RSBIR_A1_buffer_number_init);    // Store buffer index
                Fr_CC_reg_ptr[FrRSBIR] = temp_value_1;      // Store settings for shadow buffer, ch A, seg 1
                
                // Calculate the message buffer header 
                header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                                (uint16)(Fr_shadow_buffer_config_ptr->RSBIR_A1_buffer_number_init) * 5);

                // Configure MB header field reg.
                // Calculate valid Data Field Offset
                // (total_MB_number * 10) + (x_buffer_number_init * MB_segment_1_data_size * 2)
                temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                               ((uint16)(Fr_shadow_buffer_config_ptr->RSBIR_A1_buffer_number_init) * \
                               Fr_HW_config_ptr->MB_segment_1_data_size * 2);
                header_MB_ptr[3] = temp_value_2;    // Store Data Field Offset to Frame header register
            }
            
            // Configuration of receive shadow buffer for channel A and segment 2
            temp_value_1 = 0x0000;      // Clear
            if(Fr_shadow_buffer_config_ptr->RSBIR_A2_enable)    // Shadow buffer for cha A and seg 2 enabled?
            {   // Enabled
                // Configure RSBIR reg.
                temp_value_1 = (uint16)(Fr_shadow_buffer_config_ptr->RSBIR_A2_buffer_number_init);    // Store buffer index
                temp_value_1 |= 0x1000;                     // Selector field - ch A, seg 2
                Fr_CC_reg_ptr[FrRSBIR] = temp_value_1;      // Store settings for shadow buffer, ch A, seg 2
                
                // Calculate the message buffer header 
                header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                                (uint16)(Fr_shadow_buffer_config_ptr->RSBIR_A2_buffer_number_init) * 5);

                // Configure MB header field reg.
                // Calculate valid Data Field Offset
                // (total_MB_number * 10) + ((last_MB_seg_1 + 1) * MB_segment_1_data_size * 2) + \
                // ((x_buffer_number_init - (last_MB_seg_1 + 1)) * MB_segment_2_data_size * 2)
                temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                               ((Fr_HW_config_ptr->last_MB_seg_1 + 1) * Fr_HW_config_ptr->MB_segment_1_data_size * 2) + \
                               (((uint16)(Fr_shadow_buffer_config_ptr->RSBIR_A2_buffer_number_init) - \
                               (Fr_HW_config_ptr->last_MB_seg_1 + 1)) * Fr_HW_config_ptr->MB_segment_2_data_size * 2);
                header_MB_ptr[3] = temp_value_2;    // Store Data Field Offset to Frame header register
            }
            
            // Configuration of receive shadow buffer for channel B and segment 1
            temp_value_1 = 0x0000;      // Clear
            if(Fr_shadow_buffer_config_ptr->RSBIR_B1_enable)    // Shadow buffer for cha B and seg 1 enabled?
            {   // Enabled
                // Configure RSBIR reg.
                temp_value_1 = (uint16)(Fr_shadow_buffer_config_ptr->RSBIR_B1_buffer_number_init);    // Store buffer index
                temp_value_1 |= 0x2000;                     // Selector field - ch B, seg 1
                Fr_CC_reg_ptr[FrRSBIR] = temp_value_1;      // Store settings for shadow buffer, ch B, seg 1
                
                // Calculate the message buffer header 
                header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                                (uint16)(Fr_shadow_buffer_config_ptr->RSBIR_B1_buffer_number_init) * 5);

                // Configure MB header field reg.
                // Calculate valid Data Field Offset
                // (total_MB_number * 10) + (x_buffer_number_init * MB_segment_1_data_size * 2)
                temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                               ((uint16)(Fr_shadow_buffer_config_ptr->RSBIR_B1_buffer_number_init) * \
                               Fr_HW_config_ptr->MB_segment_1_data_size * 2);
                header_MB_ptr[3] = temp_value_2;    // Store Data Field Offset to Frame header register
            }
            
            // Configuration of receive shadow buffer for channel B and segment 2
            temp_value_1 = 0x0000;      // Clear
            if(Fr_shadow_buffer_config_ptr->RSBIR_B2_enable)    // Shadow buffer for cha B and seg 2 enabled?
            {   // Enabled
                // Configure RSBIR reg.
                temp_value_1 = (uint16)(Fr_shadow_buffer_config_ptr->RSBIR_B2_buffer_number_init);    // Store buffer index
                temp_value_1 |= 0x3000;                     // Selector field - ch B, seg 2
                Fr_CC_reg_ptr[FrRSBIR] = temp_value_1;      // Store settings for shadow buffer, ch B, seg 2
                
                // Calculate the message buffer header 
                header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                                (uint16)(Fr_shadow_buffer_config_ptr->RSBIR_B2_buffer_number_init) * 5);

                // Configure MB header field reg.
                // Calculate valid Data Field Offset
                // (total_MB_number * 10) + ((last_MB_seg_1 + 1) * MB_segment_1_data_size * 2) + \
                // ((x_buffer_number_init - (last_MB_seg_1 + 1)) * MB_segment_2_data_size * 2)
                temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                               ((Fr_HW_config_ptr->last_MB_seg_1 + 1) * Fr_HW_config_ptr->MB_segment_1_data_size * 2) + \
                               (((uint16)(Fr_shadow_buffer_config_ptr->RSBIR_B2_buffer_number_init) - \
                               (Fr_HW_config_ptr->last_MB_seg_1 + 1)) * Fr_HW_config_ptr->MB_segment_2_data_size * 2);
                header_MB_ptr[3] = temp_value_2;    // Store Data Field Offset to Frame header register
            }
            
            break;
        default:
            break;
        }
        temp_index++;   // Increment 
    }
    
    // Optimization verification
    // Verification whether the highest defined transmit or receive message buffer index 
    // (by FR_NUMBER_TXRX_MB parameter in the Fr_UNIFIED_cfg.h file)
    // does not exceed actually configured message buffer index for some message buffer (without shadow and FIFO)
    #ifdef FR_NUMBER_TXRX_MB
    /* The FR_NUMBER_TXRX_MB parameter is defined in the Fr_UNIFIED_cfg.h file 
    - memory consumption optimization is applied */
        if(highest_txrx_mb_number > FR_NUMBER_TXRX_MB)
        {   // The configured transmit or receive MB index is greater than it is set in the FR_NUMBER_TXRX_MB parameter
            return FR_NOT_SUCCESS;                  // Error - the value of the FR_NUMBER_TXRX_MB parameter is too low
        }
    #endif
    return FR_SUCCESS;                                      // API call has been successful
}



































//×Ô¶¨ÒåµÄ³õÊ¼»¯
Fr_return_type Fr_buffers_init_custom(T_slot_conf *slot,T_MB_Index *mb_index)
{
    volatile uint8 temp_index;                  // Temporary index used for indexing of the buffer config set structure
    volatile uint8 highest_txrx_mb_number = 0;  // The highest number of configured transmit or receive MB index -for verification
                              // whether the FR_NUMBER_TXRX_MB parameter is correctly set in the Fr_UNIFIED_cfg.h file
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation
    volatile uint16 temp_value_2;               // Temporary variable used for bit operation
    volatile uint16 temp_value_3;               // Temporary variable used for bit operation - for double buffered MB
    volatile uint16 Fr_buffer_info_set_index;   // Item index in array referenced by Fr_buffer_config_set_temp_ptr pointer
    volatile uint16 Fr_MB_registers_offset_add_temp; // Temporary offset adress of message buffer registers
    T_slot_conf *  slot_info;                                                    // configuration data
    Fr_FIFO_config_type * Fr_FIFO_config_ptr;   // Temporary pointer to receive FIFO configuration data
    volatile uint16 * FR_DATA_FAR header_MB_ptr; // Message buffer header pointer
    uint16 Fr_p;                                // Temporary counter used for FIFO configuration
    

    // Initialization of the internal FIFO information structure
    Fr_FIFO_info.FIFO_1_used = FALSE;
    Fr_FIFO_info.FIFO_1_channel = FR_NO_CHANNEL;
    Fr_FIFO_info.FIFO_1_depth = 0;
    Fr_FIFO_info.FIFO_1_entry_size = 0;
    Fr_FIFO_info.FIFO_2_used = FALSE;
    Fr_FIFO_info.FIFO_2_channel = FR_NO_CHANNEL;
    Fr_FIFO_info.FIFO_2_depth = 0;
    Fr_FIFO_info.FIFO_2_entry_size = 0;

    // Initialization of the internal calback function structure - mainly used in Fr_interrupt_handler() function
    Fr_callback_functions.Fr_module_ptr = NULL;
    Fr_callback_functions.Fr_protocol_ptr = NULL;
    Fr_callback_functions.Fr_chi_ptr = NULL;
    Fr_callback_functions.Fr_wakeup_ptr = NULL;
    Fr_callback_functions.Fr_fifo_B_ptr = NULL;
    Fr_callback_functions.Fr_fifo_A_ptr = NULL;
    Fr_callback_functions.Fr_fatal_protocol_error_ptr = NULL;
    Fr_callback_functions.Fr_internal_protocol_error_ptr = NULL;
    Fr_callback_functions.Fr_illegal_protocol_conf_ptr = NULL; 
    Fr_callback_functions.Fr_coldstart_abort_ptr = NULL;
    Fr_callback_functions.Fr_missing_rate_corr_ptr = NULL;
    Fr_callback_functions.Fr_missing_offset_corr_ptr = NULL;
    Fr_callback_functions.Fr_clock_corr_limit_ptr = NULL;
    Fr_callback_functions.Fr_max_sync_frames_ptr = NULL;
    Fr_callback_functions.Fr_mts_received_ptr = NULL;  
    Fr_callback_functions.Fr_violation_B_ptr = NULL;   
    Fr_callback_functions.Fr_violation_A_ptr = NULL;  
    Fr_callback_functions.Fr_trans_across_boundary_B_ptr = NULL;
    Fr_callback_functions.Fr_trans_across_boundary_A_ptr = NULL;
    Fr_callback_functions.Fr_timer_2_expired_ptr = NULL;
    Fr_callback_functions.Fr_timer_1_expired_ptr = NULL;
    Fr_callback_functions.Fr_cycle_start_ptr = NULL;  
    Fr_callback_functions.Fr_error_mode_changed_ptr = NULL;  
    Fr_callback_functions.Fr_illegal_protocol_command_ptr = NULL;   
    Fr_callback_functions.Fr_protocol_engine_comm_failure_ptr = NULL;  
    Fr_callback_functions.Fr_protocol_state_changed_ptr = NULL;
    Fr_callback_functions.Fr_slot_status_counter_3_inc_ptr = NULL;
    Fr_callback_functions.Fr_slot_status_counter_2_inc_ptr = NULL;
    Fr_callback_functions.Fr_slot_status_counter_1_inc_ptr = NULL;
    Fr_callback_functions.Fr_slot_status_counter_0_inc_ptr = NULL;  
    Fr_callback_functions.Fr_even_cycle_table_written_ptr = NULL;
    Fr_callback_functions.Fr_odd_cycle_table_written_ptr = NULL;

    temp_index = 0;     // Clear index
    
   
    while(mb_index[temp_index].mb_index != FR_LAST_MB&&temp_index<32)
    {   
        printf("mb_index=%d inde=%x\r\n",temp_index,mb_index[temp_index].mb_index);
        // Configure all required MBs
        Fr_buffer_info_set_index = mb_index[temp_index].slot;    // Store configuration index
        // Temporary offset address of MB registers
        Fr_MB_registers_offset_add_temp = mb_index[temp_index].mb_index * 4;
        
        switch(slot[Fr_buffer_info_set_index].buffer_type)
        {
        case FR_RECEIVE_BUFFER:         // Configure receive MB
             slot_info =&slot[Fr_buffer_info_set_index];

            // Disable appropriate MB
            if(Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] & FrMBCCSR_EDS) {
                temp_value_1 = FrMBCCSR_EDT;
                Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
            }
            
            // Configure MBCCSRn register
            temp_value_2 = 0x0000;      // Clear variable
            
            //½ÓÊÕÖÐ¶Ï
            temp_value_2 = FrMBCCSR_MBIE;            // Interrupt generation enabled
            
            // Configure MB as Receive MB and enable interrupt if required
            Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_2;
            
            // Configure MBCCFRn register
            temp_value_1 = 0x0000;      // Clear variable
            if(slot_info->receive_channel == FR_CHANNEL_A)
            {
                temp_value_1 = FrMBCCFR_CHA;    // Channel assigment - ch. A
            }
            else if(slot_info->receive_channel == FR_CHANNEL_B)
            {
                temp_value_1 = FrMBCCFR_CHB;    // Channel assigment - ch. B
            }
            else if(slot_info->receive_channel == FR_CHANNEL_AB)
            {
                temp_value_1 = (FrMBCCFR_CHA | FrMBCCFR_CHB);    // Channel assigment - ch. A
            }
        
            // Cycle counter filter settings
            if((slot_info->base_circle!=0)&&(slot_info->base_circle_interval!=0))    // Should be the cycle counter filter enabled?
            {
                temp_value_2 = 0x0000;
                // Cycle counter filter value
                temp_value_1 |= (uint16)(slot_info->base_circle & 0x3F); 
                // Cycle counter filter mask
                temp_value_2 =  (uint16)(slot_info->base_circle_interval & 0x3F);
                temp_value_1 |= (temp_value_2 << 6);
                temp_value_1 |= FrMBCCFR_CCFE;  // Cycle counter filter enabled
            }
            Fr_CC_reg_ptr[FrMBCCFR0 + Fr_MB_registers_offset_add_temp] = temp_value_1; // Store configuration into MBCCFRn register
            
            // Configure MBFIDRn register
            Fr_CC_reg_ptr[FrMBFIDR0 + Fr_MB_registers_offset_add_temp] = slot_info->frame_ID;
            
            // Store receive MB frame ID into internal MB information structure
            Fr_MB_information_internal[mb_index[temp_index].mb_index].
                                        slot_number = slot_info->frame_ID;

            
            // Calculate and configure data field offset 
            // According to a FlexRay Memory Layout descibed in FlexRay module documentation
            
            // Calculate the message buffer header 
            header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                            mb_index[temp_index].mb_index * 5);
            
            // Check if the message buffer is in segment 1 or segment 2
            if(mb_index[temp_index].mb_index < (Fr_HW_config_ptr->last_MB_seg_1 + 1))
            {   // Payload data of MB are in segment 1
                // Calculate valid Data Field Offset
                // (total_MB_number * 10) + (buffer_index_init * MB_segment_1_data_size * 2)
                temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                               (mb_index[temp_index].mb_index * \
                               Fr_HW_config_ptr->MB_segment_1_data_size * 2);
            }
            else // Payload data of MB are in segment 2
            {
                // Calculate valid Data Field Offset
                // (total_MB_number * 10) + ((last_MB_seg_1 + 1) * MB_segment_1_data_size * 2) + \
                // ((buffer_index_init - (last_MB_seg_1 + 1)) * MB_segment_2_data_size * 2)
                temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                               ((Fr_HW_config_ptr->last_MB_seg_1 + 1) * Fr_HW_config_ptr->MB_segment_1_data_size * 2) + \
                               ((mb_index[temp_index].mb_index - \
                               (Fr_HW_config_ptr->last_MB_seg_1 + 1)) * Fr_HW_config_ptr->MB_segment_2_data_size * 2);
            }
            header_MB_ptr[3] = temp_value_2;    // Store Data Field Offset to Frame header register
            
            // Message Buffer Index Registers initialization
            temp_value_1 = mb_index[temp_index].mb_index;
            Fr_CC_reg_ptr[FrMBIDXR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
            
            // Store the highest number of message buffer index into variable for optimization verification
            if(highest_txrx_mb_number < temp_value_1){
                highest_txrx_mb_number = temp_value_1;      // Update variable with the highest index number
            }
            
            // Store MB type into internal MB information structure
            // Not necessary for the current version of the UNIFIED Driver
            //Fr_MB_information_internal[temp_value_1].buffer_type =  FR_RECEIVE_BUFFER;
            
            // Enable message buffer
            temp_value_1 = Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] | FrMBCCSR_EDT;
            Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
            
            break;
        case FR_TRANSMIT_BUFFER:        // Configure transmit MB
        
             slot_info =&slot[Fr_buffer_info_set_index];
            
            temp_value_1 = FrMBCCSR_EDT;    // Store trigger enable/disbale bit into variable
           
            // Disable appropriate MB
            if(Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] & FrMBCCSR_EDS)
            {
                Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
            }
            // Disable transmit side of the double buffered MB
            if(slot_info->transmit_type== FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Disable appropriate MB
                if(Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp + 4] & FrMBCCSR_EDS)
                {
                    Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp + 4] = temp_value_1;
                }
            }
            
            // Configure MBCCSRn register
            temp_value_2 = FrMBCCSR_MTD;    // Configure MB as Transmit
            temp_value_3 = 0x0000;          // Clear temporary variable

            // Double buffered TX MB configured?
            if(slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                temp_value_2 |= FrMBCCSR_MBT;           // Double buffered transmit MB
                // Immediate commit mode configured? (only for double buffered TX MB
                if(slot_info->transmission_commit_mode == FR_IMMEDIATE_COMMIT_MODE)
                {
                    temp_value_2 |= FrMBCCSR_MCM;           // Immediate commit mode
                }
                // Store transmit MB type into internal MB information structure
                Fr_MB_information_internal[mb_index[temp_index].mb_index].
                                            transmission_mode =  FR_DOUBLE_TRANSMIT_BUFFER;
                Fr_MB_information_internal[mb_index[temp_index].mb_index + 1].
                                            transmission_mode =  FR_DOUBLE_TRANSMIT_BUFFER;
            }
            else
            {
                // Store transmit MB type into internal MB information structure
                Fr_MB_information_internal[mb_index[temp_index].mb_index].
                                            transmission_mode =  FR_SINGLE_TRANSMIT_BUFFER;
            }
            // Copy configuration into temp_value_3 variable            
            temp_value_3 = temp_value_2;    // Interrupt flag can be set for the commit or transmit side of the double buf.MB
            
            if(TRUE)   // Interrupt generation configured?
            {
                if((slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER))
                {
                    // Interrupt flag is set for the transmit side of the double buf.MB, not for commit side
                    temp_value_2 |= FrMBCCSR_MBIE;           // Interrupt generation enabled
                }
                else
                {
                    // Interrupt flag is not set for the transmit side of the double buf.MB
                    temp_value_3 |= FrMBCCSR_MBIE;           // Interrupt generation enabled
                }
            }

            // Store configuration into the MBCCSRn register
            Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_3;
            // Store configuration into transmit side of the double buffered MB
            if(slot_info->transmit_type== FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the MBCCSR[2n+1] register
                Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp + 4] = temp_value_2;
            }
            
            // Configure MBCCFRn register
            temp_value_1 = 0x0000;      // Clear variable
            if(slot_info->transmit_channel== FR_CHANNEL_A)
            {
                temp_value_1 = FrMBCCFR_CHA;    // Channel assigment - ch. A
            }
            else if(slot_info->transmit_channel == FR_CHANNEL_B)
            {
                temp_value_1 = FrMBCCFR_CHB;    // Channel assigment - ch. B
            }
            else if(slot_info->transmit_channel == FR_CHANNEL_AB)
            {
                temp_value_1 = (FrMBCCFR_CHA | FrMBCCFR_CHB);    // Channel assigment - ch. A
            }
            // Cycle counter filter settings
            if((slot_info->base_circle !=0)&&(slot_info->base_circle_interval !=0))    // Should be the cycle counter filter enabled?
            {
                temp_value_2 = 0x0000;
                // Cycle counter filter value
                temp_value_1 |= (uint16)(slot_info->base_circle & 0x3F); 
                // Cycle counter filter mask
                temp_value_2 =  (uint16)(slot_info->base_circle_interval & 0x3F);
                temp_value_1 |= (temp_value_2 << 6);
                temp_value_1 |= FrMBCCFR_CCFE;  // Cycle counter filter enabled
            }
            if(slot_info->transmission_mode == FR_STATE_TRANSMISSION_MODE) // Which transmission mode?
            {
                temp_value_1 |= FrMBCCFR_MTM;   // State transmission mode is enabled
                // Store information into internal structure
                Fr_MB_information_internal[mb_index[temp_index].mb_index].
                                            transmission_type =  FR_STATE_TRANSMISSION_MODE;
                if(slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER)
                {
                    // Store information also for transmit side of the double buffered MB
                    Fr_MB_information_internal[mb_index[temp_index].mb_index + 1].
                                            transmission_type =  FR_STATE_TRANSMISSION_MODE;
                }                            
            }
            else
            {
               // Store information into internal structure
               Fr_MB_information_internal[mb_index[temp_index].mb_index].
                                            transmission_type =  FR_EVENT_TRANSMISSION_MODE;
            }
            Fr_CC_reg_ptr[FrMBCCFR0 + Fr_MB_registers_offset_add_temp] = temp_value_1; // Store configuration into MBCCFRn register
            // Store configuration into transmit side of the double buffered MB
            if(slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the MBCCFR[2n+1] register
                Fr_CC_reg_ptr[FrMBCCFR0 + Fr_MB_registers_offset_add_temp + 4] = temp_value_1;
            }

            
            // Configure MBFIDRn register
            Fr_CC_reg_ptr[FrMBFIDR0 + Fr_MB_registers_offset_add_temp] = slot_info->frame_ID;
            // Store configuration into transmit side of the double buffered MB
            if(slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the MBFIDR[2n+1] register
                Fr_CC_reg_ptr[FrMBFIDR0 + Fr_MB_registers_offset_add_temp + 4] = slot_info->frame_ID;
            }

            
            // Store transmit MB frame ID into internal MB information structure
            Fr_MB_information_internal[mb_index[temp_index].mb_index].
                                        slot_number = slot_info->frame_ID;

            // Initialize the second part of double buffer if double buffered TX MB configured
            if(slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                Fr_MB_information_internal[mb_index[temp_index].mb_index  + 1].
                                        slot_number = slot_info->frame_ID;
            }
          
            // Calculate and configure data field offset 
            // According to a FlexRay Memory Layout descibed in FlexRay module documentation
            
            // Calculate the message buffer header 
            header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                            mb_index[temp_index].mb_index * 5);
    
            // Configure Frame Header registers
            temp_value_1 = 0x0000;          // Clear
            temp_value_1 |= (slot_info->frame_ID & 0x07FF);    // Frame ID
            if(FALSE)         // Payload preamble indicator configured?
            {
                temp_value_1 = FrF_HEADER_PPI;                                  // Configure Payload preamble indicator
            }
            header_MB_ptr[0] = temp_value_1;        // Store the first word of Frame Header Section
            // Store configuration into transmit side of the double buffered MB
            if(slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the first word of Frame Header Section
                header_MB_ptr[0+5] = temp_value_1;        // Store the first word of Frame Header Section
            }

            temp_value_2 = 0x0000;          // Clear
            temp_value_2 |= (slot_info->payload_length & 0x007F);    // Payload length
            header_MB_ptr[1] = temp_value_2;        // Store the second word of Frame Header Section
            // Store configuration into transmit side of the double buffered MB
            if(slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER){
                // Store configuration into the second word of Frame Header Section
                header_MB_ptr[1+5] = temp_value_2;        // Store the second word of Frame Header Section
            }


            temp_value_1 = 0x0000;          // Clear
            temp_value_1 |= (slot_info->header_CRC & 0x07FF);       // Header CRC
            header_MB_ptr[2] = temp_value_1;        // Store the third word of Frame Header Section
            // Store configuration into transmit side of the double buffered MB
            if(slot_info->transmit_type== FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the third word of Frame Header Section
                header_MB_ptr[2+5] = temp_value_1;        // Store the third word of Frame Header Section
            }

          
            // Check if the message buffer is in segment 1 or segment 2
            if(mb_index[temp_index].mb_index < (Fr_HW_config_ptr->last_MB_seg_1 + 1))
            {   // Payload data of MB are in segment 1
                // Calculate valid Data Field Offset
                // (total_MB_number * 10) + (buffer_index_init * MB_segment_1_data_size * 2)
                temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                               (mb_index[temp_index].mb_index * \
                               Fr_HW_config_ptr->MB_segment_1_data_size * 2);
            
                // Calculate the Data Field Offset for the transmit side of the double MB
                if(slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER)
                {
                    // Calculate valid Data Field Offset
                    // (total_MB_number * 10) + (buffer_index_init * MB_segment_1_data_size * 2) + 1 * MB_segment_1_data_size * 2
                    temp_value_1 = temp_value_2 + Fr_HW_config_ptr->MB_segment_1_data_size * 2;
                }
            }
            else // Payload data of MB are in segment 2
            {
                // Calculate valid Data Field Offset
                // (total_MB_number * 10) + ((last_MB_seg_1 + 1) * MB_segment_1_data_size * 2) + \
                // ((buffer_index_init - (last_MB_seg_1 + 1)) * MB_segment_2_data_size * 2)
                temp_value_2 = (Fr_HW_config_ptr->total_MB_number * 10) + \
                               ((Fr_HW_config_ptr->last_MB_seg_1 + 1) * Fr_HW_config_ptr->MB_segment_1_data_size * 2) + \
                               ((mb_index[temp_index].mb_index - \
                               (Fr_HW_config_ptr->last_MB_seg_1 + 1)) * Fr_HW_config_ptr->MB_segment_2_data_size * 2);
                // Calculate the Data Field Offset for the transmit side of the double MB
                if(slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER)
                {
                    // Calculate valid Data Field Offset
                    // (total_MB_number * 10) + ((last_MB_seg_1 + 1) * MB_segment_1_data_size * 2) + \
                    // ((buffer_index_init - (last_MB_seg_1 + 1)) * MB_segment_2_data_size * 2) + 1 * MB_segment_2_data_size * 2
                    temp_value_1 = temp_value_2 + Fr_HW_config_ptr->MB_segment_2_data_size * 2;
                }
            }
            header_MB_ptr[3] = temp_value_2;    // Store Data Field Offset to Frame header register
            // Store configuration into transmit side of the double buffered MB
            if(slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store the Data Field Offset to Frame Header Section
                header_MB_ptr[3+5] = temp_value_1;        // Store Data Field Offset to Frame header register
            }

 
            // Message Buffer Index Registers initialization
            temp_value_1 = mb_index[temp_index].mb_index;
            Fr_CC_reg_ptr[FrMBIDXR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
            // Store configuration into transmit side of the double buffered MB

            // Store the highest number of message buffer index into variable for optimization verification
            if(highest_txrx_mb_number < temp_value_1)
            {
                highest_txrx_mb_number = temp_value_1;      // Update variable with the highest index number
            }

            if(slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Store configuration into the MBIDXR[2n+1] register
                Fr_CC_reg_ptr[FrMBIDXR0 + Fr_MB_registers_offset_add_temp + 4] = temp_value_1 + 1;
                
                // Store the highest number of message buffer index into variable for optimization verification
                if(highest_txrx_mb_number < (temp_value_1 + 1))
                {
                    highest_txrx_mb_number = (temp_value_1 + 1);      // Update variable with the highest index number
                }
            }
            
            // Store MB type into internal MB information structure
            // Not necessary for the current version of the UNIFIED Driver
            //Fr_MB_information_internal[temp_value_1].buffer_type =  FR_TRANSMIT_BUFFER;

            // Enable message buffer
            temp_value_1 = Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] | FrMBCCSR_EDT;
          
            // Initialize the second part of double buffer if double buffered TX MB configured
            if(slot_info->transmit_type == FR_DOUBLE_TRANSMIT_BUFFER)
            {
                // Enable the MB - only in the transmit side of the double buffered - the MBCCSR[2n+1] register
                Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp + 4] = temp_value_1;
            }
            else
            {
                // Enable the MB in the MBCCSRn register
                Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
            }
            break;
        default:
            break;
        }
        temp_index++;   // Increment 
    }
    
    // Optimization verification
    // Verification whether the highest defined transmit or receive message buffer index 
    // (by FR_NUMBER_TXRX_MB parameter in the Fr_UNIFIED_cfg.h file)
    // does not exceed actually configured message buffer index for some message buffer (without shadow and FIFO)
    #ifdef FR_NUMBER_TXRX_MB
    /* The FR_NUMBER_TXRX_MB parameter is defined in the Fr_UNIFIED_cfg.h file 
    - memory consumption optimization is applied */
        if(highest_txrx_mb_number > FR_NUMBER_TXRX_MB)
        {   // The configured transmit or receive MB index is greater than it is set in the FR_NUMBER_TXRX_MB parameter
            return FR_NOT_SUCCESS;                  // Error - the value of the FR_NUMBER_TXRX_MB parameter is too low
        }
    #endif
    return FR_SUCCESS;                                      // API call has been successful
}


/************************************************************************************
* Function name:    Fr_timers_init
* Description:      This API call initializes the FlexRay CC timers
*
* @author           r62779
* @version          18/10/2006
* Function arguments: 
*                   Fr_timer_config_temp_ptr_ptr Reference to structure with timers 
*                                                configuration data
*
* Return value:
*                   none
*************************************************************************************/
void Fr_timers_init(const Fr_timer_config_type ** Fr_timer_config_temp_ptr_ptr)
{
    volatile uint8 temp_index;                  // Temporary index used for indexing of the buffer config set structure
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation
    volatile uint16 temp_value_2;               // Temporary variable used for bit operation
    volatile uint16 temp_timer_config_reg = 0;  // Timer configuration and control register

    temp_index = 0;     // Clear index
    while(Fr_timer_config_temp_ptr_ptr[temp_index] != NULL)   // Configure all required timers
    {
        // Initialization of the internal pointers, which will be used by driver later on
        Fr_timers_config_ptr = Fr_timer_config_temp_ptr_ptr[temp_index];

        switch(Fr_timers_config_ptr->timer_type)                // Which type of timer should be configured
        {
        case FR_ABSOLUTE:                   // Configure absolute timer
            if(Fr_timers_config_ptr->timer_ID == FR_TIMER_T1)    // Configure FR module registers for timer T1
            {
                // Configure TI1CYSR register
                temp_value_1 = 0x0000;                      // Clear
                temp_value_1 |= ((Fr_timers_config_ptr->timer_cycle_filter_value & 0x3F) << 8); // Set cycle filter value 
                temp_value_1 |= (Fr_timers_config_ptr->timer_cycle_filter_mask & 0x3F);         // Set cycle filter mask
                Fr_CC_reg_ptr[FrTI1CYSR] = temp_value_1;                                        // Store Timer 1 Cycle Set reg.
                
                // Configure TI1MTOR register
                temp_value_2 = 0x0000;                      // Clear
                temp_value_2 |= (uint16)(Fr_timers_config_ptr->timer_macrotick_offset & 0x3FFF); // Set macrotick value
                Fr_CC_reg_ptr[FrTI1MTOR] = temp_value_2;                                        // Store Timer 1 Macrotick Offset
                
                // The TICCR reg. will be configured later
                if(Fr_timers_config_ptr->timer_repetition == FR_REPETITIVE)
                {
                    temp_timer_config_reg |= FrTICCR_T1_REP;                                    // Repetitive mode
                }
            } 
            else if(Fr_timers_config_ptr->timer_ID == FR_TIMER_T2)  // Configure FR module registers for timer T2
            {
                // Configure TI2CR0 register
                temp_value_1 = 0x0000;                      // Clear
                temp_value_1 |= ((Fr_timers_config_ptr->timer_cycle_filter_value & 0x3F) << 8); // Set cycle filter value
                temp_value_1 |= (Fr_timers_config_ptr->timer_cycle_filter_mask & 0x3F);         // Set cycle filter mask
                Fr_CC_reg_ptr[FrTI2CR0] = temp_value_1;                                         // Store Timer 2 Configuration 0
                
                // Configure TI2CR1 register
                temp_value_2 = 0x0000;                      // Clear
                temp_value_2 |= (uint16)(Fr_timers_config_ptr->timer_macrotick_offset & 0x3FFF); // Set macrotick value
                Fr_CC_reg_ptr[FrTI2CR1] = temp_value_2;                                          // Store Timer 2 Configuration 1
                
                // The TICCR reg. will be configured later
                if(Fr_timers_config_ptr->timer_repetition == FR_REPETITIVE)
                {
                    temp_timer_config_reg |= FrTICCR_T2_REP;                                    // Repetitive mode
                }
            }
            break;
        case FR_RELATIVE:                       // Configure relative timer
            if(Fr_timers_config_ptr->timer_ID == FR_TIMER_T2)   // Configure FR module registers only for timer T2 
            {
                // Configure TI2CR0 register
                temp_value_1 = 0x0000;                      // Clear
                temp_value_1 |= (uint16)(Fr_timers_config_ptr->timer_macrotick_offset >> 16);   // Set macrotick value - high word
                Fr_CC_reg_ptr[FrTI2CR0] = temp_value_1;                                         // Store Timer 2 Configuration 0
                
                // Configure TI2CR1 register
                temp_value_2 = 0x0000;                      // Clear
                // Set macrotick value - low word
                temp_value_2 |= (uint16)(Fr_timers_config_ptr->timer_macrotick_offset & 0x0000FFFF); 
                Fr_CC_reg_ptr[FrTI2CR1] = temp_value_2;                                          // Store Timer 2 Configuration 1
                
                // The TICCR reg. will be configured later
                if(Fr_timers_config_ptr->timer_repetition == FR_REPETITIVE)
                {
                    temp_timer_config_reg |= FrTICCR_T2_REP;                                    // Repetitive mode
                    temp_timer_config_reg |= FrTICCR_T2_CFG;                                    // Relative timer
                }
                else
                {
                    temp_timer_config_reg |= FrTICCR_T2_CFG;                                    // Relative timer
                }
            }
        default:
            break;
        }
        temp_index++;   // Increment index of the configuration structure
    }
    // Configure Timer Configuration and Control register (for both timers)
    Fr_CC_reg_ptr[FrTICCR] = temp_timer_config_reg;
}

/************************************************************************************
* Function name:    Fr_slot_status_init
* Description:      This API call initializes the FlexRay CC slot status functionality
*
* @author           r62779
* @version          18/10/2006
* Function arguments: 
*                   Fr_slot_status_config_temp_ptr  Pointer to structure with slot 
*                                                   status configuration data
* Return value:
*                   none
*************************************************************************************/
void Fr_slot_status_init(const Fr_slot_status_config_type *Fr_slot_status_config_temp_ptr)
{
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation

    // Configuration of Slot Status Selection register SSSR0
    temp_value_1 = 0x0000;      // Clear
    // Configure the SSSR reg.
    temp_value_1 = Fr_slot_status_config_temp_ptr->SSSR0_slot_number;   // Store Slot Number
    //temp_value_1 |= 0x0000;               // Selector field - SSSR0
    Fr_CC_reg_ptr[FrSSSR] = temp_value_1;   // Store settings for SSSR0
    Fr_slot_status_information_internal.SSSR0_slot_number = Fr_slot_status_config_temp_ptr->SSSR0_slot_number;


    // Configuration of Slot Status Selection register SSSR1
    temp_value_1 = 0x0000;      // Clear
    // Configure the SSSR reg.
    temp_value_1 = Fr_slot_status_config_temp_ptr->SSSR1_slot_number;   // Store Slot Number
    temp_value_1 |= 0x1000;                 // Selector field - SSSR1
    Fr_CC_reg_ptr[FrSSSR] = temp_value_1;   // Store settings for SSSR1
    Fr_slot_status_information_internal.SSSR1_slot_number = Fr_slot_status_config_temp_ptr->SSSR1_slot_number;


    // Configuration of Slot Status Selection register SSSR2
    temp_value_1 = 0x0000;      // Clear
    // Configure the SSSR reg.
    temp_value_1 = Fr_slot_status_config_temp_ptr->SSSR2_slot_number;   // Store Slot Number
    temp_value_1 |= 0x2000;                 // Selector field - SSSR2
    Fr_CC_reg_ptr[FrSSSR] = temp_value_1;   // Store settings for SSSR2
    Fr_slot_status_information_internal.SSSR2_slot_number = Fr_slot_status_config_temp_ptr->SSSR2_slot_number;

    
    // Configuration of Slot Status Selection register SSSR3
    temp_value_1 = 0x0000;      // Clear
    // Configure the SSSR reg.
    temp_value_1 = Fr_slot_status_config_temp_ptr->SSSR3_slot_number;   // Store Slot Number
    temp_value_1 |= 0x3000;                 // Selector field - SSSR3
    Fr_CC_reg_ptr[FrSSSR] = temp_value_1;   // Store settings for SSSR3
    Fr_slot_status_information_internal.SSSR3_slot_number = Fr_slot_status_config_temp_ptr->SSSR3_slot_number;

    // Store information that the slot status registers are configured
    Fr_slot_status_information_internal.registers_used = TRUE;  // Store configuration into internal structure
}

/************************************************************************************
* Function name:    Fr_slot_status_counter_init
* Description:      This API call initializes the FlexRay CC slot status functionality
*
* @author           r62779
* @version          18/10/2006
* Function arguments: 
*                   Fr_slot_status_counter_config_temp_ptr_ptr  Reference to structure with 
*                                                       slot status counter configuration data
*
* Return value:
*                   none
*************************************************************************************/
void Fr_slot_status_counter_init(const Fr_slot_status_counter_config_type ** Fr_slot_status_counter_config_temp_ptr_ptr)
{
    volatile uint8 temp_index;                  // Temporary index used for indexing of the buffer config set structure
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation

    temp_index = 0;     // Clear index
    while(Fr_slot_status_counter_config_temp_ptr_ptr[temp_index] != NULL)   // Configure all required slot status counters
    {
        // Initialization of the internal pointers, which will be used by driver later on
        Fr_slot_status_counter_config_ptr = Fr_slot_status_counter_config_temp_ptr_ptr[temp_index];
        
        // Determine which slot status counter should be configured
        if(Fr_slot_status_counter_config_ptr->counter_ID == FR_SLOT_STATUS_COUNTER_0)
        {   // Slot Status Counter 0 will be configured
            // Set the Selector bit field of the SSCCR register
            temp_value_1 = 0x0000;                  // Select SSCCR0 register
        }
        else if(Fr_slot_status_counter_config_ptr->counter_ID == FR_SLOT_STATUS_COUNTER_1)
        {   // Slot Status Counter 1 will be configured
            // Set the Selector bit field of the SSCCR register
            temp_value_1 = 0x1000;                  // Select SSCCR1 register
        }
        else if(Fr_slot_status_counter_config_ptr->counter_ID == FR_SLOT_STATUS_COUNTER_2)
        {   // Slot Status Counter 2 will be configured
            // Set the Selector bit field of the SSCCR register
            temp_value_1 = 0x2000;                  // Select SSCCR2 register
        }
        else
        {   // Slot Status Counter 3 will be configured
            // Set the Selector bit field of the SSCCR register
            temp_value_1 = 0x3000;                  // Select SSCCR3 register
        }
        
        // Determine which incremental principle should be configured
        if(Fr_slot_status_counter_config_ptr->counter_configuration == FR_SLOT_STATUS_CHANNEL_A)
        {   // Slot Status Counter is incremented by 1 if condition is fulfilled on channel A
            // Set the CNTCFG bit field of the SSCCR register
            temp_value_1 |= 0x0000;
        }
        else if(Fr_slot_status_counter_config_ptr->counter_configuration == FR_SLOT_STATUS_CHANNEL_B)
        {   // Slot Status Counter is incremented by 1 if condition is fulfilled on channel B
            // Set the CNTCFG bit field of the SSCCR register
            temp_value_1 |= 0x0200;
        }
		else if(Fr_slot_status_counter_config_ptr->counter_configuration == FR_SLOT_STATUS_CHANNEL_AB_BY_1)
        {   // Slot Status Counter is incremented by 1 if condition is fulfilled on at least one channel
            // Set the CNTCFG bit field of the SSCCR register
            temp_value_1 |= 0x0400;
        }
        else if(Fr_slot_status_counter_config_ptr->counter_configuration == FR_SLOT_STATUS_CHANNEL_AB_BY_2)
        {   // Slot Status Counter is incremented by 2 if condition is fulfilled on both channels
            // Slot Status Counter is incremented by 1 if condition is fulfilled on only one channel
            // Set the CNTCFG bit field of the SSCCR register
            temp_value_1 |= 0x0600;
        }
        
        // Multi Cycle Selection - defines whether the slot status sounter accumulates over multiple com.cycles
        if(Fr_slot_status_counter_config_ptr->multi_cycle_selection)
        {   // The Slot Status Counter accumulates over multiple communication cycles
            // Set the MCY bit of the SSCCR register
            temp_value_1 |= 0x0100;
        }
        
        // Valid Frame Restriction - restricts the counter to received valid frames
        if(Fr_slot_status_counter_config_ptr->valid_frame_restriction)
        {
            // Set the VFR bit of the SSCCR register
            temp_value_1 |= 0x0080;
        }
        
        // Sync Frame Restriction - restricts the counter to received sync frames
        if(Fr_slot_status_counter_config_ptr->valid_frame_restriction)
        {
            // Set the SYF bit of the SSCCR register
            temp_value_1 |= 0x0040;
        }
        
        // Null Frame Restriction - restricts the counter to received null frames
        if(Fr_slot_status_counter_config_ptr->null_frame_restriction)
        {
            // Set the NUF bit of the SSCCR register
            temp_value_1 |= 0x0020;
        }

        // Startup Frame Restriction - restricts the counter to received startup frames
        if(Fr_slot_status_counter_config_ptr->startup_frame_restriction)
        {
            // Set the SUF bit of the SSCCR register
            temp_value_1 |= 0x0010;
        }

        // Slot Status Mask - enables the counter with respect to the four slot status error indicator bits
        if(Fr_slot_status_counter_config_ptr->syntax_error_counting)
        {   // Enables the counting for slots with the syntax error indicator bit set to 1
            // Set the STATUSMASK[3] bit of the SSCCR register
            temp_value_1 |= 0x0008;
        }
        if(Fr_slot_status_counter_config_ptr->content_error_counting)
        {   // Enables the counting for slots with the content error indicator bit set to 1
            // Set the STATUSMASK[2] bit of the SSCCR register
            temp_value_1 |= 0x0004;
        }
        if(Fr_slot_status_counter_config_ptr->boundary_violation_counting)
        {   // Enables the counting for slots with the boundary violation indicator bit set to 1
            // Set the STATUSMASK[1] bit of the SSCCR register
            temp_value_1 |= 0x0002;
        }
        if(Fr_slot_status_counter_config_ptr->transmission_conflict_counting)
        {   // Enables the counting for slots with the transmission conflict indicator bit set to 1
            // Set the STATUSMASK[0] bit of the SSCCR register
            temp_value_1 |= 0x0001;
        }
        
        Fr_CC_reg_ptr[FrSSCCR] = temp_value_1;      // Store configuration into Slot Status Counter Condition reg.
        temp_index++;   // Increment index of the configuration structure
    }
}


/************************************************************************************
* Function name:    Fr_leave_configuration_mode
* Description:      This API call initiates a transition from FR_POCSTATE_CONFIG 
*                   state into FR_POCSTATE_READY state.
*
* @author           r62779
* @version          27/07/2006
* Function arguments: 
*                   None
*
* Return value:
*                   FR_SUCCESS	        API call has been successful
*                   FR_NOT_SUCCESS	    Error occured during transition 
*                                       into FR_POCSTATE_READY
*************************************************************************************/
Fr_return_type Fr_leave_configuration_mode(void)
{
    uint8 i;                                            // Temporary counter
    
    /* Force the FlexRay CC into FR_POCSTATE_READY */
    i = 0;      // Clear temporary counter
    while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
    {
        i++;        // Increment temporary counter
    }
    if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error
    
    Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_CONFIG_COMPLETE | FrPOCR_WME);  // Transition to FR_POCSTATE_READY
    
    i = 0;      // Clear temporary counter
    while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
    {
        i++;        // Increment temporary counter
    }
    if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error

    // Wait till FlexRay CC is not in FR_POCSTATE_READY
    i = 0;      // Clear temporary counter
    while((i < FR_MAX_WAIT_CYCLES) && ( (Fr_CC_reg_ptr[FrPSR0] & 0x0700) != FrPSR0_PROTSTATE_READY))
    {
        i++;        // Increment temporary counter
    }
    if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error
    
    return FR_SUCCESS;                                      // API call has been successful
}

/************************************************************************************
* Function name:    Fr_start_communication
* Description:      This API call initiates the start of the communication of the 
*                   FlexRay CC - initiates a transition from FR_POCSTATE_READY state 
*                   into FR_POCSTATE_STARTUP state. If the start up of the FlexRay CC 
*                   is successful, the state FR_POCSTATE_NORMAL_ACTIVE is 
*                   automatically reached. If the FlexRay CC is not in 
*                   FR_POCSTATE_READY state, but e.g. in FR_POCSTATE_HALT state, this 
*                   API call returns FR_NOT_SUCCESS and remains in the state it is in
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   None
*
* Return value:
*                   FR_SUCCESS	        API call has been successful
*                   FR_NOT_SUCCESS	    Error occured during transition 
*                                       into FR_POCSTATE_STARTUP
*************************************************************************************/
Fr_return_type Fr_start_communication(void)
{
    uint8 i;                                    // Temporary counter
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation
    
    temp_value_1 = (Fr_CC_reg_ptr[FrPSR0] & 0x0700);    // Load Protocol State field of the PSR0 reg.
   
    // Check whether the FlexRay CC is in the FR_POCSTATE_READY?
    if(temp_value_1 == FrPSR0_PROTSTATE_READY)
    {   // Yes
        
        /* Force the FlexRay CC into FR_POCSTATE_STARTUP */
        i = 0;      // Clear temporary counter
        while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
        {
            i++;        // Increment temporary counter
        }
        if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error
        
        if(Fr_HW_config_ptr->allow_cold_start_enable)           // Node is ColdStart node?
        {   // Yes
            // Immediately activate capability of node to cold start cluster
            Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_ALLOW_COLDSTART  | FrPOCR_WME);
            
            i = 0;      // Clear temporary counter
            // Wait till Protocol Command Write is not busy
            while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY))
            {
                i++;        // Increment temporary counter
            }
            if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error
        }
        
        Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_RUN  | FrPOCR_WME);  // Immediately transition to the FR_POCSTATE_STARTUP
        
        i = 0;      // Clear temporary counter
        while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
        {
            i++;        // Increment temporary counter
        }
        if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error

        return FR_SUCCESS;                                      // API call has been successful
    }
    else
    {
        return FR_NOT_SUCCESS;                                  // Return FR_NOT_SUCCESS in case any error
    }
}

/************************************************************************************
* Function name:    Fr_stop_communication
* Description:      This API call halts the communication of the FlexRay CC at the end
*                   of current FlexRay cycle in case that the FR_HALT_COMMUNICATION parameter is placed.
*                   The API call immediately aborts the communication of the FlexRay CC 
*                   in case that the FR_ABORT_COMMUNICATION parameter is placed
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   Fr_stop_option	FR_HALT_COMMUNICATION	- the API call halts the communication 
*                                               of the FlexRay CC at the end of current FlexRay cycle
*	                                FR_ABORT_COMMUNICATION	- the API call immediately aborts the 
*                                                             communication of the FlexRay CC
*
* Return value:
*                   FR_SUCCESS	        API call has been successful
*                   FR_NOT_SUCCESS	    Error occured during transition 
*                                       into FR_POCSTATE_STARTUP
*************************************************************************************/
Fr_return_type Fr_stop_communication(Fr_stop_communication_type Fr_stop_option)
{
    uint8 i;                                    // Temporary counter

    i = 0;      // Clear temporary counter
    while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
    {
        i++;        // Increment temporary counter
    }
    if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error

    
    switch(Fr_stop_option)
    {
    case FR_HALT_COMMUNICATION:
        // Transition to halt state on completion of current comm. cycle
        Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_HALT  | FrPOCR_WME); 
        break;
    case FR_ABORT_COMMUNICATION:
        Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_FREEZE  | FrPOCR_WME);   // Immediately transition to halt state
        break;
    default:
        return FR_NOT_SUCCESS;                              // Return FR_NOT_SUCCESS - wrong input parameter
        break;
    }
        
    i = 0;      // Clear temporary counter
    // Wait till Protocol Command Write is not busy
    while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY))
    {
        i++;        // Increment temporary counter
    }
    if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case of any error

    return FR_SUCCESS;                                      // API call has been successful
}


/************************************************************************************
* Function name:    Fr_send_wakeup
* Description:      This API call sends a Wakeup Pattern over configured channel
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   None
*
* Return value:
*                   FR_SUCCESS	        API call has been successful
*                   FR_NOT_SUCCESS	    Error occured
*
*************************************************************************************/
Fr_return_type Fr_send_wakeup(void)
{
    uint8 i;                                    // Temporary counter
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation
    
    temp_value_1 = (Fr_CC_reg_ptr[FrPSR0] & 0x0700);    // Load Protocol State field of the PSR0 reg.
    
    if(temp_value_1 == FrPSR0_PROTSTATE_READY)  // Is the FlexRay CC in the FR_POCSTATE_READY state?
    {   // Yes
        /* Initiate wakeup procedure */
        i = 0;      // Clear temporary counter
        while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
        {
            i++;        // Increment temporary counter
        }
        if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;  // Return FR_NOT_SUCCESS in case of any error

        Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_WAKEUP  | FrPOCR_WME);       // Immediately transition to the FR_POCSTATE_WAKEUP
        
        i = 0;      // Clear temporary counter
        while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
        {
            i++;        // Increment temporary counter
        }
        if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;  // Return FR_NOT_SUCCESS in case of any error

        return FR_SUCCESS;                                  // API call has been successful
    }
    else
    {
        return FR_NOT_SUCCESS;                              // Return FR_NOT_SUCCESS in case of any error
    }
}

/************************************************************************************
* Function name:    Fr_get_wakeup_state
* Description:      Retrieve the wakeup state of the FlexRay CC, 
*                   which is part of the startup of a FlexRay CC	
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   None
*
* Return value:
*            FR_WAKEUPSTATE_UNDEFINED           Undefined
*            FR_WAKEUPSTATE_RECEIVED_HEADER     Received_Header
*            FR_WAKEUPSTATE_RECEIVED_WUP        Receiver_WUP
*            FR_WAKEUPSTATE_COLLOSION_HEADER    Collision_Header
*            FR_WAKEUPSTATE_COLLISION_WUP       Collision_WUP
*            FR_WAKEUPSTATE_COLLISION_UNKNOWN   Collision_Unknow
*            FR_WAKEUPSTATE_TRANSMITTED         Transmitted
*                   					
*************************************************************************************/
Fr_wakeup_state_type Fr_get_wakeup_state(void)
{
    volatile uint16 temp_value_1 = 0;           // Temporary variable used for bit operation

    // Read current WAKEUPSTATUS
    temp_value_1 = Fr_CC_reg_ptr[FrPSR0];
    temp_value_1 &= 0x0007;                     // Select only WAKEUPSTATUS field
    
    switch(temp_value_1)                        // Test Protocol State
    {
    case 0x0000:                                // FR_WAKEUPSTATE_UNDEFINED
        return FR_WAKEUPSTATE_UNDEFINED;        // Return current Wakeup State
        break;
    case FrPSR0_WAKEUPSTATUS_RECEIVED_HEADER:   // FR_WAKEUPSTATE_RECEIVED_HEADER
        return FR_WAKEUPSTATE_RECEIVED_HEADER;  // Return current Wakeup State
        break;
    case FrPSR0_WAKEUPSTATUS_RECEIVED_WUP:      // FR_WAKEUPSTATE_RECEIVED_WUP
        return FR_WAKEUPSTATE_RECEIVED_WUP;     // Return current Wakeup State
        break;
    case FrPSR0_WAKEUPSTATUS_COLLISION_HEADER:  // FR_WAKEUPSTATE_COLLISION_HEADER
        return FR_WAKEUPSTATE_COLLISION_HEADER; // Return current Wakeup State
        break;
    case FrPSR0_WAKEUPSTATUS_COLLISION_WUP:     // FR_WAKEUPSTATE_COLLISION_WUP
        return FR_WAKEUPSTATE_COLLISION_WUP;    // Return current Wakeup State
        break;
    case FrPSR0_WAKEUPSTATUS_COLLISION_UNKNOWN: // FR_WAKEUPSTATE_COLLISION_UNKNOWN
        return FR_WAKEUPSTATE_COLLISION_UNKNOWN; // Return current Wakeup State
        break;
    case FrPSR0_WAKEUPSTATUS_TRANSMITTED:       // FR_WAKEUPSTATE_TRANSMITTED
        return FR_WAKEUPSTATE_TRANSMITTED;      // Return current Wakeup State
        break;    
    default:
        break;
    }
    return FR_WAKEUPSTATE_UNDEFINED;			// FR_WAKEUPSTATE_UNDEFINED in case of any error
}

/************************************************************************************
* Function name:    Fr_get_POC_state
* Description:      Query the current value of the POC state of FlexRay CC	
*
* @author           r62779
* @version          26/07/2006
* Function arguments: 
*                   None
*
* Return value:
*                   FR_POCSTATE_CONFIG	Configuration state
*                   FR_POCSTATE_DEFAULT_CONFIG	State prior to config, only left with 
*                                   an explicit configuration request
*                   FR_POCSTATE_HALT	Error state, can only be left by a reinitialization
*                   FR_POCSTATE_NORMAL_ACTIVE	Normal operation
*                   FR_POCSTATE_NORMAL_PASSIVE	Errors detected, no transmission of data,
*                                   but attempting to return to normal operation
*                   FR_POCSTATE_READY	State reached from FR_POCSTATE_CONFIG after 
*                                   concluding the configuration
*                   FR_POCSTATE_STARTUP	CC transmits only startup frames
*                   FR_POCSTATE_WAKEUP	CC sends a wakeup pattern if it couldn’t find 
*                                   anything on the bus
*                   
*************************************************************************************/
Fr_POC_state_type Fr_get_POC_state(void)
{
    volatile uint16 temp_value_1 = 0;           // Temporary variable used for bit operation

    // The state of the PSR0 register (the PROSTATE field) is not updated if FREEZE occured
    temp_value_1 = Fr_CC_reg_ptr[FrPSR1];       // Check if immediate halt due to FREEZE or internal error condition occured
    if(temp_value_1 & FrPSR1_FRZ) return FR_POCSTATE_HALT;  // The FlexRay module is in FR_POCSTATE_HALT state 
    
    // Read current FR_POCSTATE
    temp_value_1 = Fr_CC_reg_ptr[FrPSR0];
    temp_value_1 &= 0x0700;                     // Select only PROSTATE field
    
    switch(temp_value_1)                        // Test Protocol State
    {
    case FrPSR0_PROTSTATE_DEFAULT_CONFIG:       // FR_POCSTATE_DEFAULT_CONFIG
        return FR_POCSTATE_DEFAULT_CONFIG;      // Return current Protocol State
        break;
    case FrPSR0_PROTSTATE_CONFIG:               // FR_POCSTATE_CONFIG
        return FR_POCSTATE_CONFIG;              // Return current Protocol State
        break;
    case FrPSR0_PROTSTATE_WAKEUP:               // FR_POCSTATE_WAKEUP
        return FR_POCSTATE_WAKEUP;              // Return current Protocol State
        break;
    case FrPSR0_PROTSTATE_READY:                // FR_POCSTATE_READY
        return FR_POCSTATE_READY;               // Return current Protocol State
        break;
    case FrPSR0_PROTSTATE_NORMAL_PASSIVE:       // FR_POCSTATE_READY
        return FR_POCSTATE_NORMAL_PASSIVE;      // Return current Protocol State
        break;
    case FrPSR0_PROTSTATE_NORMAL_ACTIVE:        // FR_POCSTATE_NORMAL_ACTIVE
        return FR_POCSTATE_NORMAL_ACTIVE;       // Return current Protocol State
        break;
    case FrPSR0_PROTSTATE_HALT:                 // FR_POCSTATE_HALT
        return FR_POCSTATE_HALT;                // Return current Protocol State
        break;    
    case FrPSR0_PROTSTATE_STARTUP:              // FR_POCSTATE_STARTUP
        return FR_POCSTATE_STARTUP;             // Return current Protocol State
        break;
    default:
        break;
    }   
}

/************************************************************************************
* Function name:    Fr_get_sync_state
* Description:      Query wheter or not the FlexRay module is synchronous to the rest of the cluster	
*
* @author           r62779
* @version          1/08/2006
* Function arguments: 
*                   None
*
* Return value:
*                   FR_ASYNC    The local FlexRay CC is asynchronous to the FlexRay global time
*                   FR_SYNC     The local FlexRay CC is synchronous to the FlexRay global time
*                   
*************************************************************************************/
Fr_sync_state_type Fr_get_sync_state(void)
{
    volatile uint16 temp_value_1 = 0;           // Temporary variable used for bit operation

    // The state of the PSR0 register (the PROSTATE field) is not updated if FREEZE occured
    temp_value_1 = Fr_CC_reg_ptr[FrPSR1];       // Check if immediate halt due to FREEZE or internal error condition occured
    if(temp_value_1 & FrPSR1_FRZ) return FR_ASYNC;  // The FlexRay module is in FR_POCSTATE_HALT state - not synchronous
    
    // Read current FR_POCSTATE
    temp_value_1 = Fr_CC_reg_ptr[FrPSR0];
    temp_value_1 &= 0x0700;                     // Select only PROSTATE field

    // Check the Protocol state field
    if(temp_value_1 == FrPSR0_PROTSTATE_NORMAL_ACTIVE)  // Is the FlexRay module in FR_POCSTATE_NORMAL_ACTIVE state?
    {   // Yes
        return FR_SYNC;                         // This FlexRay module is synchronous to the cluster
    }
    else return FR_ASYNC;                       // This FlexRay module is not synchronous to the cluster
}


/************************************************************************************
* Function name:    Fr_enter_configuration_mode
* Description:      This API call determines in which POC state of the FlexRay protocol 
*                   and performs following sequences:
*                   - if the state of the protocol is FR_POCSTATE_HALT, the function sends 
*                     the DEFAULT_CONFIG command and later on the CONFIG command
*                   - if the state of the protocol is FR_POCSTATE_DEFAULT_CONFIG or FR_POCSTATE_READY,
*                     the function sends the the CONFIG command
*
*                   If the transition is not successful this API call returns FR_NOT_SUCCESS.
*
* @author           r62779
* @version          27/09/2006
* Function arguments: 
*                   None
*
* Return value:
*                   FR_SUCCESS	        API call has been successful
*                   FR_NOT_SUCCESS	    Error occured during transition 
*                                       into FR_POCSTATE_CONFIG
*************************************************************************************/
Fr_return_type Fr_enter_configuration_mode(void)
{
    uint8 i;                                    // Temporary counter
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation
    volatile uint16 temp_value_2;               // Temporary variable used for bit operation
    
    temp_value_1 = (Fr_CC_reg_ptr[FrPSR0] & 0x0700);    // Load Protocol State field of the PSR0 reg.
    temp_value_2 = Fr_CC_reg_ptr[FrPSR1];               // Load the PSR1 register - to check if freeze occured
   
    // Is the FlexRay CC in the FR_POCSTATE_HALT state?
    // Also check if immediate halt due to FREEZE or internal error condition occured
    if((temp_value_1 == FrPSR0_PROTSTATE_HALT) || (temp_value_2 & FrPSR1_FRZ))
    {   // Yes
        
        /* Force the FlexRay CC into FR_POCSTATE_DEFAULT_CONFIG */
        i = 0;      // Clear temporary counter
        while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
        {
            i++;        // Increment temporary counter
        }
        if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error
        
        
        // Protocol Reset Command sequence
        // Or to solve an issue (MUCts03059) described in Errata Document
        {   // Start of sequence
            // Repeatedly send the command FREEZE, until the freeze bit is set
            // Load the PSR1 register - to check if freeze occured
            while((Fr_CC_reg_ptr[FrPSR1] & FrPSR1_FRZ) != FrPSR1_FRZ)
            {
                // Repeatedly send the command FREEZE, until the freeze bit is set
                Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_FREEZE  | FrPOCR_WME);       // Immediately transition to halt state
                
                i = 0;      // Clear temporary counter
                // Wait till Protocol Command Write is not busy
                while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY))
                {
                    i++;        // Increment temporary counter
                }
                if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case of any error
            }
            
            // Repeatedly send the command DEFAULT_CONFIG , until the freeze bit PSR1[FRZ] is cleared and 
            // the field PSR0[PROTSTATE] indicates DEFAULT_CONFIG
            while(((Fr_CC_reg_ptr[FrPSR1] & FrPSR1_FRZ) == FrPSR1_FRZ) || \
                    ((Fr_CC_reg_ptr[FrPSR0] & 0x0700) != FrPSR0_PROTSTATE_DEFAULT_CONFIG))
            {
                // Immediately transition to the FR_POCSTATE_DEFAULT_CONFIG
                Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_DEFAULT_CONFIG  | FrPOCR_WME);   
                
                i = 0;      // Clear temporary counter
                // Wait till Protocol Command Write is not busy
                while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY))
                {
                    i++;        // Increment temporary counter
                }
                if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case of any error
            }																			
        }   // End of sequence
        
        i = 0;      // Clear temporary counter
        while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
        {
            i++;        // Increment temporary counter
        }
        if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error

        i = 0;      // Clear temporary counter
        // Wait till the FlexRay CC is not in the FR_POCSTATE_DEFAULT_CONFIG
        while((i < FR_MAX_WAIT_CYCLES) && ((Fr_CC_reg_ptr[FrPSR0] & 0x0700) != FrPSR0_PROTSTATE_DEFAULT_CONFIG))
        {
            i++;        // Increment temporary counter
        }
        if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error
        temp_value_1 = (Fr_CC_reg_ptr[FrPSR0] & 0x0700);        // Load Protocol State field of the PSR0 reg.
    }
        
    // Is the FlexRay CC in the FR_POCSTATE_DEFAULT_CONFIG or FR_POCSTATE_READY state?
    if((temp_value_1 == FrPSR0_PROTSTATE_DEFAULT_CONFIG) || (temp_value_1 == FrPSR0_PROTSTATE_READY))
    {   // Yes
        
        /* Force the FlexRay CC into FR_POCSTATE_CONFIG */
        i = 0;      // Clear temporary counter
        while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
        {
            i++;        // Increment temporary counter
        }
        if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error
        
        Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_CONFIG  | FrPOCR_WME);   // Immediately transition to the FR_POCSTATE_CONFIG
        
        i = 0;      // Clear temporary counter
        while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY)) // Wait till Protocol Command Write is not busy
        {
            i++;        // Increment temporary counter
        }
        if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error

        // Wait till FlexRay CC is not in FR_POCSTATE_CONFIG
        i = 0;      // Clear temporary counter
        while((i < FR_MAX_WAIT_CYCLES) && ( (Fr_CC_reg_ptr[FrPSR0] & 0x0700) != FrPSR0_PROTSTATE_CONFIG))
        {
            i++;        // Increment temporary counter
        }
        if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case any error
    } 
    else    // The protocol status is not in FR_POCSTATE_DEFAULT_CONFIG or FR_POCSTATE_READY
            // -> it is not possible to move into FR_POCSTATE_CONFIG
    {
        return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS - inappropriate state of protocol
    }
        
    return FR_SUCCESS;                                      // API call has been successful
}

/************************************************************************************
* Function name:    Fr_reset_protocol_engine
* Description:      This API call immediatelly resets the Protocol engine and initiates 
*                   a transition into FR_POCSTATE_DEFAULT_CONFIG protocol state
*
* @author           r62779
* @version          27/09/2006
* Function arguments: 
*                   none
*
* Return value:
*                   FR_SUCCESS	        API call has been successful
*                   FR_NOT_SUCCESS	    Error occured
*************************************************************************************/
Fr_return_type Fr_reset_protocol_engine(void)
{
    uint8 i = 0;                                            // Temporary counter

    Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_RESET  | FrPOCR_WME);            // Immediately reset the Protocol engine
        
    // Wait till Protocol Command Write is not busy
    while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY))
    {
        i++;        // Increment temporary counter
    }
    if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case of any error

    // Protocol Reset Command sequence
    // Or to solve an issue (MUCts03059) described in Errata Document
    {   // Start of sequence
        // Repeatedly send the command FREEZE, until the freeze bit is set
        // Load the PSR1 register - to check if freeze occured
        while((Fr_CC_reg_ptr[FrPSR1] & FrPSR1_FRZ) != FrPSR1_FRZ)
        {
            // Repeatedly send the command FREEZE, until the freeze bit is set
            Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_FREEZE  | FrPOCR_WME);   // Immediately transition to halt state
            
            i = 0;      // Clear temporary counter
            // Wait till Protocol Command Write is not busy
            while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY))
            {
                i++;        // Increment temporary counter
            }
            if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case of any error
        }
        
        // Repeatedly send the command DEFAULT_CONFIG , until the freeze bit PSR1[FRZ] is cleared and 
        // the field PSR0[PROTSTATE] indicates DEFAULT_CONFIG
        while(((Fr_CC_reg_ptr[FrPSR1] & FrPSR1_FRZ) == FrPSR1_FRZ) || \
                ((Fr_CC_reg_ptr[FrPSR0] & 0x0700) != FrPSR0_PROTSTATE_DEFAULT_CONFIG))
        {
            // Immediately transition to the FR_POCSTATE_DEFAULT_CONFIG
            Fr_CC_reg_ptr[FrPOCR] = (FrPOCR_POCCMD_DEFAULT_CONFIG  | FrPOCR_WME);   
            
            i = 0;      // Clear temporary counter
            // Wait till Protocol Command Write is not busy
            while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY))
            {
                i++;        // Increment temporary counter
            }
            if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case of any error
        }																			
    }   // End of sequence
    
    i = 0;      // Clear temporary counter
    // Wait till Protocol Command Write is not busy
    while((i < FR_MAX_WAIT_CYCLES) && (Fr_CC_reg_ptr[FrPOCR] & FrPOCR_BSY))
    {
        i++;        // Increment temporary counter
    }
    if(i == FR_MAX_WAIT_CYCLES) return FR_NOT_SUCCESS;      // Return FR_NOT_SUCCESS in case of any error
    
    return FR_SUCCESS;                                      // API call has been successful
}


/************************************************************************************
* Function name:    Fr_get_global_time
* Description:      Query the global time (current cycle and macrotick values) of the FlexRay cluster	
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   Fr_cycle_ptr	Reference to a memory location where the current 
*                                   FlexRay communication cycle value will be stored
*                   Fr_macrotick_ptr	Reference to a memory location where the current 
*                                   FlexRay communication macrotick value will be stored
*
* Return value:
*                   none
*************************************************************************************/
void Fr_get_global_time(uint8 * Fr_cycle_ptr, uint16 * Fr_macrotick_ptr)
{
    *Fr_macrotick_ptr = Fr_CC_reg_ptr[FrMTCTR];     // Store the current macrotick value
    *Fr_cycle_ptr = (uint8)(Fr_CC_reg_ptr[FrCYCTR]);    // Store the current cycle value
}

/************************************************************************************
* Function name:    Fr_send_MTS
* Description:      This API call configures the FlexRay module for sending Media Test Symbol (MTS)
*                   on given channel	
*
* @author           r62779
* @version          18/10/2006
* Function arguments: 
*                   Fr_channel  FR_CHANNEL_A    An media test symbol will be transmitted on channel A
*                               FR_CHANNEL_B    An media test symbol will be transmitted on channel B
*                   Fr_MTS_config_temp_ptr      Reference to structure with configuration data for Media 
*                                               Test Symbol transmission from given channel	
*
* Return value:
*                   FR_SUCCESS      API call has been successful	
*                   FR_NOT_SUCCESS  Function has been called with Incorrect value of the Fr_channel parameter	
*
*************************************************************************************/
Fr_return_type Fr_send_MTS(Fr_channel_type Fr_channel, const Fr_MTS_config_type *Fr_MTS_config_temp_ptr)
{
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation
    volatile uint16 temp_value_2;               // Temporary variable used for bit operation

    temp_value_1 = (uint16)(Fr_MTS_config_temp_ptr->cycle_counter_value & 0x3F);    // Cycle counter value
    temp_value_2 = (uint16)(Fr_MTS_config_temp_ptr->cycle_counter_mask & 0x3F);     // Cycle counter mask
    temp_value_1 |= (temp_value_2 << 8);    // Move the CYCCNTMSK bit field

    if(Fr_channel == FR_CHANNEL_A)  // On which channel should be MTS sent?
    {  // An media test symbol will be transmitted on channel A
        temp_value_1 |= FrMTSACFR_MTE;          // Set the MTE bit - MTS transmission enabled
        Fr_CC_reg_ptr[FrMTSACFR] = temp_value_1;    // Store configuration into MTS A Configuration register
    }
    else if(Fr_channel == FR_CHANNEL_B)  // On which channel should be MTS sent?
    {  // An media test symbol will be transmitted on channel B
        temp_value_1 |= FrMTSBCFR_MTE;          // Set the MTE bit - MTS transmission enabled
        Fr_CC_reg_ptr[FrMTSBCFR] = temp_value_1;    // Store configuration into MTS B Configuration register
    }
    else return FR_NOT_SUCCESS;     // Incorrect value of the Fr_channel inserted
    
    return FR_SUCCESS;              // API call has been successful
}

/************************************************************************************
* Function name:    Fr_get_MTS_state
* Description:      This function is used to retrieve the MTS state on given channel
*
* @author           r62779
* @version          28/07/2006
* Function arguments: 
*                   Fr_channel  FR_CHANNEL_A    Media test symbol state will be checked for channel A
*                               FR_CHANNEL_B    Media test symbol state will be checked for channel B
* Return value:
*
*                   FR_MTS_RCV              A valid MTS has been received
*                   FR_MTS_RCV_SYNERR       A valid MTS has been received and a Syntax Error was detected
*                   FR_MTS_RCV_BVIO         A valid MTS has been received and a Boundary Violation has been detected
*                   FR_MTS_RCV_SYNERR_BVIO  A valid MTS has been received and a Syntax Error and 
*                                           a Boundary Violation has been detected
*                   FR_MTS_NOT_RCV          No valid MTS has been received or function has been called with an incorrect
*                                           value of the Fr_channel parameter
*                   FR_MTS_NOT_RCV_SYNERR   No valid MTS has been received and a Syntax Error was detected
*                   FR_MTS_NOT_RCV_BVIO     No valid MTS has been received and 
*                                           a Boundary Violation has been detected
*                   FR_MTS_NOT_RCV_SYNERR_BVIO  No valid MTS has been received and a Syntax Error and 
*                                               a Boundary Violation has been detected
*
*************************************************************************************/
Fr_MTS_state_type Fr_get_MTS_state(Fr_channel_type Fr_channel)
{
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation

    temp_value_1 = Fr_CC_reg_ptr[FrPSR2];       // Load the PSR2 register

    if(Fr_channel == FR_CHANNEL_B)              // Which channel should be checked?
    {											// Channel B will be checked
        // Select only SBVB, SSEB and MTB bits from PSR2 register (STCB bit is not loaded)
        switch(temp_value_1 & 0x1C00)
        {
        case FrPSR2_MTSB_RCV:           // Media Access Test Symbol MTS has been received on channel B
            return FR_MTS_RCV;          // Return FR_MTS_RCV value
            break;

        case FrPSR2_MTSB_RCV_SYNERR:    // A valid MTS has been received and a Syntax Error was detected
            return FR_MTS_RCV_SYNERR;   // Return FR_MTS_RCV_SYNERR value
            break;

        case FrPSR2_MTSB_RCV_BVIO:      // A valid MTS has been received and a Boundary Violation has been detected
            return FR_MTS_RCV_BVIO;		// Return FR_MTS_RCV_BVIO value
            break;

        case FrPSR2_MTSB_RCV_SYNERR_BVIO:   // A valid MTS has been received and a Syntax Error and 
                                            // a Boundary Violation has been detected
            return FR_MTS_RCV_SYNERR_BVIO;  // Return FR_MTS_RCV_SYNERR_BVIO value
            break;

        case FrPSR2_MTSB_NOT_RCV:       // No valid MTS has been received
            return FR_MTS_NOT_RCV;		// Return FR_MTS_NOT_RCV value
            break;

        case FrPSR2_MTSB_NOT_RCV_SYNERR:    // No valid MTS has been received and a Syntax Error was detected
            return FR_MTS_NOT_RCV_SYNERR;   // Return FR_MTS_NOT_RCV_SYNERR value
            break;

        case FrPSR2_MTSB_NOT_RCV_BVIO:		// No valid MTS has been received and a Boundary Violation has been detected
            return FR_MTS_NOT_RCV_BVIO;		// Return FR_MTS_NOT_RCV_BVIO value
            break;

        case FrPSR2_MTSB_NOT_RCV_SYNERR_BVIO:   //  No valid MTS has been received and a Syntax Error and 
                                                // a Boundary Violation has been detected
            return FR_MTS_NOT_RCV_SYNERR_BVIO;  // Return FR_MTS_NOT_RCV_SYNERR_BVIO value
            break;
        default:
            break;
        }
    }
    else if(Fr_channel == FR_CHANNEL_A)         // Which channel should be checked?
    {											// Channel A will be checked
        // Select only SBVA, SSEA and MTA bits from PSR2 register (STCA bit is not loaded)
        switch(temp_value_1 & 0x0070)
        {
        case FrPSR2_MTSA_RCV:           // Media Access Test Symbol MTS has been received on channel B
            return FR_MTS_RCV;          // Return FR_MTS_RCV value
            break;

        case FrPSR2_MTSA_RCV_SYNERR:    // A valid MTS has been received and a Syntax Error was detected
            return FR_MTS_RCV_SYNERR;   // Return FR_MTS_RCV_SYNERR value
            break;

        case FrPSR2_MTSA_RCV_BVIO:      // A valid MTS has been received and a Boundary Violation has been detected
            return FR_MTS_RCV_BVIO;		// Return FR_MTS_RCV_BVIO value
            break;

        case FrPSR2_MTSA_RCV_SYNERR_BVIO:   // A valid MTS has been received and a Syntax Error and 
                                            // a Boundary Violation has been detected
            return FR_MTS_RCV_SYNERR_BVIO;  // Return FR_MTS_RCV_SYNERR_BVIO value
            break;

        case FrPSR2_MTSA_NOT_RCV:       // No valid MTS has been received
            return FR_MTS_NOT_RCV;		// Return FR_MTS_NOT_RCV value
            break;

        case FrPSR2_MTSA_NOT_RCV_SYNERR:    // No valid MTS has been received and a Syntax Error was detected
            return FR_MTS_NOT_RCV_SYNERR;   // Return FR_MTS_NOT_RCV_SYNERR value
            break;

        case FrPSR2_MTSA_NOT_RCV_BVIO:		// No valid MTS has been received and a Boundary Violation has been detected
            return FR_MTS_NOT_RCV_BVIO;		// Return FR_MTS_NOT_RCV_BVIO value
            break;

        case FrPSR2_MTSA_NOT_RCV_SYNERR_BVIO:   //  No valid MTS has been received and a Syntax Error and 
                                                // a Boundary Violation has been detected
            return FR_MTS_NOT_RCV_SYNERR_BVIO;  // Return FR_MTS_NOT_RCV_SYNERR_BVIO value
            break;
        default:
            break;
        }
    }
    // Return FR_MTS_NOT_RCV value in case that this function has been called with an incorrect channel value
    else return FR_MTS_NOT_RCV;
}


/************************************************************************************
* Function name:    Fr_get_network_management_vector
* Description:      Get the vector managment vectors and copy to memory location given by reference	
*
* @author           r62779
* @version          26/07/2006
* Function arguments: 
*                   Fr_vector_ptr   This reference points to a array where the network 
*                                   management vectors shall be stored
*
* Return value:
*                   none
*************************************************************************************/
void Fr_get_network_management_vector(uint16 * Fr_vector_ptr)
{
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation
    volatile uint16 temp_value_2;               // Temporary variable used for bit operation
    uint8 Fr_p;                                 // Temporary counter used for copying    
    
    temp_value_1 = Fr_CC_reg_ptr[FrNMVLR];      // Load network management vector length (in Bytes)
    temp_value_2 = temp_value_1 / 2;            // Divide the length by 2 - conversion Bytes -> Words
    
    if(temp_value_1 & 0x0001)                   // Is the length the odd number?
    {   // The length is odd number -> it is necessary to read another vector
        temp_value_2++;                         // Increment the number of loaded vectors by 1 (in Words)
    }
    
    // Copy vectors into memory location given by reference
    for(Fr_p = 0; Fr_p < temp_value_2; Fr_p++)
    {
        Fr_vector_ptr[Fr_p] = Fr_CC_reg_ptr[FrNMVR0 + Fr_p];   // Copy all items
    }
}


/************************************************************************************
* Function name:    Fr_get_slot_status_reg_value
* Description:      Get the status vector of the required slot for the static/dynamic segment, 
*                   for the symbol window or for the NIT, on a per channel base
*
*                   Slot status observation for slots in the static and dynamic segments:
*                   This function returns content of related slot status register
*
*                   Slot status observation of the symbol window and the NIT:
*                   This function returns content of the PSR2 register
*
* @author           r62779
* @version          17/07/2006
* Function arguments: 
*					Fr_slot_number  This index is used to select a slot for which status is read	
*                   Fr_channel  FR_CHANNEL_A    Channel A is used for slot number comparison
*                               FR_CHANNEL_B    Channel B is used for slot number comparison
*                   Fr_slot_status_required FR_SLOT_STATUS_CURRENT  The newest updated slot status 
*                                                                   information is required from given slot
*                                           FR_SLOT_STATUS_PREVIOUS Not the newest slot status information 
*                                                                   is required from given slot
*                   Fr_status_vector_ptr    Reference to a memory location where the required status 
*                                           vector will be stored	
* Return value:
*                   FR_SUCCESS      API call has been successful	
*                   FR_NOT_SUCCESS  Slot status registers have not been configured or slot status 
*                                   monitoring is not configured for required slot number	
*************************************************************************************/
Fr_return_type Fr_get_slot_status_reg_value(uint16 Fr_slot_number, Fr_channel_type Fr_channel, 
                            Fr_slot_status_required_type Fr_slot_status_required, uint16 *Fr_status_vector_ptr)
{
    uint8 Fr_current_cycle;                         // Current number of communication cycle
    uint16 Fr_current_slot_A;                       // Current number of slot for channel A
    uint16 Fr_current_slot_B;                       // Current number of slot for channel B
    boolean Fr_current_slot_greater;                // Is the current slot number greater than number of the required slot?
    uint8 Fr_selection_register;                    // Which SSSRn register will be used for reading?
    		   
    Fr_current_cycle = (uint8)(Fr_CC_reg_ptr[FrCYCTR]); // Store current number of communication cycle
    Fr_current_slot_A = Fr_CC_reg_ptr[FrSLTCTAR];   // Store current number of slot for channel A
    Fr_current_slot_B = Fr_CC_reg_ptr[FrSLTCTBR];   // Store current number of slot for channel B
    
    if(Fr_channel == FR_CHANNEL_B)              // Which channel is used for slot number comparison
    {										    // Channel B is used to compare the slot number
        if(Fr_slot_number < Fr_current_slot_B)  // Comparison of the slot numbers
        {                                   
            Fr_current_slot_greater = TRUE;     // The current slot number is greater than number of the required slot
        }
        else
        {
            Fr_current_slot_greater = FALSE;    // The current slot number is smaller than number of the required slot
        }
    }
    else 		                                // Channel A is used to compare the slot number
    {
        if(Fr_slot_number < Fr_current_slot_A)  // Comparison of the slot numbers
        {                                   
            Fr_current_slot_greater = TRUE;     // The current slot number is greater than number of the required slot
        }
        else
        {
            Fr_current_slot_greater = FALSE;    // The current slot number is smaller than number of the required slot
        }
    }
    
    /* Slot status observation of the symbol window and the NIT */ 
    // It is better to use the PSR2 register for observation of the Symbol window and the NIT 
    // instead of the relevant slot status register
    if(Fr_slot_number == 0)     // Symbol window or the NIT status vector is required
    {
        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrPSR2];  // Store the content of the PSR2 register
        return FR_SUCCESS;              // API call has been successful 
    }
    /* Slot status observation for slots in the static and dynamic segments */
    // Slot status is observed by means of the SSR0 and SSR1 register?
    else if(Fr_slot_status_information_internal.SSSR0_slot_number == Fr_slot_number) 
    {
        Fr_selection_register = 0;          // Required slot is monitored by means od SSR0 or SSR1 register
    }
    // Slot status is observed by means of the SSR2 and SSR3 register?
    else if(Fr_slot_status_information_internal.SSSR1_slot_number == Fr_slot_number)
    {
        Fr_selection_register = 1;          // Required slot is monitored by means od SSR2 or SSR3 register
    }
	// Slot status is observed by means of the SSR4 and SSR5 register?
    else if(Fr_slot_status_information_internal.SSSR2_slot_number == Fr_slot_number)
    {
        Fr_selection_register = 2;          // Required slot is monitored by means od SSR4 or SSR5 register
    }
    // Slot status is observed by means of the SSR6 and SSR7 register?
    else if(Fr_slot_status_information_internal.SSSR3_slot_number == Fr_slot_number)
    {
        Fr_selection_register = 3;          // Required slot is monitored by means od SSR6 or SSR7 register
    }
    // Slot status monitoring is not configured for required slot number
    else
    {
        return FR_NOT_SUCCESS;  // Slot status monitoring is not configured for required slot number
    }

    /* Slot status observation for slots in the static and dynamic segments */
    if(Fr_slot_status_information_internal.registers_used)  // Slot status registers already congigured?
    {                               // The slot status registers have been already configured
        if(Fr_slot_status_required == FR_SLOT_STATUS_CURRENT) // The newest updated status vector should be read?
        {       // The newest updated slot status information is required from given slot
            if(Fr_current_slot_greater)			// Has been status vector already stored in this communication cycle?
            {
                // Status vector is available from this communication cycle (n) -> cycle n will be read
                if(Fr_current_cycle & 0x01)     // Determine if the cycle number is odd or even
                {                               // Current cycle number is odd
                    // Store status vector of the odd part of slot status registers
                    switch(Fr_selection_register)       // By which SSSRn register is status monitored?
                    {
                    case 0:                             // SSSR0
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR1];
                        break;
                    case 1:                             // SSSR1
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR3];
                        break;
                    case 2:                             // SSSR2
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR5];
                        break;
                    case 3:                             // SSSR3
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR7];
                        break;
                    default:
                        return FR_NOT_SUCCESS;      // Return error value
                        break;
                    }
                }
                else                            // Current cycle number is even
                {
                    // Store status vector of the even part of slot status registers
                    switch(Fr_selection_register)       // By which SSSRn register is status monitored?
                    {
                    case 0:                             // SSSR0
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR0];
                        break;
                    case 1:                             // SSSR1
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR2];
                        break;
                    case 2:                             // SSSR2
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR4];
                        break;
                    case 3:                             // SSSR3
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR6];
                        break;
                    default:
                        return FR_NOT_SUCCESS;      // Return error value
                        break;
                    }
                }
            }
            else
            {
                // Status vector is available from previous communication cycle (n-1) -> cycle n-1 will be read
                if(Fr_current_cycle & 0x01)     // Determine if the cycle number is odd or even
                {                               // Current cycle number is odd
                    // Store status vector of the even part of slot status registers
                    switch(Fr_selection_register)       // By which SSSRn register is status monitored?
                    {
                    case 0:                             // SSSR0
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR0];
                        break;
                    case 1:                             // SSSR1
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR2];
                        break;
                    case 2:                             // SSSR2
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR4];
                        break;
                    case 3:                             // SSSR3
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR6];
                        break;
                    default:
                        return FR_NOT_SUCCESS;      // Return error value
                        break;
                    }
                }
                else                            // Current cycle number is even
                {
                    // Store status vector of the odd part of slot status registers
                    switch(Fr_selection_register)       // By which SSSRn register is status monitored?
                    {
                    case 0:                             // SSSR0
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR1];
                        break;
                    case 1:                             // SSSR1
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR3];
                        break;
                    case 2:                             // SSSR2
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR5];
                        break;
                    case 3:                             // SSSR3
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR7];
                        break;
                    default:
                        return FR_NOT_SUCCESS;      // Return error value
                        break;
                    }
                }
            }
        }
        else    // Not the newest slot status information is required from given slot
        {
            if(Fr_current_slot_greater)			// Has been status vector already stored in this communication cycle?
            {
                // Status vector is available from this communication cycle (n) -> cycle n-1 will be read
                if(Fr_current_cycle & 0x01)     // Determine if the cycle number is odd or even
                {                               // Current cycle number is odd
                    // Store status vector of the even part of slot status registers
                    switch(Fr_selection_register)       // By which SSSRn register is status monitored?
                    {
                    case 0:                             // SSSR0
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR0];
                        break;
                    case 1:                             // SSSR1
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR2];
                        break;
                    case 2:                             // SSSR2
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR4];
                        break;
                    case 3:                             // SSSR3
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR6];
                        break;
                    default:
                        return FR_NOT_SUCCESS;      // Return error value
                        break;
                    }
                }
                else                            // Current cycle number is even
                {
                    // Store status vector of the odd part of slot status registers
                    switch(Fr_selection_register)       // By which SSSRn register is status monitored?
                    {
                    case 0:                             // SSSR0
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR1];
                        break;
                    case 1:                             // SSSR1
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR3];
                        break;
                    case 2:                             // SSSR2
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR5];
                        break;
                    case 3:                             // SSSR3
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR7];
                        break;
                    default:
                        return FR_NOT_SUCCESS;      // Return error value
                        break;
                    }
                }
            }
            else
            {
                // Status vector is available from previous communication cycle (n-1) -> cycle n-2 will be read
                if(Fr_current_cycle & 0x01)     // Determine if the cycle number is odd or even
                {                               // Current cycle number is odd
                    // Store status vector of the odd part of slot status registers
                    switch(Fr_selection_register)       // By which SSSRn register is status monitored?
                    {
                    case 0:                             // SSSR0
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR1];
                        break;
                    case 1:                             // SSSR1
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR3];
                        break;
                    case 2:                             // SSSR2
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR5];
                        break;
                    case 3:                             // SSSR3
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR7];
                        break;
                    default:
                        return FR_NOT_SUCCESS;      // Return error value
                        break;
                    }
                }
                else                            // Current cycle number is even
                {
                    // Store status vector of the even part of slot status registers
                    switch(Fr_selection_register)       // By which SSSRn register is status monitored?
                    {
                    case 0:                             // SSSR0
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR0];
                        break;
                    case 1:                             // SSSR1
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR2];
                        break;
                    case 2:                             // SSSR2
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR4];
                        break;
                    case 3:                             // SSSR3
                        *Fr_status_vector_ptr = Fr_CC_reg_ptr[FrSSR6];
                        break;
                    default:
                        return FR_NOT_SUCCESS;      // Return error value
                        break;
                    }
                }
            }
        }
    }
    else                            // The slot status registers have not been configured
    {
        return FR_NOT_SUCCESS;      // Return error value
    }
    return FR_SUCCESS;              // API call has been successful 
}

/************************************************************************************
* Function name:    Fr_get_channel_status_error_counter_value
* Description:      This API call reads the current value of the channel status 
*                   error counter for given channel
*
* @author           r62779
* @version          17/07/2006
* Function arguments: 
*                   Fr_channel  FR_CHANNEL_A    Channel A Status Error Counter value will 
*                                               be stored to given memory location
*                               FR_CHANNEL_B    Channel B Status Error Counter value will 
*                                               be stored to given memory location
*                   Fr_counter_value_ptr    Reference to a memory location where the required 
*                                           error counter value will be stored	
* Return value:
*                   none	
*************************************************************************************/
void Fr_get_channel_status_error_counter_value(Fr_channel_type Fr_channel, uint16 *Fr_counter_value_ptr)
{
    if(Fr_channel == FR_CHANNEL_B)          // For which channel should be error counter read?
    {                                       // Channel B
        *Fr_counter_value_ptr = Fr_CC_reg_ptr[FrCBSERCR];   // Store content of the CBSERCR reg into given memory location
    }
    else
    {                                       // Channel A
        *Fr_counter_value_ptr = Fr_CC_reg_ptr[FrCASERCR];   // Store content of the CASERCR reg into given memory location
    }
}

/************************************************************************************
* Function name:    Fr_get_slot_status_counter_value
* Description:      This API call reads the current value of relevant slot status counter
*
* @author           r62779
* @version          24/07/2006
* Function arguments: 
*                   Fr_counter_idx  FR_SLOT_STATUS_COUNTER_0  Content of the Slot Status Counter 0
*                                                             will be stored to given memory location
*                                   FR_SLOT_STATUS_COUNTER_1  Content of the Slot Status Counter 1
*                                                             will be stored to given memory location
*                                   FR_SLOT_STATUS_COUNTER_2  Content of the Slot Status Counter 2
*                                                             will be stored to given memory location
*                                   FR_SLOT_STATUS_COUNTER_3  Content of the Slot Status Counter 3
*                                                             will be stored to given memory location
*                   Fr_counter_value_ptr    Reference to a memory location where the required slot 
*                                           status counter value will be stored	
* Return value:
*                   none	
*************************************************************************************/
void Fr_get_slot_status_counter_value(Fr_slot_status_counter_ID_type Fr_counter_idx, uint16 *Fr_counter_value_ptr)
{
    // Determine which slot status counter should be configured
    if(Fr_counter_idx == FR_SLOT_STATUS_COUNTER_0)
    {   // Slot Status Counter 0 will be loaded
        *Fr_counter_value_ptr = Fr_CC_reg_ptr[FrSSCR0];   // Store content of the SSCR0 reg into given memory location
    }
    else if(Fr_counter_idx == FR_SLOT_STATUS_COUNTER_1)
    {   // Slot Status Counter 1 will be loaded
        *Fr_counter_value_ptr = Fr_CC_reg_ptr[FrSSCR1];   // Store content of the SSCR1 reg into given memory location
    }
    else if(Fr_counter_idx == FR_SLOT_STATUS_COUNTER_2)
    {   // Slot Status Counter 2 will be loaded
        *Fr_counter_value_ptr = Fr_CC_reg_ptr[FrSSCR2];   // Store content of the SSCR2 reg into given memory location
    }
    else
    {   // Slot Status Counter 3 will be loaded
        *Fr_counter_value_ptr = Fr_CC_reg_ptr[FrSSCR3];   // Store content of the SSCR3 reg into given memory location
    }
}

/************************************************************************************
* Function name:    Fr_reset_slot_status_counter
* Description:      This API call resets desired slot status counter
*					The application can reset the counter by calling the Fr_reset_slot_status_counter function 
*                   and waiting for the next cycle start, when the FlexRay module clears the counter. 
*                   Subsequently, the counter can be set into the multicycle mode again by means of 
*                   the Fr_slot_status_counter_init function.
*
* @author           r62779
* @version          24/07/2006
* Function arguments: 
*                   Fr_counter_idx  FR_SLOT_STATUS_COUNTER_0  The Slot Status Counter 0 will be reset
*                                   FR_SLOT_STATUS_COUNTER_1  The Slot Status Counter 1 will be reset
*                                   FR_SLOT_STATUS_COUNTER_2  The Slot Status Counter 2 will be reset
*                                   FR_SLOT_STATUS_COUNTER_3  The Slot Status Counter 3 will be reset
*                                                             
* Return value:
*                   none	
*************************************************************************************/
void Fr_reset_slot_status_counter(Fr_slot_status_counter_ID_type Fr_counter_idx)
{
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation
    volatile uint16 temp_value_2;               // Temporary variable used for bit operation

    // Determine which slot status counter should be reset
    if(Fr_counter_idx == FR_SLOT_STATUS_COUNTER_0)
    {   // Slot Status Counter 0 will be reset
        temp_value_1 = 0x0000;                  // Select SSCCR0 register
    }
    else if(Fr_counter_idx == FR_SLOT_STATUS_COUNTER_1)
    {   // Slot Status Counter 1 will be reset
        // Set the Selector bit field of the SSCCR register
        temp_value_1 = 0x1000;                  // Select SSCCR1 register
    }
    else if(Fr_counter_idx == FR_SLOT_STATUS_COUNTER_2)
    {   // Slot Status Counter 2 will be reset
        // Set the Selector bit field of the SSCCR register
        temp_value_1 = 0x2000;                  // Select SSCCR2 register
    }
    else
    {   // Slot Status Counter 3 will be reset
        // Set the Selector bit field of the SSCCR register
        temp_value_1 = 0x3000;                  // Select SSCCR3 register
    }

    Fr_CC_reg_ptr[FrSSCCR] = (temp_value_1 | FrSSCCR_WMD);  // Select related SSCCRn for following reading
    temp_value_2 = (Fr_CC_reg_ptr[FrSSCCR] & 0x07FF);   // Load content of related SSCCRn register and select the important bits
    temp_value_1 |= (temp_value_2 & (~FrSSCCR_MCY));    // Clear the MCY bit
    Fr_CC_reg_ptr[FrSSCCR] = temp_value_1;              // Store modified configuration back into the SSCCR register
}

/************************************************************************************
* Function name:    Fr_transmit_data
* Description:      This function is used to transmit a data from given memory array. 
*                   If Fr_data_length is zero, the function determines (reads) the length 
*                   of the data directly from the FlexRay CC. If the FlexRay driver detects 
*                   that it cannot lock the message buffer, the FR_TXMB_NO_ACCESS parameter 
*                   shall be returned. The function clears the message buffer interrupt 
*                   flag even if the buffer has not been updated or locked	
*
* @author           r62779
* @version          18/10/2006
* Function arguments: 
*                   Fr_buffer_idx	This index is used to select a transmit/receive message 
*                                   buffer of FlexRay CC
*                   Fr_data_ptr	This reference points to a array where the data to be transmitted 
*                                   is stored
*                   Fr_data_length	Determines the length of the data to be transmitted. 
*                                   If Fr_data_length is zero, the function determines (reads)
*                                   the length of the data directly from the FlexRay CC
* Return value:
*                   FR_TXMB_UPDATED	The transmit message buffer has been successfully updated 
*                                   with new data
*                   FR_TXMB_NO_ACCESS	The transmit message buffer has not been 
*                                   successfully locked
*************************************************************************************/
Fr_tx_MB_status_type Fr_transmit_data(uint16 Fr_buffer_idx, const uint16 *Fr_data_ptr, uint8 Fr_data_length)
{
    volatile uint16 * FR_DATA_FAR header_MB_ptr;        // Message buffer header pointer
    volatile uint16 * FR_DATA_FAR Fr_CC_data_ptr;       // Reference to data array
    volatile uint16 temp_value_1 = 0, temp_value_2 = 0; // Temporary variables used for bit operation
    volatile uint16 Fr_MB_registers_offset_add_temp; // Temporary offset adress of message buffer registers
    uint16 Fr_p;                                        // Temporary counter used for copying
    
    // Temporary offset address of MB registers
    Fr_MB_registers_offset_add_temp = Fr_buffer_idx * 4;

    // Read the MB Configuration, Control and Status reg.
    // Load MBCCSRn register and select only necessary bits
    temp_value_1 = (Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] & 0xF900);
    temp_value_1 |= FrMBCCSR_LCKT;              // Set Lock Trigger bit
    Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;  // Lock MB
    
    // Read the MB again for checking if the MB is locked
    temp_value_2 = Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp];  
    if(temp_value_2 & FrMBCCSR_LCKS)            // Is the MB locked?
    {                                           // Yes
        // Clear Transmit Buffer Interrupt Flag    
        temp_value_2 &= 0xF900;                 // Select only necessary bits    
        temp_value_2 |= FrMBCCSR_MBIF;          // Set MBIF flag - clear TX MB interrupt flag 
        Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_2;  // Clear Transmit Buffer Interrupt Flag

        // Load current MB index
        Fr_buffer_idx = Fr_CC_reg_ptr[FrMBIDXR0 + Fr_MB_registers_offset_add_temp];

        // Calculate the message buffer header 
        header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + Fr_buffer_idx * 5);
        
        // Calculate MB Data pointer
        Fr_CC_data_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                         (header_MB_ptr[3] / 2));

        // Test if given data length is not zero, then read length from 
        if(Fr_data_length == 0)
        {
            Fr_data_length = (uint8)(header_MB_ptr[1] & 0x007F);   // Load the data payload length value from Frame header register
        }
        
        // Copy data
        for(Fr_p = 0; Fr_p < Fr_data_length; Fr_p++)
        {
            Fr_CC_data_ptr[Fr_p] = Fr_data_ptr[Fr_p];   // Copy all items
        }
        
        // Set MB to commit
        // Load MBCCSRn register and select only necessary bits
        temp_value_1 = (Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] & 0xF900);
        temp_value_1 |= FrMBCCSR_CMT;
        Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
        
        // Attempt to unlock MB
        // Load MBCCSRn register and select only necessary bits
        temp_value_1 = (Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] & 0xF900);
        temp_value_1 |= FrMBCCSR_LCKT;      // Trigger lock/unlock
        Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
    }
    else
    {
        // Clear Transmit Buffer Interrupt Flag    
        // Read MBCCSRn and select only necessary bits
        temp_value_2 = (Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] & 0xF900);
        temp_value_2 |= FrMBCCSR_MBIF;          // Set MBIF flag - clear TX MB interrupt flag 
        Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_2;  // Clear Transmit Buffer Interrupt Flag
        return FR_TXMB_NO_ACCESS;       // The transmit MB has not been successfully locked
    }
    return FR_TXMB_UPDATED;                 // The transmit MB has been updated with new data
}

/************************************************************************************
* Function name:    Fr_receive_data
* Description:      This function is used to receive a data and copy to given memory array. 
*                   If a data has been successfully received, tha data will be copied 
*                   to the buffer referenced by Fr_data_ptr and FR_RXMB_RECEIVED parameter 
*                   will be returned.
*                   If a data has not been received, no copying will take place to the 
*                   message aray referenced by Fr_data_ptr and FR_RXMB_NOT_RECEIVED parameter 
*                   will be returned.
*                   If a Nullframe has been received, no copying will take place to the 
*                   message aray referenced by Fr_data_ptr, FR_RXMB_NULL_FRAME_RECEIVED parameter 
*                   will be returned and the length “0” will be stored in Fr_data_length.
*                   If the FlexRay driver detects that it cannot lock the message buffer, 
*                   the FR_RXMB_NO_ACCESS parameter shall be returned.	
*                   The function clears the message buffer interrupt flag
*
* @author           r62779
* @version          18/10/2006
* Function arguments: 
*                   Fr_buffer_idx	This index is used to select a transmit/receive message 
*                                   buffer of FlexRay CC
*                   Fr_data_ptr	    This reference points to a array where the data to be 
*                                   received shall be stored
*                   Fr_data_length_ptr	This reference points to the memory location where 
*                                   the length of received data shall be stored
*                   Fr_slot_status_ptr	This reference points to the memory location where 
*                                   the slot status of received data shall be stored
* Return value:
*                   FR_RXMB_RECEIVED	Data has been received
*                   FR_RXMB_NOT_RECEIVED	Data has not been received
*                   FR_RXMB_NULL_FRAME_RECEIVED	Received frame is a NULL frame
*                   FR_RXMB_NO_ACCESS	The receive message buffer has not been successfully locked
*************************************************************************************/
Fr_rx_MB_status_type Fr_receive_data(uint16 Fr_buffer_idx, uint16 *Fr_data_ptr, 
                                    uint8 *Fr_data_length_ptr,uint16 *Fr_slot_status_ptr)
{
    volatile uint16 * FR_DATA_FAR header_MB_ptr;    // Message buffer header pointer
    volatile uint16 * FR_DATA_FAR Fr_CC_data_ptr;   // Reference to data array
    volatile uint16 temp_value_1 = 0;           // Temporary variable used for bit operation
    volatile uint16 temp_value_2 = 0;           // Temporary variable used for bit operation
    volatile uint16 Fr_MB_registers_offset_add_temp; // Temporary offset adress of message buffer registers
    volatile uint8 Fr_data_length;              // Data payload length
    uint16 Fr_p;                                // Temporary counter used for copying

    // Temporary offset address of MB registers
    //Fr_MB_registers_offset_add_temp = Fr_buffers_config_ptr[Fr_buffer_info_set_index].buffer_index_init * 4;
    Fr_MB_registers_offset_add_temp = Fr_buffer_idx * 4;
   
    // Read the MB Configuration, Control and Status reg.
    temp_value_1 = Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp];
    
    temp_value_1 |= FrMBCCSR_LCKT;      // Set Lock Trigger bit
    Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;  // Lock MB

    // Read the MB again for checking if the MB is locked
    temp_value_2 = Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp];  
    if(temp_value_2 & FrMBCCSR_LCKS)            // Is the MB locked?
    {
        // Clear Receive Buffer Interrupt Flag    
        temp_value_2 &= 0xF900;                 // Select only necessary bits    
        temp_value_2 |= FrMBCCSR_MBIF;          // Set MBIF flag - clear RX MB interrupt flag 
        Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_2;  // Clear Transmit Buffer Interrupt Flag

        // Load current MB index
        Fr_buffer_idx = Fr_CC_reg_ptr[FrMBIDXR0 + Fr_MB_registers_offset_add_temp];
        // Calculate the message buffer header 
        header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + Fr_buffer_idx * 5);
        
        // Read Slot Status 
        temp_value_2 = header_MB_ptr[4];        // Load Slot Status Information
        *Fr_slot_status_ptr = temp_value_2;     // Store the slot status into Fr_slot_status_ptr
    
        // If DUP == 1 : Frame header and MB data field updated
        if(temp_value_1 & FrMBCCSR_DUP) 
        {   // Yes, Valid non-null frame received
            // Calculate MB Data pointer
            Fr_CC_data_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                             (header_MB_ptr[3] / 2));
            // Load data length from Frame header reg.
            Fr_data_length = (uint8)(header_MB_ptr[1] & 0x007F);   // Load the data payload length value from Frame header register
            
            // Copy data
            for(Fr_p = 0; Fr_p < Fr_data_length; Fr_p++)
            {
                Fr_data_ptr[Fr_p] = Fr_CC_data_ptr[Fr_p];   // Copy all items
            }
        }
        else    // DUP == 0 : Data has not been updated, however NULL frame may have been received
        {
            // Attempt to unlock MB
            // Read the MBCCSRn register and select only necessary bits
            temp_value_1 = (Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] & 0xF900);
            temp_value_1 |= FrMBCCSR_LCKT;
            Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;

            // Check if at least one valid frame has been received - from Slot Status reg., VFA and VFB bits
            if((temp_value_2 & FrF_HEADER_VFA) || (temp_value_2 & FrF_HEADER_VFB)) // Valid frame received?
            {   // Yes, DUP == 0 and VFA or VFB == 1 -> NFA or NFB == 0 -> valid null frame receved
                *Fr_data_length_ptr = 0x00;             // Store the length "0" into Fr_data_length_ptr
                return FR_RXMB_NULL_FRAME_RECEIVED;     // Valid null frame has been received
            }
            else    // No valid frame received
            {
                return FR_RXMB_NOT_RECEIVED;       // No valid frame has been received
            }
        }
    }
    else
    {
        // Clear Receive Buffer Interrupt Flag    
        temp_value_1 &= 0xF900;                 // Select only necessary bits    
        temp_value_1 |= FrMBCCSR_MBIF;          // Set MBIF flag - clear RX MB interrupt flag 
        Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;  // Clear Transmit Buffer Interrupt Flag
        return FR_RXMB_NO_ACCESS;         // The receive MB has not been successfully locked
    }
    // Attempt to unlock MB
    // Read the MBCCSRn register and select only necessary bits
    temp_value_1 = (Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] & 0xF900);
    temp_value_1 |= FrMBCCSR_LCKT;
    Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;
    
    *Fr_data_length_ptr = Fr_data_length;   // Store length into Fr_data_length_ptr
    return FR_RXMB_RECEIVED;                // The receive MB has been updated with new data
}

/************************************************************************************
* Function name:    Fr_receive_fifo_data
* Description:      This function is used to receive a data and copy to given memory array. 
*                   If a data has been successfully received, tha data will be copied to the buffer 
*                   referenced by Fr_data_ptr and FR_FIFO_RECEIVED parameter will be returned.
*                   If a data has not been received, no copying will take place to the message 
*                   aray referenced by Fr_data_ptr and FR_FIFO_NOT_RECEIVED parameter will be returned.
*                   
*
* @author           r62779
* @version          10/11/2006
* Function arguments: 
*                   Fr_buffer_read_idx  	This index is used to select next available receive 
*                                           FIFO message buffer that the application can read
*                   Fr_data_ptr	            This reference points to a array where the data to be 
*                                           received shall be stored
*                   Fr_data_length	        This reference points to the memory location where 
*                                           the length of received data shall be stored
*                   Fr_slot_idx_ptr	        This reference points to the memory location where 
*                                           the received frame ID will be stored
*                   Fr_slot_status	        This reference points to the memory location where 
*                                           the slot status of received data shall be stored
* Return value:
*                   FR_FIFO_RECEIVED        Data has been received
*                   FR_FIFO_NOT_RECEIVED    Data has not been received
*                   
*************************************************************************************/
Fr_FIFO_status_type Fr_receive_fifo_data(uint16 Fr_buffer_read_idx, uint16 *Fr_data_ptr, uint8 *Fr_data_length_ptr,
                                        uint16 *Fr_slot_idx_ptr, uint16 *Fr_slot_status_ptr)
{
    volatile uint16 * FR_DATA_FAR header_MB_ptr;    // Message buffer header pointer
    volatile uint16 * FR_DATA_FAR Fr_CC_data_ptr;   // Reference to data array
    volatile uint16 temp_value_1 = 0;           // Temporary variable used for bit operation
    volatile uint16 temp_value_2 = 0;           // Temporary variable used for bit operation
    volatile uint8 Fr_data_length;              // Data payload length
    uint16 Fr_p;                                // Temporary counter used for copying

    // Calculate the message buffer header 
    header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                    Fr_buffer_read_idx * 5);

    // Read Slot Status 
    temp_value_1 = header_MB_ptr[0];                // Load MB header field
    temp_value_2 = header_MB_ptr[4];                // Load Slot Status Information
    *Fr_slot_status_ptr = temp_value_2;             // Store the slot status into Fr_slot_status_ptr
    *Fr_slot_idx_ptr = (header_MB_ptr[0] & 0x07FF); // Store the Frame ID into Fr_slot_idx_ptr

    // If NUF == 1 : Null frame received
    if(temp_value_1 & FrF_HEADER_NUF)               // Valid non-null frame received?
    {   // Yes, Valid non-null frame received
        // Calculate MB Data pointer
        Fr_CC_data_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                        (header_MB_ptr[3] / 2));
        // Load data length from Frame header reg.
        Fr_data_length = (uint8)(header_MB_ptr[1] & 0x007F);   // Load the data payload length value from Frame header register
        
        // Copy data
        for(Fr_p = 0; Fr_p < Fr_data_length; Fr_p++)
        {
            Fr_data_ptr[Fr_p] = Fr_CC_data_ptr[Fr_p];   // Copy all items
        }
    }
    else    // (NULL frame may have been received - not implemented on current FlexRay modules, reportted bug FR_UNI_15)
    {
        return FR_FIFO_NOT_RECEIVED;            // No valid frame has been received

        /* // NULL frame may have been received - not implemented on current FlexRay modules, reportted bug FR_UNI_15
        // Check if Valid Frame has been received - from Slot Status reg., VFA and VFB bits
        if((temp_value_2 & FrF_HEADER_VFA) || (temp_value_2 & FrF_HEADER_VFB)) // Valid frame received?
        {   // Yes -> valid null frame receved

            // Receive FIFO Buffer Interrupt Flag is not cleared in this function

            *Fr_data_length_ptr = 0x00;             // Store the length "0" into Fr_data_length_ptr

            // Valid null frame has been received - not implemented on current FlexRay modules, reportted bug FR_UNI_15
            //return FR_FIFO_NULL_FRAME_RECEIVED;
            
        }
        else    // No valid frame received
        {
            // Receive FIFO Buffer Interrupt Flag is not cleared in this function

            return FR_FIFO_NOT_RECEIVED;            // No valid frame has been received
        }
        */
    }
    
    *Fr_data_length_ptr = Fr_data_length;           // Store length into Fr_data_length_ptr
    return FR_FIFO_RECEIVED;                        // The receive FIFO MB has been updated with new data
}


/************************************************************************************
* Function name:    Fr_enable_interrupts
* Description:      Thi API call enables an interrupt in the GIFER, PIER0 and PIER1 registers. 
*                   An user may put combinated defined macros into input parametrs. 
*                   If Fr_global_interrupt input parameter is zero, 
*                   this function will not update the GIFER register,
*                   otherwise the GIFER register will be updated.
*                   If Fr_protocol_0_interrupt parameter is zero, this function will not update the PIER0 
*                   register, otherwise the PIER0 register will be updated.
*                   If Fr_protocol_1_interrupt parameter is zero, this function will not update the PIER1 
*                   register, otherwise the PIER1 register will be updated.
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   Fr_global_interrupt          The name of interrupt in the GIFER reg.
*                   Fr_protocol_0_interrupt      The name of interrupt in the PIFR0 reg.
*                   Fr_protocol_1_interrupt		 The name of interrupt in the PIFR1 reg.
* Return value:
*                   none     
*************************************************************************************/
void Fr_enable_interrupts(uint16 Fr_global_interrupt, uint16 Fr_protocol_0_interrupt, uint16 Fr_protocol_1_interrupt)
{
    volatile uint16 temp_value_1;       // Temporary varibale used for bit operations
    
    if(Fr_global_interrupt)             // Fr_global_interrupt parameter is not equal to zero
    {
        // Enable Interrupts in the GIFER register
        temp_value_1 = Fr_CC_reg_ptr[FrGIFER];  // Load content of GIFER reg.
        temp_value_1 |= Fr_global_interrupt;    // Set bit(s)
        Fr_CC_reg_ptr[FrGIFER] = temp_value_1;  // Store new settings
    }
    temp_value_1 = 0x0000;              // Clear variable
    if(Fr_protocol_0_interrupt)         // Fr_protocol_0_interrupt parameter is not equal to zero
    {
        // Enable Interrupts in the PIER0 register
        temp_value_1 = Fr_CC_reg_ptr[FrPIER0];      // Load content of PIER0 reg.
        temp_value_1 |= Fr_protocol_0_interrupt;    // Set bit(s)
        Fr_CC_reg_ptr[FrPIER0] = temp_value_1;      // Store new settings
    }
    temp_value_1 = 0x0000;              // Clear variable
    if(Fr_protocol_1_interrupt)         // Fr_protocol_1_interrupt parameter is not equal to zero
    {
        // Enable Interrupts in the PIER1 register
        temp_value_1 = Fr_CC_reg_ptr[FrPIER1];      // Load content of PIER0 reg.
        temp_value_1 |= Fr_protocol_1_interrupt;    // Set bit(s)
        Fr_CC_reg_ptr[FrPIER1] = temp_value_1;      // Store new settings
    }
}

/************************************************************************************
* Function name:    Fr_disable_interrupts
* Description:      Thi API call disables an interrupt in the GIFER, PIER0 and PIER1 registers. 
*                   An user may put combinated defined macros into input parametrs. 
*                   If Fr_global_interrupt input parameter is zero, 
*                   this function will not update the GIFER register,
*                   otherwise the GIFER register will be updated.
*                   If Fr_protocol_0_interrupt parameter is zero, this function will not update the PIER0 
*                   register, otherwise the PIER0 register will be updated.
*                   If Fr_protocol_1_interrupt parameter is zero, this function will not update the PIER1 
*                   register, otherwise the PIER1 register will be updated.
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   Fr_global_interrupt          The name of interrupt in the GIFER reg.
*                   Fr_protocol_0_interrupt      The name of interrupt in the PIFR0 reg.
*                   Fr_protocol_1_interrupt		 The name of interrupt in the PIFR1 reg.
* Return value:
*                   none     
*************************************************************************************/
void Fr_disable_interrupts(uint16 Fr_global_interrupt, uint16 Fr_protocol_0_interrupt, uint16 Fr_protocol_1_interrupt)
{
    volatile uint16 temp_value_1;       // Temporary varibale used for bit operations
    
    if(Fr_global_interrupt)             // Fr_global_interrupt parameter is not equal to zero
    {
        // Disable Interrupts in the GIFER register
        temp_value_1 = Fr_CC_reg_ptr[FrGIFER];  // Load content of GIFER reg.
        temp_value_1 &= (~Fr_global_interrupt); // Clear bit(s)
        Fr_CC_reg_ptr[FrGIFER] = temp_value_1;  // Store new settings
    }
    temp_value_1 = 0x0000;              // Clear variable
    if(Fr_protocol_0_interrupt)         // Fr_protocol_0_interrupt parameter is not equal to zero
    {
        // Disable Interrupts in the PIER0 register
        temp_value_1 = Fr_CC_reg_ptr[FrPIER0];      // Load content of PIER0 reg.
        temp_value_1 &= (~Fr_protocol_0_interrupt); // Clears bit(s)
        Fr_CC_reg_ptr[FrPIER0] = temp_value_1;      // Store new settings
    }
    temp_value_1 = 0x0000;              // Clear variable
    if(Fr_protocol_1_interrupt)         // Fr_protocol_1_interrupt parameter is not equal to zero
    {
        // Disbale Interrupts in the PIER1 register
        temp_value_1 = Fr_CC_reg_ptr[FrPIER1];      // Load content of PIER0 reg.
        temp_value_1 &= (~Fr_protocol_1_interrupt); // Clear bit(s)
        Fr_CC_reg_ptr[FrPIER1] = temp_value_1;      // Store new settings
    }
}


/************************************************************************************
* Function name:    Fr_interrupt_handler
* Description:      This function shoudl be called in interrupt service routine when 
*                   an interrupt occured. The API call check which interrupt source 
*                   generates the interrupt and if an appropriate callback function is called 
*                   if configured. The interrupt flags are cleared except the transmit or 
*                   receive message buffer interrupts (these are cleared by 
*                   Fr_transmit_data or Fr_receive_data)
*
* @author           r62779
* @version          1/11/2006
* Function arguments: 
*                   none
*
* Return value:
*                   none     
*************************************************************************************/
void Fr_interrupt_handler(void)
{
    volatile uint16 isr_event;              // Content of the GIFER register is stored into this varibale
    volatile uint16 temp_isr_event;         // Temporary variable intended for bit operation with isr_event variable
    uint8 buffer_idx;                       // Which MB generated the interrupt
    uint16 buffer_fifo_idx;                 // Which FIFO MB generated the interrupt
    volatile uint16 chierfr_event;          // Content of the CHIERFR register is stored into this variable
    volatile uint16 isr_prif_0_event;       // Content of the PIFR0 register is stored into this varibale
    volatile uint16 isr_prif_1_event;       // Content of the PIFR1 register is stored into this varibale
    volatile uint16 temp_value_1, temp_value_2;     // Temporary variables intended for bit operations
    
    isr_event = Fr_CC_reg_ptr[FrGIFER];     // Read the GIFER register
    temp_isr_event = (isr_event & 0x00FF);  // Select only lower eight bit
    isr_event &= (temp_isr_event << 8);     // Move selected bits and separate only enabled interrupt flags
        
    if((isr_event & FrGIFER_TBIF) != 0)     // Transmit buffer interrupt flag set?
    {   // Yes, tx interrupt occured
        buffer_idx = (uint8)((Fr_CC_reg_ptr[FrMBIVEC] & 0x7F00) >> 8); // Read Transmit Buffer Interrupt Vector
        if(Fr_MB_information_internal[buffer_idx].Fr_MB_function_ptr != NULL)       // Service callback function was configured?
        {
            Fr_MB_information_internal[buffer_idx].Fr_MB_function_ptr(buffer_idx);  // Call service callback function
        }
    }
    if((isr_event & FrGIFER_RBIF) != 0)     // Receive buffer interrupt flag set?
    {   // Yes, rx interrupt occured
        buffer_idx = (uint8)(Fr_CC_reg_ptr[FrMBIVEC] & 0x007F);    // Read Receive Buffer Interrupt Vector
        if(Fr_MB_information_internal[buffer_idx].Fr_MB_function_ptr != NULL)       // Service callback function was configured?
        {
            Fr_MB_information_internal[buffer_idx].Fr_MB_function_ptr(buffer_idx);  // Call service callback function
        }
    }
    if((isr_event & FrGIFER_FNEAIF) != 0)     // FIFO A interrupt flag set?
    {   // Yes, FIFO A Not empty interrupt occured
        buffer_fifo_idx = (Fr_CC_reg_ptr[FrRFARIR] & 0x03FF);   // Read Receive FIFO A Read Index
        if(Fr_callback_functions.Fr_fifo_A_ptr != NULL)     // Service callback function was configured?
        {
            Fr_callback_functions.Fr_fifo_A_ptr(buffer_fifo_idx);   // Call service callback function
        }
        // Read the GIFER reg. and clear the WUPIF, FNEBIF and FNEAIF bits
        temp_value_1 = (Fr_CC_reg_ptr[FrGIFER] & 0xE3FF);
        Fr_CC_reg_ptr[FrGIFER] = (FrGIFER_FNEAIF | temp_value_1);   // Clear interrupt flag
    }
    if((isr_event & FrGIFER_FNEBIF) != 0)     // FIFO B interrupt flag set?
    {   // Yes, FIFO B Not empty interrupt occured
        buffer_fifo_idx = (uint8)(Fr_CC_reg_ptr[FrRFBRIR] & 0x03FF);    // Read Receive FIFO B Read Index
        if(Fr_callback_functions.Fr_fifo_B_ptr != NULL)     // Service callback function was configured?
        {
            Fr_callback_functions.Fr_fifo_B_ptr(buffer_fifo_idx);   // Call service callback function
        }
        // Read the GIFER reg. and clear the WUPIF, FNEBIF and FNEAIF bits
        temp_value_2 = (Fr_CC_reg_ptr[FrGIFER] & 0xE3FF);
        Fr_CC_reg_ptr[FrGIFER] = (FrGIFER_FNEBIF | temp_value_2);   // Clear interrupt flag
    }
    if((isr_event & FrGIFER_WUPIF) != 0)     // Wakeup interrupt flag set?
    {   // Yes, Wakeup interrupt occured
        if(Fr_callback_functions.Fr_wakeup_ptr != NULL)     // Service callback function was configured?
        {
            if(Fr_CC_reg_ptr[FrPSR3] & FrPSR3_WUA)          // Determine on which channel has been wakeup received
            {                                               // On channel A
                Fr_callback_functions.Fr_wakeup_ptr(FR_CHANNEL_A);  // Call service callback function
                Fr_CC_reg_ptr[FrPSR3] = FrPSR3_WUA;         // Clear flag
            }
            else if(Fr_CC_reg_ptr[FrPSR3] & FrPSR3_WUB)     // Determine on which channel has been wakeup received
            {                                               // On channel B
                Fr_callback_functions.Fr_wakeup_ptr(FR_CHANNEL_B);  // Call service callback function
                Fr_CC_reg_ptr[FrPSR3] = FrPSR3_WUB;         // Clear flag
            }
        }
        // Read the GIFER reg. and clear the WUPIF, FNEBIF and FNEAIF bits
        temp_value_2 = (Fr_CC_reg_ptr[FrGIFER] & 0xE3FF);
        Fr_CC_reg_ptr[FrGIFER] = (FrGIFER_WUPIF | temp_value_2);    // Clear interrupt flag
    }

    if((isr_event & FrGIFER_CHIF) != 0)         // CHI interrupt flag set?
    {   // Yes, at least one CHI error flag is asserted
        chierfr_event = Fr_CC_reg_ptr[FrCHIERFR];       // Read CHI Error Flag register
        if(Fr_callback_functions.Fr_chi_ptr != NULL)    // Service callback function was configured?
        {
            Fr_callback_functions.Fr_chi_ptr(chierfr_event);    // Call service callback function
        }
        Fr_CC_reg_ptr[FrCHIERFR] = chierfr_event;        // Clear interrupt flag
    }

    if((isr_event & FrGIFER_PRIF) != 0)     // Protocol interrupt flag set?
    {   // Yes, protocol interrupt occured
        // It is possible to use either one callback function for all Protocol Interrupt flags or 
        // more separate functions for individual flags
        isr_prif_0_event = Fr_CC_reg_ptr[FrPIFR0];      // Read the PIFR0 register
        isr_prif_1_event = Fr_CC_reg_ptr[FrPIFR1];      // Read the PIFR1 register
        temp_value_1 = Fr_CC_reg_ptr[FrPIER0];          // Read the PIER0 register
        isr_prif_0_event &= temp_value_1;               // Select only enabled interrupt
        temp_value_2 = Fr_CC_reg_ptr[FrPIER1];          // Read the PIER1 register
        isr_prif_1_event &= temp_value_2;               // Select only enabled interrupt

        /* More separate callback functions for individual Protocol Interrupt flags */
        /* PIFR0 register */
        if((isr_prif_0_event & FR_CYCLE_START_IRQ) != 0)                // Cycle Start interrupt flag set?
        {   // Yes, cycle start occured
            if(Fr_callback_functions.Fr_cycle_start_ptr != NULL)        // Service callback function was configured?
            {
                Fr_callback_functions.Fr_cycle_start_ptr();             // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_CYCLE_START_IRQ;                // Clear interrupt flag
        }

        if((isr_prif_0_event & FR_TIMER_1_EXPIRED_IRQ) != 0)            // Timer 1 expired interrupt flag set?
        {   // Yes, timer 1 expired
            if(Fr_callback_functions.Fr_timer_1_expired_ptr != NULL)    // Service callback function was configured?
            {
                Fr_callback_functions.Fr_timer_1_expired_ptr();         // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_TIMER_1_EXPIRED_IRQ;            // Clear interrupt flag
        }

        if((isr_prif_0_event & FR_TIMER_2_EXPIRED_IRQ) != 0)            // Timer 2 expired interrupt flag set?
        {   // Yes, timer 2 expired
            if(Fr_callback_functions.Fr_timer_2_expired_ptr != NULL)    // Service callback function was configured?
            {
                Fr_callback_functions.Fr_timer_2_expired_ptr();         // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_TIMER_2_EXPIRED_IRQ;            // Clear interrupt flag
        }

        if((isr_prif_0_event & FR_TRANSMISSION_ACROSS_BOUNDARY_CHA_IRQ) != 0) // Transmission across boundary occured on channel A?
        {   // Yes, transmission across boundary occured
            if(Fr_callback_functions.Fr_trans_across_boundary_A_ptr != NULL)    // Service callback function was configured?
            {
                Fr_callback_functions.Fr_trans_across_boundary_A_ptr();         // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_TRANSMISSION_ACROSS_BOUNDARY_CHA_IRQ;   // Clear interrupt flag
        }

        if((isr_prif_0_event & FR_TRANSMISSION_ACROSS_BOUNDARY_CHB_IRQ) != 0) // Transmission across boundary occured on channel B?
        {   // Yes, transmission across boundary occured
            if(Fr_callback_functions.Fr_trans_across_boundary_B_ptr != NULL)    // Service callback function was configured?
            {
                Fr_callback_functions.Fr_trans_across_boundary_B_ptr();         // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_TRANSMISSION_ACROSS_BOUNDARY_CHB_IRQ;   // Clear interrupt flag
        }
       
        if((isr_prif_0_event & FR_VIOLATION_CHA_IRQ) != 0)              // pLatestTx violation occured on channel A?
        {   // Yes, pLatestTx violation occured
            if(Fr_callback_functions.Fr_violation_A_ptr != NULL)        // Service callback function was configured?
            {
                Fr_callback_functions.Fr_violation_A_ptr();             // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_VIOLATION_CHA_IRQ;              // Clear interrupt flag
        }

        if((isr_prif_0_event & FR_VIOLATION_CHB_IRQ) != 0)              // pLatestTx violation occured on channel B?
        {   // Yes, pLatestTx violation occured
            if(Fr_callback_functions.Fr_violation_B_ptr != NULL)        // Service callback function was configured?
            {
                Fr_callback_functions.Fr_violation_B_ptr();             // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_VIOLATION_CHB_IRQ;              // Clear interrupt flag
        }

        if((isr_prif_0_event & FR_MEDIA_ACCESS_TEST_SYMBOL_RECEIVED_IRQ) != 0)  // MTS symbol received?
        {   // Yes, MTS symbol received
            if(Fr_callback_functions.Fr_mts_received_ptr != NULL)       // Service callback function was configured?
            {
                Fr_callback_functions.Fr_mts_received_ptr();            // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_MEDIA_ACCESS_TEST_SYMBOL_RECEIVED_IRQ;  // Clear interrupt flag
        }

        if((isr_prif_0_event & FR_MAX_SYNC_FRAMES_DETECTED_IRQ) != 0)   // Max Sync Frames Detected?
        {   // Yes, More than node_sync_max sync frames detected
            if(Fr_callback_functions.Fr_max_sync_frames_ptr != NULL)    // Service callback function was configured?
            {
                Fr_callback_functions.Fr_max_sync_frames_ptr();         // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_MAX_SYNC_FRAMES_DETECTED_IRQ;   // Clear interrupt flag
        }

        if((isr_prif_0_event & FR_CLOCK_CORRECTION_LIMIT_IRQ) != 0)     // Clock correction limit reached?
        {   // Yes, Offset or rate correction limit reached
            if(Fr_callback_functions.Fr_clock_corr_limit_ptr != NULL)   // Service callback function was configured?
            {
                Fr_callback_functions.Fr_clock_corr_limit_ptr();        // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_CLOCK_CORRECTION_LIMIT_IRQ;     // Clear interrupt flag
        }

        if((isr_prif_0_event & FR_MISSING_OFFSET_CORRECTION_IRQ) != 0)  // Missing Offset Correction Interrupt?
        {   // Yes, Insufficient number of measurements for offset correction detected
            if(Fr_callback_functions.Fr_missing_offset_corr_ptr != NULL) // Service callback function was configured?
            {
                Fr_callback_functions.Fr_missing_offset_corr_ptr();     // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_MISSING_OFFSET_CORRECTION_IRQ;  // Clear interrupt flag
        }

        if((isr_prif_0_event & FR_MISSING_RATE_CORRECTION_IRQ) != 0)    // Missing Rate Correction Interrupt?
        {   // Yes, Insufficient number of measurements for rate correction detected
            if(Fr_callback_functions.Fr_missing_rate_corr_ptr != NULL)  // Service callback function was configured?
            {
                Fr_callback_functions.Fr_missing_rate_corr_ptr();       // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_MISSING_RATE_CORRECTION_IRQ;    // Clear interrupt flag
        }

        if((isr_prif_0_event & FR_COLDSTART_ABORT_IRQ) != 0)            // Coldstart Abort Interrupt?
        {   // Yes, Coldstart aborted and no more coldstart attempts allowed
            if(Fr_callback_functions.Fr_coldstart_abort_ptr != NULL)    // Service callback function was configured?
            {
                Fr_callback_functions.Fr_coldstart_abort_ptr();         // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_COLDSTART_ABORT_IRQ;            // Clear interrupt flag
        }

        if((isr_prif_0_event & FR_ILLEGAL_PROTOCOL_CONFIGURATION_IRQ) != 0) // Illegal protocol congiguration detected?
        {   // Yes, illegal protocol configuration detected
            if(Fr_callback_functions.Fr_illegal_protocol_conf_ptr != NULL)  // Service callback function was configured?
            {
                Fr_callback_functions.Fr_illegal_protocol_conf_ptr();       // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_ILLEGAL_PROTOCOL_CONFIGURATION_IRQ; // Clear interrupt flag
        }																	

        if((isr_prif_0_event & FR_INTERNAL_PROTOCOL_ERROR_IRQ) != 0)    // Internal protocol error detected?
        {   // Yes, internal protocol error detected
            if(Fr_callback_functions.Fr_internal_protocol_error_ptr != NULL)    // Service callback function was configured?
            {
                Fr_callback_functions.Fr_internal_protocol_error_ptr(); // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_INTERNAL_PROTOCOL_ERROR_IRQ;    // Clear interrupt flag
        }																	

        if((isr_prif_0_event & FR_FATAL_PROTOCOL_ERROR_IRQ) != 0)       // Fatal protocol error detected?
        {   // Yes, fatal protocol error detected
            if(Fr_callback_functions.Fr_fatal_protocol_error_ptr != NULL)   // Service callback function was configured?
            {
                Fr_callback_functions.Fr_fatal_protocol_error_ptr(); // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR0] = FR_FATAL_PROTOCOL_ERROR_IRQ;    // Clear interrupt flag
        }																	

        /* PIFR1 register */
        if((isr_prif_1_event & FR_ERROR_MODE_CHANGED_IRQ) != 0)         // Error mode Changed Interrupt?
        {   // Yes, ERRMODE field changed
            if(Fr_callback_functions.Fr_error_mode_changed_ptr != NULL) // Service callback function was configured?
            {
                Fr_callback_functions.Fr_error_mode_changed_ptr();      // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR1] = FR_ERROR_MODE_CHANGED_IRQ;         // Clear interrupt flag
        }

        if((isr_prif_1_event & FR_ILLEGAL_PROTOCOL_COMMAND_IRQ) != 0)   // Illegal protocol command detected?
        {   // Yes, illegal protocol state change command detected
            if(Fr_callback_functions.Fr_illegal_protocol_command_ptr != NULL) // Service callback function was configured?
            {
                Fr_callback_functions.Fr_illegal_protocol_command_ptr();      // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR1] = FR_ILLEGAL_PROTOCOL_COMMAND_IRQ;         // Clear interrupt flag
        }
        
        if((isr_prif_1_event & FR_PROTOCOL_ENGINE_COM_FAILURE) != 0)    // Protocol engine communication failure detected?
        {   // Yes, protocol engine communication failure detected
            if(Fr_callback_functions.Fr_protocol_engine_comm_failure_ptr != NULL) // Service callback function was configured?
            {
                Fr_callback_functions.Fr_protocol_engine_comm_failure_ptr();    // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR1] = FR_PROTOCOL_ENGINE_COM_FAILURE;            // Clear interrupt flag
        }

        if((isr_prif_1_event & FR_PROTOCOL_STATE_CHANGED_IRQ) != 0)     // Protocol state changed?
        {   // Yes, protocol state changed
            if(Fr_callback_functions.Fr_protocol_state_changed_ptr != NULL) // Service callback function was configured?
            {
                Fr_callback_functions.Fr_protocol_state_changed_ptr();  // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR1] = FR_PROTOCOL_STATE_CHANGED_IRQ;     // Clear interrupt flag
        }

        if((isr_prif_1_event & FR_SLOT_STATUS_COUNTER_3_INCREMENTED_IRQ) != 0)  // Slot status counter 3 incremented?
        {   // Yes, slot status counter 3 has incremented
            if(Fr_callback_functions.Fr_slot_status_counter_3_inc_ptr != NULL)  // Service callback function was configured?
            {
                Fr_callback_functions.Fr_slot_status_counter_3_inc_ptr();       // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR1] = FR_SLOT_STATUS_COUNTER_3_INCREMENTED_IRQ;  // Clear interrupt flag
        }
        
        if((isr_prif_1_event & FR_SLOT_STATUS_COUNTER_2_INCREMENTED_IRQ) != 0)  // Slot status counter 2 incremented?
        {   // Yes, slot status counter 2 has incremented
            if(Fr_callback_functions.Fr_slot_status_counter_2_inc_ptr != NULL)  // Service callback function was configured?
            {
                Fr_callback_functions.Fr_slot_status_counter_2_inc_ptr();       // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR1] = FR_SLOT_STATUS_COUNTER_2_INCREMENTED_IRQ;  // Clear interrupt flag
        }
        
        if((isr_prif_1_event & FR_SLOT_STATUS_COUNTER_1_INCREMENTED_IRQ) != 0)  // Slot status counter 1 incremented?
        {   // Yes, slot status counter 1 has incremented
            if(Fr_callback_functions.Fr_slot_status_counter_1_inc_ptr != NULL)  // Service callback function was configured?
            {
                Fr_callback_functions.Fr_slot_status_counter_1_inc_ptr();       // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR1] = FR_SLOT_STATUS_COUNTER_1_INCREMENTED_IRQ;  // Clear interrupt flag
        }
        
        if((isr_prif_1_event & FR_SLOT_STATUS_COUNTER_0_INCREMENTED_IRQ) != 0)  // Slot status counter 0 incremented?
        {   // Yes, slot status counter 0 has incremented
            if(Fr_callback_functions.Fr_slot_status_counter_0_inc_ptr != NULL)  // Service callback function was configured?
            {
                Fr_callback_functions.Fr_slot_status_counter_0_inc_ptr();       // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR1] = FR_SLOT_STATUS_COUNTER_0_INCREMENTED_IRQ;  // Clear interrupt flag
        }

        if((isr_prif_1_event & FR_EVEN_CYCLE_TABLE_WRITTEN_IRQ) != 0)           // Even cycle table written?
        {   // Yes, sync frame measurement table written
            if(Fr_callback_functions.Fr_even_cycle_table_written_ptr != NULL)  // Service callback function was configured?
            {
                Fr_callback_functions.Fr_even_cycle_table_written_ptr();       // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR1] = FR_EVEN_CYCLE_TABLE_WRITTEN_IRQ;           // Clear interrupt flag
        }

        if((isr_prif_1_event & FR_ODD_CYCLE_TABLE_WRITTEN_IRQ) != 0)           // Odd cycle table written?
        {   // Yes, sync frame measurement table written
            if(Fr_callback_functions.Fr_odd_cycle_table_written_ptr != NULL)  // Service callback function was configured?
            {
                Fr_callback_functions.Fr_odd_cycle_table_written_ptr();       // Call service callback function
            }
            Fr_CC_reg_ptr[FrPIFR1] = FR_ODD_CYCLE_TABLE_WRITTEN_IRQ;           // Clear interrupt flag
        }
        
        /* One callback function for all Protocol interrupt flags */
        if(Fr_callback_functions.Fr_protocol_ptr != NULL)   // Service callback function was configured?
        {
            Fr_callback_functions.Fr_protocol_ptr();        // Call service callback function
            // Be careful - all enabled flags in the PIFR0 and PIFR1 are cleared
            Fr_CC_reg_ptr[FrPIFR0] = isr_prif_0_event;      // Clear interrupt flag
            Fr_CC_reg_ptr[FrPIFR1] = isr_prif_1_event;      // Clear interrupt flag
        }
    }

    /* One callback function for all interrupt flags - no interrupt flas are cleared */
    if((isr_event & FrGIFER_MIF) != 0)         // Module interrupt flag set?
    {   // Yes, at least one of the other interrupt flags in the GIFER reg. is asserted
        if(Fr_callback_functions.Fr_module_ptr != NULL) // Service callback function was configured?
        {
            Fr_callback_functions.Fr_module_ptr();      // Call service callback function
            // Be careful - no interrupt flags are cleared by this function call
            // For interrupt flag clearing, it is neccessary to set a service routine for appropriate individual interrupt
        }
    }
}

/************************************************************************************
* Function name:    Fr_set_MB_callback
* Description:      This function stores the reference to the callback function into internal 
*                   structure. When an transmit/receive interrupt arrises and Fr_interrupt_handler 
*                   function is called, the function referenced by MB_callback_ptr pointer 
*                   is called with the number of MB (in Fr_int_buffer_idx) which generated 
*                   the interrupt.
*                   This API call does not set the MBIE bit in the MBCCSRn register or 
*                   the TBIE/RBIE bit in the GIFER register, i.e. it does not enable 
*                   the transmit/receive interrupt
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   MB_callback_ptr  The reference to the function which should be 
*                                   called by Fr_interrupt_handler function when 
*                                   an interrupt occured
*                   Fr_buffer_idx	This index is used to select a transmit message 
*                                   buffer of FlexRay CC.
*
* Return value:
*                   none     
*************************************************************************************/
void Fr_set_MB_callback(void (* MB_callback_ptr)(uint8 Fr_int_buffer_idx), uint8 Fr_buffer_idx)
{
    // Store the reference to the function into internal structure
    Fr_MB_information_internal[Fr_buffer_idx].Fr_MB_function_ptr = MB_callback_ptr;
}

/************************************************************************************
* Function name:    Fr_set_global_IRQ_callback
* Description:      This function stores the reference to the callback function into internal 
*                   structure. When an interrupt arrises and Fr_interrupt_handler 
*                   function is called, the function referenced by callback_ptr pointer 
*                   is called.
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   callback_ptr    The reference to the function which should be 
*                                   called by Fr_interrupt_handler function when 
*                                   an interrupt occured
*				    Fr_global_interrupt     The name of interrupt which should be selected
* Return value:
*                   FR_SUCCESS	        API call has been successful
*                   FR_NOT_SUCCESS	    Wrong input parameter
*************************************************************************************/
Fr_return_type Fr_set_global_IRQ_callback(void (* callback_ptr)(void), uint16 Fr_global_interrupt)
{
    // Store the reference to the function into internal structure
    switch(Fr_global_interrupt)
    {
    case FR_MODULE_IRQ:
        Fr_callback_functions.Fr_module_ptr = callback_ptr;
        break;
    case FR_PROTOCOL_IRQ:
        Fr_callback_functions.Fr_protocol_ptr = callback_ptr;
        break;

    //case FR_CHI_IRQ:
    // CHI Interrupt callback function can be defined by means od Fr_set_chi_IRQ_callback function

    //case FR_WAKEUP_IRQ:
    // Wakeup Interrupt callback function can be defined by means od Fr_set_wakeup_IRQ_callback function
    
    //case FR_FIFO_B_IRQ:
    // FIFO B Interrupt callback function can be defined by means of Fr_set_FIFO_IRQ_callback function
    
    //case FR_FIFO_A_IRQ:
    // FIFO A Interrupt callback function can be defined by means of Fr_set_FIFO_IRQ_callback function    
    
    // Transmit and receive MB callback interrupt functions can be defined by Fr_set_MB_callback function
    
    default:
        return FR_NOT_SUCCESS;      // API call has not been successful - wrong parameter
        break;
    }
    return FR_SUCCESS;              // API call has been successful
}

/************************************************************************************
* Function name:    Fr_set_protocol_0_IRQ_callback
* Description:      This function stores the reference to the callback function 
*                   into internal structure. When an Protocol interrupt 
*                   (asserted with PIFR0 register) arrises and Fr_interrupt_handler function 
*                   is called, the function referenced by callback_ptr pointer is called. 
*                   This API call does not enable any interrupt
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   callback_ptr    The reference to the function which should be 
*                                   called by Fr_interrupt_handler function when 
*                                   an protocol interrupt occured
*				    Fr_protocol_interrupt     The name of interrupt which should be selected
* Return value:
*                   FR_SUCCESS	        API call has been successful
*                   FR_NOT_SUCCESS	    Wrong input parameter
*************************************************************************************/
Fr_return_type Fr_set_protocol_0_IRQ_callback(void (* callback_ptr)(void), uint16 Fr_protocol_interrupt)
{
    // Store the reference to the function into internal structure
    switch(Fr_protocol_interrupt)
    {
    case FR_FATAL_PROTOCOL_ERROR_IRQ:
        Fr_callback_functions.Fr_fatal_protocol_error_ptr = callback_ptr;
        break;
    case FR_INTERNAL_PROTOCOL_ERROR_IRQ:
        Fr_callback_functions.Fr_internal_protocol_error_ptr = callback_ptr;
        break;
    case FR_ILLEGAL_PROTOCOL_CONFIGURATION_IRQ:
        Fr_callback_functions.Fr_illegal_protocol_conf_ptr = callback_ptr;
        break;
    case FR_COLDSTART_ABORT_IRQ:
        Fr_callback_functions.Fr_coldstart_abort_ptr = callback_ptr;
        break;
    case FR_MISSING_RATE_CORRECTION_IRQ:
        Fr_callback_functions.Fr_missing_rate_corr_ptr = callback_ptr;
        break;
    case FR_MISSING_OFFSET_CORRECTION_IRQ:
        Fr_callback_functions.Fr_missing_offset_corr_ptr = callback_ptr;
        break;
    case FR_CLOCK_CORRECTION_LIMIT_IRQ:
        Fr_callback_functions.Fr_clock_corr_limit_ptr = callback_ptr;
        break;
    case FR_MAX_SYNC_FRAMES_DETECTED_IRQ:
        Fr_callback_functions.Fr_max_sync_frames_ptr = callback_ptr;
        break;
    case FR_MEDIA_ACCESS_TEST_SYMBOL_RECEIVED_IRQ:
        Fr_callback_functions.Fr_mts_received_ptr = callback_ptr;
        break;
    case FR_VIOLATION_CHB_IRQ:
        Fr_callback_functions.Fr_violation_B_ptr = callback_ptr;
        break;
    case FR_VIOLATION_CHA_IRQ:
        Fr_callback_functions.Fr_violation_A_ptr = callback_ptr;
        break;
    case FR_TRANSMISSION_ACROSS_BOUNDARY_CHB_IRQ:
        Fr_callback_functions.Fr_trans_across_boundary_B_ptr = callback_ptr;
        break;
    case FR_TRANSMISSION_ACROSS_BOUNDARY_CHA_IRQ:
        Fr_callback_functions.Fr_trans_across_boundary_A_ptr = callback_ptr;
        break;
    case FR_TIMER_2_EXPIRED_IRQ:
        Fr_callback_functions.Fr_timer_2_expired_ptr = callback_ptr;
        break;
    case FR_TIMER_1_EXPIRED_IRQ:
        Fr_callback_functions.Fr_timer_1_expired_ptr = callback_ptr;
        break;
    case FR_CYCLE_START_IRQ:
        Fr_callback_functions.Fr_cycle_start_ptr = callback_ptr;
        break;
    default:
        return FR_NOT_SUCCESS;      // API call has not been successful - wrong parameter
        break;
    }
    return FR_SUCCESS;              // API call has been successful
}

/************************************************************************************
* Function name:    Fr_set_protocol_1_IRQ_callback
* Description:      This function stores the reference to the callback function 
*                   into internal structure. When an Protocol interrupt 
*                   (asserted with PIFR1 register) arrises and Fr_interrupt_handler function 
*                   is called, the function referenced by callback_ptr pointer is called. 
*                   This API call does not enable any interrupt
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   callback_ptr    The reference to the function which should be 
*                                   called by Fr_interrupt_handler function when 
*                                   an protocol interrupt occured
*				    Fr_protocol_interrupt     The name of interrupt which should be selected
* Return value:
*                   FR_SUCCESS	        API call has been successful
*                   FR_NOT_SUCCESS	    Wrong input parameter
*************************************************************************************/
Fr_return_type Fr_set_protocol_1_IRQ_callback(void (* callback_ptr)(void), uint16 Fr_protocol_interrupt)
{
    // Store the reference to the function into internal structure
    switch(Fr_protocol_interrupt)
    {
    case FR_ERROR_MODE_CHANGED_IRQ:
        Fr_callback_functions.Fr_error_mode_changed_ptr = callback_ptr;
        break;
    case FR_ILLEGAL_PROTOCOL_COMMAND_IRQ:
        Fr_callback_functions.Fr_illegal_protocol_command_ptr = callback_ptr;
        break;
    case FR_PROTOCOL_ENGINE_COM_FAILURE:
        Fr_callback_functions.Fr_protocol_engine_comm_failure_ptr = callback_ptr;
        break;
    case FR_PROTOCOL_STATE_CHANGED_IRQ:
        Fr_callback_functions.Fr_protocol_state_changed_ptr = callback_ptr;
        break;
    case FR_SLOT_STATUS_COUNTER_3_INCREMENTED_IRQ:
        Fr_callback_functions.Fr_slot_status_counter_3_inc_ptr = callback_ptr;
        break;
    case FR_SLOT_STATUS_COUNTER_2_INCREMENTED_IRQ:
        Fr_callback_functions.Fr_slot_status_counter_2_inc_ptr = callback_ptr;
        break;
    case FR_SLOT_STATUS_COUNTER_1_INCREMENTED_IRQ:
        Fr_callback_functions.Fr_slot_status_counter_1_inc_ptr = callback_ptr;
        break;
    case FR_SLOT_STATUS_COUNTER_0_INCREMENTED_IRQ:
        Fr_callback_functions.Fr_slot_status_counter_0_inc_ptr = callback_ptr;
        break;
    case FR_EVEN_CYCLE_TABLE_WRITTEN_IRQ:
        Fr_callback_functions.Fr_even_cycle_table_written_ptr = callback_ptr;
        break;
    case FR_ODD_CYCLE_TABLE_WRITTEN_IRQ:
        Fr_callback_functions.Fr_odd_cycle_table_written_ptr = callback_ptr;
        break;
    default:
        return FR_NOT_SUCCESS;      // API call has not been successful - wrong parameter
        break;
    }
    return FR_SUCCESS;              // API call has been successful
}


/************************************************************************************
* Function name:    Fr_set_wakeup_IRQ_callback
* Description:      This function stores the reference to the callback function into 
*                   internal structure. When Wakeup interrupt arrises and Fr_interrupt_handler 
*                   function is called, the function referenced by callback_ptr pointer is 
*                   called with information on which channle the wakeup symbol was received
*                   (in wakeup_channel). This API call does not enable the Wakeup interrupt
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   callback_ptr    The reference to the function which should be 
*                                   called by Fr_interrupt_handler function when 
*                                   Wakeup interrupt occured
* Return value:
*                   none
*************************************************************************************/
void Fr_set_wakeup_IRQ_callback(void (* callback_ptr)(Fr_channel_type wakeup_channel))
{
    // Store the reference to the function into internal structure
    Fr_callback_functions.Fr_wakeup_ptr = callback_ptr;
}
    
/************************************************************************************
* Function name:    Fr_set_wakeup_IRQ_callback
* Description:      This function stores the reference to the callback function into 
*                   internal structure. When CHI interrupt arrises and Fr_interrupt_handler 
*                   function is called, the function referenced by callback_ptr pointer is 
*                   called with content of CHIERFR register (in chi_error). This API call 
*                   does not enable the CHI interrupt
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   callback_ptr    The reference to the function which should be 
*                                   called by Fr_interrupt_handler function when 
*                                   CHI interrupt occured
* Return value:
*                   none
*************************************************************************************/
void Fr_set_chi_IRQ_callback(void (* callback_ptr)(uint16 chi_error))
{
    // Store the reference to the function into internal structure
    Fr_callback_functions.Fr_chi_ptr = callback_ptr;
}


/************************************************************************************
* Function name:    Fr_set_fifo_IRQ_callback
* Description:      This function stores the reference to the callback function into 
*                   internal structure. When an FIFO interrupt arrises and Fr_interrupt_handler 
*                   function is called, the function referenced by callback_ptr pointer is 
*                   called with content of the RFARIR or RFBRIR register (in header_index). This API call 
*                   does not enable the FIFO interrupts
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   callback_ptr    The reference to the function which should be 
*                                   called by Fr_interrupt_handler function when 
*                                   an FIFO interrupt occured
*				    Fr_global_interrupt     The name of interrupt which should be selected
*
* Return value:
*                   FR_SUCCESS	        API call has been successful
*                   FR_NOT_SUCCESS	    Wrong input parameter
*************************************************************************************/
Fr_return_type Fr_set_fifo_IRQ_callback(void (* callback_ptr)(uint16 header_index), uint16 Fr_global_interrupt)
{
    // Store the reference to the function into internal structure
    switch(Fr_global_interrupt)
    {
    case FR_FIFO_B_IRQ:
        Fr_callback_functions.Fr_fifo_B_ptr = callback_ptr;
        break;
    case FR_FIFO_A_IRQ:
        Fr_callback_functions.Fr_fifo_A_ptr = callback_ptr;
        break;
    default:
        return FR_NOT_SUCCESS;      // API call has not been successful - wrong parameter
        break;
    }
    return FR_SUCCESS;              // API call has been successful
}

/************************************************************************************
* Function name:    Fr_clear_MB_interrupt_flag
* Description:      This function clears an interrupt flag of a transmit/receive message buffer. 
*                   Interrupt flag are cleared also by Fr_transmit_data or Fr_receive_data function. 
*                   This function can be called e.g. for clearing interrupt flag of transmit side 
*                   of the double message buffer
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   Fr_buffer_idx		This index is used to select a transmit/receive message 
*                                       buffer of FlexRay CC for which an interrupt flag will be cleared
* Return value:
*                   None
*
*************************************************************************************/
void Fr_clear_MB_interrupt_flag(uint8 Fr_buffer_idx)
{
    volatile uint16 temp_value_1 = 0;           // Temporary variable used for bit operation
    volatile uint16 Fr_MB_registers_offset_add_temp; // Temporary offset adress of message buffer registers

    // Temporary offset address of MB registers
    Fr_MB_registers_offset_add_temp = Fr_buffer_idx * 4;

    // Clear Transmit Buffer Interrupt Flag    
    // Read MBCCSRn and select only necessary bits
    temp_value_1 = (Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] & 0xF900);
    temp_value_1 |= FrMBCCSR_MBIF;          // Set MBIF flag - clear TX MB interrupt flag 
    Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp] = temp_value_1;  // Clear Transmit Buffer Interrupt Flag
}

/************************************************************************************
* Function name:    Fr_start_timer
* Description:      This function starts timer T1 or T2	
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   timer_ID	This parameter determines which timer (T1 or T2) 
*                               will start
* Return value:
*                   none
*************************************************************************************/
void Fr_start_timer(Fr_timer_ID_type timer_ID)
{
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation
    
    temp_value_1 = Fr_CC_reg_ptr[FrTICCR];      // Load content of the Timer Configuration and Control reg.
    if(timer_ID == FR_TIMER_T1)                 // Which timer should start
    {                                           // Timer T1
        temp_value_1 |= FrTICCR_T1TR;           // Start bit for T1
    }
    else if(timer_ID == FR_TIMER_T2)            // Which timer should start
    {                                           // Timer T2
        temp_value_1 |= FrTICCR_T2TR;           // Start bit for T2
    }
    Fr_CC_reg_ptr[FrTICCR] = temp_value_1;  // Store new value to the Timer Configuration and Control reg.
}

/************************************************************************************
* Function name:    Fr_stop_timer
* Description:      This function stops timer T1 or T2	
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   timer_ID	This parameter determines which timer (T1 or T2) 
*                               will stop
* Return value:
*                   none
*************************************************************************************/
void Fr_stop_timer(Fr_timer_ID_type timer_ID)
{
    volatile uint16 temp_value_1;               // Temporary variable used for bit operation
    
    temp_value_1 = Fr_CC_reg_ptr[FrTICCR];      // Load content of the Timer Configuration and Control reg.
    if(timer_ID == FR_TIMER_T1)                 // Which timer should stop
    {                                           // Timer T1
        temp_value_1 |= FrTICCR_T1SP;           // Stop bit for T1
    }
    else if(timer_ID == FR_TIMER_T2)            // Which timer should stop
    {                                           // Timer T2
        temp_value_1 |= FrTICCR_T2SP;           // Stop bit for T2
    }
    Fr_CC_reg_ptr[FrTICCR] = temp_value_1;  // Store new value to the Timer Configuration and Control reg.
}


/************************************************************************************
* Function name:    Fr_check_tx_status
* Description:      Check whether or not the transmission of the data referenced 
*                   by Fr_buffer_idx has been performed	
*
* @author           r62779
* @version          10/06/2006
* Function arguments: 
*                   Fr_buffer_idx	This index is used to select a transmit message 
*                                   buffer of FlexRay CC.
* Return value:
*                   FR_TRANSMITTED	Data has been transmitted
*                                   commit side - transferred
*                                   transmit side - transmitted
*                   FR_NOT_TRANSMITTED	Data has not been transmitted
*                                   commit side - not transferred
*                                   transmit side - not transmitted
*                   FR_INTERNAL_MESSAGE_TRANSFER_DONE - only for double MB
*                                   commit side - transferred
*                                   transmit side - not transmitted
*                   FR_TRANSMIT_SIDE_TRANSMITTED - only for double MB
*                                   commit side - not transferred
*                                   transmit side - transmitted
*************************************************************************************/
Fr_tx_status_type Fr_check_tx_status(uint8 Fr_buffer_idx)
{
    volatile uint16 temp_value_1 = 0, temp_value_2 = 0; // Temporary variable used for bit operation
    volatile uint16 Fr_MB_registers_offset_add_temp; // Temporary offset adress of message buffer registers

    // Temporary offset address of MB registers
    Fr_MB_registers_offset_add_temp = Fr_buffer_idx * 4;
    
    if(Fr_MB_information_internal[Fr_buffer_idx].transmission_mode == FR_DOUBLE_TRANSMIT_BUFFER)
    {
        // Read the MB Configuration, Control and Status reg. - the commit side of the double buffered MB
        temp_value_1 = Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp];
        // Read the MB Configuration, Control and Status reg. - the transmit side of the double buffered MB
        temp_value_2 = Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp + 4];
        
        // The flag operation are different for STATE and EVENT mode, so determine in which mode the MB works
        if(Fr_MB_information_internal[Fr_buffer_idx].transmission_type == FR_STATE_TRANSMISSION_MODE)
        {   // MB works in the STATE transmission mode, DVAL bit has to be check at the transmit side, CMT bit at the commit side
            if(temp_value_2 & FrMBCCSR_DVAL)    // Check if DVAL bit is set at the transmit side
            {                                   // Yes -> data has been transmitted from transmit side
                if(temp_value_1 & FrMBCCSR_CMT) // Check if CMT bit is set at the commit side
                {                               // Yes -> data has not been transmitted from commit side
                    return FR_TRANSMIT_SIDE_TRANSMITTED;
                }
                else                            // No -> data has been transmitted from commit side
                {
                    return FR_TRANSMITTED;
                }
            }
            else                                // No -> data has not been transmitted yet from transmit side
            {
                if(temp_value_1 & FrMBCCSR_CMT) // Check if CMT bit is set at the commit side
                {                               // Yes -> data has not been transmitted from commit side
                    return FR_NOT_TRANSMITTED;
                }
                else                            // No -> data has been transmitted from commit side
                {
                    return FR_INTERNAL_MESSAGE_TRANSFER_DONE;
                }
            }
        }
        else // MB works in the EVENT transmission mode, CMT bit has to be check at the transmit side and also at the commit side
        {
            if(temp_value_2 & FrMBCCSR_CMT)     // Check if CMT bit is set at the transmit side
            {                                   // Yes -> data has not been transmitted yet
                if(temp_value_1 & FrMBCCSR_CMT) // Check if CMT bit is set at the commit side
                {                               // Yes -> data has not been transmitted from commit side
                    return FR_NOT_TRANSMITTED;
                }
                else                            // No -> data has been transmitted from commit side
                {
                    return FR_INTERNAL_MESSAGE_TRANSFER_DONE;
                }
            }
            else                                // No -> data has been transmitted from the transmit side
            {
                if(temp_value_1 & FrMBCCSR_CMT) // Check if CMT bit is set at the commit side
                {                               // Yes -> data has not been transmitted from commit side
                    return FR_TRANSMIT_SIDE_TRANSMITTED;
                }
                else                            // No -> data has been transmitted from commit side
                {
                    return FR_TRANSMITTED;
                }
            }
        }
    }
    else
    {
        // Read the MB Configuration, Control and Status reg.
        temp_value_1 = Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp];

        // The flag operation are different for STATE and EVENT mode, so determine in which mode the MB works
        if(Fr_MB_information_internal[Fr_buffer_idx].transmission_type == FR_STATE_TRANSMISSION_MODE)
        {   // MB works in the STATE transmission mode, DVAL bit has to be check
            if(temp_value_1 & FrMBCCSR_DVAL)    // Check if DVAL bit is set
            {                                   // Yes -> data has been transmitted
                return FR_TRANSMITTED;
            }
            else                                // No -> data has not been transmitted yet
            {
                return FR_NOT_TRANSMITTED;
            }
        }
        else    // MB works in the EVENT transmission mode, CMT bit has to be check
        {
            if(temp_value_1 & FrMBCCSR_CMT)     // Check if CMT bit is set
            {                                   // Yes -> data has not been transmitted yet
                return FR_NOT_TRANSMITTED;
            }
            else                                // No -> data has been transmitted
            {
                return FR_TRANSMITTED;
            }
        }
    }
}

/************************************************************************************
* Function name:    Fr_check_rx_status
* Description:      Check whether or not the reception of the frame referenced by 
*                    Fr_buffer_idx has been performed
*
* @author           r62779
* @version          18/10/2006
* Function arguments: 
*                   Fr_buffer_idx	This index is used to select a receive message 
*                                   buffer of FlexRay CC.
* Return value:
*                   FR_RECEIVED	- valid non-null frame has been received
*                   FR_NOT_RECEIVED	- no valid frame has been received
*                   FR_NULL_FRAME_RECEIVED	- null frame has been received
*************************************************************************************/
Fr_rx_status_type Fr_check_rx_status(uint8 Fr_buffer_idx)
{
    volatile uint16 * FR_DATA_FAR header_MB_ptr;    // Message buffer header pointer
    volatile uint16 temp_value_1 = 0;           // Temporary variable used for bit operation
    volatile uint16 temp_value_2 = 0;           // Temporary variable used for bit operation
    volatile uint16 Fr_MB_registers_offset_add_temp; // Temporary offset adress of message buffer registers

    // Temporary offset address of MB registers
    Fr_MB_registers_offset_add_temp = Fr_buffer_idx * 4;
   
    // Read the MB Configuration, Control and Status reg.
    temp_value_1 = Fr_CC_reg_ptr[FrMBCCSR0 + Fr_MB_registers_offset_add_temp];

    // Calculate the message buffer header 
    header_MB_ptr = ((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + \
                        Fr_buffer_idx * 5);

    // If DUP == 1 : Frame header and MB data field updated
    if(temp_value_1 & FrMBCCSR_DUP) 
    {   // Yes, Valid non-null frame received
        return FR_RECEIVED;                // Valid non-null frame received
    }
    else    // DUP == 0 : Data has not been updated, however NULL frame may have been received
    {
        // Read Slot Status 
        temp_value_2 = header_MB_ptr[4];    // Load Slot Status Information

        // Check if at least one valid frame has been received - from Slot Status reg., VFA and VFB bits
        if((temp_value_2 & FrF_HEADER_VFA) || (temp_value_2 & FrF_HEADER_VFB)) // Valid frame received?
        {   // Yes, DUP == 0 and VFA or VFB == 1 -> NFA or NFB == 0 -> valid null frame receved
            return FR_NULL_FRAME_RECEIVED;     // Null frame has been received
        }
        else    // No valid frame received
        {
            return FR_NOT_RECEIVED;       // No valid frame has been received
        }
    }
}

/************************************************************************************
* Function name:    Fr_check_CHI_error
* Description:      Check whether or not the CHI related error flags has been occurred 
*                   and clear the flag in the CHIERFR register
*
* @author           r62779
* @version          10/07/2006
* Function arguments: 
*                   none
* Return value:
*                   Fr_CHI_error_type - content of the CHIERFR register
*************************************************************************************/
Fr_CHI_error_type Fr_check_CHI_error(void)
{
    volatile uint16 temp_value_1 = 0;           // Temporary variable used for bit operation
    temp_value_1 = Fr_CC_reg_ptr[FrCHIERFR];    // Read the CHIERFR register
    if(temp_value_1 != 0)                       // Clear flags
    {
        Fr_CC_reg_ptr[FrCHIERFR] = temp_value_1;    // Clear error flags
    }
    return temp_value_1;                        // Return the content of the CHIERFR register
}

/************************************************************************************
* Function name:    Fr_check_cycle_start
* Description:      Check whether or not the communication cycle has been started.
*                   The function should clear pending interrupt flag
*
* @author           r62779
* @version          12/07/2006
* Function arguments: 
*                   Fr_cycle_ptr    Reference to a memory location where the current FlexRay
*                                   cummunication cycle will be stored
* Return value:
*                   FALSE           Communication cycle has not been started
*                   TRUE            Communication cycle started
*************************************************************************************/
boolean Fr_check_cycle_start(uint8 *Fr_cycle_ptr)
{
    if(Fr_CC_reg_ptr[FrPIFR0] & FR_CYCLE_START_IRQ)
    {
        *Fr_cycle_ptr = (uint8)(Fr_CC_reg_ptr[FrCYCTR] & 0x003F);   // Store the number of current communication cycle
        Fr_CC_reg_ptr[FrPIFR0] = FR_CYCLE_START_IRQ;                // Clear interrupt flag
        return TRUE;                                                // Return TRUE - communication cycle has already started
    }
    return FALSE;                                                   // Return FALSE - communication cycle has not been started
}

/************************************************************************************
* Function name:    Fr_check_transmission_across_boundary
* Description:      Check whether or not the frame transmission across boundary 
*                   error flag has been occurred. If the FR_CHANNEL_AB parameter is placed,
*                   the function will test both channels and return TRUE value in case that 
*                   error flag occurs on at least one of channels.
*                   The function should clear pending interrupt flag
*
* @author           r62779
* @version          12/07/2006
* Function arguments: 
*                   Fr_channel  This parameter is used to select a channel for which 
*                               an error flag is tested
* Return value:
*                   FALSE       The frame transmission across boundary error flag 
*                               has not been occurred
*                   TRUE        The frame transmission across boundary error flag has been occurred
*************************************************************************************/
boolean Fr_check_transmission_across_boundary(Fr_channel_type Fr_channel)
{
    volatile uint16 temp_value_1 = 0;       // Temporary variable used for bit operation

    temp_value_1 = Fr_CC_reg_ptr[FrPIFR0];  // Store the PIFR0 register into temporary variable
    
    switch(Fr_channel)                      // Which channel should be tested?
    {
    case FR_CHANNEL_A: 						// Channel A
        if(temp_value_1 & FR_TRANSMISSION_ACROSS_BOUNDARY_CHA_IRQ)     // TBVA_IF interrupt flag set?
        {   // Yes, Transmission across boundary violation occured on channel A
            Fr_CC_reg_ptr[FrPIFR0] = FR_TRANSMISSION_ACROSS_BOUNDARY_CHA_IRQ;    // Clear interrupt flag
            return TRUE;                    // Return TRUE - Interrupt has been occurred on channel A
        }
        break;
    case FR_CHANNEL_B:						// Channel B
        if(temp_value_1 & FR_TRANSMISSION_ACROSS_BOUNDARY_CHB_IRQ)     // TBVB_IF interrupt flag set?
        {   // Yes, Transmission across boundary violation occured on channel B
            Fr_CC_reg_ptr[FrPIFR0] = FR_TRANSMISSION_ACROSS_BOUNDARY_CHB_IRQ;   // Clear interrupt flag
            return TRUE;                    // Return TRUE - Interrupt has been occurred on channel B
        }
        break;
    case FR_CHANNEL_AB:						// Both channels
        // TBVA_IF or/and TBVB_IF interrupt flag set?
        if((temp_value_1 & FR_TRANSMISSION_ACROSS_BOUNDARY_CHA_IRQ) || (temp_value_1 & FR_TRANSMISSION_ACROSS_BOUNDARY_CHB_IRQ))
		{                                   // Transmission across boundary violation occured on channel A and/or B
            // Clear both interrupt flags
            Fr_CC_reg_ptr[FrPIFR0] = (FR_TRANSMISSION_ACROSS_BOUNDARY_CHB_IRQ | FR_TRANSMISSION_ACROSS_BOUNDARY_CHA_IRQ);   
            return TRUE;                    // Return TRUE - Interrupt has been occurred on channel A and/or channel B
		}
        break;
    default:
        break;
    }
    return FALSE;                           // Error flag has not been set or wrong input parameter
}

/************************************************************************************
* Function name:    Fr_check_violation
* Description:      Check whether or not the frame transmission in dynamic segment 
*                   exceeded the dynamic segment boundary. If the FR_CHANNEL_AB parameter 
*                   is placed, the function will test both channels and return TRUE value 
*                   in case that error flag occurs on at least one of channels.
*                   The function should clear pending interrupt flag
*
* @author           r62779
* @version          13/07/2006
* Function arguments: 
*                   Fr_channel	This parameter is used to select a channel for which 
*                               an error flag is tested
* Return value:
*                   FALSE   The violation flag has not been occurred
*                   TRUE    The violation flag has been occurred
*************************************************************************************/
boolean Fr_check_violation(Fr_channel_type Fr_channel)
{
    volatile uint16 temp_value_1 = 0;       // Temporary variable used for bit operation

    temp_value_1 = Fr_CC_reg_ptr[FrPIFR0];  // Store the PIFR0 register into temporary variable
    
    switch(Fr_channel)                      // Which channel should be tested?
    {
    case FR_CHANNEL_A: 						// Channel A
        if(temp_value_1 & FR_VIOLATION_CHA_IRQ)                 // LTXA_IF interrupt flag set?
        {   // Yes, pdLatestTx Violation interrupt occured on channel A
            Fr_CC_reg_ptr[FrPIFR0] = FR_VIOLATION_CHA_IRQ;      // Clear interrupt flag
            return TRUE;                    // Return TRUE - Interrupt has been occurred on channel A
        }
        break;
    case FR_CHANNEL_B:						// Channel B
        if(temp_value_1 & FR_VIOLATION_CHB_IRQ)                 // LTXB_IF interrupt flag set?
        {   // Yes, pdLatestTx Violation interrupt occured on channel B
            Fr_CC_reg_ptr[FrPIFR0] = FR_VIOLATION_CHB_IRQ;      // Clear interrupt flag
            return TRUE;                    // Return TRUE - Interrupt has been occurred on channel B
        }
        break;
    case FR_CHANNEL_AB:						// Both channels
        // LTXA_IF or/and LTXB_IF interrupt flag set?
        if((temp_value_1 & FR_VIOLATION_CHA_IRQ) || (temp_value_1 & FR_VIOLATION_CHB_IRQ))
		{                                   // pdLatestTx Violation interrupt occured on channel A and/or B
            // Clear both interrupt flags
            Fr_CC_reg_ptr[FrPIFR0] = (FR_VIOLATION_CHA_IRQ | FR_VIOLATION_CHB_IRQ);   
            return TRUE;                    // Return TRUE - Interrupt has been occurred on channel A and/or channel B
		}
        break;
    default:
        break;
    }
    return FALSE;                           // Error flag has not been set or wrong input parameter
}

/************************************************************************************
* Function name:    Fr_check_max_sync_frame
* Description:      Check whether or not the number of synchronization frames detected 
*                   in the current communication cycle exceeded the value of the 
*                   node_sync_max field in the Protocol Configuration Register 30.
*                   The function should clear pending interrupt flag
*
* @author           r62779
* @version          13/07/2006
* Function arguments: 
*                   none
* Return value:
*                   FALSE   The number of synchronization frames doesn’t exceed the value 
*                           of the node_sync_max
*                   TRUE    The number of synchronization frames detected in the current 
*                           communication cycle exceeds the value of the node_sync_max
*************************************************************************************/
boolean Fr_check_max_sync_frame(void)
{
    if(Fr_CC_reg_ptr[FrPIFR0] & FR_MAX_SYNC_FRAMES_DETECTED_IRQ)    // MXS_IF interrupt flag set?
    {   // Yes, Max Sync Frames Detected interrupt occured
        Fr_CC_reg_ptr[FrPIFR0] = FR_MAX_SYNC_FRAMES_DETECTED_IRQ;      // Clear interrupt flag
        return TRUE;                    // Return TRUE - Error flag has been set
    }
    else 
    {
        return FALSE;                   // Error flag has not been set
    }
}

/************************************************************************************
* Function name:    Fr_check_clock_correction_limit_reached
* Description:      Check whether or not the internal calculated values have reached or 
*                   exceeded its configured thresholds as given by the offset_correction_out 
*                   field in the Protocol Configuration Register 9 and the rate_correction_out field 
*                   in the Protocol Configuration Register 14.
*                   The function should clear pending interrupt flag
*
* @author           r62779
* @version          13/07/2006
* Function arguments: 
*                   none
* Return value:
*                   FALSE   Offset or rate correction limit has not been reached
*                   TRUE    Offset or rate correction limit has been reached
*************************************************************************************/
boolean Fr_check_clock_correction_limit_reached(void)
{
    if(Fr_CC_reg_ptr[FrPIFR0] & FR_CLOCK_CORRECTION_LIMIT_IRQ)    // CCL_IF interrupt flag set?
    {   // Yes, Clock Correction Limit Reached interrupt occured
        Fr_CC_reg_ptr[FrPIFR0] = FR_CLOCK_CORRECTION_LIMIT_IRQ;      // Clear interrupt flag
        return TRUE;                    // Return TRUE - Error flag has been set
    }
    else 
    {
        return FALSE;                   // Error flag has not been set
    }
}

/************************************************************************************
* Function name:    Fr_check_missing_offset_correction
* Description:      Check whether or not an insufficient number of measurements is 
*                   available for offset correction.
*                   The function should clear pending interrupt flag
*
* @author           r62779
* @version          13/07/2006
* Function arguments: 
*                   none
* Return value:
*                   FALSE   Insufficient number of measurements for offset correction 
*                           has not been detected
*                   TRUE    Insufficient number of measurements for offset correction 
*                           has been detected
*************************************************************************************/
boolean Fr_check_missing_offset_correction(void)
{
    if(Fr_CC_reg_ptr[FrPIFR0] & FR_MISSING_OFFSET_CORRECTION_IRQ)    // MOC_IF interrupt flag set?
    {   // Yes, Missing Offset Correction interrupt occured
        Fr_CC_reg_ptr[FrPIFR0] = FR_MISSING_OFFSET_CORRECTION_IRQ;   // Clear interrupt flag
        return TRUE;                    // Return TRUE - Error flag has been set
    }
    else 
    {
        return FALSE;                   // Error flag has not been set
    }
}

/************************************************************************************
* Function name:    Fr_check_missing_rate_correction
* Description:      Check whether or not an insufficient number of measurements is 
*                   available for rate correction.
*                   The function should clear pending interrupt flag
*
* @author           r62779
* @version          13/07/2006
* Function arguments: 
*                   none
* Return value:
*                   FALSE   Insufficient number of measurements for rate correction 
*                           has not been detected
*                   TRUE    Insufficient number of measurements for rate correction 
*                           has been detected
*************************************************************************************/
boolean Fr_check_missing_rate_correction(void)
{
    if(Fr_CC_reg_ptr[FrPIFR0] & FR_MISSING_RATE_CORRECTION_IRQ)    // MRC_IF interrupt flag set?
    {   // Yes, Missing Rate Correction interrupt occured
        Fr_CC_reg_ptr[FrPIFR0] = FR_MISSING_RATE_CORRECTION_IRQ;   // Clear interrupt flag
        return TRUE;                    // Return TRUE - Error flag has been set
    }
    else 
    {
        return FALSE;                   // Error flag has not been set
    }
}

/************************************************************************************
* Function name:    Fr_check_coldstart_abort
* Description:      Check whether or not the configured number of allowed cold start 
*                   attempts has been reached and none of these attempts was successful.
*                   The function should clear pending interrupt flag
*
* @author           r62779
* @version          13/07/2006
* Function arguments: 
*                   none
* Return value:
*                   FALSE   The configured number of allowed cold start attempts has not 
*                           been reached
*                   TRUE    The configured number of allowed cold start attempts has been 
*                           reached and none of these attempts was successful
*************************************************************************************/
boolean Fr_check_coldstart_abort(void)
{
    if(Fr_CC_reg_ptr[FrPIFR0] & FR_COLDSTART_ABORT_IRQ)    // CSA_IF interrupt flag set?
    {   // Yes, Cold Start Abort interrupt occured
        Fr_CC_reg_ptr[FrPIFR0] = FR_COLDSTART_ABORT_IRQ;   // Clear interrupt flag
        return TRUE;                    // Return TRUE - Error flag has been set
    }
    else 
    {
        return FALSE;                   // Error flag has not been set
    }
}

/************************************************************************************
* Function name:    Fr_check_internal_protocol_error
* Description:      Check whether or not the protocol engine has detected an internal 
*                   protocol error. In this case the protocol engine goes into the halt 
*                   state immediately.
*                   The function should clear pending interrupt flag
*
* @author           r62779
* @version          13/07/2006
* Function arguments: 
*                   none
* Return value:
*                   FALSE   The protocol engine has not detected an internal protocol error
*                   TRUE    The protocol engine has detected an internal protocol error
*************************************************************************************/
boolean Fr_check_internal_protocol_error(void)
{
    if(Fr_CC_reg_ptr[FrPIFR0] & FR_INTERNAL_PROTOCOL_ERROR_IRQ)    // INTL_IF interrupt flag set?
    {   // Yes, Internal Protocol Error interrupt occured
        Fr_CC_reg_ptr[FrPIFR0] = FR_INTERNAL_PROTOCOL_ERROR_IRQ;   // Clear interrupt flag
        return TRUE;                    // Return TRUE - Error flag has been set
    }
    else 
    {
        return FALSE;                   // Error flag has not been set
    }
}

/************************************************************************************
* Function name:    Fr_check_fatal_protocol_error
* Description:      Check whether or not the protocol engine has detected a fatal protocol error. 
*                   In this case the protocol engine goes into the halt state immediately. 
*                   The fatal protocol error are:
*                   1) pLatestTx violation
*                   2) transmission across slot boundary violation
*                   The function should clear pending interrupt flag
*
* @author           r62779
* @version          13/07/2006
* Function arguments: 
*                   none
* Return value:
*                   FALSE   The protocol engine has not detected a fatal protocol error
*                   TRUE    The protocol engine has detected an fatal protocol error
*************************************************************************************/
boolean Fr_check_fatal_protocol_error(void)
{
    if(Fr_CC_reg_ptr[FrPIFR0] & FR_FATAL_PROTOCOL_ERROR_IRQ)    // FATL_IF interrupt flag set?
    {   // Yes, Fatal Protocol Error interrupt occured
        Fr_CC_reg_ptr[FrPIFR0] = FR_FATAL_PROTOCOL_ERROR_IRQ;   // Clear interrupt flag
        return TRUE;                    // Return TRUE - Error flag has been set
    }
    else 
    {
        return FALSE;                   // Error flag has not been set
    }
}

/************************************************************************************
* Function name:    Fr_check_protocol_state_changed
* Description:      Check whether or not the protocol state has been changed.
*                   The function should clear pending interrupt flag
*
* @author           r62779
* @version          13/07/2006
* Function arguments: 
*                   none
* Return value:
*                   FALSE   The protocol state has not been changed
*                   TRUE    The protocol state has been changed
*************************************************************************************/
boolean Fr_check_protocol_state_changed(void)
{
    if(Fr_CC_reg_ptr[FrPIFR1] & FR_PROTOCOL_STATE_CHANGED_IRQ)    // PSC_IF interrupt flag set?
    {   // Yes, Protocol State Changed interrupt occured
        Fr_CC_reg_ptr[FrPIFR1] = FR_PROTOCOL_STATE_CHANGED_IRQ;   // Clear interrupt flag
        return TRUE;                    // Return TRUE - Error flag has been set
    }
    else 
    {
        return FALSE;                   // Error flag has not been set
    }
}

/************************************************************************************
* Function name:    Fr_check_protocol_engine_com_failure
* Description:      Check whether or not the protocol engine communication failure has been detected.
*                   The function should clear pending interrupt flag
*
* @author           r62779
* @version          13/07/2006
* Function arguments: 
*                   none
* Return value:
*                   FALSE   The protocol engine communication failure has not been detected
*                   TRUE    The protocol engine communication failure has been detected
*************************************************************************************/
boolean Fr_check_protocol_engine_com_failure(void)
{
    if(Fr_CC_reg_ptr[FrPIFR1] & FR_PROTOCOL_ENGINE_COM_FAILURE)    // PECF_IF interrupt flag set?
    {   // Yes, Protocol State Changed interrupt occured
        Fr_CC_reg_ptr[FrPIFR1] = FR_PROTOCOL_ENGINE_COM_FAILURE;   // Clear interrupt flag
        return TRUE;                    // Return TRUE - Error flag has been set
    }
    else 
    {
        return FALSE;                   // Error flag has not been set
    }
}

/************************************************************************************
* Function name:    Fr_low_level_access_read_reg
* Description:      Read value of the given register
*
* @author           r62779
* @version          19/07/2006
* Function arguments: 
*                   Fr_required_register    This parameter determines which 
*                                           FlexRay register shall be read
* Return value:
*                   The content of required register
*
*************************************************************************************/
uint16 Fr_low_level_access_read_reg(uint16 Fr_required_register)
{
    return Fr_CC_reg_ptr[Fr_required_register]; // Read the content of the required FlexRay register
}

/************************************************************************************
* Function name:    Fr_low_level_access_write_reg
* Description:      Write given value to the register
*
* @author           r62779
* @version          19/07/2006
* Function arguments: 
*                   Fr_required_register    This parameter determines to which FlexRay 
*                                           register shall be written the value put in Fr_value parameter
*                   Fr_value                Value which should be written into required register
* Return value:
*                   none
*************************************************************************************/
void Fr_low_level_access_write_reg(uint16 Fr_required_register, uint16 Fr_value)
{
    Fr_CC_reg_ptr[Fr_required_register] = Fr_value;     // Write given value to the FlexRay register
}

/************************************************************************************
* Function name:    Fr_low_level_access_read_memory
* Description:      Read value of the FlexRay memory field. 
*                   The value of the Fr_memory_address parameter is any offset of the corresponding 
*                   FlexRay memory field with respect to the FlexRay memory base address 
*                   as provided by the CC_FlexRay_memory_base_address parameter 
*                   in the Fr_HW_config_type structure
*
* @author           r62779
* @version          6/11/2006
* Function arguments: 
*                   Fr_memory_address   This parameter determines which FlexRay memory 
*                                       field to be read. The host has to put the value
*                                       in bytes and needs to be even number
* Return value:
*                   The content of required FlexRay memory field
*
*************************************************************************************/
uint16 Fr_low_level_access_read_memory(uint16 Fr_memory_address)
{
    // Read the content of the required FlexRay memory field
    return *((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + (Fr_memory_address / 2));
}

/************************************************************************************
* Function name:    Fr_low_level_access_write_memory
* Description:      Write given value to the FlexRay memory field
*
* @author           r62779
* @version          6/11/2006
* Function arguments: 
*                   Fr_memory_address       This parameter determines to which 
*                                           FlexRay memory field shall be written the value put in Fr_value parameter
*                   Fr_value                Value which should be written into required register
* Return value:
*                   none
*************************************************************************************/
void Fr_low_level_access_write_memory(uint16 Fr_memory_address, uint16 Fr_value)
{
     // Write given value to the FlexRay register
    *((volatile uint16 * FR_DATA_FAR)(Fr_HW_config_ptr->CC_FlexRay_memory_base_address) + (Fr_memory_address / 2)) = Fr_value;
}
