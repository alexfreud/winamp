#ifndef NULLSOFT_WINAMP_EXTENDEDREADER_H
#define NULLSOFT_WINAMP_EXTENDEDREADER_H

#include <stddef.h>
#include "../Agave/DecodeFile/ifc_audiostream.h"
#include "../Agave/DecodeFile/api_decodefile.h"
#include "CommonReader.h"

typedef intptr_t (__cdecl *OpenFunc)(const char *filename, int *size, int *bps, int *nch, int *srate);
typedef intptr_t (__cdecl *OpenWFunc)(const wchar_t *filename, int *size, int *bps, int *nch, int *srate);
typedef size_t (__cdecl *GetDataFunc)(intptr_t handle, void *buffer, size_t bufferBytes, int *killswitch);
typedef void (__cdecl *CloseFunc)(intptr_t);
typedef int (__cdecl *SetTimeFunc)(intptr_t handle, int millisecs);

class ExtendedReader :  public CommonReader
{
public:
	ExtendedReader(OpenFunc _open, OpenWFunc _openW, OpenFunc _openFloat, OpenWFunc _openWFloat, GetDataFunc _getData, CloseFunc _close, SetTimeFunc _setTime = 0);
	~ExtendedReader();
	bool Open(const wchar_t *filename, AudioParameters *parameters);
	size_t ReadAudio(void *buffer, size_t sizeBytes);
	size_t ReadAudio_kill(void *buffer, size_t sizeBytes, int *killswitch, int *error);
	BOOL SeekToTimeMs(int millisecs);
	int CanSeek();

	OpenFunc open, openFloat;
	OpenWFunc openW, openWFloat;
	GetDataFunc getData;
	CloseFunc close;
	SetTimeFunc setTime;
	intptr_t handle;
protected:
	RECVS_DISPATCH;
};


#endif