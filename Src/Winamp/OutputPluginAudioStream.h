#ifndef NULLSOFT_WINAMP_OUTPUTPLUGINAUDIOSTREAM_H
#define NULLSOFT_WINAMP_OUTPUTPLUGINAUDIOSTREAM_H

#include "../Agave/DecodeFile/ifc_audiostream.h"
#include "in2.h"
#include "../Agave/DecodeFile/api_decodefile.h"
#include "CommonReader.h"

class OutputPluginAudioStream : public CommonReader
{
public:
	OutputPluginAudioStream();
	~OutputPluginAudioStream();
	bool Open(In_Module *in, const wchar_t *filename, AudioParameters *parameters);
	size_t ReadAudio(void *buffer, size_t sizeBytes);
	
protected:
	RECVS_DISPATCH;

	size_t oldBits;
	bool oldSurround, oldMono, oldRG;
};

#endif