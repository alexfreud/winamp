#ifndef NULLSOFT_WINAMP_API_INFLATE_H
#define NULLSOFT_WINAMP_API_INFLATE_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>

/* TODO: this should be renamed api_zlib */
class NOVTABLE api_inflate : public Dispatchable
{
protected:
	api_inflate() {}
	~api_inflate() {}
public:
	int inflateReset(void *strm);
	int inflateInit_(void *strm,const char *version, int stream_size);
	int inflateInit2_(void *strm, int windowBits, const char *version, int stream_size);
	int inflate(void *strm, int flush);
	int inflateEnd(void *strm);
	unsigned long crc32(unsigned long crc, const unsigned  char *buf, unsigned int len);
	
	int deflateReset(void *strm);
	int deflateInit2_(void *strm, int level, int method, int windowBits, int memLevel, int strategy, const char *version, int stream_size);		
	int deflate(void *strm, int flush);
	int deflateEnd(void *strm);
public:
	DISPATCH_CODES
	{
		API_INFLATE_INFLATERESET = 10,
			API_INFLATE_INFLATEINIT = 20,
			API_INFLATE_INFLATEINIT2 = 21,
			API_INFLATE_INFLATE = 30,
			API_INFLATE_INFLATEEND = 40,
			API_INFLATE_CRC32 = 50,

			API_INFLATE_DEFLATERESET = 60,
			API_INFLATE_DEFLATEINIT2 = 70,
			API_INFLATE_DEFLATE = 80,
			API_INFLATE_DEFLATEEND = 90,
	};
};

inline int api_inflate::inflateReset(void *strm)
{
	return _call(API_INFLATE_INFLATERESET, (int)-4, strm);
}

inline int api_inflate::inflateInit_(void *strm, const char *version, int stream_size)
{
	return _call(API_INFLATE_INFLATEINIT, (int)-4, strm, version, stream_size);
}

inline int api_inflate::inflateInit2_(void *strm, int windowBits, const char *version, int stream_size)
{
	return _call(API_INFLATE_INFLATEINIT2, (int)-4, strm, windowBits, version, stream_size);
}

inline int api_inflate::inflate(void *strm, int flush)
{
	return _call(API_INFLATE_INFLATE, (int)-4, strm, flush);
}

inline int api_inflate::inflateEnd(void *strm)
{
	return _call(API_INFLATE_INFLATEEND, (int)-2, strm);
}

inline unsigned long api_inflate::crc32(unsigned long crc, const unsigned  char *buf, unsigned int len)
{
	return _call(API_INFLATE_CRC32, (unsigned long)0, crc, buf, len);
}

inline int api_inflate::deflateReset(void *strm)
{
	return _call(API_INFLATE_DEFLATERESET, (unsigned long)-4, strm);
}

inline int api_inflate::deflateInit2_(void *strm, int level, int method, int windowBits, int memLevel, int strategy, const char *version, int stream_size)
{
	return _call(API_INFLATE_DEFLATEINIT2, (int)-4, strm, level, method, windowBits, memLevel, strategy, version, stream_size);
}

inline int api_inflate::deflateEnd(void *strm)
{
	return _call(API_INFLATE_DEFLATEEND, (int)-2, strm);
}

inline int api_inflate::deflate(void *strm, int flush)
{
	return _call(API_INFLATE_DEFLATE, (int)-4, strm, flush);
}

	// {8A4C0BAA-83D0-440e-BB59-A5C70A92EFFF}
static const GUID inflateGUID =
{ 0x8a4c0baa, 0x83d0, 0x440e, { 0xbb, 0x59, 0xa5, 0xc7, 0xa, 0x92, 0xef, 0xff } };

#endif