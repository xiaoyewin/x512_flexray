/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2005 Freescale Semiconductor, Inc.
* (c) Copyright 2001-2004 Motorola, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************//*!
*
* @file      Fr_UNIFIED_types.h
*
* @author    rg003c
* 
* @version   1.0.1.0
* 
* @date      Apr-26-2007
* 
* @brief     FlexRay UNIFIED Driver implementation
*            This file can be changed according to compiler requirements
*
******************************************************************************/

#ifndef FR_UNIFIED_TYPES
#define FR_UNIFIED_TYPES

/******************************************************************************
* Includes
******************************************************************************/


/******************************************************************************
* Types
******************************************************************************/

/* FlexRay module specific pointer class, this forces to compiler to use G-instructions
for accessing the global addresses on the MC9S12 microcontrollers */
/* Used only for MC9S12 microcontrollers */
//#define FR_REG_FAR __far        /* __far modifier definition for an access to FlexRay registers */
//#define FR_DATA_FAR __far       /* __far modifier definition for an access to FlexRay memory */

/* Used only for MC9S12 microcontrollers with intergrated FlexRay module */
#define FR_REG_FAR              /* __far modifier is not used for an access to FlexRay registers */
#define FR_DATA_FAR __far       /* __far modifier definition for an access to FlexRay memory */

/* Used for MAC71xx, MPC5xx, MPC55xx, 56F86xx microcontroller families */
/* or the MC9S12 microcontrollers with integrated FlexRay module and FlexRay memory mapped into the Local Memory Map */
//#define FR_REG_FAR             /* __far modifier is not used for an access to FlexRay registers */
//#define FR_DATA_FAR            /* __far modifier is not used for an access to FlexRay memory */

#include <stdio.h>

typedef unsigned char       boolean;

typedef signed char         sint8;
typedef unsigned char       uint8;
typedef signed short int    sint16;
typedef unsigned short int  uint16;
typedef signed long int     sint32;
typedef unsigned long int   uint32;
typedef unsigned short int  uint8_least;
typedef unsigned int        uint16_least;
typedef unsigned long int   uint32_least;
typedef signed short int    sint8_least;
typedef signed int          sint16_least;
typedef signed long int     sint32_least;


typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short int    int16_t;
typedef unsigned short int  uint16_t;
typedef signed long int     int32_t;
typedef unsigned long int   uint32_t;


typedef float               float32;
typedef double              float64;

typedef uint8 Fr_return_type;

#define FR_NOT_SUCCESS  0
#define FR_SUCCESS      1

/*
#ifndef TRUE
    #define TRUE ((boolean) 1)
#endif

#ifndef FALSE
    #define FALSE ((boolean) 0)
#endif

#ifndef NULL
    #define NULL (void *)(0)
#endif
*/

#endif /* FR_UNIFIED_TYPES */