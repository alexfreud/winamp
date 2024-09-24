/*
** nsvdec_vp6: vp6stub.cpp - VP6 decompressor
** 
** The VP3 portions that this plug-in links with are Copyright (C) On2, Inc.
** For information on the license of VP3, please see: http://www.vp3.com/
** (or vp32\vp3_license.txt if you got all files with this)
** 
** Copyright (C) 2001-2002 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty.  
** In no event will the authors be held liable for any damages arising from the use of this software.
**
** Permission is granted to anyone to use this software for any purpose, including commercial 
** applications, and to alter it and redistribute it freely, subject to the following restrictions:
**  1. The origin of this software must not be misrepresented; you must not claim that you wrote the 
**     original software. If you use this software in a product, an acknowledgment in the product 
**     documentation would be appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be misrepresented as 
**     being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**
*/
#ifdef _WIN32
#include <windows.h>
#endif
#include <bfc/platform/export.h>
#include "../nsv/nsvlib.h"
#include "../nsv/dec_if.h"
#include "duck_dxl.h"
#include <stddef.h>
extern "C" {
  void GetImageBufs(DXL_XIMAGE_HANDLE x, YV12_PLANES *p);
  void vp60_SetParameter ( DXL_XIMAGE_HANDLE src, int Command, uintptr_t Parameter );

};

int vp6_postProcess=6;
int vp6_cpuFree=70;
int vp6_deInterlace=0;
int vp6_addNoise=1;

typedef enum
{
	PBC_SET_POSTPROC,
	PBC_SET_CPUFREE,
    PBC_MAX_PARAM,
	PBC_SET_TESTMODE,
	PBC_SET_PBSTRUCT,
	PBC_SET_BLACKCLAMP,
	PBC_SET_WHITECLAMP,
	PBC_SET_REFERENCEFRAME,
    PBC_SET_DEINTERLACEMODE,
    PBC_SET_ADDNOISE

} PB_COMMAND_TYPE;

extern "C"
{
    int vp60_Init(void);
    int vp60_Exit(void);
}

class VP6_Decoder : public IVideoDecoder {
  public:
    VP6_Decoder(int w, int h);
    ~VP6_Decoder();
    int decode(int need_kf, 
            void *in, int in_len, 
            void **out, // out is set to a pointer to data
            unsigned int *out_type, // 'Y','V','1','2' is currently defined
            int *is_kf);
    void flush() { }

    void initMmx();

  private:
    int l_tcpu, l_pp;
    static int init;
    static int mmx_available;
    DXL_XIMAGE_HANDLE xim;
    YV12_PLANES vidbufdec;
};

int VP6_Decoder::init;
int VP6_Decoder::mmx_available;

VP6_Decoder::VP6_Decoder(int w, int h)
{
  l_tcpu=-1;
  l_pp=-1;
  if (!init)
  {
    init=1;
    DXL_InitVideo(0,0);
    vp60_Init();
    initMmx();
  }
  vidbufdec.y.baseAddr=0;
  xim = DXL_AlterXImage( NULL, (unsigned char *)"" ,NSV_MAKETYPE('V','P','6','0'), DXRGBNULL,w,h);
}

void VP6_Decoder::initMmx()
{
#ifdef _M_IX86
  unsigned __int32 retval1,retval2;	
  __try { 		
    _asm {
      mov eax, 1  // set up CPUID to return processor version and features
        // 0 = vendor string, 1 = version info, 2 = cache info
        _emit 0x0f  // code bytes = 0fh,  0a2h
        _emit 0xa2
        mov retval1, eax		
        mov retval2, edx
    }	
  } __except(EXCEPTION_EXECUTE_HANDLER) { retval1 = retval2 = 0;}
  mmx_available = retval1 && (retval2&0x800000);
#else
	mmx_available=0;
#endif
}

VP6_Decoder::~VP6_Decoder()
{
  if ( xim ) DXL_DestroyXImage( xim);
}

int VP6_Decoder::decode(int need_kf, 
        void *in, int in_len, 
        void **out, // out is set to a pointer to data
        unsigned int *out_type, // 'Y','V','1','2' is currently defined
        int *is_kf)
{
  unsigned char *data=(unsigned char *)in;

  if (!xim) return -1;

  *out_type=NSV_MAKETYPE('Y','V','1','2');
  
  if (vp6_postProcess != l_pp || vp6_cpuFree != l_tcpu)
  {
    l_pp=vp6_postProcess;
    l_tcpu=vp6_cpuFree;
		if(vp6_cpuFree)
			DXL_SetParameter(xim, PBC_SET_CPUFREE, vp6_cpuFree);
		else
			DXL_SetParameter(xim, PBC_SET_POSTPROC, vp6_postProcess);

		DXL_SetParameter(xim, PBC_SET_DEINTERLACEMODE, vp6_deInterlace );
		DXL_SetParameter(xim, PBC_SET_ADDNOISE, vp6_addNoise);

	  DXL_SetParameter(xim, PBC_SET_BLACKCLAMP,0);
		DXL_SetParameter(xim, PBC_SET_WHITECLAMP,0);
  }

  DXL_AlterXImageData( xim, data);
  DXL_SetXImageCSize(xim, in_len);

  *is_kf=!(!in_len || data[0] > 0x7f);

  *out=NULL;

  if ((need_kf && !*is_kf) || !in_len) 
  {
    return 0;
  }

	if (!DXL_dxImageToVScreen( xim, NULL))
  {
#ifdef _M_IX86
    if(mmx_available)
    {
      _asm { 
        emms; 
      };
    }
#endif
    GetImageBufs(xim,&vidbufdec);
    *out=&vidbufdec;
    return 0;
  }

  return -1;
}

#ifndef ACTIVEX_CONTROL
extern "C" {
DLLEXPORT IVideoDecoder *CreateVideoDecoder(int w, int h, double framerate, unsigned int fmt, int *flip)
{
  if (fmt == NSV_MAKETYPE('V','P','6','0') || fmt == NSV_MAKETYPE('V','P','6','1') || fmt == NSV_MAKETYPE('V','P','6','2')) 
  {
    *flip=1;
    return new VP6_Decoder(w,h);
  }
  return NULL;
}
}
#else
IVideoDecoder *VP6_CREATE(int w, int h, double framerate, unsigned int fmt, int *flip)
{
  if (fmt == NSV_MAKETYPE('V','P','6','0') || fmt == NSV_MAKETYPE('V','P','6','1') || fmt == NSV_MAKETYPE('V','P','6','2')) 
  {
    *flip=1;
    return new VP6_Decoder(w,h);
  }
  return NULL;
}
#endif