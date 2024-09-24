#include "../nsv/nsvplay/main.h"
#include "../vp32/include/duck_dxl.h"
#include "vfw.h"

extern "C" {
  void GetImageBufs(DXL_XIMAGE_HANDLE x, YV12_PLANES *p);
};

int vp3_postprocess=0;
int vp3_targetcpu=0;

class VP3_Decoder : public IVideoDecoder {
  public:
    VP3_Decoder(int w, int h, int uvflip);
    ~VP3_Decoder();
    int decode(int need_kf, 
            void *in, int in_len, 
            void **out, // out is set to a pointer to data
            unsigned int *out_type, // 'Y','V','1','2' is currently defined
            int *is_kf);
    void flush() { }

  private:
    int m_uvflip;
    int l_tcpu, l_pp;
    static int init;
    DXL_XIMAGE_HANDLE xim;
    YV12_PLANES vidbufdec;
};

int VP3_Decoder::init;

VP3_Decoder::VP3_Decoder(int w, int h, int uvflip)
{
  l_tcpu=-1;
  l_pp=-1;
  if (!init)
  {
    init=1;
    DXL_InitVideoEx(1,1);
  }
  m_uvflip=uvflip;
  vidbufdec.y.baseAddr=0;
  xim = DXL_AlterXImage( NULL, (unsigned char *)"" ,MAKEFOURCC('V','P','3','1'), DXRGBNULL,0,0);
}

VP3_Decoder::~VP3_Decoder()
{
  if ( xim ) DXL_DestroyXImage( xim);
}


int VP3_Decoder::decode(int need_kf, 
        void *in, int in_len, 
        void **out, // out is set to a pointer to data
        unsigned int *out_type, // 'Y','V','1','2' is currently defined
        int *is_kf)
{
	bool provide_width_height = (out_type[0] == 1);
  BYTE *data=(BYTE*)in;

  if (!xim) return -1;

  out_type[0]=NSV_MAKETYPE('Y','V','1','2');
  
  if (vp3_postprocess != l_pp || vp3_targetcpu != l_tcpu)
  {
    l_pp=vp3_postprocess;
    l_tcpu=vp3_targetcpu;
    if (l_pp)
    {
      int v=l_tcpu;
      if (v < 1) v=1;
      if (v > 100) v=100;
      vp31_SetParameter(xim,1, v);
      vp31_SetParameter(xim,0, 9);
    }
		else 
    {
      vp31_SetParameter(xim,1, 0);
      vp31_SetParameter(xim,0, 0);
    }
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
    GetImageBufs(xim,&vidbufdec);
    if (m_uvflip)
    {
      YV12_PLANE tmp=vidbufdec.v;
      vidbufdec.v=vidbufdec.u;
      vidbufdec.u=tmp;
    }
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
/*
IVideoDecoder *VP3_CREATE(int w, int h, double framerate, unsigned int fmt, int *flip)
{
  if (fmt == NSV_MAKETYPE('V','P','3',' ') || fmt == NSV_MAKETYPE('V','P','3','1')) 
  {
    *flip=1;
    return new VP3_Decoder(w,h,fmt == NSV_MAKETYPE('V','P','3',' '));
  }
  return NULL;
}
*/
extern "C" {
__declspec(dllexport) IVideoDecoder *CreateVideoDecoder(int w, int h, double framerate, unsigned int fmt, int *flip)
{
  if (fmt == NSV_MAKETYPE('V','P','3',' ') || fmt == NSV_MAKETYPE('V','P','3','0') || fmt == NSV_MAKETYPE('V','P','3','1')) 
  {
    *flip=1;
    return new VP3_Decoder(w,h,fmt == NSV_MAKETYPE('V','P','3',' '));
  }
  return NULL;
}
}