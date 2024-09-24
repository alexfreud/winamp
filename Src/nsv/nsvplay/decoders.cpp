#include <windows.h>
#include "api.h"
#include "main.h"
#include "vfw.h"
#include <api/service/services.h>
#include "../nsv/svc_nsvFactory.h"
#include <api/service/waservicefactory.h>
#include "../../Winamp/in2.h"
extern In_Module mod;

// you should probably override these in your project settings

// builtin decoders
//#define BUILTIN_MP3_SUPPORT
//#define BUILTIN_VP3_SUPPORT
//#define BUILTIN_DIVX_SUPPORT
//#define BUILTIN_PCM_SUPPORT
//#define BUILTIN_VFW_SUPPORT

// support dll decoders?
//#define DLL_DECODER_SUPPORT

//#define DLL_DECODER_SUPPORT_NOCURDIR

#ifdef WINAMP_PLUGIN
#  ifndef DLL_DECODER_SUPPORT
#    define DLL_DECODER_SUPPORT
#  endif
#  ifndef DLL_DECODER_SUPPORT_NOCURDIR
#    define DLL_DECODER_SUPPORT_NOCURDIR
#  endif
#  ifndef DLL_DECODER_SUPPORT_IN_
#    define DLL_DECODER_SUPPORT_IN_
#  endif
#  ifndef BUILTIN_PCM_SUPPORT
#    define BUILTIN_PCM_SUPPORT
#  endif
#endif

#ifdef BUILTIN_VP3_SUPPORT
#include "vp3stub.h"
#endif
#ifdef BUILTIN_VP5_SUPPORT
#include "vp5stub.h"
#endif
#ifdef BUILTIN_MP3_SUPPORT
#include "mp3stub.h"
#endif

#ifdef BUILTIN_VFW_SUPPORT

class Gen_Decoder : public IVideoDecoder {
  public:
    Gen_Decoder(int w, int h);
    ~Gen_Decoder();
    int decode(int need_kf, 
            void *in, int in_len, 
            void **out, // out is set to a pointer to data
            unsigned int *out_type, // 'Y','V','1','2' is currently defined
            int *is_kf);
    void flush() { }

    int m_err;

    int width,height;
	  BITMAPINFO gen_bmo,gen_bmi;
	  HIC gen_hic;  
    unsigned char *vidbufdec;
};

Gen_Decoder::Gen_Decoder(int w, int h)
{
  width=w;
  height=h;
  m_err=0;
  gen_hic=0;
  vidbufdec=(unsigned char*)malloc(sizeof(YV12_PLANES) + w*h*3/2);
}

Gen_Decoder::~Gen_Decoder()
{
  if (gen_hic)
  {
    ICDecompressEnd(gen_hic);
  	ICClose(gen_hic);
  }
  free(vidbufdec);
}


int Gen_Decoder::decode(int need_kf, 
        void *in, int in_len, 
        void **out, // out is set to a pointer to data
        unsigned int *out_type, // 'Y','V','1','2' is currently defined
        int *is_kf)
{
  *out_type=NSV_MAKETYPE('Y','V','1','2');
  gen_bmi.bmiHeader.biSizeImage = in_len;
  if(ICERR_OK == ICDecompress(gen_hic,0,(BITMAPINFOHEADER *) &gen_bmi, (char*)in,(BITMAPINFOHEADER *) &gen_bmo, (char*)vidbufdec+sizeof(YV12_PLANES)))
  {
    //*is_kf=!(!in_len || ((unsigned char *)in)[0] > 0x7f);
    *is_kf=1;

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


static IVideoDecoder *createVfw(int w, int h, double framerate, unsigned int type, int *flip)
{
  HIC gen_hic = ICOpen(ICTYPE_VIDEO, type, ICMODE_DECOMPRESS);

  if (!gen_hic) return 0;

  BITMAPINFO gen_bmo={0,},gen_bmi={0,};
  gen_bmi.bmiHeader.biSize=sizeof(gen_bmi.bmiHeader);
  gen_bmi.bmiHeader.biCompression = type; 
  gen_bmi.bmiHeader.biHeight=h;
  gen_bmi.bmiHeader.biWidth =w;
  gen_bmi.bmiHeader.biPlanes=1;

  gen_bmo.bmiHeader.biSize=sizeof(gen_bmo.bmiHeader);
  gen_bmo.bmiHeader.biCompression = mmioFOURCC('Y','V','1','2');
  gen_bmo.bmiHeader.biHeight=h;
  gen_bmo.bmiHeader.biWidth =w;
  gen_bmo.bmiHeader.biSizeImage=(w*h*3)/2;
  gen_bmo.bmiHeader.biPlanes=1;
  gen_bmo.bmiHeader.biBitCount=12;


  if (ICERR_OK !=ICDecompressBegin(gen_hic, &gen_bmi, &gen_bmo))
  {
    ICClose(gen_hic);
    return 0;
  }
  Gen_Decoder *t=new Gen_Decoder(w,h);
  t->gen_bmi=gen_bmi;
  t->gen_bmo=gen_bmo;
  t->gen_hic=gen_hic;

  return t;
}

#endif


#ifdef BUILTIN_DIVX_SUPPORT
#include "../../divx5/decore.h"

class CrapDivxDecoder : public IVideoDecoder {
  public:
    CrapDivxDecoder(int w, int h)
    {
      predict_keyframes=1;
	    divx_param.x_dim = w;
	    divx_param.y_dim = h;
	    divx_param.output_format = DEC_USER;
      divx_param.codec_version = 412; // indicates that the stream is DivX 4.12 compatible
      divx_param.build_number = 0; // in this case, the build field is ignored
	    divx_param.time_incr = 15; // time_incr default value
	          
	    g_decore((long) this, DEC_OPT_MEMORY_REQS, &divx_param, &decMemReqs);

      // the application allocates the data structures and the buffers
      divx_param.buffers.mp4_edged_ref_buffers = malloc(decMemReqs.mp4_edged_ref_buffers_size);
      divx_param.buffers.mp4_edged_for_buffers = malloc(decMemReqs.mp4_edged_for_buffers_size);
      divx_param.buffers.mp4_edged_back_buffers = malloc(decMemReqs.mp4_edged_back_buffers_size);
      divx_param.buffers.mp4_display_buffers = malloc(decMemReqs.mp4_display_buffers_size);
      divx_param.buffers.mp4_state = malloc(decMemReqs.mp4_state_size);
      divx_param.buffers.mp4_tables = malloc(decMemReqs.mp4_tables_size);
      divx_param.buffers.mp4_stream = malloc(decMemReqs.mp4_stream_size);
      divx_param.buffers.mp4_reference = malloc(decMemReqs.mp4_reference_size);

      memset(divx_param.buffers.mp4_state, 0, decMemReqs.mp4_state_size);
      memset(divx_param.buffers.mp4_tables, 0, decMemReqs.mp4_tables_size);
      memset(divx_param.buffers.mp4_stream, 0, decMemReqs.mp4_stream_size);
      memset(divx_param.buffers.mp4_reference, 0, decMemReqs.mp4_reference_size);

      g_decore((long) this, DEC_OPT_INIT, &divx_param, NULL);
    }
    ~CrapDivxDecoder()
    {
      if (g_decore) 
      {
        g_decore((long) this,DEC_OPT_RELEASE,NULL,NULL);
        free(divx_param.buffers.mp4_display_buffers);
        free(divx_param.buffers.mp4_edged_for_buffers);
        free(divx_param.buffers.mp4_edged_back_buffers);
        free(divx_param.buffers.mp4_edged_ref_buffers);
        free(divx_param.buffers.mp4_reference);
        free(divx_param.buffers.mp4_state);
        free(divx_param.buffers.mp4_stream);
        free(divx_param.buffers.mp4_tables);
      }
      if (!--divx_cnt)
      {
        FreeModule(hDivxLib);
        hDivxLib=0;
        g_decore=0;
      }
    }
    int decode(int need_kf, 
            void *in, int in_len, 
            void **out, // out is set to a pointer to data
            unsigned int *out_type, // 'Y','V','1','2' is currently defined
            int *is_kf)
    {
      *out_type=NSV_MAKETYPE('Y','V','1','2');
      *out=NULL;
      int kfpredict=0;
      if (predict_keyframes && in_len>3) 
      {
        kfpredict=!((unsigned char *)in)[3];
        if (need_kf && !kfpredict) return 0;
      }
      if (!in_len) return 0;
      *is_kf=kfpredict;

      DEC_PICTURE pic;
      DEC_FRAME decFrame;

	    decFrame.bitstream = in;
	    decFrame.bmp = &pic;
	    decFrame.length = in_len;
	    decFrame.render_flag = 1;

      DEC_FRAME_INFO fi;

      if (g_decore((long) this, DEC_OPT_FRAME, &decFrame, &fi) == DEC_OK)
      {
        if (!kfpredict != !fi.intra) predict_keyframes=0;
        *is_kf=fi.intra;
        if (need_kf && !fi.intra) return 0;

        image_vbd.y.baseAddr=(unsigned char *)pic.y;
        image_vbd.u.baseAddr=(unsigned char *)pic.u;
        image_vbd.v.baseAddr=(unsigned char *)pic.v;
        image_vbd.y.rowBytes=pic.stride_y;
        image_vbd.u.rowBytes=pic.stride_uv;
        image_vbd.v.rowBytes=pic.stride_uv;

        *out=&image_vbd;
        return 0;
      }

      return -1;
    }

    void flush() { }

    
    static int (STDCALL *g_decore)(
			    unsigned long handle,	// handle	- the handle of the calling entity, must be unique
			    unsigned long dec_opt, // dec_opt - the option for docoding, see below
			    void *param1,	// param1	- the parameter 1 (it's actually meaning depends on dec_opt
			    void *param2);	// param2	- the parameter 2 (it's actually meaning depends on dec_opt
    static HINSTANCE hDivxLib;
    static int divx_cnt;

  private:
    DEC_PARAM divx_param;
    YV12_PLANES image_vbd;
    DEC_MEM_REQS decMemReqs;
    int predict_keyframes;
};

int (STDCALL *CrapDivxDecoder::g_decore)(
			  unsigned long handle,	// handle	- the handle of the calling entity, must be unique
			  unsigned long dec_opt, // dec_opt - the option for docoding, see below
			  void *param1,	// param1	- the parameter 1 (it's actually meaning depends on dec_opt
			  void *param2)=0;	// param2	- the parameter 2 (it's actually meaning depends on dec_opt
HINSTANCE CrapDivxDecoder::hDivxLib=0;
int CrapDivxDecoder::divx_cnt=0;

IVideoDecoder *DIVX_CREATE(int w, int h, double framerate, unsigned int fmt, int *flip)
{
  if (fmt == NSV_MAKETYPE('D','i','v','X'))
  {
    if (!CrapDivxDecoder::divx_cnt)
    {
      CrapDivxDecoder::hDivxLib=LoadLibrary("divx.dll");
      if (CrapDivxDecoder::hDivxLib) *((void**)&CrapDivxDecoder::g_decore)=GetProcAddress(CrapDivxDecoder::hDivxLib,"decore");
    }
    CrapDivxDecoder::divx_cnt++;
    if (CrapDivxDecoder::g_decore) return new CrapDivxDecoder(w,h);
  }
  return NULL;
}

#endif // end of divx gayness

class NullVideoDecoder : public IVideoDecoder 
{
  public:
    NullVideoDecoder() { }
    ~NullVideoDecoder() { }
    int decode(int need_kf, 
            void *in, int in_len, 
            void **out, // out is set to a pointer to data
            unsigned int *out_type, // 'Y','V','1','2' is currently defined
            int *is_kf)
    {
      *out_type=NSV_MAKETYPE('Y','V','1','2');
      *is_kf=1;
      *out=NULL;
      return 0;
    }
    void flush() { }
};


class NullAudioDecoder : public IAudioDecoder
{
  public:
    NullAudioDecoder(){}
    ~NullAudioDecoder(){}
    int decode(void *in, int in_len, 
                       void *out, int *out_len,
                       unsigned int out_fmt[8])
    {
      *out_len=0;
      out_fmt[0]=NSV_MAKETYPE('N','O','N','E'); // no output
      return 0;
    }
    void flush(){}
};

#ifdef BUILTIN_PCM_SUPPORT
class PCMAudioDecoder : public IAudioDecoder
{
  public:
    PCMAudioDecoder() { fused=4; }
    ~PCMAudioDecoder(){}
    int decode(void *in, int in_len,
                       void *out, int *out_len,
                       unsigned int out_fmt[8])
    {
      if (in_len < 4)
      {
        *out_len=0;
        out_fmt[0]=0;
        return 0; // screw this frame
      }
      unsigned char *t=(unsigned char *)in;
      int bps=t[0];
      int nch=t[1];
      int srate=((int)t[2] | (((int)t[3])<<8));

      out_fmt[0]=NSV_MAKETYPE('P','C','M',' ');
      out_fmt[1]=srate;
      out_fmt[2]=nch;
      out_fmt[3]=bps;

      int l=in_len-fused;
      if (l > *out_len) l = *out_len;
      l&=~(nch*(bps/8)-1);

      if (l) memcpy(out,(char *)in + fused,l);
      fused+=l;
      *out_len=l;

      if (fused >= in_len) 
      {
        fused=4;
        return 0;
      }
      return 1;
    }
    void flush() { fused=4; }
  private:
    int fused;
};
#endif

#ifdef DLL_DECODER_SUPPORT
static char DLL_Dir[MAX_PATH];
static HINSTANCE DLL_Handles[512];
#endif


void Decoders_Init(char *wapluginspath)
{
#ifdef DLL_DECODER_SUPPORT
	HKEY hKey;

  if (!DLL_Dir[0] && RegOpenKeyExA(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion",
    0,KEY_READ,&hKey) == ERROR_SUCCESS)
  {
		DWORD l = sizeof(DLL_Dir);
		DWORD t;
    if (RegQueryValueExA(hKey,"CommonFilesDir",NULL,&t,(LPBYTE)DLL_Dir,&l ) != ERROR_SUCCESS || t != REG_SZ) DLL_Dir[0]=0;
    DLL_Dir[sizeof(DLL_Dir)-5]=0;
    CreateDirectoryA(DLL_Dir,NULL);
    strcat(DLL_Dir,"\\NSV");
    CreateDirectoryA(DLL_Dir,NULL);
    RegCloseKey(hKey);
  }

  if (!DLL_Dir[0]) GetTempPathA(sizeof(DLL_Dir),DLL_Dir);
  Decoders_Quit();

  HANDLE h;
  int x=0;
  WIN32_FIND_DATAA fd = {0};
  char buf[MAX_PATH*2+1] = {0};

#ifndef DLL_DECODER_SUPPORT_NOCURDIR
  char curdir[MAX_PATH] = {0};

  strcpy( curdir, ".\\" );

  strcpy( buf, curdir );
  strcat( buf, "nsvdec_*.dll" );

  OutputDebugString( buf ); OutputDebugString( "\n" );

  h = FindFirstFile(buf,&fd);
  if (h != INVALID_HANDLE_VALUE) 
  {
    do
    {
	  strcpy(buf,curdir);
      strcat(buf,fd.cFileName);

      DLL_Handles[x]=LoadLibrary(buf);
      if (DLL_Handles[x])
      {
        if (GetProcAddress(DLL_Handles[x],"CreateVideoDecoder") ||
            GetProcAddress(DLL_Handles[x],"CreateAudioDecoder")) x++;
        else
        {
          FreeLibrary(DLL_Handles[x]);
          DLL_Handles[x]=0;
        }
      }
    } while (x < sizeof(DLL_Handles)/sizeof(DLL_Handles[0]) && FindNextFile(h,&fd));
    FindClose(h);
  }
#endif

#ifdef DLL_DECODER_SUPPORT_IN_

  if (wapluginspath && wapluginspath[0])
  {
    lstrcpynA(buf,wapluginspath,sizeof(buf)-16);
    strcat(buf,"\\in_*.dll");
    h = FindFirstFileA(buf,&fd);
    if (h != INVALID_HANDLE_VALUE) 
    {
      do
      {
        strncpy(buf, wapluginspath, MAX_PATH);
        strncat(buf, "\\", MAX_PATH);
        strncat(buf, fd.cFileName, MAX_PATH);

        DLL_Handles[x]=LoadLibraryA(buf);
        if (DLL_Handles[x])
        {
          if (GetProcAddress(DLL_Handles[x],"CreateVideoDecoder") ||
              GetProcAddress(DLL_Handles[x],"CreateAudioDecoder")) x++;
          else
          {
            FreeLibrary(DLL_Handles[x]);
            DLL_Handles[x]=0;
          }
        }
      } while (x < sizeof(DLL_Handles)/sizeof(DLL_Handles[0]) && FindNextFileA(h,&fd));
      FindClose(h);
    }
    lstrcpynA(buf,wapluginspath,sizeof(buf)-16);
    strcat(buf,"\\nsvdec_*.dll");
    h = FindFirstFileA(buf,&fd);
    if (h != INVALID_HANDLE_VALUE) 
    {
      do
      {
        strncpy(buf, wapluginspath, MAX_PATH);
        strncat(buf, "\\", MAX_PATH);
        strncat(buf, fd.cFileName, MAX_PATH);

        DLL_Handles[x]=LoadLibraryA(buf);
        if (DLL_Handles[x])
        {
          if (GetProcAddress(DLL_Handles[x],"CreateVideoDecoder") ||
              GetProcAddress(DLL_Handles[x],"CreateAudioDecoder")) x++;
          else
          {
            FreeLibrary(DLL_Handles[x]);
            DLL_Handles[x]=0;
          }
        }
      } while (x < sizeof(DLL_Handles)/sizeof(DLL_Handles[0]) && FindNextFileA(h,&fd));
      FindClose(h);
    }
  }
#endif

#ifndef WINAMPX
  strncpy(buf, DLL_Dir, MAX_PATH);
  strncat(buf, "\\nsvdec_*.dll", MAX_PATH);
  h = FindFirstFileA(buf,&fd);
  if (h != INVALID_HANDLE_VALUE) 
  {
    do
    {
      strncpy(buf, DLL_Dir, MAX_PATH);
      strncat(buf, "\\", MAX_PATH);
      strncat(buf, fd.cFileName, MAX_PATH);

      DLL_Handles[x]=LoadLibraryA(buf);
      if (DLL_Handles[x])
      {
        if (GetProcAddress(DLL_Handles[x],"CreateVideoDecoder") ||
            GetProcAddress(DLL_Handles[x],"CreateAudioDecoder")) x++;
        else
        {
          FreeLibrary(DLL_Handles[x]);
          DLL_Handles[x]=0;
        }
      }
    } while (x < sizeof(DLL_Handles)/sizeof(DLL_Handles[0]) && FindNextFileA(h,&fd));
    FindClose(h);
  }
#endif

#endif
}


void Decoders_Quit()
{
#ifdef DLL_DECODER_SUPPORT
  int x;
  for (x = 0; x < sizeof(DLL_Handles)/sizeof(DLL_Handles[0]) && DLL_Handles[x]; x ++)
  {
    FreeLibrary(DLL_Handles[x]);
    DLL_Handles[x]=0;
  }
#endif
}

static IAudioDecoder *CreateAudioDecoderWasabi(unsigned int type, IAudioOutput **output)
{
  int n = 0;
  waServiceFactory *sf = 0;
  while (sf = mod.service->service_enumService(WaSvc::NSVFACTORY, n++))
  {
    svc_nsvFactory *factory = (svc_nsvFactory *)sf->getInterface();
    if (factory)
    {
      IAudioDecoder *decoder = factory->CreateAudioDecoder(type, output);
      sf->releaseInterface(factory);
      if (decoder)
        return decoder;
    }
  }
  return 0;
}

static IVideoDecoder *CreateVideoDecoderWasabi(int w, int h, double framerate, unsigned int type, int *flip)
{
  int n=0;
  waServiceFactory *sf = 0;
  while (sf = mod.service->service_enumService(WaSvc::NSVFACTORY, n++))
  {
    svc_nsvFactory *factory = (svc_nsvFactory *)sf->getInterface();
    if (factory)
    {
      IVideoDecoder *decoder = factory->CreateVideoDecoder(w, h, framerate, type, flip);
      sf->releaseInterface(factory);
      if (decoder)
        return decoder;
    }
  }
  return 0;
}

IAudioDecoder *CreateAudioDecoder(unsigned int type, int *wasNotNull, IAudioOutput **output)
{
  IAudioDecoder *a=NULL;
	if (mod.service && !a)
		a = CreateAudioDecoderWasabi(type, output);
#ifdef BUILTIN_MP3_SUPPORT
  if (!a) a=MP3_CREATE(type);
#endif
#ifdef BUILTIN_PCM_SUPPORT
  if (!a && type == NSV_MAKETYPE('P','C','M',' ')) a=new PCMAudioDecoder;
#endif
#ifdef BUILTIN_AAC_SUPPORT
  extern IAudioDecoder *AAC_CREATE(unsigned int fmt, IAudioOutput **output);
  if (!a && (type == NSV_MAKETYPE('A','A','C',' ') || type == NSV_MAKETYPE('V','L','B',' '))) a=AAC_CREATE(type,NULL);
#endif
#ifdef BUILTIN_AACP_SUPPOT
	extern IAudioDecoder *AACP_CREATE(unsigned int fmt, IAudioOutput **output);
  if (!a && (type == NSV_MAKETYPE('A','A','C','P') || type == NSV_MAKETYPE('A','A','C',' '))) a=AAC_CREATE(type,NULL);
#endif
#ifdef DLL_DECODER_SUPPORT
  int x;
  for (x = 0; !a && x < sizeof(DLL_Handles)/sizeof(DLL_Handles[0]) && DLL_Handles[x]; x ++)
  {
    IAudioDecoder *(*cad)(unsigned int type, IAudioOutput **output);
    *((void**)&cad) = (void*)GetProcAddress(DLL_Handles[x],"CreateAudioDecoder");
    if (cad) a=cad(type,output);
  }
#endif

  if (!a)
  {
    *wasNotNull=0;
		void *mem = WASABI_API_MEMMGR->sysMalloc(sizeof(NullAudioDecoder));
		a = new (mem) NullAudioDecoder();
  }
  else *wasNotNull=1;
  return a;
}

IVideoDecoder *CreateVideoDecoder(int w, int h, double framerate, unsigned int type, int *flip, int *wasNotNull)
{
  IVideoDecoder *v=NULL;
	if (mod.service && !v)
		v = CreateVideoDecoderWasabi(w, h, framerate, type, flip);
#ifdef BUILTIN_DIVX_SUPPORT
  if (!v) v=DIVX_CREATE(w,h,framerate,type,flip);
#endif
#ifdef BUILTIN_VP3_SUPPORT
  if (!v) v=VP3_CREATE(w,h,framerate,type,flip);
#endif
#ifdef BUILTIN_VP5_SUPPORT
  if (!v) v=VP5_CREATE(w,h,framerate,type,flip);
#endif
#ifdef BUILTIN_VP6_SUPPORT
  extern IVideoDecoder *VP6_CREATE(int w, int h, double framerate, unsigned int fmt, int *flip);
  if (!v) v=VP6_CREATE(w,h,framerate,type,flip);
#endif
#ifdef DLL_DECODER_SUPPORT
  int x;
  for (x = 0; !v && x < sizeof(DLL_Handles)/sizeof(DLL_Handles[0]) && DLL_Handles[x]; x ++)
  {
    IVideoDecoder *(*cvd)(int w, int h, double framerate, unsigned int type, int *flip);
    *((void**)&cvd) = (void*)GetProcAddress(DLL_Handles[x],"CreateVideoDecoder");
    if (cvd) v=cvd(w,h,framerate,type,flip);
  }
#endif
#ifdef BUILTIN_VFW_SUPPORT
  if (!v)
  {
    v=createVfw(w,h,framerate,type,flip);
  }
#endif
  if (!v) 
  {
    if (wasNotNull) *wasNotNull=0;
		void *mem = WASABI_API_MEMMGR->sysMalloc(sizeof(NullVideoDecoder));
		v = new (mem) NullVideoDecoder();
  }
  else if (wasNotNull) *wasNotNull=1;

  return v;
}