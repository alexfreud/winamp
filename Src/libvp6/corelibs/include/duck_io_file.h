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


#ifndef _duck_io_h_old
#define _duck_io_h_old

#if defined(__cplusplus)
extern "C" {
#endif

#if defined (_WIN32)
typedef __int64 int64_t;
#else
typedef long long int64_t;
#endif

#include "duck_io.h"

int duck_open_file(const char *fname, unsigned long userData); 

void duck_close_file(int ghndl);

int duck_read_file(int ghndl,unsigned char *buf, int nbytes);

int64_t duck_seek_file(int gHndl,int64_t offs, int origin);

int duck_name_file(int handle, char fname[], size_t maxLen); /* EMH 9-23-03 */

int64_t duck_available_data_file(int handle); /* EMH 10-23-03 */

#if defined(__cplusplus)
}
#endif

#endif
