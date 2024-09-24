#pragma once
#include "dataio.h"
#include "vlbout.h"
class VLBIn : public DataIOControl
{
public:
	VLBIn() { pos = size = 0; }
	virtual int IO( void *buf, int size, int count );
	virtual int Seek( long offset, int origin )
	{
		return 0;
	}
	virtual int Close() { return 0; }
	virtual int EndOf( void ) { return 0; }
	virtual int DICGetLastError() { return DATA_IO_ERROR_NONE; }
	virtual int DICGetDirection() { return DATA_IO_READ; }
	int GetInputFree() { return BUFSIZE - size; }
	int GetSize() { return size; }
	void Fill( unsigned char *buf, int nbytes );
	void Empty() { pos = size = 0; }
private:
	unsigned char data[BUFSIZE];
	int size, pos;
};
