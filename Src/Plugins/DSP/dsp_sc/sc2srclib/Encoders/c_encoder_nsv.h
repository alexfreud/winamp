/* This is an abstract class to use the NSV style of encoder.
 */

#ifndef __C_ENCODER_NSV_H__
#define __C_ENCODER_NSV_H__

#include "c_encoder.h"
#include "enc_if.h"
#include <windows.h>
#include <Shlobj.h>
#include "../../Resource/resource.h"

struct T_ENCODER_NSV_INFO {
	unsigned int output_bitRate;
	unsigned int input_numChannels;
	unsigned int input_sampleRate;
};

class C_ENCODER_NSV : public C_ENCODER {
private:
	HANDLE hMutex;
protected:
	// These are exported by enc_*.dll
    AudioCoder* (*CreateAudio3)(int nch, int srate, int bps, unsigned int srct, unsigned int *outt, char *configfile);
    int (*GetAudioTypes3)(int idx, char *desc);
    HWND (*ConfigAudio3)(HWND hwndParent, HINSTANCE hinst, unsigned int outt, char *configfile);
	void (*SetWinampHWND)(HWND hwnd);
	int (*SetConfigItem)(unsigned int outt, char *item, char *data, char *configfile);
	int (*GetConfigItem)(unsigned int outt, char *item, char *data, int len, char *configfile);
    /* We don't need the rest of the exports
    AudioCoder *(*FinishAudio3)(char *fn, AudioCoder *c);
    void (*PrepareToFinish)(const char *filename, AudioCoder *coder);
    */

    // our encoder (the AudioCoder class is defined in enc_if.h)
    AudioCoder* encoder;

    // the type of the output format
    unsigned int fourcc;

    // fill up the attribute list (using AddAttrib)
    virtual void FillAttribs()=0;

    // child classes MUST call this in their constructor
	// note: encoderNum defaults to 0 which resolves to the first encoder
	//       in most enc_* but make sure to set this correctly for others
    virtual void SetEncoder(void * CreateAudio3, void * GetAudioTypes3, void * ConfigAudio3, void * SetWinampHWND, int encoderNum=0);

    // this is used in Configure()
    virtual HINSTANCE GetEncoderInstance()=0;

	// this is used for esternal encoders so they can be correctly localised
	HWND winampWnd;

public:
	C_ENCODER_NSV(int ExtInfoSize = sizeof(T_ENCODER_NSV_INFO));
    virtual ~C_ENCODER_NSV();

    virtual void Close();
    virtual void Reset();

    virtual int Encode(const void *inputbuf, const unsigned int inputbufsize, void *outputbuf, const unsigned int outputbufsize, int *inputamtused); /* all values are in BYTES! */

    // show configuration dialog
    virtual void Configure(HWND parent,HINSTANCE hDllInstance);

    virtual bool UseNsvConfig() { return true; };

    // populate the configuration file with current settings
    virtual void FillConfFile(char * conf_file, char * section=NULL)=0;

    // read the configuration file and change current settings
    virtual void ReadConfFile(char * conf_file, char * section=NULL)=0;
};

#endif /* !__C_ENCODER_NSV_H__ */