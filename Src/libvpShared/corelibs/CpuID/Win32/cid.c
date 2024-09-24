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


#include <windows.h>
#include <stdarg.h>
#include "cpuidlib.h"
#include "cidasm.h"
#include <process.h>
#include <stdio.h>

extern int	WillametteNewInstructionOSSupport();
extern int	WillametteNewInstructionHWSupport();


/*
 * **-DoesOSSupportXMM
 *
 * This function will check to see if the operating supports the XMM (Pentium III) instructions
 * The XMM functionality adds 8 128-bit registers to the pentium II register set.  With the addition
 * of the new registers the OS needs to preserve and restore the registers on task switches.
 *
 * Inputs:
 *  None
 *
 * Outputs:
 *  True returned if the OS supports the XMM instructions.
 *  False returned if the OS does not suppor the XMM instructions.
 */
int DoesOSSupportXMM( void )
{
   OSVERSIONINFO OSInformation;           // Data structure where OS version will be filled in
   int           ReturnValue = FALSE;     // Preload to fail

   // need to initilize size of OS info structure before calling GetVersionEx
   OSInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   
   if( !GetVersionEx( &OSInformation ) )  // Get OS information
   {
      /*
       * having trouble getting OS information
       * to be safe will return that we do not support XMM
       * instructions
       */
      // ReturnValue = FALSE;
   }

   if( OSInformation.dwPlatformId == VER_PLATFORM_WIN32_NT )
   //   if( 1 )
   {
      /*
       * If we are on a windows NT system we cannot directly
       * read the control registers to see if the OS supports
       * the XMM instructions.  We will just check to see if
       * service pack 4 is installed.
       */
       int ServicePackNumber;
       
       if( strcmp(OSInformation.szCSDVersion, "" ) != 0 ) // is there a service pack installed?
       {
           // Yes, get service pack revision
           char Junk[132], Junk2[132];

           sscanf( OSInformation.szCSDVersion, "%s %s %d", Junk, Junk2, &ServicePackNumber );
       }
       else
       {
           ServicePackNumber = 0;
       }

       if( OSInformation.dwMajorVersion == 4 && // must be versio 4 or greater
           ServicePackNumber >= 4 ||              // must have service pack 4 or greater
			OSInformation.dwMajorVersion >=5)
       {
           ReturnValue = TRUE;
       }
       else
       {
           // ReturnValue = FALSE;
       }
           
#if 0
       // some handy debugging info if you are desperate
       printf("OS Major Revision %d\n", OSInformation.dwMajorVersion );
       printf("OS Minor REvision %d\n", OSInformation.dwMinorVersion  );
       printf("Service Pack Number %d\n", ServicePackNumber );
#endif
   }
   else
   {
      /*
       * we are on a Windows 9x system.
       */
      //if( Does9xOSSupportXMM())         // does the Windows 9x support the XMM instructions?
      {
         ReturnValue = TRUE;            // yup
      }
      //else
      //{
         //ReturnValue = FALSE;           // Nope, don't support XMM instructions
      //}
   }

   return( ReturnValue );
}

/*
 * **-findCPUId
 *
 * See cpuidlib.h for a detailed description of this function
 */
PROCTYPE findCPUId( void )
{
   PROCTYPE CpuType;
// return 0;
// return (PII);         // drop to next lowest type of CPU which should be the Pentium II processor

   CpuType = getCPUType();      // Get version of processor

   // The code to check whether willammete instructions are called attempts to run 
   // an illegal instruction.  Under 98 mplayer crashes the os as soon as the illegal 
   // instruction is called, so I've disabled it.  

   if( CpuType == XMM )         // If the CPU supports XMM (Pentium III) instructions
   {
//      if( DoesOSSupportXMM())   // need to check to see if the OS supports the XMM instructions
      {
		  
		 if( WillametteNewInstructionHWSupport()&&
		  WillametteNewInstructionOSSupport())
		  {
					CpuType = WMT;					
		  }
      }
//      else
//      {
         // os does not support the XMM instructions
//         CpuType = PII;         // drop to next lowest type of CPU which should be the Pentium II processor
//      }
   }
   return( CpuType );
}


