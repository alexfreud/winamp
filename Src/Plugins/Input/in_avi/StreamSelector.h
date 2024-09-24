#pragma once
#include "../Winamp/wa_ipc.h"
#include <bfc/platform/types.h>
class Streams : public ITrackSelector
{
public:
	void Reset();
	void AddAudioStream(int stream_num);
	void AddVideoStream(int stream_num);

	void SetAudioStream(int stream_num);
	void SetVideoStream(int stream_num);

	uint16_t audio_streams[256];
	int num_audio_streams;
	int current_audio_stream;
	uint16_t video_streams[256];
	int num_video_streams;
	int current_video_stream;

	/* ITrackSelector interface */
	int getNumAudioTracks();
	void enumAudioTrackName(int n, char *buf, int size);
	int getCurAudioTrack();

	int getNumVideoTracks();
	void enumVideoTrackName(int n, char *buf, int size);
	int getCurVideoTrack();

	void setAudioTrack(int n);
	void setVideoTrack(int n);
};
