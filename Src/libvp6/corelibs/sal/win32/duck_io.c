/***********************************************\
??? duck_io.c
\***********************************************/

#include <stdio.h> 
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include "duck_io.h"
#include "duck_hfb.h"

#include <assert.h>


int read_count;


int duck_readFinished(int han, int flag)
{
	return 1;
}




int duck_open(const char *name, unsigned long userData)
{
    int f;   
	FILE *fp;
	unsigned long x;
    
	read_count = 0;

	fp = fopen(name, "rb");

	if (!fp)
		assert(0);

	x = (unsigned long ) fp;

	/* high bit is set, the cast is a bad idea ! */
	if (x & 0x90000000)
		assert(0);
	
	f = (int ) x;

	return f;

}




void duck_close(int handle)
{
	fclose((FILE *) handle);
}



static long totalRead = 0;
static long tellNo = 0;
long duck_read(int handle,unsigned char *buffer,long bytes)
{
    int x;


	if (buffer == NULL){
		duck_seek(handle,bytes,SEEK_CUR);
		return bytes;
	}

	tellNo = ftell((FILE *) handle);

	if (bytes == 0)
		return 0;


	x = fread(buffer,sizeof(char) ,bytes, (FILE *) handle);


	if (feof((FILE *) handle) && (x != (int ) bytes))
		return -1;


	totalRead += x;

	if (x == -1L)
		assert(0);
	
	return x ;
}


long duck_seek(int handle,long offset,int origin)
{
	long x = fseek((FILE *) handle,offset,origin);

	tellNo = ftell((FILE *) handle);

	return tellNo ;

	
}


