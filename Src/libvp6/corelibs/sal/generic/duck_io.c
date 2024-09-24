/***********************************************\
??? duck_io.c
\***********************************************/

#include <stdio.h> 
#include <string.h>
#include <fcntl.h>
#include "duck_io.h"
#include "duck_io_http.h"
#include "duck_io_file.h"
#include "duck_hfb.h"

#include <assert.h>

#define MAKE_FOUR_CC(b1, b2, b3, b4 ) \
        ((b4 << 24) | (b3 << 16) | (b2 << 8) | (b1 << 0))

int duck_readFinished(int han, int flag)
{
    (void)han;  
    (void)flag;
	return 1;
}


bool g_isHttp = false;

int duck_open(const char *name, unsigned long userData)
{
    if (strstr(name, "http://"))
		return duck_open_http(name, userData);
	else 
		return duck_open_file(name, userData);

}




void duck_close(int handle)
{
	unsigned long schemeCC = *((unsigned long *) handle);

	if (schemeCC == MAKE_FOUR_CC('h','t','t','p'))
		duck_close_http(handle);
	else if (schemeCC == MAKE_FOUR_CC('f','i','l','e'))
		duck_close_file(handle);
	else
		assert(0);
}


int duck_read(int handle,unsigned char *buffer,int bytes)
{
    unsigned long schemeCC = *((unsigned long *) handle);

	if (schemeCC == MAKE_FOUR_CC('h','t','t','p'))
		return duck_read_http(handle, buffer, bytes);
	else if (schemeCC == MAKE_FOUR_CC('f','i','l','e'))
		return duck_read_file(handle, buffer, bytes);
	else
	{
		assert(0);
		return -1;
	}
}

int duck_read_blocking(int handle,unsigned char *buffer,int bytes)
{
    unsigned long schemeCC = *((unsigned long *) handle);

	if (schemeCC == MAKE_FOUR_CC('h','t','t','p'))
		return duck_read_blocking_http(handle, buffer, bytes);
	else if (schemeCC == MAKE_FOUR_CC('f','i','l','e'))
		return duck_read_file(handle, buffer, bytes);
	else
	{
		assert(0);
		return -1;
	}
}



int64_t duck_seek(int handle,int64_t offset,int origin)
{

	unsigned long schemeCC = *((unsigned long *) handle);

	if (schemeCC == MAKE_FOUR_CC('h','t','t','p'))
		return duck_seek_http(handle, offset, origin);
	else if (schemeCC == MAKE_FOUR_CC('f','i','l','e'))
		return duck_seek_file(handle, offset, origin);
	else
	{
		assert(0);
		return -1;
	}
	
}

int duck_name(int handle, char name[], size_t maxLen)
{
   	unsigned long schemeCC = *((unsigned long *) handle);

	if (schemeCC == MAKE_FOUR_CC('h','t','t','p'))
		return duck_name_http(handle, name, maxLen);
	else if (schemeCC == MAKE_FOUR_CC('f','i','l','e'))
		return duck_name_file(handle, name, maxLen);
	else
	{
		assert(0);
		return -1;
	}
}

int64_t duck_available_data(int handle)
{
   	unsigned long schemeCC = *((unsigned long *) handle);

	if (schemeCC == MAKE_FOUR_CC('h','t','t','p'))
		return duck_available_data_http(handle);
	else if (schemeCC == MAKE_FOUR_CC('f','i','l','e'))
		return duck_available_data_file(handle);
	else
	{
		assert(0);
		return -1;
	}
}
