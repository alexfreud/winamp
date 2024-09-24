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


#ifndef _duck_io_h
#define _duck_io_h

#if defined(__cplusplus)
extern "C" {
#endif

#if defined (_WIN32)
	typedef __int64 int64_t;
#endif
typedef struct
{
    int64_t  offset;     // offset to start from
    int    blocking;    // non-zero for blocking
} ReOpen_t;


typedef enum {
        SAL_ERR_MAX                 = -10,
        SAL_ERROR                   = -11, // Default error
		SAL_ERR_WSASTARTUP			= -12,
		SAL_ERR_SOCKET_CREATE		= -13,
		SAL_ERR_RESOLVING_HOSTNAME	= -14,
		SAL_ERR_SERVER_CONNECTION	= -15,
		SAL_ERR_SENDING_DATA		= -16,
		SAL_ERR_RECEIVING_DATA		= -17,
		SAL_ERR_404_FILE_NOT_FOUND	= -18,
		SAL_ERR_PARSING_HTTP_HEADER = -19,
		SAL_ERR_PARSING_CONTENT_LEN = -20,
		SAL_ERR_CONNECTION_TIMEOUT  = -21,
        SAL_ERR_FILE_OPEN_FAILED    = -22,
        SAL_ERR_MIN                 = -23
} SAL_ERR; /* EMH 1-15-03 */


typedef struct SalErrMap_temp
{
	SAL_ERR code;
	const char* decode;

} SalErrMap_t;


static char* SalErrText(SAL_ERR e)
{
    int t;
    const SalErrMap_t gSalErrMap[] = 
	{
		{	SAL_ERR_WSASTARTUP,				"Error with WSAStartup"			},	
		{	SAL_ERR_SOCKET_CREATE,			"Error creating socket"			},
		{	SAL_ERR_RESOLVING_HOSTNAME,		"Error resolving hostname"		},
		{	SAL_ERR_SERVER_CONNECTION,		"Error connecting to server"	},	
		{	SAL_ERR_SENDING_DATA,			"Error sending data"			},		
		{	SAL_ERR_RECEIVING_DATA,			"Error receiving data"			},
		{	SAL_ERR_404_FILE_NOT_FOUND,		"Error file not found "			},
		{	SAL_ERR_PARSING_HTTP_HEADER,	"Error parsing http header"		},
		{	SAL_ERR_PARSING_CONTENT_LEN,	"Error parsing content length"  },
		{	SAL_ERR_CONNECTION_TIMEOUT,		"Error Connection timed out"	},
		{   SAL_ERR_FILE_OPEN_FAILED,       "Error opening file"            }
	};
	
	for(t = 0; t < sizeof(gSalErrMap)/sizeof(SalErrMap_t); t++)
	{
	     if (e == gSalErrMap[t].code)
	     		return (char *) gSalErrMap[t].decode;
	}
	
	return 0;
}







int duck_open(const char *fname, unsigned long userData); 

void duck_close(int ghndl);

int duck_read(int ghndl,unsigned char *buf, int nbytes);

int64_t duck_seek(int gHndl,int64_t offs, int origin);

int duck_readFinished(int han, int flag); /* FWG 7-9-99 */

int duck_name(int handle, char name[], size_t maxLen); /* EMH 9-23-03 */

int duck_read_blocking(int handle,unsigned char *buffer,int bytes); /* EMH 9-23-03 */

int64_t duck_available_data(int handle); /* EMH 10-23-03 */

#if defined(__cplusplus)
}
#endif

#endif
