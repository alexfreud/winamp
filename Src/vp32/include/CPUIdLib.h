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



#ifndef _CPUIDLIB_H
#define _CPUIDLIB_H

typedef enum PROCTYPE { 
    X86         = 0,            /* 486, Pentium plain, or any other x86 compatible */
    PMMX        = 1,            /* Pentium with MMX */
    PPRO        = 2,            /* Pentium Pro */
    PII         = 3,            /* Pentium II */
    C6X86       = 4,
    C6X86MX     = 5,
    AMDK63D     = 6,
    AMDK6       = 7,
    AMDK5       = 8,
    MACG3		= 9,
    MAC68K		= 10,
    XMM         = 11,           /* SIMD instructions */
	WMT			= 12,			/* Willamette Processor */
    SpecialProc = -1            /* Will NEVER be returned by CPUID, function dependent meaning */
}PROCTYPE;

#ifdef __cplusplus                          /* this ifdef only works correctly for Microsoft visual C compilers */

extern "C" PROCTYPE findCPUId(void);

#else

/*
 * **-findCPUId
 *
 * This function will return the type of CPU that you have in your system.
 *
 * Assumptions:
 *   None
 *
 * Inputs:
 *   None
 *
 * Output:
 *   The type of CPU that you have in your system is returned
 *
 */
extern PROCTYPE findCPUId(void);

#endif

#endif /* CPUIDLIB_H */
