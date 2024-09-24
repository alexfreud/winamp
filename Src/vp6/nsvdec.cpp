#include "nsvdec.h"

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
int vp60_getWH(DXL_XIMAGE_HANDLE src, int *w, int *h);
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


VP6_Decoder::VP6_Decoder(int w, int h)
{
  l_tcpu=-1;
  l_pp=-1;

  vidbufdec.y.baseAddr=0;
  xim = DXL_AlterXImage( NULL, (unsigned char *)"" ,NSV_MAKETYPE('V','P','6','0'), DXRGBNULL,0,0);
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
	bool provide_width_height = (out_type[0] == 1);
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
      _asm { 
        emms; 
      };
#endif
    GetImageBufs(xim,&vidbufdec);
    *out=&vidbufdec;
		if (provide_width_height)
		{
			int w, h;
			vp60_getWH(xim, &w, &h);
			out_type[1] = w;
			out_type[2] = h;
		}
    return 0;
  }

  return -1;
}
