/****************************************************************************
*
*   Module Title :     CPUIdLib.h
*
*   Description  :     CPU specific definitions.
*
****************************************************************************/
#ifndef __INC_CPUIDLIB_H
#define __INC_CPUIDLIB_H

/****************************************************************************
* Typedefs
****************************************************************************/
typedef enum PROCTYPE 
{ 
    X86         = 0,  /* 486, Pentium plain, or any other x86 compatible */
    PMMX        = 1,  /* Pentium with MMX */
    PPRO        = 2,  /* Pentium Pro */
    PII         = 3,  /* Pentium II */
    C6X86       = 4,
    C6X86MX     = 5,
    AMDK63D     = 6,
    AMDK6       = 7,
    AMDK5       = 8,
    MACG3		= 9,
    MAC68K		= 10,
    XMM         = 11, /* SIMD instructions */
	WMT			= 12, /* Willamette Processor */
    SpecialProc = -1  /* Will NEVER be returned by CPUID, function dependent meaning */
} PROCTYPE;

/****************************************************************************
* Exports
****************************************************************************/

/****************************************************************************
 * 
 *  ROUTINE       :     findCPUId
 *
 *  INPUTS        :     None
 *
 *  OUTPUTS       :     None.
 *
 *  RETURNS       :     PROCTYPE: processor type.
 *
 *  FUNCTION      :     Returns type of CPU in your system.
 *
 *  SPECIAL NOTES :     None. 
 *
 ****************************************************************************/
#ifdef __cplusplus              /* this ifdef only works correctly for Microsoft visual C compilers */
extern "C" PROCTYPE findCPUId ( void );
#else
extern     PROCTYPE findCPUId ( void );
#endif

#endif
