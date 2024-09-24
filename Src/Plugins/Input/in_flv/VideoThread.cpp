#include "main.h"
#include "VideoThread.h"
#include "api__in_flv.h"
#include "FLVVideoHeader.h"
#include <shlwapi.h>
#include <windows.h>
#include "../nu/threadname.h"
#include <api/service/waservicefactory.h>
#include "../nu/AutoLock.h"
#include "../nu/SampleQueue.h"

int width, height;
IVideoOutput *videoOutput=0;
static HANDLE videoThread=0;
static volatile LONG video_flush=0;
static ifc_flvvideodecoder *videoDecoder=0;
bool video_opened=false;
static HANDLE coded_frames_event=0;
static HANDLE video_flush_event=0;
static Nullsoft::Utility::LockGuard coded_frames_guard;
extern bool video_only;

void Video_Init()
{
	video_opened=false;
	videoDecoder=0;
	videoThread=0;
	width=0;
	height=0;

	if (coded_frames_event == 0)
		coded_frames_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (video_flush_event == 0)
		video_flush_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	video_flush=0;
}

struct FRAMEDATA
{
	FRAMEDATA()
	{
		data=0;
		length=0;
		timestamp=0;
	}

	~FRAMEDATA()
	{
		free(data);
	}
	void Reset()
	{
		free(data);
		data=0;
		length=0;
		timestamp=0;
	}
	void Set(void *_data, size_t _length, uint32_t ts)
	{
		data = _data;
		length = _length;
		timestamp = ts;
	}
	void *data;
	size_t length;
	uint32_t timestamp;
};

static SampleQueue<FRAMEDATA> coded_frames;

extern int GetOutputTime();
static void DecodeVideo(FRAMEDATA *framedata)
{
	if (WaitForSingleObject(killswitch, 0) != WAIT_OBJECT_0)
	{
		int decodeResult = videoDecoder->DecodeSample(framedata->data, framedata->length, framedata->timestamp);

		if (decodeResult == FLV_VIDEO_SUCCESS)
		{
			void *data, *decoder_data;
			uint64_t timestamp=framedata->timestamp;
			while (videoDecoder->GetPicture(&data, &decoder_data, &timestamp) == FLV_VIDEO_SUCCESS)
			{
				if (!video_opened)
				{
					int color_format;
					if (videoDecoder->GetOutputFormat(&width, &height, &color_format) == FLV_VIDEO_SUCCESS)
					{
						videoOutput->extended(VIDUSER_SET_THREAD_SAFE, 1, 0);
						videoOutput->open(width, height, 0, 1.0, color_format);
						video_opened=true;
					}
				}
				if (video_opened)
				{
again:
					int realTime =(int)GetOutputTime();
					if (timestamp > (realTime+5))
					{
						HANDLE handles[] = {killswitch, video_flush_event};
						int ret=WaitForMultipleObjects(2, handles, FALSE, (DWORD)(timestamp-realTime));
						if (ret != WAIT_TIMEOUT)
						{
							videoDecoder->FreePicture(data, decoder_data);
							framedata->Reset();
							return ;
						}
						goto again; // TODO: handle paused state a little better than this
					}
					videoOutput->draw(data);
				}
				videoDecoder->FreePicture(data, decoder_data);
			}
		}
	}

	framedata->Reset();
}

DWORD CALLBACK VideoProcedure(LPVOID param)
{
	SetThreadName(-1,"FLV_VideoProcedure");
	HANDLE wait_handles[] = { killswitch, video_flush_event, coded_frames_event};
	int ret;
	do
	{
		ret = WaitForMultipleObjects(3, wait_handles, FALSE, INFINITE);
		if (ret == WAIT_OBJECT_0 + 1)
		{
			if (video_flush)
			{
				InterlockedDecrement(&video_flush);
				if (videoDecoder)
					videoDecoder->Flush();
			}
			ResetEvent(video_flush_event);
		}
		else if (ret == WAIT_OBJECT_0 + 2)
		{
			FRAMEDATA *frame_data = 0;
			while (frame_data = coded_frames.PopProcessed())
			{
				DecodeVideo(frame_data);
				frame_data->Reset();
				coded_frames.PushFree(frame_data);
			}
		}
	} while (ret != WAIT_OBJECT_0);

	if (video_opened && videoOutput)
		videoOutput->close();
	video_opened=false;
	return 0;
}

bool Video_IsSupported(int type)
{
	size_t n = 0;
	waServiceFactory *factory = NULL;
	while (factory = plugin.service->service_enumService(WaSvc::FLVDECODER, n++))
	{
		svc_flvdecoder *creator = (svc_flvdecoder *)factory->getInterface();
		if (creator)
		{
			int supported = creator->HandlesVideo(type);
			factory->releaseInterface(creator);
			if (supported == svc_flvdecoder::CREATEDECODER_SUCCESS)
				return true;
		}		
	}
	return false;
}

static ifc_flvvideodecoder *CreateVideoDecoder(int type)
{
	ifc_flvvideodecoder *video_decoder = 0;
	size_t n = 0;
	waServiceFactory *factory = NULL;
	while (factory = plugin.service->service_enumService(WaSvc::FLVDECODER, n++))
	{
		svc_flvdecoder *creator = (svc_flvdecoder *)factory->getInterface();
		if (creator)
		{
			if (creator->CreateVideoDecoder(type, width, height, &video_decoder) == FLV_VIDEO_SUCCESS)
				return video_decoder;

			factory->releaseInterface(creator);
		}		
	}
	return 0;
}
// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };
bool OnVideo(void *data, size_t length, int type, unsigned __int32 timestamp)
{
	if (!videoDecoder)
	{
		videoDecoder = CreateVideoDecoder(type);
	}

	if (videoDecoder)
	{
		if (!video_only && !videoThread)
		{
			DWORD threadId;
			videoThread = CreateThread(0, 0, VideoProcedure, 0, 0, &threadId);
			SetThreadPriority(videoThread, (INT)AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
		}

		FRAMEDATA *new_frame = coded_frames.PopFree();
		if (new_frame)
		{
			new_frame->Set(data, length, timestamp);
			if (video_only)
			{
				DecodeVideo(new_frame);
				new_frame->Reset();
				coded_frames.PushFree(new_frame);
			}
			else
			{
				coded_frames.PushProcessed(new_frame);
				SetEvent(coded_frames_event);
			}
		}

		return true;
	}

	return false;
}

void Video_Stop()
{
	if (video_only)
	{
		ResetEvent(coded_frames_event);
		Nullsoft::Utility::AutoLock l(coded_frames_guard);
		coded_frames.Trim();
		if (video_opened && videoOutput)
			videoOutput->close();
		video_opened=false;
	}
	else
	{
		if (videoThread)
			WaitForSingleObject(videoThread, INFINITE);
		videoThread=0;

		InterlockedIncrement(&video_flush);
		ResetEvent(coded_frames_event);
		Nullsoft::Utility::AutoLock l(coded_frames_guard);
		coded_frames.Trim();
	}
}

void Video_Close()
{
	video_opened=false;

	if (videoDecoder)
	{
		videoDecoder->Close();
		videoDecoder=0;
	}
}

void VideoFlush()
{
	if (video_only)
	{
		if (videoDecoder)
			videoDecoder->Flush();
	}
	else if (videoThread)
	{
		InterlockedIncrement(&video_flush);
		ResetEvent(coded_frames_event);
		coded_frames.Trim();
		SetEvent(video_flush_event);
	}
}

bool Video_DecoderReady()
{
	if (!videoDecoder)
		return true;

	return !!videoDecoder->Ready();
}
