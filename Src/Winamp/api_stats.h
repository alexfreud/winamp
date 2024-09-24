#ifndef NULLSOFT_API_STATS_H
#define NULLSOFT_API_STATS_H

#include <bfc/dispatch.h>
/* super secret user spying code goes here */

class api_stats : public Dispatchable
{
	public:
		enum
	{
		LAUNCHES, // st1
		TIME_RUNNING, // st2
		TIME_VISIBLE,// st3
		TIME_PLAYING,// st4
		TIME_MB,// st5
		TIME_VISIBLE_PLAYING, // st6
		TIME_MB_PLAYING, // st7
		FILES_PLAYED, // st8
		CDS_PLAYED, // st9
		STREAMS_PLAYED, // st10
		VIDEOS_PLAYED, // st11
		LIBRARY_SIZE, // st12
		REGVER, // st13
		PLEDIT_LENGTH, // st14
		PLAYLIST_COUNT, // st15
		PODCAST_COUNT, // st16
		PMP_TRANSFER_COUNT, // st17
		REPLAYGAIN_COUNT, // st18
		TRANSCODE_COUNT, // st19
		TRANSCODE_FORMAT, // st20
		RIP_COUNT, // st21
		RIP_FORMAT, // st22
		AVI_AUDIO_FORMAT, // st23
		AVI_VIDEO_FOURCC, // st24
		BOOKMARK_COUNT, // st25
		PLG_COUNT, // st26
		NUM_STATS,
	};
protected:
	api_stats() {}
	~api_stats() {}
public:
	void SetStat(int stat, int value);
	void IncrementStat(int stat);
	void SetString(const char *key, const wchar_t *value);
	enum
	{
		SETSTAT = 0,
		INCREMENTSTAT = 1,
		SETSTRING = 2,
	};
};

inline void api_stats::SetStat(int stat, int value)
{
	_voidcall(SETSTAT, stat, value);
}

inline void api_stats::IncrementStat(int stat)
{
	_voidcall(INCREMENTSTAT, stat);
}

inline void api_stats::SetString(const char *key, const wchar_t *value)
{
	_voidcall(SETSTRING, key, value);
}

// {E23D9470-A095-4f02-97A1-88A8859DE0C2}
static const GUID AnonymousStatsGUID = 
{ 0xe23d9470, 0xa095, 0x4f02, { 0x97, 0xa1, 0x88, 0xa8, 0x85, 0x9d, 0xe0, 0xc2 } };

#endif // !NULLSOFT_API_STATS_H