/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2005 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************//*!
*
* @file      Fr_UNIFIED.h
*
* @author    R62779
* 
* @version   1.0.49.0
* 
* @date      Mar-23-2007
* 
* @brief     FlexRay UNIFIED Driver implementation
*
******************************************************************************/

#ifndef FR_H
#define FR_H

/******************************************************************************
* Includes
******************************************************************************/

#include "Fr_UNIFIED_types.h"

/******************************************************************************
* Constants
******************************************************************************/

/* This constant is used to correctly handle the situation when the driver
waits till the PE accepts a command (e.g. the protocol control command).
In such a case, the driver will be waiting in the loop (maximum FR_MAX_WAIT_CYCLES
iterations) until a proper value appears in the required register. */
#define FR_MAX_WAIT_CYCLES      100

#define FR_LAST_MB            0xFF  // Used in buffer configuration structure to determine the last used MB

#define FR_MAX_NUMBER_TXRX_MB  128   // Maximal number of used MB except FIFO storage and shadow message buffers

#define FrMVR                 0x0000
#define FrMCR                 0x0001
#define FrSYMBADHR            0x0002
#define FrSYMBADLR            0x0003
#define FrSTBSCR              0x0004
#define FrSTBPCR              0x0005
#define FrMBDSR               0x0006
#define FrMBSSUTR             0x0007

#define FrPOCR                0x000A
#define FrGIFER               0x000B
#define FrPIFR0               0x000C
#define FrPIFR1               0x000D
#define FrPIER0               0x000E
#define FrPIER1               0x000F
#define FrCHIERFR             0x0010
#define FrMBIVEC              0x0011
#define FrCASERCR             0x0012
#define FrCBSERCR             0x0013
#define FrPSR0                0x0014
#define FrPSR1                0x0015
#define FrPSR2                0x0016
#define FrPSR3                0x0017
#define FrMTCTR               0x0018
#define FrCYCTR               0x0019
#define FrSLTCTAR             0x001A
#define FrSLTCTBR             0x001B
#define FrRTCORVR             0x001C
#define FrOFCORVR             0x001D
#define FrCIFRR               0x001E
#define FrSYMATOR             0x001F

#define FrSFCNTR              0x0020
#define FrSFTOR               0x0021
#define FrSFTCCSR             0x0022
#define FrSFIDRFR             0x0023
#define FrSFIDAFVR            0x0024
#define FrSFIDAFMR            0x0025
#define FrNMVR0               0x0026
#define FrNMVR1               0x0027
#define FrNMVR2               0x0028
#define FrNMVR3               0x0029
#define FrNMVR4               0x002A
#define FrNMVR5               0x002B
#define FrNMVLR               0x002C
#define FrTICCR               0x002D
#define FrTI1CYSR             0x002E
#define FrTI1MTOR             0x002F
#define FrTI2CR0              0x0030
#define FrTI2CR1              0x0031
#define FrSSSR                0x0032
#define FrSSCCR               0x0033
#define FrSSR0                0x0034
#define FrSSR1                0x0035
#define FrSSR2                0x0036
#define FrSSR3                0x0037
#define FrSSR4                0x0038
#define FrSSR5                0x0039
#define FrSSR6                0x003A
#define FrSSR7                0x003B
#define FrSSCR0               0x003C
#define FrSSCR1               0x003D
#define FrSSCR2               0x003E
#define FrSSCR3               0x003F
#define FrMTSACFR             0x0040
#define FrMTSBCFR             0x0041
#define FrRSBIR               0x0042
#define FrRFSR                0x0043
#define FrRFSIR               0x0044
#define FrRFDSR               0x0045
#define FrRFARIR              0x0046
#define FrRFBRIR              0x0047
#define FrRFMIDAFVR           0x0048
#define FrRFMIAFMR            0x0049
#define FrRFFIDRFVR           0x004A
#define FrRFFIDRFMR           0x004B
#define FrRFRFCFR             0x004C
#define FrRFRFCTR             0x004D
#define FrLDTXSLAR            0x004E
#define FrLDTXSLBR            0x004F
#define FrPCR0                0x0050
#define FrPCR1                0x0051
#define FrPCR2                0x0052
#define FrPCR3                0x0053
#define FrPCR4                0x0054
#define FrPCR5                0x0055
#define FrPCR6                0x0056
#define FrPCR7                0x0057
#define FrPCR8                0x0058
#define FrPCR9                0x0059
#define FrPCR10               0x005A
#define FrPCR11               0x005B
#define FrPCR12               0x005C
#define FrPCR13               0x005D
#define FrPCR14               0x005E
#define FrPCR15               0x005F
#define FrPCR16               0x0060
#define FrPCR17               0x0061
#define FrPCR18               0x0062
#define FrPCR19               0x0063
#define FrPCR20               0x0064
#define FrPCR21               0x0065
#define FrPCR22               0x0066
#define FrPCR23               0x0067
#define FrPCR24               0x0068
#define FrPCR25               0x0069
#define FrPCR26               0x006A
#define FrPCR27               0x006B
#define FrPCR28               0x006C
#define FrPCR29               0x006D
#define FrPCR30               0x006E

#define FrDER                 0x0070
#define FrCRSR                0x0071

#define FrPIDR                0x0078
#define FrAVNR                0x0079
#define FrHIPDSR              0x007A
#define FrPLPDSR              0x007B
#define FrHIPPER              0x007C
#define FrHIPPCR              0x007D
#define FrPLPPER              0x007E
#define FrPLPPCR              0x007F
#define FrMBCCSR0             0x0080
#define FrMBCCFR0             0x0081
#define FrMBFIDR0             0x0082
#define FrMBIDXR0             0x0083

#define FrMCR_MEN             0x8000
#define FrMCR_SCM             0x2000
#define FrMCR_CHB             0x1000
#define FrMCR_CHA             0x0800
#define FrMCR_SFFE            0x0400
#define FrMCR_CLKSEL          0x0010

#define FrPOCR_WME            0x8000
#define FrPOCR_BSY            0x0080
#define FrPOCR_WMC            0x0080

#define FrGIFER_TBIE            0x0001
#define FrGIFER_RBIE            0x0002
#define FrGIFER_FNEAIE          0x0004
#define FrGIFER_FNEBIE          0x0008
#define FrGIFER_WUPIE           0x0010
#define FrGIFER_CHIE            0x0020
#define FrGIFER_PRIE            0x0040
#define FrGIFER_MIE             0x0080
#define FrGIFER_TBIF            0x0100
#define FrGIFER_RBIF            0x0200
#define FrGIFER_FNEAIF          0x0400
#define FrGIFER_FNEBIF          0x0800
#define FrGIFER_WUPIF           0x1000
#define FrGIFER_CHIF            0x2000
#define FrGIFER_PRIF            0x4000
#define FrGIFER_MIF             0x8000

#define FrPIFR0_CYS_IF          0x0001
#define FrPIFR0_TI1_IF          0x0002
#define FrPIFR0_TI2_IF          0x0004
#define FrPIFR0_TBVA_IF         0x0008
#define FrPIFR0_TBVB_IF         0x0010
#define FrPIFR0_LTXA_IF         0x0020
#define FrPIFR0_LTXB_IF         0x0040
#define FrPIFR0_MTX_IF          0x0080
#define FrPIFR0_MXS_IF          0x0100
#define FrPIFR0_CCL_IF          0x0200
#define FrPIFR0_MOC_IF          0x0400
#define FrPIFR0_MRC_IF          0x0800
#define FrPIFR0_CSA_IF          0x1000
#define FrPIFR0_ILCF_IF         0x2000
#define FrPIFR0_INTL_IF         0x4000
#define FrPIFR0_FATL_IF         0x8000

#define FrPIER0_CYS_IE          0x0001
#define FrPIER0_TI1_IE          0x0002
#define FrPIER0_TI2_IE          0x0004
#define FrPIER0_TBVA_IE         0x0008
#define FrPIER0_TBVB_IE         0x0010
#define FrPIER0_LTXA_IE         0x0020
#define FrPIER0_LTXB_IE         0x0040
#define FrPIER0_MTX_IE          0x0080
#define FrPIER0_MXS_IE          0x0100
#define FrPIER0_CCL_IE          0x0200
#define FrPIER0_MOC_IE          0x0400
#define FrPIER0_MRC_IE          0x0800
#define FrPIER0_CSA_IE          0x1000
#define FrPIER0_ILCF_IE         0x2000
#define FrPIER0_INTL_IE         0x4000
#define FrPIER0_FATL_IE         0x8000

#define FrPSR1_FRZ              0x0020
#define FrPSR1_CSAA             0x8000

#define FrPSR2_NBVB                     0x8000
#define FrPSR2_NSEB                     0x4000
#define FrPSR2_STCB                     0x2000
#define FrPSR2_SBVB                     0x1000
#define FrPSR2_SSEB                     0x0800
#define FrPSR2_MTB                      0x0400
#define FrPSR2_NBVA                     0x0200
#define FrPSR2_NSEA                     0x0100
#define FrPSR2_STCA                     0x0080
#define FrPSR2_SBVA                     0x0040
#define FrPSR2_SSEA                     0x0020
#define FrPSR2_MTA                      0x0010

#define FrPSR2_MTSB_RCV                   0x0400
#define FrPSR2_MTSB_RCV_SYNERR            0x0C00
#define FrPSR2_MTSB_RCV_BVIO              0x1400
#define FrPSR2_MTSB_RCV_SYNERR_BVIO       0x1C00
#define FrPSR2_MTSB_NOT_RCV               0x0000
#define FrPSR2_MTSB_NOT_RCV_SYNERR        0x0800
#define FrPSR2_MTSB_NOT_RCV_BVIO          0x1000
#define FrPSR2_MTSB_NOT_RCV_SYNERR_BVIO   0x1800

#define FrPSR2_MTSA_RCV                   0x0010
#define FrPSR2_MTSA_RCV_SYNERR            0x0030
#define FrPSR2_MTSA_RCV_BVIO              0x0050
#define FrPSR2_MTSA_RCV_SYNERR_BVIO       0x0070
#define FrPSR2_MTSA_NOT_RCV               0x0000
#define FrPSR2_MTSA_NOT_RCV_SYNERR        0x0020
#define FrPSR2_MTSA_NOT_RCV_BVIO          0x0040
#define FrPSR2_MTSA_NOT_RCV_SYNERR_BVIO   0x0060

#define FrPSR3_WUB              0x2000
#define FrPSR3_WUA              0x0020

#define FrSSCCR_MCY             0x0100
#define FrSSCCR_WMD             0x8000

#define FrTICCR_T1ST            0x0001
#define FrTICCR_T1TR            0x0002
#define FrTICCR_T1SP            0x0004
#define FrTICCR_T1_REP          0x0010
#define FrTICCR_T2ST            0x0100
#define FrTICCR_T2TR            0x0200
#define FrTICCR_T2SP            0x0400
#define FrTICCR_T2_REP          0x1000
#define FrTICCR_T2_CFG          0x2000

#define FrMTSACFR_MTE           0x8000
#define FrMTSBCFR_MTE           0x8000

// Definition for Fr_enable_interrupts() function
// Definition of flags in the GIFER register
#define FR_MODULE_IRQ                               0x0080
#define FR_PROTOCOL_IRQ                             0x0040
#define FR_CHI_IRQ                                  0x0020
#define FR_WAKEUP_IRQ                               0x0010
#define FR_FIFO_B_IRQ                               0x0008
#define FR_FIFO_A_IRQ                               0x0004
#define FR_RECEIVE_IRQ                              0x0002
#define FR_TRANSMIT_IRQ                             0x0001
// Definition of flags in the PIER0/PIFR0 register
#define FR_FATAL_PROTOCOL_ERROR_IRQ                 0x8000
#define FR_INTERNAL_PROTOCOL_ERROR_IRQ              0x4000
#define FR_ILLEGAL_PROTOCOL_CONFIGURATION_IRQ       0x2000
#define FR_COLDSTART_ABORT_IRQ                      0x1000
#define FR_MISSING_RATE_CORRECTION_IRQ              0x0800
#define FR_MISSING_OFFSET_CORRECTION_IRQ            0x0400
#define FR_CLOCK_CORRECTION_LIMIT_IRQ               0x0200
#define FR_MAX_SYNC_FRAMES_DETECTED_IRQ             0x0100
#define FR_MEDIA_ACCESS_TEST_SYMBOL_RECEIVED_IRQ    0x0080
#define FR_VIOLATION_CHB_IRQ                        0x0040
#define FR_VIOLATION_CHA_IRQ                        0x0020
#define FR_TRANSMISSION_ACROSS_BOUNDARY_CHB_IRQ     0x0010
#define FR_TRANSMISSION_ACROSS_BOUNDARY_CHA_IRQ     0x0008
#define FR_TIMER_2_EXPIRED_IRQ                      0x0004
#define FR_TIMER_1_EXPIRED_IRQ                      0x0002
#define FR_CYCLE_START_IRQ                          0x0001
// Definition of flags in the PIER1/PIFR0 register
#define FR_ERROR_MODE_CHANGED_IRQ                   0x8000
#define FR_ILLEGAL_PROTOCOL_COMMAND_IRQ             0x4000
#define FR_PROTOCOL_ENGINE_COM_FAILURE              0x2000
#define FR_PROTOCOL_STATE_CHANGED_IRQ               0x1000
#define FR_SLOT_STATUS_COUNTER_3_INCREMENTED_IRQ    0x0800
#define FR_SLOT_STATUS_COUNTER_2_INCREMENTED_IRQ    0x0400
#define FR_SLOT_STATUS_COUNTER_1_INCREMENTED_IRQ    0x0200
#define FR_SLOT_STATUS_COUNTER_0_INCREMENTED_IRQ    0x0100
#define FR_EVEN_CYCLE_TABLE_WRITTEN_IRQ             0x0020
#define FR_ODD_CYCLE_TABLE_WRITTEN_IRQ              0x0010

// Definition of slot status register
#define FrSSRn_VFB	                                0x8000
#define FrSSRn_SYB	                                0x4000
#define FrSSRn_NFB                                  0x2000
#define FrSSRn_SUB	                                0x1000
#define FrSSRn_SEB	                                0x0800
#define FrSSRn_CEB	                                0x0400
#define FrSSRn_BVB	                                0x0200
#define FrSSRn_TCB	                                0x0100
#define FrSSRn_VFA	                                0x0080
#define FrSSRn_SYA	                                0x0040
#define FrSSRn_NFA                                  0x0020
#define FrSSRn_SUA	                                0x0010
#define FrSSRn_SEA	                                0x0008
#define FrSSRn_CEA	                                0x0004
#define FrSSRn_BVA	                                0x0002
#define FrSSRn_TCA	                                0x0001

#define FrRFRFCFR_IBD           0x4000
#define FrRFRFCFR_WMD           0x8000

#define FrRFRFCTR_F0EN          0x0001
#define FrRFRFCTR_F1EN          0x0002
#define FrRFRFCTR_F2EN          0x0004
#define FrRFRFCTR_F3EN          0x0008
#define FrRFRFCTR_F0MD          0x0100
#define FrRFRFCTR_F1MD          0x0200
#define FrRFRFCTR_F2MD          0x0400
#define FrRFRFCTR_F3MD          0x0800

#define FrPOCR_POCCMD_ALLOW_COLDSTART   0x0000
#define FrPOCR_POCCMD_ALL_SLOTS         0x0001
#define FrPOCR_POCCMD_CONFIG            0x0002
#define FrPOCR_POCCMD_FREEZE            0x0003
#define FrPOCR_POCCMD_READY             0x0004
#define FrPOCR_POCCMD_CONFIG_COMPLETE   0x0004
#define FrPOCR_POCCMD_RUN               0x0005
#define FrPOCR_POCCMD_DEFAULT_CONFIG    0x0006
#define FrPOCR_POCCMD_HALT              0x0007
#define FrPOCR_POCCMD_WAKEUP            0x0008
#define FrPOCR_POCCMD_RESET             0x000C

#define FrPSR0_ERRMODE_ACTIVE           0x0000
#define FrPSR0_ERRMODE_PASSIVE          0x4000
#define FrPSR0_ERRMODE_COMM_HALT        0x8000
#define FrPSR0_SLOTMODE_SINGLE          0x0000
#define FrPSR0_SLOTMODE_ALL_PENDING     0x1000
#define FrPSR0_SLOTMODE_ALL             0x2000

#define FrPSR0_PROTSTATE_DEFAULT_CONFIG 0x0000
#define FrPSR0_PROTSTATE_CONFIG         0x0100
#define FrPSR0_PROTSTATE_WAKEUP         0x0200
#define FrPSR0_PROTSTATE_READY          0x0300
#define FrPSR0_PROTSTATE_NORMAL_PASSIVE 0x0400
#define FrPSR0_PROTSTATE_NORMAL_ACTIVE  0x0500
#define FrPSR0_PROTSTATE_HALT           0x0600
#define FrPSR0_PROTSTATE_STARTUP        0x0700

#define FrPSR0_STARTUPSTATE_COLDSTART_COLLISION_RESOLUTION  0x0020
#define FrPSR0_STARTUPSTATE_COLDSTART_LISTEN                0x0030
#define FrPSR0_STARTUPSTATE_INTEGRATION_CONSISTENCY_CHECK   0x0040
#define FrPSR0_STARTUPSTATE_INTEGRATION_LISTEN              0x0050
#define FrPSR0_STARTUPSTATE_INITIALIZE_SCHEDULE             0x0070
#define FrPSR0_STARTUPSTATE_COLDSTART_CONSISTENCY_CHECK     0x00A0
#define FrPSR0_STARTUPSTATE_INTEGRATION_COLDSTART_CHECK     0x00D0
#define FrPSR0_STARTUPSTATE_COLDSTART_GAP                   0x00E0
#define FrPSR0_STARTUPSTATE_COLDSTART_JOIN                  0x00F0

#define FrPSR0_WAKEUPSTATUS_RECEIVED_HEADER                 0x0001
#define FrPSR0_WAKEUPSTATUS_RECEIVED_WUP                    0x0002
#define FrPSR0_WAKEUPSTATUS_COLLISION_HEADER                0x0003
#define FrPSR0_WAKEUPSTATUS_COLLISION_WUP                   0x0004
#define FrPSR0_WAKEUPSTATUS_COLLISION_UNKNOWN               0x0005
#define FrPSR0_WAKEUPSTATUS_TRANSMITTED                     0x0006

#define FrRSBIR_WMD                     0x8000
#define FrRSBIR_SEL_RSBIR_A1            0x0000
#define FrRSBIR_SEL_RSBIR_A2            0x1000
#define FrRSBIR_SEL_RSBIR_B1            0x2000
#define FrRSBIR_SEL_RSBIR_B2            0x3000

#define FrMBCCSR_MCM                    0x4000
#define FrMBCCSR_MBT                    0x2000
#define FrMBCCSR_MTD                    0x1000
#define FrMBCCSR_CMT                    0x0800
#define FrMBCCSR_EDT                    0x0400
#define FrMBCCSR_LCKT                   0x0200
#define FrMBCCSR_MBIE                   0x0100
#define FrMBCCSR_DUP                    0x0010
#define FrMBCCSR_DVAL                   0x0008
#define FrMBCCSR_EDS                    0x0004
#define FrMBCCSR_LCKS                   0x0002
#define FrMBCCSR_MBIF                   0x0001

#define FrMBCCFR_MTM                    0x8000
#define FrMBCCFR_CHA                    0x4000
#define FrMBCCFR_CHB                    0x2000
#define FrMBCCFR_CCFE                   0x1000

#define FrF_HEADER_PPI                  0x4000
#define FrF_HEADER_NUF                  0x2000
#define FrF_HEADER_VFB                  0x8000
#define FrF_HEADER_VFA                  0x0080

/******************************************************************************
* Types
******************************************************************************/

// The values are used to return the CHI related error flags
typedef uint16 Fr_CHI_error_type;

// The values are used to determine which message buffers defined in 
// a Fr_buffer_info_type structure will be used for the FlexRay module configuration
typedef uint8 Fr_index_selector_type;

// The values of this enumeration are used to determine if communication stops 
// immediately or at the end of current FlexRay cycle
typedef enum
{
    FR_HALT_COMMUNICATION = 0,
    FR_ABORT_COMMUNICATION
} Fr_stop_communication_type;

// The values of this enumeration are used to provide information whether or not the
// local FlexRay CC is synchronous to the FR global time
typedef enum
{
    FR_ASYNC = 0,
    FR_SYNC
} Fr_sync_state_type;

// The values are used to hold the protocol state
typedef enum
{
    FR_POCSTATE_CONFIG = 0,
    FR_POCSTATE_DEFAULT_CONFIG,
    FR_POCSTATE_HALT,
    FR_POCSTATE_NORMAL_ACTIVE,
    FR_POCSTATE_NORMAL_PASSIVE,
    FR_POCSTATE_READY,
    FR_POCSTATE_STARTUP,
    FR_POCSTATE_WAKEUP
} Fr_POC_state_type;

// These values are used to determine if a message buffer has been updated with new data
typedef enum
{
    FR_TXMB_UPDATED = 0,
    FR_TXMB_NO_ACCESS
} Fr_tx_MB_status_type;

// These values are used to determine if a data has been transmitted
// For single message buffers are used only the FR_TRANSMITTED and FR_NOT_TRANSMITTED values
typedef enum 
{
    FR_TRANSMITTED = 0,
    FR_NOT_TRANSMITTED,
    FR_INTERNAL_MESSAGE_TRANSFER_DONE,
    FR_TRANSMIT_SIDE_TRANSMITTED
} Fr_tx_status_type;

// These values are used to determine if a data has been received
typedef enum
{
    FR_RXMB_RECEIVED = 0,
    FR_RXMB_NOT_RECEIVED,
    FR_RXMB_NULL_FRAME_RECEIVED,
    FR_RXMB_NO_ACCESS
} Fr_rx_MB_status_type;

// These values are used to determine if a message buffer has received a frame
typedef enum
{
    FR_RECEIVED = 0,
    FR_NOT_RECEIVED,
    FR_NULL_FRAME_RECEIVED
} Fr_rx_status_type;

// These values are used to determine if a data has been received into a FIFO storage
typedef enum
{
    FR_FIFO_RECEIVED = 0,
    FR_FIFO_NOT_RECEIVED
} Fr_FIFO_status_type;

// The values determine the state of the wakeup mechanism
typedef enum
{
    FR_WAKEUPSTATE_UNDEFINED = 0,
    FR_WAKEUPSTATE_RECEIVED_HEADER,
    FR_WAKEUPSTATE_RECEIVED_WUP,
    FR_WAKEUPSTATE_COLLISION_HEADER,
    FR_WAKEUPSTATE_COLLISION_WUP,
    FR_WAKEUPSTATE_COLLISION_UNKNOWN,
    FR_WAKEUPSTATE_TRANSMITTED
} Fr_wakeup_state_type;

// The values determine the state of the Media Test Symbol 
// (MTS) reception in Symbol Window of a communication cycle
typedef enum
{
    FR_MTS_RCV = 0,
    FR_MTS_RCV_SYNERR,
    FR_MTS_RCV_BVIO,
    FR_MTS_RCV_SYNERR_BVIO,
    FR_MTS_NOT_RCV,
    FR_MTS_NOT_RCV_SYNERR,
    FR_MTS_NOT_RCV_BVIO,
    FR_MTS_NOT_RCV_SYNERR_BVIO
} Fr_MTS_state_type;

// The values are used to reference a channel on a CC
typedef enum
{
    FR_CHANNEL_A = 0,
    FR_CHANNEL_B,
    FR_CHANNEL_AB,
    FR_NO_CHANNEL
} Fr_channel_type;

// The values are used to determine which 
// slot status information is read by the Fr_get_slot_status_reg_value function
typedef enum
{
    FR_SLOT_STATUS_CURRENT = 0,
    FR_SLOT_STATUS_PREVIOUS
} Fr_slot_status_required_type;

/*
typedef enum
{
  FR_OFFSET_INC = 0,
  FR_OFFSET_DEC,
  FR_OFFSET_NOCHANGE
} Fr_offset_correction_type;

typedef enum
{
  FR_RATE_INC = 0,
  FR_RATE_DEC,
  FR_RATE_NOCHANGE
} Fr_rate_correction_type;

*/

/* Structure of this type contains configuration information
   for all receive shadow buffers */
typedef struct
{
    boolean RSBIR_A1_enable;                  // Rx shadow buffer for channel A, seg 1 - enabled?
    boolean RSBIR_A2_enable;                  // Rx shadow buffer for channel A, seg 2 - enabled?
    boolean RSBIR_B1_enable;                  // Rx shadow buffer for channel B, seg 1 - enabled?
    boolean RSBIR_B2_enable;                  // Rx shadow buffer for channel B, seg 2 - enabled?
    uint8 RSBIR_A1_buffer_number_init;        // Ch A, seg 1 - the current index of the MB header field
    uint8 RSBIR_A2_buffer_number_init;        // Ch A, seg 2 - the current index of the MB header field
    uint8 RSBIR_B1_buffer_number_init;        // Ch B, seg 1 - the current index of the MB header field
    uint8 RSBIR_B2_buffer_number_init;        // Ch B, seg 2 - the current index of the MB header field
} Fr_receive_shadow_buffers_config_type;

/* Type definition for the Fr_FIFO_range_filter_mode field
   of the Fr_FIFO_range_filter_config_type structure. */
typedef enum
{
   FR_ACCEPTANCE = 0,
   FR_REJECTION
} Fr_FIFO_range_filter_mode_type;

/* Type definition for the FIFO_range_filters_config field
   of the Fr_receive_FIFO_config structure. */
typedef struct
{
    boolean range_filter_enable;					// TRUE - Range Filter is enabled
    Fr_FIFO_range_filter_mode_type range_filter_mode;  // Acceptance or Rejection mode
    uint16 range_filter_lower_interval;			// SID0 - Slot ID - Lower interval boundary
    uint16 range_filter_upper_interval;           // SID1 - Slot ID - Upper interval boundary
} Fr_FIFO_range_filters_type;

/* Structure of this type contains configuration information
   for receive FIFO structure. */
typedef struct 
{
    Fr_channel_type FIFO_channel;	                        // FIFO channel; Channel A = FR_CHANNEL_A, channel B = FR_CHANNEL_B
    uint8 FIFO_depth;                                       // FIFO depth
    uint8 FIFO_entry_size;                                  // FIFO entry size
    uint16 FIFO_message_ID_acceptance_filter_value;         // FIFO message ID acceptance filter value
    uint16 FIFO_message_ID_acceptance_filter_mask;          // FIFO message ID acceptance filter mask
    uint16 FIFO_frame_ID_rejection_filter_value;            // FIFO frame ID rejection filter value
    uint16 FIFO_frame_ID_rejection_filter_mask;             // FIFO frame ID rejection filter mask
    Fr_FIFO_range_filters_type FIFO_range_filters_config[4];  // Reference to range filters configuration structure
    boolean FIFO_interrupt_enable;                          // FIFO interrupt enable, can be enabled also by Fr_enable_interrupts() function
} Fr_FIFO_config_type;

/* Internal FlexRay UNIFIED Driver structure
   The structure contains information about already configured FIFO storages.
   FlexRay driver stores a FIFO configuration data into this structure immediately it configures
   FIFO registers. Driver uses this structure for Data Field Offset calculation. */
typedef struct 
{
    boolean FIFO_1_used;            // The first FIFO already configured?
    Fr_channel_type FIFO_1_channel; // The first configured FIFO - FIFO A or FIFO B
    uint8 FIFO_1_depth;             // The first configured FIFO - FIFO depth
    uint8 FIFO_1_entry_size;        // The first configured FIFO - FIFO entry size
    boolean FIFO_2_used;            // The second FIFO already configured?
    Fr_channel_type FIFO_2_channel; // The second configured FIFO - FIFO A or FIFO B
    uint8 FIFO_2_depth;             // The second configured FIFO - FIFO depth
    uint8 FIFO_2_entry_size;        // The second configured FIFO - FIFO entry size
} Fr_FIFO_info_type;

/* Structure of this type contains configuration
   information for one receive buffer. */
typedef struct
{
    uint16 receive_frame_ID;                    // Receive frame ID
    Fr_channel_type receive_channel_enable;     // Receive channel enable
    boolean rx_cycle_counter_filter_enable;     // Receive cycle counter filter enable
    uint8 rx_cycle_counter_filter_value;        // Receive cycle counter filter value
    uint8 rx_cycle_counter_filter_mask;         // Receive cycle counter filter mask
    boolean rx_MB_interrupt_enable;             // Receive MB interrupt enable
} Fr_receive_buffer_config_type;

/* Type definition for the transmit_MB_buffering field of
   the Fr_transmit_buffer_config_type structure. */
typedef enum
{
    FR_SINGLE_TRANSMIT_BUFFER = 0,
    FR_DOUBLE_TRANSMIT_BUFFER
} Fr_transmit_MB_type;

/* Type definition for the transmission_mode field of
   the Fr_transmit_buffer_config_type structure. */
typedef enum
{
    FR_EVENT_TRANSMISSION_MODE = 0,
    FR_STATE_TRANSMISSION_MODE
} Fr_transmission_type;

/* Type definition for the transmission_commit_mode field
   of the Fr_transmit_buffer_config_type structure. */
typedef enum
{
    FR_STREAMING_COMMIT_MODE = 0,
    FR_IMMEDIATE_COMMIT_MODE
} Fr_transmission_commit_type;

/* Structure of this type contains configuration
   information for one transmit buffer. */
typedef struct
{
    uint16 transmit_frame_ID;								     // Transmit frame ID
    uint16 header_CRC;                                           // Header CRC
    uint8 payload_length;                                        // Payload length
    Fr_transmit_MB_type transmit_MB_buffering;                   // Transmit MB buffering
    Fr_transmission_type transmission_mode;                      // Transmission mode
    Fr_transmission_commit_type transmission_commit_mode;        // Transmission commit mode
    Fr_channel_type transmit_channel_enable;                     // Transmit channels
    boolean payload_preamble;                                    // Payload preamble
    boolean tx_cycle_counter_filter_enable;                      // Transmit cycle counter filter enable
    uint8 tx_cycle_counter_filter_value;                         // Transmit cycle counter filter value
    uint8 tx_cycle_counter_filter_mask;                          // Transmit cycle counter filter mask
    boolean tx_MB_interrupt_enable;                              // Transmit MB interrupt enable
    // If tx interrupt is enabled by tx_MB_interrupt_enable parameter, tx_MB_interrupt_transmit_side_enable determines
    // whether the interrupt will be enabled at commit side (FALSE - default) or at tranmsit side (TRUE) of the double MB
    boolean tx_MB_interrupt_transmit_side_enable;   // Used only for double buffered MB, FALSE - commit side, TRUE - transmit side
} Fr_transmit_buffer_config_type;

/* Type definition of the configured buffer */
typedef enum
{
    FR_TRANSMIT_BUFFER = 0,         // Transmit message buffer
    FR_RECEIVE_BUFFER,              // Receive message buffer
    FR_RECEIVE_FIFO,                // Receive FIFO buffer
    FR_RECEIVE_SHADOW               // Receive shadow buffer
} Fr_buffer_type;

/* Structure of this type contains configuration
   information for one physical message buffer */
typedef struct
{
    Fr_buffer_type buffer_type;           // TRANSMIT, RECEIVE, SHADOW or FIFO
    const void * const buffer_config_ptr;  // Reference to buffer configuration structure
    // Individual MB - MB index; FIFO buffers - Start index; Receive shadow buffers - no sense
    uint16 buffer_index_init;
    //uint16 data_field_offset_init;        // Data field offset init - reserved
} Fr_buffer_info_type;

/* Type definition of timer ID */
typedef enum 
{
    FR_TIMER_T1 = 0,				// Timer T1 is selected
    FR_TIMER_T2                     // Timer T2 is selected
} Fr_timer_ID_type;


/* Type definition of timer timebase */
typedef enum 
{
    FR_ABSOLUTE = 0,			    // Absolute timer timebase
    FR_RELATIVE                     // Relative timer timebase
} Fr_timer_timebase_type;

/* Type definition of timer repetition mode */
typedef enum 
{
    FR_NON_REPETITIVE = 0,          // Non repetitive timer mode
    FR_REPETITIVE                   // Repetitive timer mode
} Fr_timer_repetition_type;

/* Structure of this type contains configuration 
   information for one absolute or relative timer */
typedef struct 
{
    Fr_timer_ID_type timer_ID;                  // Timer number (T1 or T2)
    Fr_timer_timebase_type timer_type;          // Timer timebase
    Fr_timer_repetition_type timer_repetition;  // Timer repetition mode
    uint32 timer_macrotick_offset;              // Timer macrotick offset
    uint8 timer_cycle_filter_mask;              // Timer cycle filter mask, only for absolute timer
    uint8 timer_cycle_filter_value;             // Timer cycle filter value, only for absolute timer
} Fr_timer_config_type;

/* Structure of this type contains configuration information for four internal non-memory mapped slot 
   status selection register SSSR0 to SSSR3.
   If the value of any slot number is set to 0, the related slot status register provides 
   the status of the symbol window after the NIT start, and provides the status of the NIT 
   after the cycle start. */
typedef struct
{
    uint16 SSSR0_slot_number;       // Static Slot number - the number of the slot whose status will be saved in the SSR0 and SSR1
    uint16 SSSR1_slot_number;       // Static Slot number - the number of the slot whose status will be saved in the SSR2 and SSR3
    uint16 SSSR2_slot_number;       // Static Slot number - the number of the slot whose status will be saved in the SSR4 and SSR5
    uint16 SSSR3_slot_number;       // Static Slot number - the number of the slot whose status will be saved in the SSR6 and SSR7
} Fr_slot_status_config_type;

/* Type definition of Slot Status Counter ID */
typedef enum 
{
    FR_SLOT_STATUS_COUNTER_0 = 0,   // Slot Status Counter 0 is configured
    FR_SLOT_STATUS_COUNTER_1,       // Slot Status Counter 1 is configured
    FR_SLOT_STATUS_COUNTER_2,       // Slot Status Counter 2 is configured
    FR_SLOT_STATUS_COUNTER_3        // Slot Status Counter 3 is configured
} Fr_slot_status_counter_ID_type;

/* Slot Status Counter configuration - the channel related incrementing of the slot status counter */
typedef enum
{										// Slot status counter is:
    FR_SLOT_STATUS_CHANNEL_A = 0,       // Incremented by 1 if condition is fulfilled on channel A
    FR_SLOT_STATUS_CHANNEL_B,           // Incremented by 1 if condition is fulfilled on channel B
    FR_SLOT_STATUS_CHANNEL_AB_BY_1,     // Incremented by 1 if condition is fulfilled on at least one channel
    FR_SLOT_STATUS_CHANNEL_AB_BY_2      // Incremented by 2 if condition is fulfilled on both channels
                                        // Incremented by 1 if condition is fulfilled on only one channel
} Fr_slot_status_counter_channel_type;

/* Structure of this type contains configuration 
   information for one slot status counter */
typedef struct 
{
    Fr_slot_status_counter_ID_type counter_ID;      // Select one of the four internal slot counter condition register for access
    Fr_slot_status_counter_channel_type counter_configuration;  // Controls the channel related incrementing
    boolean multi_cycle_selection;                  // Define whether the counter accumulates over multiple com.cycle
    boolean valid_frame_restriction;                // Restrict the counter to receive valid frames
    boolean sync_frame_restriction;                 // Restrict the counter to receive sync frames
    boolean null_frame_restriction;                 // Restrict the counter to receive null frames
    boolean startup_frame_restriction;              // Restrict the counter to receive startup frames
    boolean syntax_error_counting;                  // Enable the counting for slots with the syntax error indicator bit set to 1
    boolean content_error_counting;                 // Enable the counting for slots with set content error indicator
    boolean boundary_violation_counting;            // Enable the counting for slots with set boundary violation indicator
    boolean transmission_conflict_counting;         // Enable the counting for slots with set transmission conflict indicator
} Fr_slot_status_counter_config_type;

/* Structure of this type contains configuration information for Media Test Symbol (MTS) */
typedef struct
{
    uint8 cycle_counter_value;              // Defines the filter value for the MTS cycle count filter
    uint8 cycle_counter_mask;               // Defines the filter mask for the MTS cycle count filter
} Fr_MTS_config_type;

/* Structure of this type contains configuration
   information of the one low level parameters set */
#pragma pack(1) 
typedef struct
{
    uint8   G_COLD_START_ATTEMPTS;
    uint8   GD_ACTION_POINT_OFFSET;
    uint8   GD_CAS_RX_LOW_MAX;
    uint8   GD_DYNAMIC_SLOT_IDLE_PHASE;
    uint8   GD_MINISLOT;
    uint8   GD_MINI_SLOT_ACTION_POINT_OFFSET;
    uint16  GD_STATIC_SLOT;
    uint8   GD_SYMBOL_WINDOW;
    uint8   GD_TSS_TRANSMITTER;
    uint8   GD_WAKEUP_SYMBOL_RX_IDLE;
    uint8   GD_WAKEUP_SYMBOL_RX_LOW;
    uint16  GD_WAKEUP_SYMBOL_RX_WINDOW;
    uint8   GD_WAKEUP_SYMBOL_TX_IDLE;
    uint8   GD_WAKEUP_SYMBOL_TX_LOW;
    uint8   G_LISTEN_NOISE;
    uint16  G_MACRO_PER_CYCLE;
    uint8   G_MAX_WITHOUT_CLOCK_CORRECTION_PASSIVE;
    uint8   G_MAX_WITHOUT_CLOCK_CORRECTION_FATAL;
    uint16  G_NUMBER_OF_MINISLOTS;
    uint16  G_NUMBER_OF_STATIC_SLOTS;
    uint16  G_OFFSET_CORRECTION_START;
    uint8   G_PAYLOAD_LENGTH_STATIC;
    uint8   G_SYNC_NODE_MAX;
    uint8   G_NETWORK_MANAGEMENT_VECTOR_LENGTH;
    boolean P_ALLOW_HALT_DUE_TO_CLOCK;
    uint8   P_ALLOW_PASSIVE_TO_ACTIVE;
    Fr_channel_type  P_CHANNELS;
    uint16  PD_ACCEPTED_STARTUP_RANGE;
    uint8   P_CLUSTER_DRIFT_DAMPING;
    uint8   P_DECODING_CORRECTION;
    uint8   P_DELAY_COMPENSATION_CHA;
    uint8   P_DELAY_COMPENSATION_CHB;
    uint32  PD_LISTEN_TIMEOUT;
    uint16  PD_MAX_DRIFT;
    uint8   P_EXTERN_OFFSET_CORRECTION;
    uint8   P_EXTERN_RATE_CORRECTION;
    uint16  P_KEY_SLOT_ID;
    boolean P_KEY_SLOT_USED_FOR_STARTUP;
    boolean P_KEY_SLOT_USED_FOR_SYNC;
    uint16  P_KEY_SLOT_HEADER_CRC;
    uint16  P_LATEST_TX;
    uint8   P_MACRO_INITIAL_OFFSET_A;
    uint8   P_MACRO_INITIAL_OFFSET_B;
    uint8   P_MICRO_INITIAL_OFFSET_A;
    uint8   P_MICRO_INITIAL_OFFSET_B;
    uint32  P_MICRO_PER_CYCLE;
    uint16  P_OFFSET_CORRECTION_OUT;
    uint16  P_RATE_CORRECTION_OUT;
    boolean P_SINGLE_SLOT_ENABLED;
    Fr_channel_type  P_WAKEUP_CHANNEL;
    uint8   P_WAKEUP_PATTERN;
    uint8   P_MICRO_PER_MACRO_NOM;
    uint8   P_PAYLOAD_LENGTH_DYN_MAX;
} Fr_low_level_config_type;



//这个需要默认的配置
//frame_ID 和 list_num 有个对应的关系 

typedef struct slot_conf{
  uint16 frame_ID;
  uint8 FrameType;	//帧类型 静态 或动态
  Fr_buffer_type buffer_type;           // TRANSMIT, RECEIVE, SHADOW or FIFO
  Fr_channel_type receive_channel;
	uint8 base_circle ;
  uint8 base_circle_interval;
	uint8 payload_length;                    //负载长度
	Fr_transmission_type         transmission_mode;    //事件还是状态
	Fr_transmission_commit_type transmission_commit_mode;     
	Fr_channel_type transmit_channel;    //发送的通道\
	
	//下面暂时不进行交互	                 
	Fr_transmit_MB_type transmit_type;
  uint16_t header_CRC; //发送的CRC
    
  uint8_t  start_mb_index; //对用的开始的MB
  uint8_t Circle;     //周期         第一个Bit 为接收接收标志位
	uint16_t *data;//存放起始的地址  
} T_slot_conf;

//这边有个块的索引最好要核对好 
typedef struct{
	uint16_t mb_index; 
	uint16_t slot;//存放的SLOT 索引
} T_MB_Index;



//来计算数据的  

#define  MAX_DATA_BUF 1024

//每次更新，先重新分配 
typedef struct recv_data_buf{
    uint16 data[MAX_DATA_BUF];   //如果后续有动态大小的话，最好这样好
    uint16 cur_used; //当前已经使用到哪个地址了 
    
} T_Recv_Data_Buf;



#pragma pack() 


/* Type definition for the connected_HW field
   of the Fr_HW_config_type structure. 
   The values determine the type of used microcontroller */
typedef enum
{
    FR_MFR4300 = 0,
    FR_MC9S12XFR128,
    FR_MPC5567,          // MPC5567 FlexRay module Revision 0
    FR_MFR4310,
    FR_MPC5561,
    FR_MPC5516,
    FR_MC9S12XF,
    FR_MPC5567_REVA,     // MPC5567 FlexRay module Revision A
    FR_MPC5514
} Fr_connected_HW_type;

/* Type definition for the clock_source field
   of the Fr_HW_config_type structure. 
   The values determine the type of used oscillator */
typedef enum
{
    FR_EXTERNAL_OSCILLATOR = 0,     // Crystal oscillator is used
    FR_INTERNAL_SYSTEM_BUS_CLOCK    // Not used for the MFR4300 and MFR4310
} Fr_clock_source_type;

/* Type definition for the clock_source field
   of the Fr_HW_config_type structure. 
   The values determine the type of used oscillator */
typedef enum
{
    FR_DUAL_CHANNEL_MODE = 0,       // Dual channel mode
    FR_SINGLE_CHANNEL_MODE          // Single channel mode
} Fr_single_channel_mode_type;

/* Hardware configuration structure determines used hardware, 
   allocated memory addresses and configuration message buffer segments */
typedef struct
{
    uint32 CC_base_address;
    uint32 CC_FlexRay_memory_base_address;
    Fr_connected_HW_type connected_HW;            // Which type of Freescale FlexRay module is used
    boolean synchronization_filtering_enable;     // Synchronization frame filter enable
    Fr_clock_source_type clock_source;            // Protocol engine clock source select
    uint8 prescaler_value;                        // The value of the PRESCALE or BITRATE bit field in the MCR register
    uint8 MB_segment_1_data_size;                 // Data size - segment 1
    uint8 MB_segment_2_data_size;                 // Data size - segment 2
    uint8 last_MB_seg_1;                          // Last MB in segment 1, MBIDXEn with n = [0..last_MB_seg1]
    uint8 last_MB_util;                           // Last individual MB (Receive, Transmit, Receive Shadow) utilized
    // Total number of used MB including FIFO; in range [1..1024]
    // E.g. last_MB_util = 20 and FIFO A starts as the 21st MB and it's depth is 10; then
    // Number of individual MB = 1 + 20 = 21; Number of FIFO MBs = 10 and
    // total_MB_number = 31
    uint16 total_MB_number;
    boolean allow_cold_start_enable;
    // The value of the TIMEOUT bit field in the SYMATOR register - not implemented for MFR4300, MPC5567, MC9S12XFR128 and MFR4310
    uint8 timeout;                                // Stored only if the value is not equal to 0
    // Offset of the Sync Frame Table in the FlexRay memory
    uint16 sync_frame_table_offset;               // Stored only if the value is not equal to 0
    // TRUE - single channel mode, FALSE - dual channel mode
    Fr_single_channel_mode_type single_channel_mode;
} Fr_HW_config_type;

/* Structure of this type contains MB configuration parameters which are stored by driver for internal use */
typedef struct
{
    uint16 slot_number;             // Slot number - either 'transmit_frame_ID' or 'receive_frame_ID'
    //Fr_buffer_type buffer_type;     // TRANSMIT, RECEIVE MB - not necessary for the current version of the UNIFIED Driver
    Fr_transmit_MB_type transmission_mode;   // SINGLE or DOUBLE buffered MB
    Fr_transmission_type transmission_type;  // STATE or EVENT transmission mode
    void (* Fr_MB_function_ptr) (uint8 buffer_index_init);  // Callback function with index of MB (except FIFO)
} Fr_MB_information_internal_type;

/* Structure of this type contains slot status configuration parameters which are stored by driver for internal use */
typedef struct
{
    boolean registers_used;         // Slot Status registers are already configured
    uint16 SSSR0_slot_number;       // Static Slot number - the number of the slot whose status will be saved in the SSR0 and SSR1
    uint16 SSSR1_slot_number;       // Static Slot number - the number of the slot whose status will be saved in the SSR2 and SSR3
    uint16 SSSR2_slot_number;       // Static Slot number - the number of the slot whose status will be saved in the SSR4 and SSR5
    uint16 SSSR3_slot_number;       // Static Slot number - the number of the slot whose status will be saved in the SSR6 and SSR7
} Fr_slot_status_internal_type;

/* Structure of this type contains names of service callback functions which are stored by driver for internal use and 
    will be called by the Fr_interrupt_handler() function when an interrupt occurs. */
typedef struct
{
    void (* Fr_module_ptr) (void);      // Callback function for Module Interrupt Flag in the GIFER reg.
    void (* Fr_protocol_ptr) (void);    // Callback function for Protocol Interrupt Flag in the GIFER reg.
    void (* Fr_chi_ptr) (uint16 chi_error); // Callback function for CHI Interrupt Flag in the GIFER reg.
    void (* Fr_wakeup_ptr) (Fr_channel_type wakeup_channel); // Callback function for Wakeup Interrupt Flag in the GIFER reg.
    void (* Fr_fifo_B_ptr) (uint16 header_index);   // Callback function for FIFO B Interrupt Flag in the GIFER reg.
    void (* Fr_fifo_A_ptr) (uint16 header_index);   // Callback function for FIFO A Interrupt Flag in the GIFER reg.
    void (* Fr_fatal_protocol_error_ptr) (void);    // Callback function for FATL_IF Interrupt Flag in the PIFR0 reg.
    void (* Fr_internal_protocol_error_ptr) (void); // Callback function for INTL_IF Interrupt Flag in the PIFR0 reg.
    void (* Fr_illegal_protocol_conf_ptr) (void);   // Callback function for ILCF_IF Interrupt Flag in the PIFR0 reg.
    void (* Fr_coldstart_abort_ptr) (void);         // Callback function for CSA_IF Interrupt Flag in the PIFR0 reg.
    void (* Fr_missing_rate_corr_ptr) (void);       // Callback function for MRC_IF Interrupt Flag in the PIFR0 reg.
    void (* Fr_missing_offset_corr_ptr) (void);     // Callback function for MOC_IF Interrupt Flag in the PIFR0 reg.
    void (* Fr_clock_corr_limit_ptr) (void);        // Callback function for CCL_IF Interrupt Flag in the PIFR0 reg.
    void (* Fr_max_sync_frames_ptr) (void);         // Callback function for MXS_IF Interrupt Flag in the PIFR0 reg.
    void (* Fr_mts_received_ptr) (void);            // Callback function for MTX_IF Interrupt Flag in the PIFR0 reg.
    void (* Fr_violation_B_ptr) (void);             // Callback function for LTXB_IF Interrupt Flag in the PIFR0 reg.
    void (* Fr_violation_A_ptr) (void);             // Callback function for LTXA_IF Interrupt Flag in the PIFR0 reg.
    void (* Fr_trans_across_boundary_B_ptr) (void); // Callback function for TBVB_IF Interrupt Flag in the PIFR0 reg.
    void (* Fr_trans_across_boundary_A_ptr) (void); // Callback function for TBVA_IF Interrupt Flag in the PIFR0 reg.    
    void (* Fr_timer_2_expired_ptr) (void);         // Callback function for TI2_IF Interrupt Flag in the PIFR0 reg.    
    void (* Fr_timer_1_expired_ptr) (void);         // Callback function for TI1_IF Interrupt Flag in the PIFR0 reg.    
    void (* Fr_cycle_start_ptr) (void);             // Callback function for CYS_IF Interrupt Flag in the PIFR0 reg.    
    void (* Fr_error_mode_changed_ptr) (void);      // Callback function for EMC_IF Interrupt Flag in the PIFR1 reg.    
    void (* Fr_illegal_protocol_command_ptr) (void);    // Callback function for IPC_IF Interrupt Flag in the PIFR1 reg.
    void (* Fr_protocol_engine_comm_failure_ptr) (void); // Callback function for PECF_IF Interrupt Flag in the PIFR1 reg.
    void (* Fr_protocol_state_changed_ptr) (void);  // Callback function for PSC_IF Interrupt Flag in the PIFR1 reg. 
    void (* Fr_slot_status_counter_3_inc_ptr) (void);   // Callback function for SSI3_IF Interrupt Flag in the PIFR1 reg.
    void (* Fr_slot_status_counter_2_inc_ptr) (void);   // Callback function for SSI2_IF Interrupt Flag in the PIFR1 reg.
    void (* Fr_slot_status_counter_1_inc_ptr) (void);   // Callback function for SSI1_IF Interrupt Flag in the PIFR1 reg.
    void (* Fr_slot_status_counter_0_inc_ptr) (void);   // Callback function for SSI0_IF Interrupt Flag in the PIFR1 reg.
    void (* Fr_even_cycle_table_written_ptr) (void);    // Callback function for EVT_IF Interrupt Flag in the PIFR1 reg.
    void (* Fr_odd_cycle_table_written_ptr) (void);     // Callback function for ODT_IF Interrupt Flag in the PIFR1 reg.
} Fr_callback_functions_type;

/******************************************************************************
* Global variables
******************************************************************************/


/******************************************************************************
* Global functions
******************************************************************************/
extern Fr_return_type Fr_init(const Fr_HW_config_type *Fr_HW_config_temp_ptr, \
                            const Fr_low_level_config_type *Fr_low_level_config_temp_ptr);
extern void Fr_set_configuration(const Fr_HW_config_type *Fr_HW_config_temp_ptr, \
                            const Fr_low_level_config_type *Fr_low_level_config_temp_ptr);
extern Fr_return_type Fr_buffers_init(const Fr_buffer_info_type *Fr_buffers_config_temp_ptr, \
                            const Fr_index_selector_type *Fr_buffer_config_set_temp_ptr);


extern Fr_return_type Fr_buffers_init_custom(T_slot_conf *slot,T_MB_Index *mb_index);

extern void Fr_timers_init(const Fr_timer_config_type ** Fr_timer_config_temp_ptr_ptr);
extern void Fr_slot_status_init(const Fr_slot_status_config_type *Fr_slot_status_config_temp_ptr);
extern void Fr_slot_status_counter_init(const Fr_slot_status_counter_config_type ** Fr_slot_status_counter_config_temp_ptr_ptr);
extern Fr_return_type Fr_leave_configuration_mode(void);
extern Fr_return_type Fr_start_communication(void);
extern Fr_return_type Fr_stop_communication(Fr_stop_communication_type Fr_stop_option);
extern Fr_return_type Fr_send_wakeup(void);
extern Fr_wakeup_state_type Fr_get_wakeup_state(void);
extern Fr_POC_state_type Fr_get_POC_state(void);
extern Fr_sync_state_type Fr_get_sync_state(void);
extern Fr_return_type Fr_enter_configuration_mode(void);
extern Fr_return_type Fr_reset_protocol_engine(void);
extern void Fr_get_global_time(uint8 * Fr_cycle_ptr, uint16 * Fr_macrotick_ptr);
extern Fr_return_type Fr_send_MTS(Fr_channel_type Fr_channel, const Fr_MTS_config_type *Fr_MTS_config_temp_ptr);
extern Fr_MTS_state_type Fr_get_MTS_state(Fr_channel_type Fr_channel);
extern void Fr_get_network_management_vector(uint16 * Fr_vector_ptr);
extern Fr_return_type Fr_get_slot_status_reg_value(uint16 Fr_slot_number, Fr_channel_type Fr_channel, \
                            Fr_slot_status_required_type Fr_slot_status_required, uint16 *Fr_status_vector_ptr);
extern void Fr_get_channel_status_error_counter_value(Fr_channel_type Fr_channel, uint16 *Fr_counter_value_ptr);
extern void Fr_get_slot_status_counter_value(Fr_slot_status_counter_ID_type Fr_counter_idx, uint16 *Fr_counter_value_ptr);
extern void Fr_reset_slot_status_counter(Fr_slot_status_counter_ID_type Fr_counter_idx);
extern Fr_tx_MB_status_type Fr_transmit_data(uint16 Fr_buffer_idx, const uint16 *Fr_data_ptr, uint8 Fr_data_length);
extern Fr_rx_MB_status_type Fr_receive_data(uint16 Fr_buffer_idx, uint16 *Fr_data_ptr, \
                            uint8 *Fr_data_length_ptr,uint16 *Fr_slot_status_ptr);
extern Fr_FIFO_status_type Fr_receive_fifo_data(uint16 Fr_buffer_read_idx, uint16 *Fr_data_ptr, \
                            uint8 *Fr_data_length_ptr, uint16 *Fr_slot_idx_ptr, uint16 *Fr_slot_status_ptr);
extern void Fr_enable_interrupts(uint16 Fr_global_interrupt, uint16 Fr_protocol_0_interrupt, uint16 Fr_protocol_1_interrupt);
extern void Fr_disable_interrupts(uint16 Fr_global_interrupt, uint16 Fr_protocol_0_interrupt, uint16 Fr_protocol_1_interrupt);
extern void Fr_interrupt_handler(void);
extern void Fr_set_MB_callback(void (* MB_callback_ptr)(uint8 Fr_int_buffer_idx), uint8 Fr_buffer_idx);
extern Fr_return_type Fr_set_global_IRQ_callback(void (* callback_ptr)(void), uint16 Fr_global_interrupt);
extern Fr_return_type Fr_set_protocol_0_IRQ_callback(void (* callback_ptr)(void), uint16 Fr_protocol_interrupt);
extern Fr_return_type Fr_set_protocol_1_IRQ_callback(void (* callback_ptr)(void), uint16 Fr_protocol_interrupt);
extern void Fr_set_wakeup_IRQ_callback(void (* callback_ptr)(Fr_channel_type wakeup_channel));
extern void Fr_set_chi_IRQ_callback(void (* callback_ptr)(uint16 chi_error));
extern Fr_return_type Fr_set_fifo_IRQ_callback(void (* callback_ptr)(uint16 header_index), uint16 Fr_global_interrupt);
extern void Fr_clear_MB_interrupt_flag(uint8 Fr_buffer_idx);
extern void Fr_start_timer(Fr_timer_ID_type timer_ID);
extern void Fr_stop_timer(Fr_timer_ID_type timer_ID);
extern Fr_tx_status_type Fr_check_tx_status(uint8 Fr_buffer_idx);
extern Fr_rx_status_type Fr_check_rx_status(uint8 Fr_buffer_idx);
extern Fr_CHI_error_type Fr_check_CHI_error(void);
extern boolean Fr_check_cycle_start(uint8 *Fr_cycle_ptr);
extern boolean Fr_check_transmission_across_boundary(Fr_channel_type Fr_channel);
extern boolean Fr_check_violation(Fr_channel_type Fr_channel);
extern boolean Fr_check_max_sync_frame(void);
extern boolean Fr_check_clock_correction_limit_reached(void);
extern boolean Fr_check_missing_offset_correction(void);
extern boolean Fr_check_missing_rate_correction(void);
extern boolean Fr_check_coldstart_abort(void);
extern boolean Fr_check_internal_protocol_error(void);
extern boolean Fr_check_fatal_protocol_error(void);
extern boolean Fr_check_protocol_state_changed(void);
extern boolean Fr_check_protocol_engine_com_failure(void);
extern uint16 Fr_low_level_access_read_reg(uint16 Fr_required_register);
extern void Fr_low_level_access_write_reg(uint16 Fr_required_register, uint16 Fr_value);
extern uint16 Fr_low_level_access_read_memory(uint16 Fr_memory_address);
extern void Fr_low_level_access_write_memory(uint16 Fr_memory_address, uint16 Fr_value);


/******************************************************************************
* Inline functions
******************************************************************************/

#endif /* FR_H */