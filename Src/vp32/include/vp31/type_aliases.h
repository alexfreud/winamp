//==========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1999 - 2001  On2 Technologies Inc. All Rights Reserved.
//
//--------------------------------------------------------------------------


/****************************************************************************
*
*   Module Title :     TYPE_ALIASES.H
*
*   Description  :     Standard type aliases
*
*
*****************************************************************************
*/

#ifndef TYPE_ALIASES
#define TYPE_ALIASES

#define EXPORT         
#define IMPORT extern  /* Used to declare imported data & routines */ 
#define PRIVATE static /* Used to declare & define module-local data */ 
#define LOCAL static   /* Used to define all persistent routine-local data */ 
#define STD_IN_PATH 0  /* Standard input path */
#define STD_OUT_PATH    1  /* Standard output path */
#define STD_ERR_PATH    2  /* Standard error path */
#define STD_IN_FILE stdin    /* Standard input file pointer */
#define STD_OUT_FILE    stdout   /* Standard output file pointer */
#define STD_ERR_FILE    stderr   /* Standard error file pointer */
#define  MAX_int    0x7FFFFFFF

#define __export   
#define _export  

typedef signed char INT8;
typedef unsigned char UINT8;
typedef signed short INT16;
typedef unsigned short UINT16;  

typedef signed int INT32;
typedef unsigned int UINT32; 
typedef int BOOL;

#ifdef LINUX
#define _inline __inline__
#endif

#if defined( MACPPC ) || defined( LINUX )
typedef long long INT64;
#else
typedef __int64 INT64;
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

/*   Floating point value. */
typedef  double     FLOAT64;      
typedef  float      FLOAT32;
typedef  unsigned char   BOOLEAN;
#define CCONV

#endif
