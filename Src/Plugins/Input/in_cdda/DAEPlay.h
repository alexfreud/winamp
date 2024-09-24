#ifndef NULLSOFT_DAEPLAYH
#define NULLSOFT_DAEPLAYH

#include "Main.h"
#include "CDPlay.h"
#include "../nu/AutoLock.h"
#include <map>
#include <winioctl.h>
#include "Ntddcdrm.h"

#define CD_BLOCKS_PER_SECOND 75
#define DEF_SECTORS_PER_READ 8
#define CDROM_RAW_BYTES_PER_SECTOR 2352
#define CDROM_COOKED_BYTES_PER_SECTOR 2048

using namespace Nullsoft::Utility;
class DAEPlay : public C_CDPlay
{
public:
	DAEPlay();
	~DAEPlay();
	int open(wchar_t drive, int track);
	int play(wchar_t drive, int track);

	static DWORD WINAPI threadProc(LPVOID lpParameter)
	{
		DAEPlay *wp = (DAEPlay *)lpParameter;
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

	typedef std::map<int, wchar_t*> CDTextArray;
	CDTextArray* getCDText();

	enum PacketTypes {
		CD_TEXT_TITLE,
		CD_TEXT_PERFORMER,
		CD_TEXT_SONGWRITER,
		CD_TEXT_COMPOSER,
		CD_TEXT_ARRANGER,
		CD_TEXT_MESSAGES,
		CD_TEXT_DISC_ID,
		CD_TEXT_GENRE,
		CD_TEXT_TOC_INFO,
		CD_TEXT_TOC_INFO2,
		CD_TEXT_RESERVED_1,
		CD_TEXT_RESERVED_2,
		CD_TEXT_RESERVED_3,
		CD_TEXT_RESERVED_4,
		CD_TEXT_CODE,
		CD_TEXT_SIZE,
		CD_TEXT_NUM_PACKS
	};

private:

	BYTE * data;
	CDTextArray* cd_text;
	unsigned char *sbuf;
	long bytes_in_sbuf;
	int buf_size;
	int g_track, g_nch, g_srate, g_bps;
	int killswitch;
	HANDLE hDrive, hThread;
	int need_seek;

	DWORD start_address, track_length, current_sector;

	int getTrackInfo();
	int fillBuffer(int kill);

	DWORD MSFToBlocks(UCHAR msf[4])
	{
		return (((msf[1] * (CD_BLOCKS_PER_SECOND * 60)) + (msf[2] * CD_BLOCKS_PER_SECOND) + msf[3]) - 150);
	}
};

#endif