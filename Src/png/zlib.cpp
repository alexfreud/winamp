#include "api.h"

extern "C"
{

extern "C" {
  int inflateReset(void *strm)
  {
    if (WASABI_API_INFLATE) return WASABI_API_INFLATE->inflateReset(strm);
    return -4;
  }
  int inflateInit_(void *strm,const char *version, int stream_size)
  {
    if (WASABI_API_INFLATE) return WASABI_API_INFLATE->inflateInit_(strm,version,stream_size);
    return -4;
  }
  int inflate(void *strm, int flush)
  {
    if (WASABI_API_INFLATE) return WASABI_API_INFLATE->inflate(strm,flush);
    return -4;

  }
  int inflateEnd(void *strm)
  {
    if (WASABI_API_INFLATE) return WASABI_API_INFLATE->inflateEnd(strm);
    return -2;
  }
  unsigned long crc32(unsigned long crc, const unsigned  char *buf, unsigned int len)
  {
    if (WASABI_API_INFLATE) return WASABI_API_INFLATE->crc32(crc,buf,len);
    return 0;
  }
	
	int deflateReset(void * strm)
	{
		if (WASABI_API_INFLATE) return WASABI_API_INFLATE->deflateReset(strm);
		return -4;
	}

	int deflateInit2_(void * strm, int level, int method, int windowBits, int memLevel, int strategy, const char *version, int stream_size)
	{
	if (WASABI_API_INFLATE) return WASABI_API_INFLATE->deflateInit2_(strm, level, method, windowBits, memLevel, strategy, version, stream_size);
			return -4;
	}

	int deflate(void * strm, int flush)
	{
		if (WASABI_API_INFLATE) return WASABI_API_INFLATE->deflate(strm, flush);
			return -2;
	}

	int deflateEnd(void * strm)
	{
		if (WASABI_API_INFLATE) return WASABI_API_INFLATE->deflateEnd(strm);
			return -4;
	}
};

}