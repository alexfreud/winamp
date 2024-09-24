#pragma once
#include <windows.h>

namespace nu
{
class VideoClock
{
public:
	VideoClock()
	{
		video_sync_start_time=0;
		pause_start_time=0;
		length_paused=0;
		paused=0;
	}

	void Reset()
	{
		length_paused = 0;
		paused=0;
	}

	void Pause()
	{
		paused=1;
		pause_start_time = GetTickCount();
	}

	void Unpause()
	{
		paused=0;
		length_paused += (GetTickCount() - pause_start_time);
	}

	int GetOutputTime()
	{
		if (paused)
		{
			return pause_start_time - video_sync_start_time - length_paused;
		}
		else
		{
			return GetTickCount() - video_sync_start_time - length_paused;
		}
	}

	void Seek(int time_ms)
	{
		video_sync_start_time = GetTickCount() - time_ms;
		length_paused = 0;
	}

	void Start()
	{
		video_sync_start_time = GetTickCount();
	}
private:
	DWORD video_sync_start_time;
	DWORD pause_start_time;
	DWORD length_paused;
	int paused;
};
}