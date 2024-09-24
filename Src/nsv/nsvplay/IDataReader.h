#ifndef NULLSOFT_NSV_NSVPLAY_IDATAREADER_H
#define NULLSOFT_NSV_NSVPLAY_IDATAREADER_H

#include <stddef.h>
#include <bfc/platform/types.h>

class IDataReader 
{
public:
	virtual ~IDataReader() { }
	virtual size_t read(char *buf, size_t len)=0; // returns bytes read
	virtual bool iseof()=0;
	virtual char *geterror()=0;
	virtual char *gettitle() { return 0; }
	virtual char *getheader(char *header_name) { return 0; }
	virtual bool canseek() { return 0; }
	virtual int seek(uint64_t newpos) { return 1; }
	virtual uint64_t getsize() { return ~0; }
};

#endif