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


#ifndef _duck_io_h_http
#define _duck_io_h_http

#include <string.h> /* get size_t */
#include "duck_io.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if defined (_WIN32)
typedef __int64 int64_t;
#else
typedef long long int64_t;
#endif


int duck_open_http(const char *fname, unsigned long userData); 

void duck_close_http(int ghndl);

int duck_read_http(int ghndl,unsigned char *buf, int nbytes);

int64_t duck_seek_http(int gHndl,int64_t offs, int origin);

int duck_sal_error_http(void* handle, SAL_ERR* lastErrorCode, char buffer[], size_t maxLen); /* EMH 1-15-03 */

char* duck_init_http(char* url); /* EMH 1-17-03 */

void duck_exit_http(int handle); /* EMH  6-09-03 */

int duck_sal_fill(void * handle, bool blocking); /* EMH 6-12-03 */

void duck_http_timeout(int handle, unsigned long milliseconds);

int duck_sal_buff_percent(void* handle); /* debug */

int64_t duck_available_data_http(int handle); /* EMH 10-23-03 */

int64_t duck_content_len(void *handle);

int duck_name_http(int handle, char url[], size_t maxLen); /* EMH 9-23-03 */

int duck_read_blocking_http(int handle,unsigned char *buffer, int bytes); /* EMH 9-23-03 */

#if defined(__cplusplus)
}
#endif

#endif
