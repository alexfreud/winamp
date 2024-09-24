#ifndef NULLSOFT_VERITASPLAYH
#define NULLSOFT_VERITASPLAYH

#include "CDPlay.h"
#include <windows.h>
#include "SpeedLimiter.h"
#include "main.h"

using namespace Nullsoft::Utility;
class VeritasBuffer
{
public:
	VeritasBuffer() : buffer(0), readSize(0), sector(0), offset(0)
	{
	}
	~VeritasBuffer()
	{
		Destroy();
	}
	void Create(int numBuffers)
	{
		buffer = new BYTE[numBuffers * 2352];
		Clear();
	}
	void Destroy()
	{
		if (buffer)
			delete buffer;
		buffer = 0;
		Clear();
	}
	void Clear()
	{
		readSize = 0;
		sector = 0;
	}
	BYTE *buffer;
	BYTE *internal;
	DWORD readSize;
	DWORD sector;
	int offset;
};

class VeritasPlay : public C_CDPlay
{
public:
	VeritasPlay(bool _ripping = false);
	~VeritasPlay();
	//void Delete() 	{if (primo)		delete primo;		primo = 0;	}
	void CreateBuffers();
	void DestroyBuffers();
	int open(char drive, int track); //called by winampGetExtendedRead
	int play(char drive, int track); //called by winamp2
	int openVeritasTrack(DWORD start, DWORD length);

	static DWORD WINAPI threadProc(LPVOID lpParameter)
	{
		VeritasPlay *wp = (VeritasPlay *)lpParameter;
		return wp->threadProc2();
	}

	void OutputBuffer(VeritasBuffer &buffer);
	int read(char *dest, int len, int *killswitch);
	void Abort();
	void Seek();
	void SeekAndFlush();
	void Output(char *buffer, int len);
	void OutputOverflow();
	int CopyOverflow(char *sample_buffer, int len);

	size_t CopyBuffer(VeritasBuffer &buffer, char *&sample_buffer, int &bytes);
	int threadProc2();
	void stop();
	void pause()
	{
		line.outMod->Pause(1);
	}
	void unpause()
	{
		line.outMod->Pause(0);
	}
	int getlength()
	{
		return g_playlength;
	}
	int getoutputtime()
	{
		return line.outMod->GetOutputTime();
	}
	void setoutputtime(int time_in_ms);
	void setvolume(int a_v, int a_p);
	
	void *submitHandle;
private:
	void Close();
	void AbortAsync();
	
	void WaitForAbort(int time);
	int killswitch;
	HANDLE hThread;
	int decode_pos_ms;
	int need_seek;
	
	DWORD unit, start_sector, end_sector, lastseek, sec_length;
	int end;
	int total_extract_len;
	DWORD extract_start_time;
	BYTE *overflowBuffer;
	long overflow;
	VeritasBuffer *buffers;
	int buf_size, currentBuffer, nb_veritas_buf;
	int g_nch;
	bool ripping, opened;
	SpeedLimiter speedLimiter;
	bool padStart, padEnd;

	#ifndef IGNORE_PRIMO
	obj_primo *primo;
	#endif
};

#endif
