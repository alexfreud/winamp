#include "MP4Writer.h"

#include <strsafe.h>

MP4Writer::MP4Writer()
{
	wchar_t tmppath[MAX_PATH-14] = {0};
	GetTempPathW(MAX_PATH-14,tmppath);
	GetTempFileNameW(tmppath, L"mp4", 0, tempfile);
	mp4File = MP4Create(tempfile);
	if(!mp4File)
	{
		return;
	}
}

MP4Writer::~MP4Writer()
{
	/* in case it's lingering open */
	if (mp4File)
		MP4Close(mp4File);
}

void MP4Writer::CloseTo(const wchar_t *filename)
{
	MP4Close(mp4File);
	mp4File=0;
	MP4MakeIsmaCompliant(tempfile, 0, true);
	DeleteFileW(filename);
	if (MoveFileW(tempfile,filename) == 0) // if the function fails
	{
		CopyFileW(tempfile,filename, FALSE);
		DeleteFileW(tempfile);
	}
}

void MP4Writer::WriteGaps(uint32_t pregap, uint32_t postgap, uint64_t totalSamples)
{
	char data[128] = {0};
	StringCchPrintfA(data, 128, " %08X %08X %08X %016X %08X %08X %08X %08X %08X %08X %08X %08X", 0, pregap, postgap, totalSamples, 0, 0,0, 0,0, 0,0, 0);
	MP4SetMetadataFreeForm(mp4File, "iTunSMPB", (u_int8_t *)data, lstrlenA(data));
}

void MP4Writer::Write(const void *buf, size_t size, MP4Duration duration)
{
	MP4WriteSample(mp4File, mp4Track, (const uint8_t *)buf, size, duration);
}

void MP4Writer::AddAudioTrack(const HANDLE_MPEG4ENC_ENCODER encoder, const MPEG4ENC_SETUP *setup)
{
	MPEG4ENC_INFO info;
	MPEG4ENC_GetInfo(encoder, &info);

	MP4SetTimeScale(mp4File, info.nSamplingRate[0]);

	mp4Track = MP4AddAudioTrack(mp4File, info.nSamplingRate[0], info.nSamplesFrame[0], MP4_MPEG4_AUDIO_TYPE); 
	MP4SetAudioProfileLevel(mp4File, info.nProfLev);
	MP4SetTrackESConfiguration(mp4File, mp4Track, info.ascBuf[0].ascBuffer, (info.ascBuf[0].nAscSizeBits+7)/8);
}

void MP4Writer::WriteTool(const char *tool)
{
	MP4SetMetadataTool(mp4File, tool);
}