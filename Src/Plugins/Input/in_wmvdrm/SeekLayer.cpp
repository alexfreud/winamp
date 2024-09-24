#include "SeekLayer.h"
#include "Main.h"
#include "output/AudioOut.h"
#include "util.h"
#include <assert.h>
using namespace Nullsoft::Utility;

struct OpenThreadData
{
	IWMReaderCallback *callback;
	wchar_t *url;
	IWMReader *reader;

	void open()
	{
		reader->Open(url, callback, 0);
	}

	OpenThreadData(IWMReader *_reader, const wchar_t *_url, IWMReaderCallback *_callback)
	{
		reader = _reader;
		reader->AddRef();
		callback = _callback;
		callback->AddRef();

		url = _wcsdup(_url);
	}

	~OpenThreadData()
	{
		free(url);
		reader->Release();
		callback->Release();
	}
};
DWORD WINAPI OpenThread(void *param)
{
	OpenThreadData *data = (OpenThreadData *)param;
	data->open();
	delete data;
	return 0;
}

#define NEW_SEEK
SeekLayer::SeekLayer(IWMReader *_reader, ClockLayer *_clock)
		: seekPos(0), reader(_reader), playState(PLAYSTATE_CLOSED), metadata(NULL), clock(_clock),
		needPause(false), paused(false), needStop(false),
		seekGuard(GUARDNAME("Seek Guard")),
		oldState_buffer(PLAYSTATE_NONE)
{
	reader->AddRef();
	reader->QueryInterface(&reader2);
}

void SeekLayer::DoStop()
{
	if (paused)
		reader->Resume();

	reader->Stop();
	First().Stopping();
	First().Kill();
	if (paused)
	{
		paused = false;
		out->Pause(0);
	}
	needStop = false;
}

void SeekLayer::SeekTo(long position)
{
	AutoLock lock (seekGuard LOCKNAME("SeekTo"));
	if (paused)
	{
		reader->Resume();
	}
	First().Stopping();
	First().Kill();
	seekPos = position;
	clock->SetLastOutputTime(position);
	clock->SetStartTimeMilliseconds(position);
	out->Flush(position);
	QWORD qSeekPos = position;
	qSeekPos *= 10000;
	reader->Start(qSeekPos, 0, 1.0f, NULL);
	if (paused)
	{
		reader->Pause();
	}
}

void SeekLayer::Pause()
{
	AutoLock lock (seekGuard LOCKNAME("Pause"));
	if (playState == PLAYSTATE_STARTED)
	{
		paused = true;
		reader->Pause();
		out->Pause(1);
	}
	else
	{
		needPause = true;
	}
}

int SeekLayer::Open(const wchar_t *filename, IWMReaderCallback *callback)
{
	AutoLock lock (seekGuard LOCKNAME("Open"));
	assert(playState == PLAYSTATE_CLOSED);
	needStop = false;
	playState = PLAYSTATE_OPENING;
	DWORD dummyId;
	CreateThread(NULL, 0, OpenThread, new OpenThreadData(reader, filename, callback), 0, &dummyId);
	return 0;
}

void SeekLayer::Stop()
{
	AutoLock lock (seekGuard LOCKNAME("Stop"));
	needStop = true;

	switch (playState)
	{
	case PLAYSTATE_BUFFERING:
//				needStop=false;
		reader2->StopBuffering();
//		reader->Stop();
		break;

	case PLAYSTATE_OPENING:
		// wait for it to open (or connect) and then we'll kill it there
		break;

	case PLAYSTATE_NONE:
	case PLAYSTATE_STOPPED:
		if (FAILED(reader->Close())) // reader->Close() is sometimes synchronous, and sometimes not valid here
		{
			playState = PLAYSTATE_CLOSED;
			return ;
		}
		break;

	case PLAYSTATE_OPENED:
	case PLAYSTATE_STARTED:
		reader->Stop();
		First().Stopping();
		First().Kill();
		break;

	case PLAYSTATE_CLOSED:
		needStop = false;
		break;
		/*
			case PLAYSTATE_BUFFERING:
				reader2->StopBuffering();
				reader->Stop();
				break;*/

	case PLAYSTATE_SEEK:
		break;
	}

	while (playState != PLAYSTATE_CLOSED)
	{
		lock.ManualUnlock();
		Sleep(55);
		lock.ManualLock(MANUALLOCKNAME("[Manual Lock]Stop"));
	}
	needStop = false;
}

void SeekLayer::Unpause()
{
	AutoLock lock (seekGuard LOCKNAME("Unpause"));
	if (playState == PLAYSTATE_STARTED)
	{
		paused = false;
		out->Pause(0);
		reader->Resume();
		clock->Clock();
	}
	else
	{
		needPause = false;
	}
}

void SeekLayer::Opened()
{
	{
		AutoLock lock (seekGuard LOCKNAME("SeekLayer::Opened"));
		if (needStop)
		{
			playState = PLAYSTATE_OPENED;
			lock.ManualUnlock();
			reader->Close();
			lock.ManualLock(MANUALLOCKNAME("[Manual Lock]SeekLayer::Opened"));
			return ;
		}

		playState = PLAYSTATE_OPENED;
	}
	WMHandler::Opened();
}

void SeekLayer::Stopped()
{
	{
		AutoLock lock (seekGuard LOCKNAME("Stopped"));
		if (needStop)
		{
			playState = PLAYSTATE_STOPPED;
			lock.ManualUnlock();
			reader->Close();
			lock.ManualLock(MANUALLOCKNAME("Stopped"));
		}
		else
		{
			playState = PLAYSTATE_STOPPED;
		}

		WMHandler::Stopped();
	}
}

void SeekLayer::Started()
{
	{
		AutoLock lock (seekGuard LOCKNAME("Started"));
		playState = PLAYSTATE_STARTED;

		if (needStop)
		{
			reader->Stop();
			First().Stopping();
			First().Kill();
			return ;
		}
		else if (needPause)
		{
			Pause();
			needPause = false;
		}
	}
	WMHandler::Started();
}

void SeekLayer::Closed()
{
	playState = PLAYSTATE_CLOSED;
	paused = false;
	needPause = false;
	needStop = false;
	seekPos = 0;

	WMHandler::Closed();
}

void SeekLayer::BufferingStarted()
{
	{
		AutoLock lock (seekGuard LOCKNAME("BufferingStarted"));
		if (playState == PLAYSTATE_OPENED)
			oldState_buffer = PLAYSTATE_NONE;
		else
			oldState_buffer = playState;
		if (playState != PLAYSTATE_STARTED)
			playState = PLAYSTATE_BUFFERING;
		if (needStop)
			reader2->StopBuffering();

	}
	WMHandler::BufferingStarted();
}


void SeekLayer::BufferingStopped()
{
	{
		AutoLock lock (seekGuard LOCKNAME("BufferingStopped"));
		if (needStop)
			reader->Stop();
		playState = oldState_buffer;
	}
	WMHandler::BufferingStopped();
}


void SeekLayer::EndOfFile()
{
	{
		AutoLock lock (seekGuard LOCKNAME("EndOfFile"));
		if (needStop)
			return ;
	}
	WMHandler::EndOfFile();

}

void SeekLayer::Connecting()
{
	{
		AutoLock lock (seekGuard LOCKNAME("SeekLayer::Connecting"));
		if (needStop)
		{
			playState = PLAYSTATE_NONE;
			lock.ManualUnlock();
			reader->Close();
			lock.ManualLock(MANUALLOCKNAME("[Manual Lock]SeekLayer::Connecting"));
			return ;
		}
		playState = PLAYSTATE_NONE;
	}
	WMHandler::Connecting();
}


void SeekLayer::Locating()
{
	{
		AutoLock lock (seekGuard LOCKNAME("SeekLayer::Locating"));
		if (needStop)
		{
			playState = PLAYSTATE_NONE;
			lock.ManualUnlock();
			reader->Close();
			lock.ManualLock(MANUALLOCKNAME("[Manual Lock]SeekLayer::Locating"));
			return ;
		}
		playState = PLAYSTATE_NONE;
	}
	WMHandler::Locating();
}


void SeekLayer::OpenCalled()
{
	{
		AutoLock lock (seekGuard LOCKNAME("SeekLayer::OpenCalled"));
		if (needStop)
		{
			playState = PLAYSTATE_NONE;
			lock.ManualUnlock();
			reader->Close();
			lock.ManualLock(MANUALLOCKNAME("[Manual Lock]SeekLayer::OpenCalled"));
			return ;
		}

		playState = PLAYSTATE_NONE;
	}
	WMHandler::OpenCalled();
}

void SeekLayer::OpenFailed()
{
	{
		AutoLock lock (seekGuard LOCKNAME("SeekLayer::OpenFailed"));
		if (playState == PLAYSTATE_OPENING)
			playState = PLAYSTATE_NONE;
	}
	WMHandler::OpenFailed();
}

void SeekLayer::Error()
{
	{
		AutoLock lock (seekGuard LOCKNAME("SeekLayer::Error"));
		/*if (playState == PLAYSTATE_OPENING)
			playState = PLAYSTATE_CLOSED;
		else */if (playState != PLAYSTATE_CLOSED)
			playState = PLAYSTATE_NONE;
	}
	WMHandler::Error();
}
