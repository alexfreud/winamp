#ifndef NULLSOFT_WINAMP_INFLATEOBJECT_H
#define NULLSOFT_WINAMP_INFLATEOBJECT_H

#include "api_inflate.h"

class ZLIBInflate : public api_inflate
{
public:
	static const char *getServiceName() { return "zlib inflate"; }
	static const GUID getServiceGuid() { return inflateGUID; }
public:
	int Reset(void *strm);
	int Init(void *strm, const char *version, int stream_size);
	int Init2(void *strm, int windowBits, const char *version, int stream_size);
	int Inflate(void *strm, int flush);
	int End(void *strm);
	unsigned long CRC32(unsigned long crc, const unsigned  char *buf, unsigned int len);

	int deflateReset(void *strm);
	int deflateInit2_(void *strm, int level, int method, int windowBits, int memLevel, int strategy, const char *version, int stream_size);		
	int deflate(void *strm, int flush);
	int deflateEnd(void *strm);
protected:
	RECVS_DISPATCH;
};

extern ZLIBInflate *zlibInflate;
#endif