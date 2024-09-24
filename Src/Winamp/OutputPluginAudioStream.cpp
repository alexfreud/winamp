/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "OutputPluginAudioStream.h"
#include "out.h"
#include "api.h"
#include "WinampAttributes.h"

int m_converting = 0;
static volatile int streamsInUse = 0;
static void * volatile streamBuffer = 0;
static volatile size_t streamCanWrite = 0;
static volatile HANDLE streamWait = 0;
static volatile HANDLE streamGo = 0;
static volatile HANDLE streamKill = 0;

static volatile bool streamPlaying = false;
static volatile AudioParameters *streamParameters = 0;
static In_Module * volatile streamIn = 0;
static volatile int opens = 0;

void ConvertEOF()
{
	streamPlaying = false;
	//if (--opens==0)
	SetEvent(streamWait);
}

static int StreamOpen(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms)
{
	streamParameters->bitsPerSample = bitspersamp;
	streamParameters->channels = numchannels;
	streamParameters->sampleRate = samplerate;
	streamParameters->sizeBytes = (size_t) - 1;

	// we will try to use GetFileInfo to get a length
	int lengthMS;
	InW_GetFileInfo(streamIn, 0, 0, &lengthMS);
	if (lengthMS > 0)
	{
		streamParameters->sizeBytes = MulDiv(lengthMS, numchannels * bitspersamp * samplerate, 8000);
	}

	streamPlaying = true;

	return 0;
}

static void StreamClose()
{
	streamPlaying = false;
	// SetEvent(streamWait);
}

static int StreamWrite(char *buf, int len)
{

again:
	// null buffer means EOF
	if (buf == NULL)
	{
		streamPlaying = false;
		//SetEvent(streamWait);
		return 0;
	}

	if (streamCanWrite == 0)
	{
		//Sleep(10); // input plugin isn't checking StreamCanWrite() properly, so we'll sleep for them
		//return 1;
	}

	// copy into user-supplied buffer
	int toCopy = min((int)streamCanWrite, len);
	memcpy(streamBuffer, buf, toCopy);
	streamCanWrite -= toCopy;
	streamBuffer = ((char *)streamBuffer) + toCopy;

	// the input plugin may have given us too much data, so we'll have to check that
	// increment the user's stuff
	len -= toCopy;
	buf += toCopy;
	if (len) // len>0 implies streamCanWrite == 0
	{
		ResetEvent(streamGo);
		SetEvent(streamWait);

		/* benski> this Sleep() code causes a major slowdown,
		             probably because of high thread priorities in some plugins
		while (streamCanWrite == 0) 
			Sleep(100);
			*/
		HANDLE events[2] = {streamKill, streamGo};
		switch (WaitForMultipleObjects(2, events, FALSE, INFINITE))
		{
		case WAIT_OBJECT_0 + 1:
			goto again;
		default:
			return 0;
		}
	}

	// signal event to let ReadAudio() return, if the buffer is full
	if (streamCanWrite == 0)
		SetEvent(streamWait);
	return 0;
}


static int StreamCanWrite()
{
	if (streamCanWrite)
		return 65536;
	else
		return 0;
}

static int StreamIsPlaying()
{
	return 0;
}

static void StreamSet(int nothing)
{}

static int StreamGetOutputTime()
{
	return 0;
}

static int StreamGetWrittenTime()
{
	return 0;
}

static Out_Module streamOut =
    {
        OUT_VER,
        "dummy output",
        14000,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        StreamOpen,
        StreamClose,
        StreamWrite,
        StreamCanWrite,
        StreamIsPlaying,
        NULL,    //pause
        StreamSet,    //setvolume
        StreamSet,    //setpan
        StreamSet,    //flush
        StreamGetOutputTime,
        StreamGetWrittenTime,
    };

OutputPluginAudioStream::OutputPluginAudioStream()
{
	oldBits=config_audio_bits;
	oldSurround=config_audio_surround;
	oldMono=config_audio_mono;
	oldRG=config_replaygain;
}

bool OutputPluginAudioStream::Open(In_Module *in, const wchar_t *filename, AudioParameters *parameters)
{
	// set some globals, since winamp output plugins don't have user data/context pointers
	opens++;
	m_converting = 1;

	// this will ask the input plugin to produce our output format (not a guarantee, though)
	config_audio_bits = parameters->bitsPerSample;
	config_audio_surround = (parameters->channels > 2);
	config_audio_mono = (parameters->channels == 1);
	config_replaygain=false;

	streamWait = CreateEvent(NULL, FALSE, FALSE, NULL);
	streamGo = CreateEvent(NULL, FALSE, FALSE, NULL);
	streamKill = CreateEvent(NULL, TRUE, FALSE, NULL);
	streamCanWrite = 0;
	streamBuffer = 0;
	streamPlaying = false;
	streamParameters = parameters;
	streamIn = in;

	in->outMod = &streamOut;

	int ret = InW_Play(in, filename);
	if (ret)
	{
		parameters->errorCode = API_DECODEFILE_FAILURE;
		opens--;
		return false;
	}

	if (in->UsesOutputPlug&IN_MODULE_FLAG_USES_OUTPUT_PLUGIN)
	{
		int cnt = 5000;
		while (!streamPlaying && cnt > 0)
		{
			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, FALSE))
				WASABI_API_APP->app_messageLoopStep();
			else
			{
				Sleep(1);
				cnt--;
			}
		}
	}
	else
	{
		parameters->errorCode = API_DECODEFILE_NO_INTERFACE;
		opens--;
		return false;
	}

	if (!streamPlaying)
	{
		parameters->errorCode = API_DECODEFILE_FAILURE;
		opens--;
		return false;
	}

	return true;
}

size_t OutputPluginAudioStream::ReadAudio(void *buffer, size_t sizeBytes)
{
	streamBuffer = buffer;
	streamCanWrite = sizeBytes;
	SetEvent(streamGo);
	HANDLE events[2] = {streamKill, streamWait};
	switch (WaitForMultipleObjects(2, events, FALSE, INFINITE))
	{
	case WAIT_OBJECT_0 + 1:  // streamWait, which gets triggered when buffer is full or output is done
		return sizeBytes - streamCanWrite; // streamCanWrite will be >0 if there was a partial write, e.g. on EOF
	default:
		return 0; // no point
	}

	return sizeBytes - streamCanWrite;
}

OutputPluginAudioStream::~OutputPluginAudioStream()
{
	SetEvent(streamKill);
	streamIn->Stop();
	DeleteObject(streamWait);
	DeleteObject(streamGo);
	DeleteObject(streamKill);
	streamWait = 0;
	config_audio_bits = oldBits;
	config_audio_surround = oldSurround;
	config_audio_mono=oldMono;
	config_replaygain=oldRG;
}

#define CBCLASS OutputPluginAudioStream
START_DISPATCH;
CB(IFC_AUDIOSTREAM_READAUDIO, ReadAudio)
END_DISPATCH;
