/****************************************************************************
*
*   Module Title :     xprintf.h
*
*   Description  :     Debug print interface header file.
*
****************************************************************************/
#ifndef __INC_XPRINTF_H
#define __INC_XPRINTF_H

/****************************************************************************
*  Header Files
****************************************************************************/
#include "pbdll.h"

/****************************************************************************
*  Functions
****************************************************************************/
#if __cplusplus
extern "C"
{
#endif

// Display a printf style message on the current video frame
extern int vp6_xprintf(const PB_INSTANCE* ppbi, long pixel, const char* format, ...);

#if __cplusplus
}
#endif

#endif
