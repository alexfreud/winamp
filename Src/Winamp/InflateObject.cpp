#include "main.h"
#include "InflateObject.h"
#include "zlib/unzip.h"

int ZLIBInflate::Reset(void *strm)
{
	return ::inflateReset((z_streamp)strm);
}

int ZLIBInflate::Init(void *strm,const char *version, int stream_size)
{
	return ::inflateInit_((z_streamp)strm, version, stream_size);
}

int ZLIBInflate::Init2(void *strm,int windowBits,const char *version, int stream_size)
{
	return ::inflateInit2_((z_streamp)strm, windowBits,version, stream_size);
}

int ZLIBInflate::Inflate(void *strm, int flush)
{
	return ::inflate((z_streamp)strm, flush);
}

int ZLIBInflate::End(void *strm)
{
return 	::inflateEnd((z_streamp)strm);
}

unsigned long ZLIBInflate::CRC32(unsigned long crc, const unsigned  char *buf, unsigned int len)
{
	return ::crc32(crc, buf, len);
}

int ZLIBInflate::deflateReset(void *strm)
{
	return ::deflateReset((z_streamp)strm);
}

int ZLIBInflate::deflateInit2_(void *strm, int level, int method, int windowBits, int memLevel, int strategy, const char *version, int stream_size)
{
	return ::deflateInit2_((z_streamp)strm, level, method, windowBits, memLevel, strategy, version, stream_size);
}

int ZLIBInflate::deflateEnd(void *strm)
{
	return ::deflateEnd((z_streamp)strm);
}

int ZLIBInflate::deflate(void *strm, int flush)
{
	return ::deflate((z_streamp)strm, flush);
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS ZLIBInflate
START_DISPATCH;
  CB(API_INFLATE_INFLATERESET, Reset);
  CB(API_INFLATE_INFLATEINIT, Init);
	CB(API_INFLATE_INFLATEINIT2, Init2);
  CB(API_INFLATE_INFLATE, Inflate);
  CB(API_INFLATE_INFLATEEND, End);
	CB(API_INFLATE_CRC32, CRC32);
  CB(API_INFLATE_DEFLATERESET, deflateReset);
  CB(API_INFLATE_DEFLATEINIT2, deflateInit2_);
  CB(API_INFLATE_DEFLATEEND, deflateEnd);
  CB(API_INFLATE_DEFLATE, deflate);
END_DISPATCH;
