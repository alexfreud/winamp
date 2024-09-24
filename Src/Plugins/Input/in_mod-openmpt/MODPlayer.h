#pragma once
#include <bfc/platform/types.h>
#include <libopenmpt/libopenmpt.h>
#include "../nu/AudioOutput.h"

class MODPlayer
{
public:
	MODPlayer(const wchar_t *_filename);
	~MODPlayer();
	DWORD CALLBACK ThreadFunction();

	void Kill();
	void Seek(int seek_pos);
	int GetOutputTime() const;
private:
	enum
	{
		// values that you can return from OnXXXX()
		MOD_CONTINUE = 0, // continue processing
		MOD_ABORT = 1, // abort parsing gracefully (maybe 'stop' was pressed)
		MOD_STOP = 2, // stop parsing completely - usually returned when mkv version is too new or codecs not supported

		// values returned from errors within the Step() function itself
		MOD_EOF = 3, // end of file
		MOD_ERROR = 4, // parsing error
	};

	HANDLE killswitch, seek_event;
	wchar_t *filename;
	volatile int m_needseek;

	/* AudioOutput implementation */
	class MODWait
	{
	public:
		void Wait_SetEvents(HANDLE killswitch, HANDLE seek_event);
	protected:
		int WaitOrAbort(int time_in_ms);
	private:
		HANDLE handles[2];
	};
	nu::AudioOutput<MODWait> audio_output;
};