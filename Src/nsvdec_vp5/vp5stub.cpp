/*
** nsvdec_vp5: vp5stub.cpp - VP5 decompressor
** 
** The VP5 portions that this plug-in links with are Copyright (C) On2, Inc.
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

#include <windows.h>
#include "../nsv/nsvlib.h"
#include "../nsv/dec_if.h"
#include "../libvp6/include/duck_dxl.h"
#include <stddef.h>
extern "C" {
  void GetImageBufs(DXL_XIMAGE_HANDLE x, YV12_PLANES *p);
  void vp50_SetParameter ( DXL_XIMAGE_HANDLE src, int Command, uintptr_t Parameter );
};

int vp5_postProcess=6;
int vp5_cpuFree=70;
int vp5_deInterlace=0;

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
  PBC_SET_DEINTERLACEMODE

} PB_COMMAND_TYPE;

extern "C"
{
    int vp50_Init(void);
    int vp50_Exit(void);
}

class VP5_Decoder : public IVideoDecoder {
  public:
    VP5_Decoder(int w, int h);
    ~VP5_Decoder();
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

int VP5_Decoder::init;
int VP5_Decoder::mmx_available;

VP5_Decoder::VP5_Decoder(int w, int h)
{
  l_tcpu=-1;
  l_pp=-1;
  if (!init)
  {
    init=1;
    DXL_InitVideo(64,64);
    vp50_Init();
    initMmx();
  }
  vidbufdec.y.baseAddr=0;
  xim = DXL_AlterXImage( NULL, (unsigned char *)"" ,MAKEFOURCC('V','P','5','0'), DXRGBNULL,0,0);
}

void VP5_Decoder::initMmx()
{
#ifdef _M_IX86
  DWORD retval1,retval2;	
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
mmx_available =0;
#endif
}

VP5_Decoder::~VP5_Decoder()
{
  if ( xim ) DXL_DestroyXImage( xim);
}

int VP5_Decoder::decode(int need_kf, 
        void *in, int in_len, 
        void **out, // out is set to a pointer to data
        unsigned int *out_type, // 'Y','V','1','2' is currently defined
        int *is_kf)
{
	bool provide_width_height = (out_type[0] == 1);
  BYTE *data=(BYTE*)in;

  if (!xim) return -1;

	 out_type[0]=NSV_MAKETYPE('Y','V','1','2');
  
  if (vp5_postProcess != l_pp || vp5_cpuFree != l_tcpu)
  {
    l_pp=vp5_postProcess;
    l_tcpu=vp5_cpuFree;
		if(vp5_cpuFree)
			DXL_SetParameter(xim, PBC_SET_CPUFREE, vp5_cpuFree);
		else
			DXL_SetParameter(xim, PBC_SET_POSTPROC, vp5_postProcess);

		DXL_SetParameter(xim, PBC_SET_DEINTERLACEMODE, vp5_deInterlace );

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
    if(mmx_available)
    {
			#ifdef _M_IX86
      _asm { 
        emms; 
      };
			#endif
    }
    GetImageBufs(xim,&vidbufdec);
    *out=&vidbufdec;
		if (provide_width_height)
		{
			int x, y, w, h;
			DXL_GetXImageXYWH(xim, &x, &y, &w, &h);
			out_type[1] = w;
			out_type[2] = h;
		}
    return 0;
  }

  return -1;
}

#ifndef ACTIVEX_CONTROL
extern "C" {
__declspec(dllexport) IVideoDecoder *CreateVideoDecoder(int w, int h, double framerate, unsigned int fmt, int *flip)
{
  if (fmt == NSV_MAKETYPE('V','P','5','0')) 
  {
    *flip=1;
    return new VP5_Decoder(w,h);
  }
  return NULL;
}
}
#else
IVideoDecoder *VP5_CREATE(int w, int h, double framerate, unsigned int fmt, int *flip)
{
  if (fmt == NSV_MAKETYPE('V','P','5','0')) 
  {
    *flip=1;
    return new VP5_Decoder(w,h);
  }
  return NULL;
}
#endif