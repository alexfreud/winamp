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
*   Module Title :     Timer.h
*
*   Description  :     Video CODEC timer module
*
*    AUTHOR      :     Paul Wilkins
*
*****************************************************************************
*   Revision History
*   
*   1.00 PGW 12/10/99  Configuration baseline
*
*****************************************************************************
*/

#ifndef TIMER_H
#define TIMER_H

#include "type_aliases.h"

/****************************************************************************
*  Constants
*****************************************************************************
*/


/****************************************************************************
*  Types
*****************************************************************************
*/        

/****************************************************************************
*   Data structures
*****************************************************************************
*/


/****************************************************************************
*  Functions
*****************************************************************************
*/

extern void MyInitTimer( void );
extern UINT32 MyGetTime( void );
extern UINT32 MyGetElapsedCpuTime( void );	  

#endif
