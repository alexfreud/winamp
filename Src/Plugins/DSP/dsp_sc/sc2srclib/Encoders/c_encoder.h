#ifndef __C_ENCODER_H__
#define __C_ENCODER_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vector>

#define C_ENCODER_NameLen 1024

struct T_EncoderIOVals {
	unsigned int output_bitRate;
};

struct T_ATTRIB {
	char Text[256];
void *OutputVals;
};

class C_ENCODER {
private:
	char Name[C_ENCODER_NameLen];
    std::vector<T_ATTRIB*> AttribList;
protected:
    T_EncoderIOVals *ExtendedInfoPtr;
    int ExtendedInfoSize;
    void SetName(const char *name);
    void ClearAttribs();
    void AddAttrib(const char *Text, const void *Attributes);
public:
    C_ENCODER(int ExtInfoSize = 0);
    virtual ~C_ENCODER();

    virtual void ChangeSettings(const void *Settings);
    virtual void Create(const T_EncoderIOVals *Settings, const char *name = NULL);
    virtual void Close();
    virtual void Reset();
    virtual char *GetName();
    virtual void *GetExtInfo(int *ExtInfoSize = NULL) {
		if(ExtInfoSize != NULL) *ExtInfoSize = ExtendedInfoSize;
		return ExtendedInfoPtr;
    }

    virtual char * GetContentType();
    virtual int Encode(const void *inputbuf, const unsigned int inputbufsize, void *outputbuf, const unsigned int outputbufsize, int *inputamtused); /* all values are in BYTES! */

    virtual int GetNumAttribs();
    virtual int EnumAttrib(const int val, T_ATTRIB *buf);
    
    virtual bool UseNsvConfig() { return false; }
};

#endif /* !__C_ENCODER_H__ */