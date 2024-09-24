#ifndef NULLSOFT_WINAMP_DECODEFILE_H
#define NULLSOFT_WINAMP_DECODEFILE_H

#include "api_decodefile.h"
#include "CommonReader.h"
class DecodeFile : public api_decodefile
{
public:
	static const char *getServiceName() { return "File Decode API"; }
	static const GUID getServiceGuid() { return decodeFileGUID; }	
public:
	ifc_audiostream *OpenAudio(const wchar_t *filename, AudioParameters *parameters);
	ifc_audiostream *OpenAudioBackground(const wchar_t *filename, AudioParameters *parameters);
	void CloseAudio(ifc_audiostream *audioStream);
protected:
	RECVS_DISPATCH;
private:
	CommonReader *MakeReader(const wchar_t *filename, AudioParameters *parameters, bool useUnagi);
};

#endif