#ifndef XPRINTF_H
#define XPRINTF_H
//____________________________________________________________________________
//
//  File:  xprintf.h
//
//  Description:  Display a printf style message on the current video frame
//
//  Author:  Keith Looney
//
//____________________________________________________________________________
//  Revision History
//

//____________________________________________________________________________
//  Includes

#include "pbdll.h"

//____________________________________________________________________________
//  Defines

//____________________________________________________________________________
//  Declarations

#if __cplusplus
extern "C"
{
#endif

extern int vp5_xprintf(const PB_INSTANCE* ppbi, long pixel, const char* format, ...);

#if __cplusplus
}
#endif

#endif  //  XPRINTF_H
