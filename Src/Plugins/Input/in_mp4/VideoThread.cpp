#include "main.h"
#include "VideoThread.h"
#include "../Winamp/wa_ipc.h"

VideoSample *video_sample=0;
IVideoOutput *videoOutput=0;
static bool video_reopen=false;
static int height=0;
static int width=0;
static bool video_opened=false;
static int consecutive_early_frames;
HANDLE video_flush = 0, video_start_flushing=0, video_flush_done = 0, video_resume = 0;
static HANDLE video_thread = 0;
MP4SampleId nextVideoSampleId=1; // set in conjunction with video_flush
static void OpenVideo()
{
	if (!video_opened || video_reopen)
	{
		consecutive_early_frames = 0;
		if (!videoOutput)
			videoOutput = (IVideoOutput *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GET_IVIDEOOUTPUT);

		int color_format;
		double aspect_ratio=1.0;
		if (video && video->GetOutputFormat(&width, &height, &color_format, &aspect_ratio) == MP4_VIDEO_SUCCESS)
		{
			videoOutput->extended(VIDUSER_SET_THREAD_SAFE, 1, 0);
			videoOutput->open(width, height, 0, 1.0/aspect_ratio, color_format);
			video_opened = true;
			video_reopen = false;
		}
	}
}

static DecodedVideoSample *GetNextPicture()
{
	void *data, *decoder_data;
	MP4Timestamp timestamp=video_sample?(video_sample->timestamp):0;
	switch(video->GetPicture(&data, &decoder_data, &timestamp))
	{
	case MP4_VIDEO_OUTPUT_FORMAT_CHANGED:
		video_reopen=true;
		// fall through
	case MP4_VIDEO_SUCCESS:
		DecodedVideoSample *decoded = new DecodedVideoSample;
		decoded->decoder = video;
		decoded->decoder_data = decoder_data;
		decoded->timestamp = timestamp;
		decoded->output = data;
		return decoded;
	}
	return 0;
}

static void OutputPicture(DecodedVideoSample *decoded_video_sample)
{
	if (decoded_video_sample)
	{
		int outputTime = (int)((decoded_video_sample->timestamp*1000ULL)/(uint64_t)m_video_timescale);
again:
		MP4Duration realTime = GetClock();	
		int time_diff = outputTime - realTime;
		if (time_diff > 12 && consecutive_early_frames)  // plenty of time, go ahead and turn off frame dropping
		{
			if (--consecutive_early_frames == 0)
				video->HurryUp(0);
		}
		else if (time_diff < -50) // shit we're way late, start dropping frames
		{
			video->HurryUp(1);
			consecutive_early_frames += 3;
		}
		if (time_diff > 3)
		{
			HANDLE handles[] = {killEvent, video_start_flushing};
			if (WaitForMultipleObjects(2, handles, FALSE, outputTime-realTime) != WAIT_TIMEOUT)
			{
				delete decoded_video_sample;
				decoded_video_sample=0;
				return;
			}
			goto again; // TODO: handle paused state a little better than this
		}

		OpenVideo(); // open video if we havn't already

		videoOutput->draw(decoded_video_sample->output);

		delete decoded_video_sample;
		decoded_video_sample=0;
	/* TODO: probably want separate audio and video done flags
	if (temp->sampleId == numSamples) // done!
	done = true;
	*/
	}
}

static bool ReadNextVideoSample()
{
	while (nextVideoSampleId <= numVideoSamples)
	{
		VideoSample &sample=*video_sample;
		unsigned __int32 buffer_size = sample.inputSize;
		bool isSync=false;
		MP4Duration duration, offset;
		play_mp4_guard.Lock();
		bool sample_read=MP4ReadSample(MP4hFile, video_track, nextVideoSampleId++, (unsigned __int8 **)&sample.input, &buffer_size, &sample.timestamp, &duration, &offset, &isSync);
		play_mp4_guard.Unlock();
		if (sample_read)
		{
			// some buggy movies store signed int32 offsets, so let's deal with it
			offset = (uint32_t)offset;
			int32_t signed_offset = (int32_t)offset;

			sample.timestamp += signed_offset; 
			//int outputTime = (int)((sample.timestamp*1000ULL) /(uint64_t)m_video_timescale);
			sample.inputValid = buffer_size;
			return true;
		}
	}
	return false;
}

static DWORD WINAPI VideoPlayThread(LPVOID parameter)
{
	DWORD waitTime = 0;
	HANDLE handles[] = {killEvent, video_flush, video_start_flushing, video_resume};
	while (1)
	{
		int ret = WaitForMultipleObjects(4, handles, FALSE, waitTime);
		if (ret == WAIT_OBJECT_0) // kill
			break;
		else if (ret == WAIT_OBJECT_0+1)  // flush
		{
			if (video)
				video->Flush();
			ResetEvent(video_flush);
			waitTime = 0;
			SetEvent(video_flush_done);
		}
		else if (ret == WAIT_OBJECT_0+2) // start flushing
		{
			waitTime = INFINITE; // this will stop us from decoding samples for a while
			ResetEvent(video_start_flushing);
			SetEvent(video_flush_done);
		}
		else if (ret == WAIT_OBJECT_0+3) // resume playback (like flush but don't flush the decoder)
		{
			ResetEvent(video_resume);
			waitTime = 0;
			SetEvent(video_flush_done);
		}
		else if (ret == WAIT_TIMEOUT)
		{
			if (ReadNextVideoSample())
			{
				int ret = video->DecodeSample(video_sample->input, video_sample->inputValid, video_sample->timestamp);
				if (ret == MP4_VIDEO_OUTPUT_FORMAT_CHANGED)
					video_reopen=true;
				if (ret == MP4_VIDEO_AGAIN)
					nextVideoSampleId--;

				DecodedVideoSample *picture = 0;
				while (picture = GetNextPicture())
				{
					OutputPicture(picture);
				}
				waitTime = 0;
			}
			else
			{
				// TODO: tell decoder end-of-file and get any buffers in queue
				if (!audio)
					PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				waitTime = INFINITE; // out of stuff to do, wait for kill or flush
			}
		}
		else // error
			break;
	}
	if (videoOutput)
		videoOutput->close();
	return 0;
}

void Video_Init()
{
	width=0;
	height=0;
	video_reopen=false;
	video_opened=false;
	video_flush = CreateEvent(NULL, TRUE, FALSE, NULL);
	video_start_flushing = CreateEvent(NULL, TRUE, FALSE, NULL);
	video_flush_done = CreateEvent(NULL, FALSE, FALSE, NULL);
	video_resume = CreateEvent(NULL, TRUE, FALSE, NULL);	
	video_thread = CreateThread(0, 0, VideoPlayThread, 0, 0, 0);
}

void Video_Close()
{
	WaitForSingleObject(video_thread, INFINITE);
	CloseHandle(video_thread);
	video_thread = 0;
	CloseHandle(video_start_flushing);
	video_start_flushing=0;
	CloseHandle(video_flush);
	video_flush=0;
	CloseHandle(video_resume);
	video_resume=0;
	CloseHandle(video_flush_done);
	video_flush_done = 0;	
	
	delete video_sample;
	video_sample=0;
}