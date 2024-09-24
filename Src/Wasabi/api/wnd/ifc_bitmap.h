#ifndef NULLSOFT_WASABI_IFC_BITMAP_H
#define NULLSOFT_WASABI_IFC_BITMAP_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include <bfc/platform/platform.h>

#warning move this typedef to bfc/platform/platform.h
#ifdef _WIN32
typedef HBITMAP OSBITMAPHANDLE;
#elif defined(__APPLE__)
typedef CGImageRef OSBITMAPHANDLE;
#else
#error port me
#endif

class ifc_bitmap : public Dispatchable
{
protected:
  ifc_bitmap() {}
  ~ifc_bitmap() {}
public:
  OSBITMAPHANDLE GetBitmap();
  uint8_t *GetBits();
  void UpdateBits(uint8_t *bits); // call to signify that you've modified the underlying bits. 
  
  DISPATCH_CODES 
  {
    IFC_BITMAP_GETBITMAP = 10,
    IFC_BITMAP_GETBITS = 20,
    IFC_BITMAP_UPDATEBITS = 30,
  };
};


inline OSBITMAPHANDLE ifc_bitmap::GetBitmap()
{
  return _call(IFC_BITMAP_GETBITMAP, (OSBITMAPHANDLE)0);
}

inline uint8_t *ifc_bitmap::GetBits()
{
  return _call(IFC_BITMAP_GETBITS, (uint8_t *)0);
}

inline void ifc_bitmap::UpdateBits(uint8_t *bits)
{
  _voidcall(IFC_BITMAP_UPDATEBITS, bits);
}

#endif