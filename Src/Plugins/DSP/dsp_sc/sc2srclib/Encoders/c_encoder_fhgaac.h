#ifndef __C_ENCODER_FHGAAC_H__
#define __C_ENCODER_FHGAAC_H__

#include "c_encoder_nsv.h"

struct T_ENCODER_FHGAAC_INFO : public T_ENCODER_NSV_INFO
{
	unsigned int output_profile;
	unsigned int output_surround;
};

#define FHGAAC_DEFAULT_OUTPUTBITRATE        48
#define FHGAAC_DEFAULT_OUTPUTPROFILE        0	// automatic
#define FHGAAC_DEFAULT_OUTPUTSURROUND       0

class C_ENCODER_FHGAAC : public C_ENCODER_NSV {
private:
	HWND winamp;
protected:
	virtual void FillAttribs();
public:
    static HINSTANCE hEncoderInstance;
    C_ENCODER_FHGAAC(HWND hwnd = 0);
    virtual ~C_ENCODER_FHGAAC();
    static bool isPresent(HWND winamp);
    virtual void ReadConfFile(char * conf_file, char * section=NULL);
    virtual void FillConfFile(char * conf_file, char * section=NULL);
    static void Unload() { if(hEncoderInstance) FreeLibrary(hEncoderInstance); hEncoderInstance=0; }
    virtual char * GetContentType() { return "audio/aacp"; }
    virtual HINSTANCE GetEncoderInstance() { return hEncoderInstance; }
};

#endif /* !__C_ENCODER_AACP_H__ */