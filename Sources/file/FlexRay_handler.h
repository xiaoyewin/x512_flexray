/*******************************************************************************/
/**
Copyright (c) 2007 Freescale Semiconductor
Freescale Confidential Proprietary
\file       FlexRay_handler.h
\brief      FlexRay module handling function prototypes
\author     Freescale Semiconductor
\author     Guadalajara Applications Laboratory RTAC Americas
\author     Jaime Orozco
\version    1.0
\date       March/12/2007
*/
/*******************************************************************************/
/*                                                                             */
/* All software, source code, included documentation, and any implied know-how */
/* are property of Freescale Semiconductor and therefore considered            */ 
/* CONFIDENTIAL INFORMATION.                                                   */
/*                                                                             */
/* This confidential information is disclosed FOR DEMONSTRATION PURPOSES ONLY. */
/*                                                                             */
/* All Confidential Information remains the property of Freescale Semiconductor*/
/* and will not be copied or reproduced without the express written permission */
/* of the Discloser, except for copies that are absolutely necessary in order  */
/* to fulfill the Purpose.                                                     */
/*                                                                             */
/* Services performed by FREESCALE in this matter are performed AS IS and      */
/* without any warranty. CUSTOMER retains the final decision relative to the   */
/* total design and functionality of the end product.                          */
/*                                                                             */
/* FREESCALE neither guarantees nor will be held liable by CUSTOMER for the    */
/* success of this project.                                                    */
/*                                                                             */
/* FREESCALE disclaims all warranties, express, implied or statutory including,*/
/* but not limited to, implied warranty of merchantability or fitness for a    */
/* particular purpose on any hardware, software or advise supplied to the      */
/* project by FREESCALE, and or any product resulting from FREESCALE services. */
/*                                                                             */
/* In no event shall FREESCALE be liable for incidental or consequential       */
/* damages arising out of this agreement. CUSTOMER agrees to hold FREESCALE    */
/* harmless against any and all claims demands or actions by anyone on account */
/* of any damage,or injury, whether commercial, contractual, or tortuous,      */
/* rising directly or indirectly as a result of the advise or assistance       */
/* supplied CUSTOMER in connection with product, services or goods supplied    */
/* under this Agreement.                                                       */
/*                                                                             */
/*******************************************************************************/

/** Standard and driver types */
#include "Fr_UNIFIED_types.h"           


/** Function Prototypes */

/** Error function for debugging */
void Failed(uint8 u8number);

/** Function for transmission on Slot 1 */
void CC_interrupt_slot_1(uint8 buffer_idx);

/** Function for reception on Slot 4*/
void CC_interrupt_slot_4(uint8 buffer_idx);

/** Function for FlexRay Timer 1 */
void CC_interrupt_timer_1(void);

/** Function for FlexRay Timer 2 */
void CC_interrupt_timer_2(void);

/** Function for FlexRay cycle start interrupt */
void CC_interrupt_cycle_start(void);

/** Function for FlexRay FIFO A interrupt */
void CC_interrupt_FIFO_A(uint16 header_idx);

/** FlexRay module configuration */
void vfnFlexRay_Init();

void vfnFlexRay_Handler(void);

/*******************************************************************************/