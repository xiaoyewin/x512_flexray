/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2005 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************//*!
*
* @file      Fr_UNIFIED_cfg.c
*
* @author    rg003c
* 
* @version   1.0.1.0
* 
* @date      Apr-26-2007
* 
* @brief     FlexRay High-Level Driver implementation.
*            This file contains structures with configuration data. An user may
*            modify these structures.
*
******************************************************************************/

#include "Fr_UNIFIED.h"

/******************************************************************************
* Global variables
******************************************************************************/

// Hardware configuration structure
// Number of MB in Segment 1:   11
// Number of MB in Segment 2:   8
// FIFO Depth:                  10
const Fr_HW_config_type Fr_HW_cfg_00 =
{
    0x000400,     // FlexRay module base address
    0x0FF000,//0x0FF000     // FlexRay memory base address (MB headers start at this address)
    FR_MC9S12XF,     // Type of Freescale FlexRay module
    FALSE,          // Synchronization filtering
    //FR_EXTERNAL_OSCILLATOR,
    FR_INTERNAL_SYSTEM_BUS_CLOCK,
    0,              // Prescaler value   0?????10MHZ
    14,             // Data size - segment 1  //这个一般是静态SLOT 大小     
    8,              // Data size - segment 2
    10,             // Last MB in segment 1 (Number of MB in Segment1 - 1)
    18,             // Last individual MB (except FIFO); (Number of MB in Segment1 + Number of MB in Segment2 - 1)
    29,             // Total number of used MB (Last_individual_MB + 1 + FIFO)
    TRUE,           // Allow coldstart
    0,              // The value of the TIMEOUT bit field in the SYMATOR register - not implemented for all FlexRay modules
    0,              // Offset of the Sync Frame Table in the FlexRay memory
    FR_DUAL_CHANNEL_MODE    // Single channel mode disabled
};

// Transmit MB configuration structure
// Slot 1, payload length 16 Words, Double buffered MB, State transmission mode, 
// interrupt enabled from transmit side of double MB, channel AB, filtering disabled, streaming mode
const Fr_transmit_buffer_config_type Fr_tx_buffer_slot_01_cfg =
{
   1,                           // Transmit frame ID
   242,                         // Header CRC
   16,                          // Payload length
   FR_DOUBLE_TRANSMIT_BUFFER,   // Transmit MB buffering
   FR_STATE_TRANSMISSION_MODE,  // Transmission mode
   FR_STREAMING_COMMIT_MODE,    // Transmission commit mode
   FR_CHANNEL_AB,               // Transmit channels
   FALSE,                       // Payload preamble
   FALSE,                       // Transmit cycle counter filter enable
   0,                           // Transmit cycle counter filter value
   0,                           // Transmit cycle counter filter mask
   TRUE,                        // Transmit MB interrupt enable
   TRUE                         // FALSE - interrupt is enabled at commit side, TRUE - interrupt is enabled at transmit side
};

// Transmit MB configuration structure
// This MB is not configured in application
const Fr_transmit_buffer_config_type Fr_tx_buffer_slot_02_cfg =
{
   2,                           // Transmit frame ID
   2001,                        // Header CRC
   16,                          // Payload length
   FR_SINGLE_TRANSMIT_BUFFER,   // Transmit MB buffering
   FR_STATE_TRANSMISSION_MODE,  // Transmission mode
   FR_IMMEDIATE_COMMIT_MODE,    // Transmission commit mode
   FR_CHANNEL_AB,               // Transmit channels
   FALSE,                       // Payload preamble
   FALSE,                       // Transmit cycle counter filter enable
   0x11,                        // Transmit cycle counter filter value
   0x22,                        // Transmit cycle counter filter mask
   FALSE,                       // Transmit MB interrupt enable
   FALSE                        // FALSE - interrupt is enabled at commit side, TRUE - interrupt is enabled at transmit side
};


// Receive MB configuration structure
// This MB is not configured in application

/*
const Fr_receive_buffer_config_type  Fr_rx_buffer_slot_03_cfg =
{
   3,                           // Receive frame ID
   FR_CHANNEL_A,                // Receive channel enable
   TRUE,                        // Receive cycle counter filter enable
   0x0002,                      // Receive cycle counter filter value
   0x0003,                      // Receive cycle counter filter mask
   TRUE                         // Receive MB interrupt enable   
};

   */
const Fr_receive_buffer_config_type  Fr_rx_buffer_slot_70_cfg =
{
   70,                           // Receive frame ID
   FR_CHANNEL_A,                // Receive channel enable
   FALSE,                       // Receive cycle counter filter enable
   0,                           // Receive cycle counter filter value
   0,                           // Receive cycle counter filter mask
   TRUE                         // Receive MB interrupt enable
};

// Receive MB configuration structure
// Slot 4, channel A, filtering disabled, interrupt enabled
const Fr_receive_buffer_config_type  Fr_rx_buffer_slot_69_cfg =
{
   69,                           // Receive frame ID
   FR_CHANNEL_A,                // Receive channel enable
   FALSE,                       // Receive cycle counter filter enable
   0,                           // Receive cycle counter filter value
   0,                           // Receive cycle counter filter mask
   TRUE                         // Receive MB interrupt enable
};

// Transmit MB configuration structure
// Slot 7, payload length 16 Words, Single buffered MB, State transmission mode,
// interrupt enabled, channel AB, filtering disabled
const Fr_transmit_buffer_config_type Fr_tx_buffer_slot_07_cfg =
{
   7,                           // Transmit frame ID
   0x03EF,                      // Header CRC
   16,                          // Payload length
   FR_SINGLE_TRANSMIT_BUFFER,   // Transmit MB buffering
   FR_STATE_TRANSMISSION_MODE,  // Transmission mode
   FR_IMMEDIATE_COMMIT_MODE,    // Transmission commit mode
   FR_CHANNEL_AB,               // Transmit channels
   FALSE,                       // Payload preamble
   FALSE,                       // Transmit cycle counter filter enable
   0,                           // Transmit cycle counter filter value
   0,                           // Transmit cycle counter filter mask
   TRUE,                        // Transmit MB interrupt enable
   FALSE                        // FALSE - interrupt is enabled at commit side, TRUE - interrupt is enabled at transmit side
};

// Transmit MB configuration structure
// Slot 8, payload length 16 Words, Single buffered MB, State transmission mode,
// interrupt enabled, channel AB, filtering disabled
const Fr_transmit_buffer_config_type Fr_tx_buffer_slot_08_cfg =
{
   8,                           // Transmit frame ID
   0x0209,                      // Header CRC
   16,                          // Payload length
   FR_SINGLE_TRANSMIT_BUFFER,   // Transmit MB buffering
   FR_STATE_TRANSMISSION_MODE,  // Transmission mode
   FR_IMMEDIATE_COMMIT_MODE,    // Transmission commit mode
   FR_CHANNEL_AB,               // Transmit channels
   FALSE,                       // Payload preamble
   FALSE,                       // Transmit cycle counter filter enable
   0,                           // Transmit cycle counter filter value
   0,                           // Transmit cycle counter filter mask
   TRUE,                        // Transmit MB interrupt enable
   FALSE                        // FALSE - interrupt is enabled at commit side, TRUE - interrupt is enabled at transmit side
};

// Transmit MB configuration structure
// Slot 9, payload length 16 Words, Single buffered MB, State transmission mode,
// interrupt enabled, channel AB, filtering disabled
const Fr_transmit_buffer_config_type Fr_tx_buffer_slot_09_cfg =
{
   9,                           // Transmit frame ID
   0x03FC,                      // Header CRC
   16,                          // Payload length
   FR_SINGLE_TRANSMIT_BUFFER,   // Transmit MB buffering
   FR_STATE_TRANSMISSION_MODE,  // Transmission mode
   FR_IMMEDIATE_COMMIT_MODE,    // Transmission commit mode
   FR_CHANNEL_AB,               // Transmit channels
   FALSE,                       // Payload preamble
   FALSE,                       // Transmit cycle counter filter enable
   0,                           // Transmit cycle counter filter value
   0,                           // Transmit cycle counter filter mask
   TRUE,                        // Transmit MB interrupt enable
   FALSE                        // FALSE - interrupt is enabled at commit side, TRUE - interrupt is enabled at transmit side
};

// Transmit MB configuration structure
// This MB is not configured in application
const Fr_transmit_buffer_config_type Fr_tx_buffer_slot_10_cfg =
{
   10,                          // Transmit frame ID
   0x01E3,                      // Header CRC
   16,                          // Payload length
   FR_SINGLE_TRANSMIT_BUFFER,   // Transmit MB buffering
   FR_STATE_TRANSMISSION_MODE,  // Transmission mode
   FR_IMMEDIATE_COMMIT_MODE,    // Transmission commit mode
   FR_CHANNEL_AB,               // Transmit channels
   FALSE,                       // Payload preamble
   FALSE,                       // Transmit cycle counter filter enable
   0,                           // Transmit cycle counter filter value
   0,                           // Transmit cycle counter filter mask
   TRUE,                        // Transmit MB interrupt enable
   FALSE                        // FALSE - interrupt is enabled at commit side, TRUE - interrupt is enabled at transmit side
};


// FIFO configuration structure
// FIFO A, Depth 10, Data payload 8, receive frames only in range from 60 to 64
const Fr_FIFO_config_type Fr_FIFOA_cfg =
{
    FR_CHANNEL_A,                       // FIFO channel
    10,                                 // FIFO depth
    8,                                  // FIFO entry size
    0,                                  // FIFO message ID acceptance filter value
    0,                                  // FIFO message ID acceptance filter mask
    0,                                  // FIFO frame ID rejection filter value
    0x07FF,                             // FIFO frame ID rejection filter mask
    {                                   // Range filters (RF) configuration
        {TRUE, FR_ACCEPTANCE, 60, 64},  // 1st RF - RF enable, RF mode, RF lower interval, RF upper interval
        {FALSE, FR_ACCEPTANCE, 0, 0},   // 2nd RF - RF enable, RF mode, RF lower interval, RF upper interval
        {FALSE, FR_ACCEPTANCE, 0, 0},   // 3rd RF - RF enable, RF mode, RF lower interval, RF upper interval
        {FALSE, FR_ACCEPTANCE, 0, 0},   // 4th RF - RF enable, RF mode, RF lower interval, RF upper interval
    },
    TRUE                                // FIFO interrupt enable, can be enabled also by Fr_enable_interrupts() function
};

// Configuration data for Shadow Message Buffers
// All shadow MB are configured
const Fr_receive_shadow_buffers_config_type Fr_rx_shadow_cfg =
{
    TRUE,       // Rx shadow buffer for channel A, seg 1 - enabled?
    TRUE,       // Rx shadow buffer for channel A, seg 2 - enabled?
    TRUE,       // Rx shadow buffer for channel B, seg 1 - enabled?
    TRUE,       // Rx shadow buffer for channel B, seg 2 - enabled?
    8,          // Ch A, seg 1 - the current index of the MB header field
    17,         // Ch A, seg 2 - the current index of the MB header field
    9,          // Ch B, seg 1 - the current index of the MB header field
    18          // Ch B, seg 2 - the current index of the MB header field
};


// Following array is used to determine which message buffers defined in Fr_buffer_cfg_xx structure
// will be used for the FlexRay CC configuration
const Fr_index_selector_type Fr_buffer_cfg_set_00[] =
{     // 0, 1,2, 4, 5, FR_LAST_MB
     0,1,2,  FR_LAST_MB
};

// Array of structures with message buffer configuration information
// The MBs 2, 4, 5, 6 and 7 will not be configured

// Fr_buffer_info_type Fr_buffer_cfg_custom[MAX_MB_INDEX_SIZE]; 

const Fr_buffer_info_type Fr_buffer_cfg_00[] =
{ /* Buffer type         Configuration structure ptr    MB index       xx = configuration index used by Fr_buffer_cfg_set_xx */
    {FR_TRANSMIT_BUFFER, &Fr_tx_buffer_slot_01_cfg,     0},         // 00
    {FR_RECEIVE_BUFFER,  &Fr_rx_buffer_slot_69_cfg,     2},         // 01
    {FR_RECEIVE_BUFFER,  &Fr_rx_buffer_slot_70_cfg,     3},         // 02    
    {FR_TRANSMIT_BUFFER, &Fr_tx_buffer_slot_02_cfg,     0},         // 03    
    {FR_RECEIVE_FIFO,    &Fr_FIFOA_cfg,                 19},        // 04
    {FR_RECEIVE_SHADOW,  &Fr_rx_shadow_cfg,             0},         // 05
    {FR_TRANSMIT_BUFFER, &Fr_tx_buffer_slot_07_cfg,     4},         // 06
    {FR_TRANSMIT_BUFFER, &Fr_tx_buffer_slot_08_cfg,     5},         // 07
    {FR_TRANSMIT_BUFFER, &Fr_tx_buffer_slot_09_cfg,     6},         // 08
    {FR_TRANSMIT_BUFFER, &Fr_tx_buffer_slot_10_cfg,     7},         // 09
};


// Configuration data for absolute timer T1
const Fr_timer_config_type Fr_timer_1_cfg = 
{
    FR_TIMER_T1, 						// Timer number (T1 or T2)
    FR_ABSOLUTE, 						// Timer timebase
    FR_REPETITIVE, 						// Timer repetition mode
    2050, 								// Timer macrotick offset
    0, 									// Timer cycle filter mask, only for absolute timer
    0									// Timer cycle filter value, only for absolute timer
};

// Configuration data for relative timer T2
const Fr_timer_config_type Fr_timer_2_cfg = 
{
    FR_TIMER_T2, 						// Timer number (T1 or T2)
    FR_RELATIVE, 						// Timer timebase
    FR_REPETITIVE, 						// Timer repetition mode
    1000000, 							// Timer macrotick offset
    0, 									// Timer cycle filter mask, only for absolute timer
    0									// Timer cycle filter value, only for absolute timer
};

// Array with timers configuration information
// Following array is used to determine which timers will be used for the FlexRay CC configuration
const Fr_timer_config_type * Fr_timers_cfg_00_ptr[] =
{   
    &Fr_timer_1_cfg,                    // Pointer to configuration structure for timer T1
    &Fr_timer_2_cfg,                    // Pointer to configuration structure for timer T2
    NULL
};

/* Structure of this type contains configuration
   information of the one low level parameters set */
const Fr_low_level_config_type Fr_low_level_cfg_set_00 =
{
    10,         /* G_COLD_START_ATTEMPTS ?????????? */
    5,          /* GD_ACTION_POINT_OFFSET ????????? */
    91,         /* GD_CAS_RX_LOW_MAX */
    1,          /* GD_DYNAMIC_SLOT_IDLE_PHASE */
    8,          /* GD_MINISLOT */
    4,          /* GD_MINI_SLOT_ACTION_POINT_OFFSET */
    50,         /* GD_STATIC_SLOT  ???MT ?1us*/
    0,          /* GD_SYMBOL_WINDOW */
    11,         /* GD_TSS_TRANSMITTER */
    59,         /* GD_WAKEUP_SYMBOL_RX_IDLE */
    50,         /* GD_WAKEUP_SYMBOL_RX_LOW */
    301,        /* GD_WAKEUP_SYMBOL_RX_WINDOW */
    180,        /* GD_WAKEUP_SYMBOL_TX_IDLE */
    60,         /* GD_WAKEUP_SYMBOL_TX_LOW */
    2,          /* G_LISTEN_NOISE */
    5000,       /* G_MACRO_PER_CYCLE */
    4,         /* G_MAX_WITHOUT_CLOCK_CORRECTION_PASSIVE */
    8,         /* G_MAX_WITHOUT_CLOCK_CORRECTION_FATAL */
    120,         /* G_NUMBER_OF_MINISLOTS */     // ????120
    80,         /* G_NUMBER_OF_STATIC_SLOTS */    // ????80
    4989,       /* G_OFFSET_CORRECTION_START */
    14,         /* G_PAYLOAD_LENGTH_STATIC */  // ????14    28???
    10,          /* G_SYNC_NODE_MAX */
    0,          /* G_NETWORK_MANAGEMENT_VECTOR_LENGTH */
    TRUE,      /* G_ALLOW_HALT_DUE_TO_CLOCK */
    16,         /* G_ALLOW_PASSIVE_TO_ACTIVE */
    FR_CHANNEL_AB,  /* P_CHANNELS */
    212,        /* PD_ACCEPTED_STARTUP_RANGE */
    2,          /* P_CLUSTER_DRIFT_DAMPING */
    56,         /* P_DECODING_CORRECTION */
    1,          /* P_DELAY_COMPENSATION_A */
    1,          /* P_DELAY_COMPENSATION_B */
    401202,     /* PD_LISTEN_TIMEOUT */
    601,        /* PD_MAX_DRIFT */
    0,          /* P_EXTERN_OFFSET_CORRECTION */
    0,          /* P_EXTERN_RATE_CORRECTION */
    5,          /* P_KEY_SLOT_ID  ????????????????????????? */
    TRUE,       /* P_KEY_SLOT_USED_FOR_STARTUP   */
    TRUE,       /* P_KEY_SLOT_USED_FOR_SYNC */
    1108,       /* P_KEY_SLOT_HEADER_CRC 1747 */
    110,         /* P_LATEST_TX */
    7,          /* P_MACRO_INITIAL_OFFSET_A */
    7,          /* P_MACRO_INITIAL_OFFSET_B */
    23,         /* P_MICRO_INITIAL_OFFSET_A */
    23,         /* P_MICRO_INITIAL_OFFSET_B */
    200000,     /* P_MICRO_PER_CYCLE */
    186,       /* P_OFFSET_CORRECTION_OUT */
    601,        /* P_RATE_CORRECTION_OUT */
    FALSE,      /* P_SINGLE_SLOT_ENABLED */
    FR_CHANNEL_A,   /* P_WAKEUP_CHANNEL */
    33,         /* P_WAKEUP_PATTERN */
    40,         /* P_MICRO_PER_MACRO_NOM */
    20           /* P_PAYLOAD_LENGTH_DYN_MAX  ???????????? */
};

