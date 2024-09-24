#ifndef __C_ENCODER_AACP_H__
#define __C_ENCODER_AACP_H__

#include "c_encoder_nsv.h"

struct T_ENCODER_AACP_INFO : public T_ENCODER_NSV_INFO
{
	unsigned int output_quality;
	unsigned int output_samplerate;
	unsigned int output_channelmode;
	unsigned int output_v2enable;
};

#define AACP_DEFAULT_OUTPUTCHANNELMODE    4
#define AACP_DEFAULT_OUTPUTBITRATE        48
#define AACP_DEFAULT_OUTPUTQUALITY        2
#define AACP_DEFAULT_OUTPUTSAMPLERATE     44100
#define AACP_DEFAULT_OUTPUTV2ENABLE       1

class C_ENCODER_AACP : public C_ENCODER_NSV {
private:
	HWND winamp;
protected:
	virtual void FillAttribs();
public:
    static HINSTANCE hEncoderInstance;
    C_ENCODER_AACP(HWND hwnd = 0);
    virtual ~C_ENCODER_AACP();
    static bool isPresent(HWND winamp);
    virtual void ReadConfFile(char * conf_file, char * section=NULL);
    virtual void FillConfFile(char * conf_file, char * section=NULL);
    static void Unload() { if(hEncoderInstance) FreeLibrary(hEncoderInstance); hEncoderInstance=0; }
    virtual char * GetContentType() { return "audio/aacp"; }
    virtual HINSTANCE GetEncoderInstance() { return hEncoderInstance; }
};

#endif /* !__C_ENCODER_AACP_H__ */