#include <windows.h>
#include "Audio.h"
#include "main.h"
#include "api__in_cdda.h"

static int start_clock,paused;
static HWAVEIN hWaveIn;
static short data_latest[576*2];
static short data_buffers[2][576*2];
static WAVEHDR wave_hdrs[2];
static HANDLE hThread;
static DWORD WINAPI Thread(LPVOID _a);
int a_v = -666, a_p = 0, initted = 0;

int audioInit(/*int sample*/)
{
	WAVEFORMATEX wft = {0};
	DWORD threadid;
	start_clock=GetTickCount();
	paused=0;
//	if (!sample) return initted=0;
	wft.wFormatTag = WAVE_FORMAT_PCM;
	wft.nChannels = 2;
	wft.nSamplesPerSec = 44100;
	wft.nBlockAlign = 2*2;
	wft.nAvgBytesPerSec = wft.nSamplesPerSec*wft.nBlockAlign;
	wft.wBitsPerSample = 16;
	wft.cbSize = 0;
 	if (waveInOpen(&hWaveIn,WAVE_MAPPER,&wft,0,0,0) != MMSYSERR_NOERROR)
	{
		return 1;
	}
	for (int x = 0; x < 2; x ++)
	{
		memset(&wave_hdrs[x],0,sizeof(wave_hdrs[x]));
		wave_hdrs[x].lpData = (char *) data_buffers[x];
		wave_hdrs[x].dwBufferLength = 576*2*sizeof(short);
		waveInPrepareHeader(hWaveIn,&wave_hdrs[x],sizeof(wave_hdrs[0]));
		waveInAddBuffer(hWaveIn,&wave_hdrs[x],sizeof(wave_hdrs[0]));
	}
	initted=1;
	done=0;
	waveInStart(hWaveIn);
	hThread = CreateThread(NULL,0,Thread,(LPVOID)&done,0,&threadid);
	SetThreadPriority(hThread, (int)AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
	return 0;
}

void audioPause(int s)
{
	if (s) 
	{
		if (!paused)
		{
			paused=1;
			if (initted) waveInStop(hWaveIn);
			start_clock = GetTickCount()-start_clock;
		}
	}
	else
	{
		if (paused)
		{
			if (initted) waveInStart(hWaveIn);
			start_clock=GetTickCount()-start_clock;
			paused=0;
		}
	}
}

void audioQuit()
{
	if (!initted) return;
	done=1;
	WaitForSingleObject(hThread,INFINITE);
	waveInStop(hWaveIn);
	waveInReset(hWaveIn);
	while (waveInClose(hWaveIn) == WAVERR_STILLPLAYING) Sleep(10);
	CloseHandle(hThread);
}

int audioGetPos()
{
	if (paused) return start_clock;
	return GetTickCount()-start_clock;
}


void audioSetPos(int ms)
{
	start_clock = GetTickCount()-ms;
}

static DWORD WINAPI Thread(LPVOID _a)
{
	volatile int *a = (volatile int *)_a;
	int w;
	while (!*a)
	{
		Sleep(576000/44100);
		if (wave_hdrs[0].dwFlags & WHDR_DONE) w=0;
		else if (wave_hdrs[1].dwFlags & WHDR_DONE) w=1;
		else continue;
		memcpy(data_latest,wave_hdrs[w].lpData,576*2*sizeof(short));
		wave_hdrs[w].dwFlags=WHDR_PREPARED;
		waveInAddBuffer(hWaveIn,&wave_hdrs[w],sizeof(wave_hdrs[0]));
//		memset(data_latest,0,576*2*sizeof(short));
		line.VSAAddPCMData(data_latest,2,16,0);
		line.SAAddPCMData(data_latest,2,16,0);
	}
	return 0;
}
