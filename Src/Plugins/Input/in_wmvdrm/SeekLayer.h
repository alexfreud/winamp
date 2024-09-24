#ifndef NULLSOFT_SEEKLAYERH
#define NULLSOFT_SEEKLAYERH

#include "WMHandler.h"
#include "../nu/AutoLock.h"
#include "ClockLayer.h"
class SeekLayer : public WMHandler
{
	enum PlayState
	{
		PLAYSTATE_NONE,
		PLAYSTATE_OPENING,
		PLAYSTATE_OPENED,
		PLAYSTATE_BUFFERING,
		PLAYSTATE_STARTED,
		PLAYSTATE_STOPPED,
		PLAYSTATE_CLOSED,
		PLAYSTATE_SEEK,

	};
public:
	SeekLayer(IWMReader *_reader, ClockLayer *_clock);
	void SeekTo(long position);
	void Pause();
	void Unpause();
	void Stop();
	int Open(const wchar_t *filename, IWMReaderCallback *callback);

private:
	void BufferingStarted();
	void BufferingStopped();
	void Started();
	void Stopped();
	void Closed();
	void Opened();
	void OpenCalled();
	void Connecting();
	void Locating();
	void EndOfFile();
	void OpenFailed();
	void Error();

private:
	void DoStop();
	bool needPause, paused, needStop;
	long seekPos;
	Nullsoft::Utility::LockGuard seekGuard;
	IWMReader *reader;
	IWMReaderAdvanced2 *reader2;
	IWMMetadataEditor *metadata;
	ClockLayer *clock;
	PlayState playState, oldState_buffer;
};

#endif
