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
 *   Module Title :     Wmt_CpuID.cpp
 *
 *   Description  :     willamette processor detection functions
 *
 *
 *****************************************************************************
 */
 
/****************************************************************************
 *  Header Files
 *****************************************************************************
 */


#include <excpt.h>
#include <string.h>


extern "C" {

/****************************************************************************
 * 
 *  ROUTINE       :     WillametteNewInstructionSupport()
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     retrun true if the processor support willamette new 
 *						instructions, return false otherwise
 *						
 *
 *  FUNCTION      :     detect willamette processor
 *
 *  SPECIAL NOTES :     None. 
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/

int WillametteNewInstructionHWSupport()
{

	int HWSupport = 0;
	char brand[12];

	__try 
	{
		__asm
		{
			
			lea		esi,		brand
			mov		eax,		0
			cpuid				
			mov		[esi],		ebx
			mov		[esi+4],	edx
			mov		[esi+8],	ecx

		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{

		if(_exception_code())
		{
			//cout<<endl<<"*******CPUID is not supported**********"<<endl;
			return 0;
		}
		return 0;

	}


	if(strncmp(brand, "GenuineIntel", 12)!=0)
	{
		
		//cout<<endl<<"this is not an intel processor1"<<endl;
		return 0;
	}

	__asm 
	{
			mov		eax,	1
			cpuid	
			test	edx,	04000000h
			jz		NotFound
			mov		[HWSupport], 1
			
NotFound:
			nop

	}

	return (HWSupport);
}


/****************************************************************************
 * 
 *  ROUTINE       :     WillametteNewInstructionOSSupport()
 *
 *  INPUTS        :     
 *
 *  OUTPUTS       :     
 *
 *  RETURNS       :     retrun true if the OS support willamette new 
 *						instructions, return false otherwise
 *						
 *
 *  FUNCTION      :     detect willamette processor
 *
 *  SPECIAL NOTES :     None. 
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
int	WillametteNewInstructionOSSupport()
{
	__try
	{
		__asm xorpd		xmm0, xmm0
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		if(_exception_code())
		{
			return 0;
		}
		return 0;
	}
	return 1;
}

}