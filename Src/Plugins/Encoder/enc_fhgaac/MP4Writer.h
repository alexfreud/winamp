#pragma once

#include <mp4.h>
#include "mp4FastAAClib.h"
#include "config.h"

class MP4Writer
{
public:
	MP4Writer();
	~MP4Writer();
	
	void AddAudioTrack(const HANDLE_MPEG4ENC_ENCODER encoder, const MPEG4ENC_SETUP *setup);
	void WriteGaps(uint32_t pregap, uint32_t postgap, uint64_t totalSamples);
	void WriteTool(const char *tool);
	void Write(const void *buf, size_t size, MP4Duration duration);
	void CloseTo(const wchar_t *filename);

	bool OK() { return true; }

	MP4TrackId mp4Track;
	MP4FileHandle mp4File;
	wchar_t tempfile[MAX_PATH];
};

