#ifndef __WASABI_IMGLOADERENUM_H
#define __WASABI_IMGLOADERENUM_H

#include <api/service/svc_enum.h>
#include <bfc/string/StringW.h>

class ImgLoaderEnum : public SvcEnumT<svc_imageLoader> {
public:
  ImgLoaderEnum(uint8_t *data, int datalen) : mem(datalen, data) { }
  ImgLoaderEnum(const wchar_t *filename) : fname(filename) { }

protected:
  virtual int testService(svc_imageLoader *svc) 
	{
    if (!fname.isempty() && !svc->isMine(fname)) return 0;
    return svc->testData(mem, mem.getSizeInBytes());
  }

private:
  StringW fname;
  MemBlock<uint8_t> mem;
};

#endif