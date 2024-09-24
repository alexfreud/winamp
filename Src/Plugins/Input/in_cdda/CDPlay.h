#ifndef NULLSOFT_CDPLAYH
#define NULLSOFT_CDPLAYH

class C_CDPlay
{
public:
	virtual ~C_CDPlay() { }

	virtual int play(wchar_t drive, int track)=0;
	virtual void stop()=0;
	virtual void pause()=0;
	virtual void unpause()=0;
	virtual int getlength()=0;
	virtual int getoutputtime()=0;
	virtual void setoutputtime(int time_in_ms)=0;
	virtual void setvolume(int _a_v, int _a_p)=0;

	//called by winampGetExtendedRead_*
	virtual int open(wchar_t drive, int track) { return 1; }
	virtual int read(char *dest, int len, int *killswitch) { return 0; }

	// queries if this is the currently playing drive & track, passing track == 0 just means check drive
	bool IsPlaying(wchar_t drive, int track=0)
	{
		if (drive == g_drive && (!track))
			return true;

		return false;
	}

	wchar_t g_drive;
};

#endif