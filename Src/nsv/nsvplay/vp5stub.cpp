#include <windows.h>
#include "../nsvlib.h"
#include "../dec_if.h"

#include "vfw.h"

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

    int m_err;

  private:
    int width,height;
	  BITMAPINFO vp5_bmo,vp5_bmi;
	  HIC vp5_hic;  
    unsigned char *vidbufdec;
};

VP5_Decoder::VP5_Decoder(int w, int h)
{
  width=w;
  height=h;
  m_err=0;
  vp5_hic=0;
  vidbufdec=(unsigned char*)malloc(sizeof(YV12_PLANES) + w*h*3/2);
    // init vp5 decode
  memset((void *) &vp5_bmi,0,sizeof(BITMAPINFO));
  memset((void *) &vp5_bmo,0,sizeof(BITMAPINFO));

  vp5_bmi.bmiHeader.biCompression = mmioFOURCC('V','P','5','0'); 
  vp5_bmi.bmiHeader.biHeight=h;
  vp5_bmi.bmiHeader.biWidth =w;

  vp5_bmo.bmiHeader.biCompression = mmioFOURCC('Y','V','1','2');
  vp5_bmo.bmiHeader.biHeight=h;
  vp5_bmo.bmiHeader.biWidth =w;
  vp5_bmo.bmiHeader.biBitCount = 12;

  vp5_hic = ICOpen(ICTYPE_VIDEO, vp5_bmi.bmiHeader.biCompression, ICMODE_DECOMPRESS);
  vp5_bmo.bmiHeader.biHeight*=-1;
  if(!vp5_hic || ICERR_OK !=ICDecompressBegin(vp5_hic, &vp5_bmi, &vp5_bmo))
  {
    m_err=1;
    return;
  }
}

VP5_Decoder::~VP5_Decoder()
{
  if (vp5_hic)
  {
    ICDecompressEnd(vp5_hic);
  	ICClose(vp5_hic);
  }
  free(vidbufdec);
}


int VP5_Decoder::decode(int need_kf, 
        void *in, int in_len, 
        void **out, // out is set to a pointer to data
        unsigned int *out_type, // 'Y','V','1','2' is currently defined
        int *is_kf)
{
  *out_type=NSV_MAKETYPE('Y','V','1','2');
  vp5_bmi.bmiHeader.biSizeImage = in_len;
  if(ICERR_OK == ICDecompress(vp5_hic,0,(BITMAPINFOHEADER *) &vp5_bmi, (char*)in,(BITMAPINFOHEADER *) &vp5_bmo, (char*)vidbufdec+sizeof(YV12_PLANES)))
  {
    *is_kf=!(!in_len || ((unsigned char *)in)[0] > 0x7f);
    if (need_kf && !*is_kf) 
    {
      return 0;
    }
    YV12_PLANES *image_vbd=(YV12_PLANES *)vidbufdec;
    image_vbd->y.baseAddr=(unsigned char *)(image_vbd+1);
    image_vbd->v.baseAddr=((unsigned char *)(image_vbd+1)) + width*height;
    image_vbd->u.baseAddr=((unsigned char *)(image_vbd+1)) + width*height*5/4;
    image_vbd->y.rowBytes=width;
    image_vbd->v.rowBytes=width/2;
    image_vbd->u.rowBytes=width/2;
    *out=(void*)vidbufdec;

    return 0;
  }

  return -1;

}

IVideoDecoder *VP5_CREATE(int w, int h, double framerate, unsigned int fmt, int *flip)
{
  if (fmt == NSV_MAKETYPE('V','P','5','0')) 
  {
    *flip=0;
    VP5_Decoder *a=new VP5_Decoder(w,h);
    if (a->m_err)
    {
      delete a;
      return NULL;
    }
    return a;
  }
  return NULL;
}
