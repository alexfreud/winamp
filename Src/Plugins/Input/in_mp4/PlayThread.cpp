#include "main.h"
#include "../winamp/wa_ipc.h"
#include "VideoThread.h"
#include "AudioSample.h"
#include "api__in_mp4.h"
#include <assert.h>
#include <api/service/waservicefactory.h>
#include "../nu/AudioOutput.h"
#include <strsafe.h>

const DWORD PAUSE_TIMEOUT = 100; // number of milliseconds to sleep for when paused

static bool audio_opened;
static HANDLE events[3];
static bool done;
bool open_mp4(const wchar_t *fn);
static AudioSample *sample = 0;
static DWORD waitTime;
static MP4SampleId nextSampleId;
Nullsoft::Utility::LockGuard play_mp4_guard;

static MP4Duration first_timestamp=0;

class MP4Wait
{
public:
	int WaitOrAbort(int time_in_ms)
	{	
		WaitForMultipleObjects(3, events, FALSE, INFINITE); // pauseEvent signal state is opposite of pause state
		int ret = WaitForMultipleObjects(2, events, FALSE, time_in_ms);
		if (ret == WAIT_TIMEOUT)
			return 0;
		if (ret == WAIT_OBJECT_0+1)
			return 2;
		return 1;
	}
};

nu::AudioOutput<MP4Wait> audio_output(&mod);

MP4Duration GetClock()
{
	if (audio)
	{
		return audio_output.GetFirstTimestamp() + mod.outMod->GetOutputTime();
	}
	else if (video)
	{
		return video_clock.GetOutputTime();
	}
	else
	{
		return 0;
	}
}

static void OutputSample(AudioSample *sample)
{
	if (first)
	{
		first_timestamp = MP4ConvertFromTrackTimestamp(MP4hFile, audio_track, sample->timestamp, MP4_MSECS_TIME_SCALE);
		first = false;
	}

	if (sample->result == MP4_SUCCESS)
	{
		if (!audio_opened)
		{
			audio_opened=true;

			if (audio_output.Open(first_timestamp, sample->numChannels, sample->sampleRate, sample->bitsPerSample) == false)
			{
				PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				return ;
			}
			unsigned __int32 pregap = 0;
			unsigned __int32 postgap = 0;
			GetGaps(MP4hFile, pregap, postgap);
			audio_output.SetDelays(0, pregap, postgap);
			mod.SetInfo(audio_bitrate + video_bitrate, sample->sampleRate / 1000, sample->numChannels, 1);
		}

		int skip = 0;
		int sample_size = (sample->bitsPerSample / 8) * sample->numChannels;
		int outSamples = MulDiv(sample->outputValid, m_timescale, sample->sampleRate * sample_size);
		/* if (!audio_chunk && outSamples > sample->duration)
			outSamples = (int)sample->duration; */

		if (sample->offset > 0)
		{
			int cut = (int)min(outSamples, sample->offset);
			outSamples -= cut;
			skip = cut;
		}

		size_t outSize = MulDiv(sample_size * sample->sampleRate, outSamples, m_timescale);

		if (audio_bitrate != sample->bitrate)
		{
			audio_bitrate = sample->bitrate;
			mod.SetInfo(audio_bitrate + video_bitrate, -1, -1, 1);
		}

		if (audio_output.Write(sample->output + MulDiv(sample_size * sample->sampleRate, skip, m_timescale), outSize) == 1)
		{
			return ;
		}

		if (sample->sampleId == numSamples) // done!
			done = true; // TODO: probably don't want to bail out yet if video is playing
	}
}

static bool DecodeAudioSample(AudioSample *sample)
{
	if (m_needseek != -1)
	{
		sample->outputValid = 0;
		sample->outputCursor = sample->output;
		sample->result = MP4_SUCCESS;
		sample->sampleRate = m_timescale;
		audio->GetOutputProperties(&sample->sampleRate, &sample->numChannels, &sample->bitsPerSample);
		if (audio->GetCurrentBitrate(&sample->bitrate) != MP4_SUCCESS || !sample->bitrate)
			sample->bitrate = audio_bitrate;
	}
	else
	{
		sample->outputValid = sample->outputSize;
		sample->outputCursor = 0;
		sample->result = audio->DecodeSample(sample->input, sample->inputValid, sample->output, &sample->outputValid);
		if (sample->inputValid == 0 && sample->outputValid == 0) {
			return false;
		}
		sample->sampleRate = m_timescale;
		audio->GetOutputProperties(&sample->sampleRate, &sample->numChannels, &sample->bitsPerSample);
		if (audio->GetCurrentBitrate(&sample->bitrate) != MP4_SUCCESS || !sample->bitrate)
			sample->bitrate = audio_bitrate;
		OutputSample(sample);
	}
	return true;
}

static void ReadNextAudioSample()
{
	if (nextSampleId > numSamples)
	{
		return;
	}

	unsigned __int32 buffer_size = sample->inputSize;

	bool sample_read = false;
	play_mp4_guard.Lock();
	if (audio_chunk)
		sample_read = MP4ReadChunk(MP4hFile, audio_track, nextSampleId++, (unsigned __int8 **)&sample->input, &buffer_size, &sample->timestamp, &sample->duration);
	else
		sample_read = MP4ReadSample(MP4hFile, audio_track, nextSampleId++, (unsigned __int8 **)&sample->input, &buffer_size, &sample->timestamp, &sample->duration, &sample->offset);
	play_mp4_guard.Unlock();
	if (sample_read)
	{
		sample->inputValid = buffer_size;

		if (audio_chunk)
		{
			sample->duration = 0;
			sample->offset = 0;
		}
		sample->sampleId = nextSampleId-1;

		DecodeAudioSample(sample);
	}
}

static bool BuildAudioBuffers()
{
	size_t outputFrameSize;
	//if (audio->OutputFrameSize(&outputFrameSize) != MP4_SUCCESS	    || !outputFrameSize)
	//{
	outputFrameSize = 8192 * 6; // fallback size
	//}

	u_int32_t maxSize = 0;
	if (audio)
	{
		if (audio_chunk)
			maxSize = 65536; // TODO!!!!
		else
			maxSize = MP4GetTrackMaxSampleSize(MP4hFile, audio_track);
		if (!maxSize)
			return 0;

		sample = new AudioSample(maxSize, outputFrameSize);
		if (!sample->OK())
		{
			delete sample;
			return false;
		}
	}

	if (video)
	{
		maxSize = MP4GetTrackMaxSampleSize(MP4hFile, video_track);

		video_sample = new VideoSample(maxSize);
		if (!video_sample->OK())
		{
			delete video_sample;
			return false;
		}
	}

	return true;
}

DWORD WINAPI PlayProc(LPVOID lpParameter)
{
	// set an event when we start.  this keeps Windows from queueing an APC before the thread proc even starts (evil, evil windows)
	HANDLE threadCreatedEvent = (HANDLE)lpParameter;
	SetEvent(threadCreatedEvent);

	video=0;
	if (!open_mp4(lastfn))
	{
		if (WaitForSingleObject(killEvent, 200) != WAIT_OBJECT_0)
			PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
		return 0;
	}

	audio_output.Init(mod.outMod);

	if (videoOutput && video)
	{
		// TODO: this is really just a placeholder, we should do smarter stuff
		// like query the decoder object for a name rather than guess
		char set_info[256] = {0};
		char *audio_info = MP4PrintAudioInfo(MP4hFile, audio_track);
		char *video_info = 0;
		if (video_track != MP4_INVALID_TRACK_ID)
			video_info = MP4PrintVideoInfo(MP4hFile, video_track);

		if (video_info)
		{
			StringCchPrintfA(set_info, 256, "%s, %s %ux%u", audio_info, video_info,  MP4GetTrackVideoWidth(MP4hFile, video_track), MP4GetTrackVideoHeight(MP4hFile, video_track));
			videoOutput->extended(VIDUSER_SET_INFOSTRING,(INT_PTR)set_info,0);
			MP4Free(video_info);
		}
		MP4Free(audio_info);
	}

	if (!BuildAudioBuffers())
	{
		// TODO: benski> more cleanup work has to be done here!
		if (WaitForSingleObject(killEvent, 200) != WAIT_OBJECT_0)
			PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
		return 0;
	}

	nextSampleId = 1;
	nextVideoSampleId = 1;
	if (video)
		Video_Init();

	first = true;
	audio_opened = false;
	first_timestamp= 0;

	events[0]=killEvent;
	events[1]=seekEvent;
	events[2]=pauseEvent;
	waitTime = audio?0:INFINITE;
	done = false;

	while (!done)
	{
		int ret = WaitForMultipleObjects(2, events, FALSE, waitTime);
		switch (ret)
		{
			case WAIT_OBJECT_0: // kill event
				done = true;
				break;

			case WAIT_OBJECT_0 + 1: // seek event
				{
					bool rewind = m_needseek < GetClock();
					// TODO: reset pregap?
					MP4SampleId new_video_sample = MP4_INVALID_SAMPLE_ID;
					if (video)
					{
						SetEvent(video_start_flushing);
						WaitForSingleObject(video_flush_done, INFINITE);

						MP4Duration duration = MP4ConvertToTrackDuration(MP4hFile, video_track, m_needseek, MP4_MSECS_TIME_SCALE);
						if (duration != MP4_INVALID_DURATION)
						{
							new_video_sample = MP4GetSampleIdFromTime(MP4hFile, video_track, duration, true, rewind);
							if (new_video_sample == MP4_INVALID_SAMPLE_ID)
								new_video_sample = MP4GetSampleIdFromTime(MP4hFile, video_track, duration, false); // try again without keyframe seeking

							/* TODO: make sure the new seek direction is in the same as the request seek direction.  
							e.g. make sure a seek FORWARD doesn't go BACKWARD
							MP4Timestamp video_timestamp = MP4GetSampleTime(MP4hFile, video_track, seek_video_sample);
							int new_time = MP4ConvertFromTrackTimestamp(MP4hFile, video_track, video_timestamp, MP4_MSECS_TIME_SCALE);
							if (m_needseek < GetClock())
							video_timestamp = MP4GetSampleIdFromTime(MP4hFile, video_track, duration, true); // first closest keyframe prior
							*/
							if (new_video_sample != MP4_INVALID_SAMPLE_ID)
							{
								int m_old_needseek = m_needseek;

								MP4Timestamp video_timestamp = MP4GetSampleTime(MP4hFile, video_track, new_video_sample);
								m_needseek = MP4ConvertFromTrackTimestamp(MP4hFile, video_track, video_timestamp, MP4_MSECS_TIME_SCALE);
								if (!audio)
								{
									MP4Timestamp video_timestamp = MP4GetSampleTime(MP4hFile, video_track, new_video_sample);
									m_needseek = MP4ConvertFromTrackTimestamp(MP4hFile, video_track, video_timestamp, MP4_MSECS_TIME_SCALE);
									video_clock.Seek(m_needseek);
									m_needseek = -1;
								}
								else
								{
									// TODO check this will just do what is needed
									//		aim of this is when there is 1 artwork
									//		frame then we don't lock audio<->video
									//		as it otherwise prevents audio seeking
									if (!m_needseek && m_old_needseek != m_needseek && new_video_sample == 1)
									{
										m_needseek = m_old_needseek;
									}
								}
							}
						}
					}

					if (audio)
					{
						MP4Duration duration = MP4ConvertToTrackDuration(MP4hFile, audio_track, m_needseek, MP4_MSECS_TIME_SCALE);
						if (duration != MP4_INVALID_DURATION)
						{
							MP4SampleId newSampleId = audio_chunk?MP4GetChunkIdFromTime(MP4hFile, audio_track, duration):MP4GetSampleIdFromTime(MP4hFile, audio_track, duration);
							if (newSampleId != MP4_INVALID_SAMPLE_ID)
							{
								audio->Flush();

								if (video)
								{
									if (new_video_sample == MP4_INVALID_SAMPLE_ID)
									{
										SetEvent(video_resume);
									}
									else
									{
										nextVideoSampleId = new_video_sample;
										SetEvent(video_flush);
									}
									WaitForSingleObject(video_flush_done, INFINITE);
								}
								m_needseek = MP4ConvertFromTrackTimestamp(MP4hFile, audio_track, duration, MP4_MILLISECONDS_TIME_SCALE);
								ResetEvent(seekEvent);
								audio_output.Flush(m_needseek);
								m_needseek = -1;
								nextSampleId = newSampleId;
								continue;
							}
						}
					}
					else
					{
						if (new_video_sample == MP4_INVALID_SAMPLE_ID)
						{
							SetEvent(video_resume);
						}
						else
						{
							nextVideoSampleId = new_video_sample;
							SetEvent(video_flush);
						}
						WaitForSingleObject(video_flush_done, INFINITE);

						ResetEvent(seekEvent);
						continue;
					}
				}
				break;

			case WAIT_TIMEOUT:
				ReadNextAudioSample();
				break;
		}
	}

	if (WaitForSingleObject(killEvent, 0) == WAIT_TIMEOUT) // if (!killed)
	{
		// tell audio decoder about end-of-stream and get remaining audio
		/* if (audio) {
			audio->EndOfStream();
			
			sample->inputValid = 0;
			while (DecodeAudioSample(sample)) {
			}
		}
		*/
		audio_output.Write(0,0);
		audio_output.WaitWhilePlaying();

		if (WaitForSingleObject(killEvent, 0) == WAIT_TIMEOUT)
			PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
	}
	SetEvent(killEvent);

	// eat the rest of the APC messages
	while (SleepEx(0, TRUE) == WAIT_IO_COMPLETION) {}

	if (video)
		Video_Close();

	return 0;
}