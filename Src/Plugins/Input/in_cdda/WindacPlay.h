#ifndef NULLSOFT_WINDACPLAYH
#define NULLSOFT_WINDACPLAYH

#include "Main.h"
#include "CDPlay.h"
#include "windac/Dac32.h"
#include "../nu/AutoLock.h"

using namespace Nullsoft::Utility;
class WindacPlay : public C_CDPlay
{
public:
	WindacPlay();
	~WindacPlay();
	int open(wchar_t drive, int track);
	int play(wchar_t drive, int track);
	static DWORD WINAPI threadProc(LPVOID lpParameter)
	{
		WindacPlay *wp = (WindacPlay *)lpParameter;
		return wp->threadProc2();
	}

	int read(char *dest, int len, int *killswitch);
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
	void setoutputtime(int time_in_ms)
	{
		need_seek = time_in_ms;
	}
	void setvolume(int _a_v, int _a_p)
	{
		line.outMod->SetVolume(_a_v);
		line.outMod->SetPan(_a_p);
	}

private:
	void getTrackInfos(int *drivenum, char driveletter);
	unsigned char *sbuf;
	long bytes_in_sbuf;
	int buf_size;
	int start, end;
	int g_nch, g_srate, g_bps;
	int killswitch;
	HANDLE hThread;
	int decode_pos_ms;
	int need_seek;

	BOOL inited;

	CMapDrive *m_pMapDrive;
	CSCSICD *scsi;
	TDriveInfo drive_info;
	CCDAdress start_sector, current_sector, end_sector;
	int slength;

	DWORD last_eject_scan;

	bool needsToClose;
};

#endif