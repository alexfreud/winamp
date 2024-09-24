#include "main.h"
#include "Process.h"

int ProcessReplayGain::Open(int _mode)
{
	mode=_mode;
	if (mode != RG_INDIVIDUAL_TRACKS
		&& mode != RG_ALBUM)
		return RG_MODE_NOT_SUPPORTED;
	context=CreateRG();
	if (!context)
		return RG_FAILURE;

	StartRG(context);
	return RG_SUCCESS;
}

int ProcessReplayGain::ProcessTrack(const wchar_t *filename)
{
	int killSwitch=0;
	RGWorkFile workFile(filename);
		
	CalculateRG(context, workFile.filename, workFile.track_gain, workFile.track_peak, 0, &killSwitch, albumPeak);
	queue.push_back(workFile);

	return RG_SUCCESS;
}

int ProcessReplayGain::Write()
{
	if (mode == RG_ALBUM)
	{
		wchar_t album_gain[64]=L"", album_peak[64]=L"";
		CalculateAlbumRG(context, album_gain, album_peak, albumPeak);
		CopyAlbumData(queue, album_gain, album_peak);
	}
	WriteAlbum(queue);

	return RG_SUCCESS;
}

void ProcessReplayGain::Close()
{
	DestroyRG(context);
}

#define CBCLASS ProcessReplayGain
START_DISPATCH;
CB(OBJ_REPLAYGAIN_OPEN, Open)
CB(OBJ_REPLAYGAIN_PROCESSTRACK, ProcessTrack)
CB(OBJ_REPLAYGAIN_WRITE, Write)
VCB(OBJ_REPLAYGAIN_CLOSE,Close)
END_DISPATCH;
