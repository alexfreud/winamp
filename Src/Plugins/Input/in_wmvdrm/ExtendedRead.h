#ifndef NULLSOFT_IN_WMVDRM_EXTENDEDREAD_H
#define NULLSOFT_IN_WMVDRM_EXTENDEDREAD_H

#include "AudioFormat.h"
#include "../Agave/DecodeFile/ifc_audiostream.h"
#include "main.h"

struct ExtendedReadStruct : public AudioFormat, public ifc_audiostream
{
public:
	ExtendedReadStruct();
	ExtendedReadStruct(IWMSyncReader *_reader);
	~ExtendedReadStruct();

	bool Open(const wchar_t *filename);
	bool FindOutput(int bits, int channels);
	size_t ReadAudio(void *buffer, size_t sizeBytes);

	IWMSyncReader *reader;
	WORD streamNum;

	INSSBuffer *buffer;
	size_t bufferUsed;
	bool endOfFile;
	QWORD length;
protected:
	RECVS_DISPATCH;
};

#endif