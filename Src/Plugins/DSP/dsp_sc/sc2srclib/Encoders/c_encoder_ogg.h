#ifndef __C_ENCODER_OGG_H__
#define __C_ENCODER_OGG_H__

#include "c_encoder_nsv.h"
//#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

typedef struct 
{
  bool cfg_abr_use_max,cfg_abr_use_min;
  UINT cfg_mode;

  float cfg_vbrquality;
  UINT cfg_abr_nominal;
  UINT cfg_abr_max;
  UINT cfg_abr_min;
} configtype;

struct T_ENCODER_OGG_INFO : public T_ENCODER_NSV_INFO
{
	float output_quality;
	unsigned int output_samplerate;
	unsigned int output_channelmode;
};

#define OGG_DEFAULT_OUTPUTMODE			0
#define OGG_DEFAULT_OUTPUTBITRATE		192
#define OGG_DEFAULT_OUTPUTSAMPLERATE	44100
#define OGG_DEFAULT_OUTPUTQUALITY		2.0f

class C_ENCODER_OGG : public C_ENCODER_NSV {
private:
	HWND winamp;
protected:
	virtual void FillAttribs();
public:
    static HINSTANCE hEncoderInstance;
    C_ENCODER_OGG(HWND hwnd = 0);
    virtual ~C_ENCODER_OGG();
    static bool isPresent(HWND winamp);
    virtual void ReadConfFile(char * conf_file, char * section=NULL);
    virtual void FillConfFile(char * conf_file, char * section=NULL);
    static void Unload() { if(hEncoderInstance) FreeLibrary(hEncoderInstance); hEncoderInstance=0; }
    virtual char * GetContentType() { return "audio/ogg"; }
    virtual HINSTANCE GetEncoderInstance() { return hEncoderInstance; }
};

#endif /* !__C_ENCODER_OGG_H__ */