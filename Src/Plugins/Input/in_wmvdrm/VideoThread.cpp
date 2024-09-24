#include "Main.h"
#include "VideoThread.h"
#include "VideoLayer.h"
#include "config.h"
#include <windows.h>

DWORD WINAPI VidThread_stub(void *ptr)
{
	((VideoThread *)ptr)->VidThread();
	return 0;
}

VideoThread::VideoThread() : converter(0), clock(0)
{
	drm = false;

	DWORD id;
	thread = CreateThread(NULL, 256*1024, VidThread_stub, (void *)this, NULL, &id);
	SetThreadPriority(thread, AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
}

void VideoThread::Start(VideoDataConverter *_converter, WMHandler *_clock)
{
	clock = _clock;
	if (converter != _converter)
	{
		if (converter)
			delete converter;
		converter = _converter;
	}
	ResetEvent(stopped);
	QueueUserAPC(MediaThread_StartAPC, thread, reinterpret_cast<ULONG_PTR>(this));
}

void VideoThread::VidThread()
{
	while (true)
	{
		switch (WaitForSingleObjectEx(killEvent, wait, TRUE))
		{
			case WAIT_OBJECT_0:
				//StopAPC();
				return;

			case WAIT_TIMEOUT:
			{
				if (buffers.empty())
		        {
					SetEvent(bufferFreed);
					continue;
				}

				MediaBuffer *buffer = buffers.front();

				__int64 diff;
				clock->TimeToSync(buffer->timestamp, diff);
				if (diff < VIDEO_ACCEPTABLE_JITTER)
				{
					void *data;
					DWORD size;

					buffer->buffer->GetBufferAndLength((BYTE **)&data, &size);
					if (buffer->drmProtected)
						winamp.EncryptedDrawFrame(converter->Convert(data));
					else
						winamp.DrawFrame(converter->Convert(data));

					try {
						buffer->buffer->Release();
						delete buffer;
					} catch (...) {}

					//buffers.pop_front();
					if (buffers.size())
					{
						buffers.erase(buffers.begin());
					}
				}
				if (buffers.size() < config_video_cache_frames)
					SetEvent(bufferFreed);
				}
				continue;

			default:
				continue;
		}
    }
}

void VideoThread::AddAPC(MediaBuffer *buffer)
{
	if (buffers.empty())
	{
		__int64 diff;
		clock->TimeToSync(buffer->timestamp, diff);
		if (diff < VIDEO_ACCEPTABLE_JITTER)
		{
			void *data;
			DWORD size;
			buffer->buffer->GetBufferAndLength((BYTE **)&data, &size);
			if (buffer->drmProtected)
				winamp.EncryptedDrawFrame(converter->Convert(data));
			else
				winamp.DrawFrame(converter->Convert(data));

			buffer->buffer->Release();
			if (buffers.size() >= config_video_cache_frames)
				ResetEvent(bufferFreed);
			return;
		}
	}

	OrderedInsert(buffer);

	if (buffers.size() >= config_video_cache_frames)
		ResetEvent(bufferFreed);
}

struct VideoOpenParameters
{
	int width;
	int height;
	int color_format;
	double aspect;
	int flip;
	bool drm;
};

VOID CALLBACK VideoThread::VideoThread_VideoOpenAPC(ULONG_PTR params)
{
	VideoOpenParameters *p = (VideoOpenParameters *)params;
	if (p->drm)
	{
		winamp.OpenEncryptedVideo(p->width, p->height, !!p->flip, p->aspect, p->color_format);
	}
	else
	{
		winamp.OpenVideo(p->width, p->height, !!p->flip, p->aspect, p->color_format);
	}
}

void VideoThread::OpenVideo(bool drm, int width, int height, bool flip, double aspect, int fourcc)
{
	VideoOpenParameters *p = new VideoOpenParameters;
	p->width = width;
	p->height = height;
	p->color_format = fourcc;
	p->aspect = aspect;
	p->flip = flip;
	p->drm = drm;
	this->drm = drm;
	QueueUserAPC(VideoThread_VideoOpenAPC, thread, reinterpret_cast<ULONG_PTR>(p));
}

VOID CALLBACK VideoThread::VideoThread_VideoCloseAPC(ULONG_PTR params)
{
	if (params)
		winamp.CloseEncryptedVideo();
	else
		winamp.CloseVideo();
}

void VideoThread::CloseVideo(bool drm)
{
	QueueUserAPC(VideoThread_VideoCloseAPC, thread, static_cast<ULONG_PTR>(drm));
}