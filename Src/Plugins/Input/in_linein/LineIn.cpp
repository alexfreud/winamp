#include "LineIn.h"
#include "main.h"
#include "audio.h"
int LineIn::Play()
{
	paused = false;
	posinms = 0;
	line.is_seekable = 0;
	line.SetInfo(44*4*8, 44, 2, 1);
	line.SAVSAInit(0, 44100);
	line.VSASetInfo(2, 44100);
	{
		short dta[576*2] = {0, };
		line.VSAAddPCMData(dta, 2, 16, 0);
		line.SAAddPCMData(dta, 2, 16, 0);
	}
	if (audioInit(1))
	{}
	return 0;
}

void LineIn::Stop()
{
	audioQuit();
}

void LineIn::Pause()
{
	posinms = audioGetPos();
	audioPause(1);
	paused = true;
}

void LineIn::Unpause()
{
	audioPause(0);
	paused = false;
}

int LineIn::GetLength()
{
	return -1000;
}

int LineIn::GetOutputTime()
{
	if (paused)
		return posinms;
	return audioGetPos();
}
