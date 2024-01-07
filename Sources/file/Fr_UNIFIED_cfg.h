/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2005 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************//*!
*
* @file      Fr_UNIFIED_cfg.h
*
* @author    rg003c
* 
* @version   1.0.1.0
* 
* @date      Apr-26-2007
* 
* @brief     FlexRay UNIFIED Driver implementation
*            Files that contains declaration of the external variables/structures/constants
*            This file can be changed according to an user configuration
*
******************************************************************************/

#ifndef _FR_UNIFIED_CFG_H
#define _FR_UNIFIED_CFG_H

/******************************************************************************
* Constants
******************************************************************************/

// The host can optimize the driver memory consumption by passing the number of the highest 
// used transmit or receive message buffer index (not shadow and FIFO) - configured 
// in the instance of the Fr_buffer_info_type structure
// Note - it is not necessary to define the FR_NUMBER_TXRX_MB parameter for correct driver operation
#define FR_NUMBER_TXRX_MB 3     // The driver will allocate only limited number of elements in an internal array

/******************************************************************************
* Global variables
******************************************************************************/

extern const Fr_low_level_config_type Fr_low_level_cfg_set_00;
extern const Fr_HW_config_type Fr_HW_cfg_00;
extern const Fr_buffer_info_type Fr_buffer_cfg_00[];
extern const uint8 Fr_buffer_cfg_set_00[];
extern const Fr_timer_config_type * Fr_timers_cfg_00_ptr[];
#endif /* _FR_UNIFIED_CFG_H */