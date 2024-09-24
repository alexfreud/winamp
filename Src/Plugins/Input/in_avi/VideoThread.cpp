#include "main.h"
#include "api__in_avi.h"
#include "../nu/AutoLock.h"
#include "../nu/SampleQueue.h"
#include "../Winamp/wa_ipc.h"
#include "interfaces.h"
#include "../nsavi/nsavi.h"
#include "../nu/ThreadName.h"
#include "player.h"

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

extern nsavi::HeaderList header_list;
extern int video_stream_num;

int width, height;
extern IVideoOutput *video_output;
static HANDLE video_thread=0;
extern ifc_avivideodecoder *video_decoder;
bool video_opened=false;
static HANDLE coded_frames_event=0;
static Nullsoft::Utility::LockGuard coded_frames_guard;
uint64_t video_total_time=0;
HANDLE video_break=0, video_flush=0, video_flush_done=0, video_resume=0, video_ready=0;

static int GetStreamNumber(uint32_t id)
{
	char *stream_data = (char *)(&id); 
	if (!isxdigit(stream_data[0]) || !isxdigit(stream_data[1]))
		return -1;

	stream_data[2] = 0;
	int stream_number = strtoul(stream_data, 0, 16);
	return stream_number;
}

void Video_Init()
{
	video_opened=false;
	video_decoder=0;
	video_thread=0;
	width=0;
	height=0;
	video_total_time=0;

	if (coded_frames_event == 0)
		coded_frames_event = CreateEvent(NULL, FALSE, FALSE, NULL);

	/* video events */
	if (!video_break)
		video_break = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!video_flush_done)
		video_flush_done = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!video_flush)
		video_flush = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!video_resume)
		video_resume = CreateEvent(NULL, TRUE, FALSE, NULL);	

	if (!video_ready)
		video_ready = CreateEvent(NULL, TRUE, FALSE, NULL);	
}

struct FRAMEDATA
{
	FRAMEDATA()
	{
		type = 0;
		data=0;
		length=0;
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
		type = 0;
	}
	void Set(uint16_t _type, void *_data, size_t _length)
	{
		type = _type;
		data = _data;
		length = _length;
	}
	void *data;
	size_t length;
	uint16_t type;
};

static SampleQueue<FRAMEDATA> coded_frames;

extern int GetOutputTime();
struct VideoContext
{
	nsavi::avi_reader *reader;
	nsavi::Demuxer *demuxer;
	int video_stream_num;
};

static void DecodeVideo(FRAMEDATA *frame_data)
{
	HANDLE handles[] = {killswitch, video_break};
	if (WaitForMultipleObjects(2, handles, FALSE, 0) == WAIT_TIMEOUT)
	{
		int decodeResult = video_decoder->DecodeChunk(frame_data->type, frame_data->data, frame_data->length);

		if (decodeResult == ifc_avivideodecoder::AVI_SUCCESS)
		{
			void *data, *decoder_data;
			while (video_decoder->GetPicture(&data, &decoder_data) == ifc_avivideodecoder::AVI_SUCCESS)
			{
				if (!video_opened)
				{
					int color_format;
					double aspect_ratio=1.0;
					int flip=0;
					if (video_decoder->GetOutputProperties(&width, &height, &color_format, &aspect_ratio, &flip) == ifc_avivideodecoder::AVI_SUCCESS)
					{
						nsavi::VPRP *vprp = header_list.stream_list[video_stream_num].video_properties;
						if (vprp)
						{
							uint32_t asp = vprp->aspect_ratio;
							uint32_t aspect_x = HIWORD(asp);
							uint32_t aspect_y = LOWORD(asp);

							aspect_ratio = (double)vprp->frame_width / (double)vprp->frame_height / ((double)aspect_x / (double)aspect_y);
						}
						else
							aspect_ratio = 1.0/aspect_ratio;

						video_output->extended(VIDUSER_SET_THREAD_SAFE, 1, 0);
						video_output->open(width, height, flip, aspect_ratio, color_format);
						video_opened=true;
					}
				}
				if (video_opened)
				{
					int timestamp=(int)(video_total_time*1000ULL/(uint64_t) header_list.stream_list[video_stream_num].stream_header->rate);
again:
					int real_time =(int)GetOutputTime();
					if (timestamp > (real_time+5))
					{
						HANDLE handles[] = {killswitch, video_break};
						int ret=WaitForMultipleObjects(2, handles, FALSE, timestamp-real_time);
						if (ret != WAIT_TIMEOUT)
						{
							video_decoder->FreePicture(data, decoder_data);
							frame_data->Reset();
							return ;
						}
						goto again; // TODO: handle paused state a little better than this
					}
					RGB32 *palette=0;
					if (video_decoder->GetPalette(&palette) == ifc_avivideodecoder::AVI_SUCCESS)
					{
						video_output->extended(VIDUSER_SET_PALETTE, (INT_PTR)palette, 0);
					}
					video_output->draw(data);
				}
				video_total_time += header_list.stream_list[video_stream_num].stream_header->scale;
				video_decoder->FreePicture(data, decoder_data);
			}
		}

	}

	//	frame_data->Reset();
}

static DWORD CALLBACK VideoProcedure(LPVOID param)
{
	SetThreadName(-1,"AVI_VideoProcedure");
	HANDLE wait_handles[] = { killswitch, video_break, video_flush, video_resume, coded_frames_event };
	while (1)
	{
		int ret = WaitForMultipleObjects(5, wait_handles, FALSE, INFINITE);

		if (ret == WAIT_OBJECT_0)
		{
			break;
		}
		if (ret == WAIT_OBJECT_0 + 1) // video break
		{
			ResetEvent(coded_frames_event); 
			ResetEvent(video_break);
			SetEvent(video_flush_done);
		}
		else if (ret == WAIT_OBJECT_0 + 2) // video flush
		{
			if (video_decoder)
				video_decoder->Flush();
			ResetEvent(video_flush);
			coded_frames.Trim();
			SetEvent(video_flush_done);
		}
		else if (ret == WAIT_OBJECT_0 + 3) // video resume
		{
			ResetEvent(video_resume);
			SetEvent(coded_frames_event);
			SetEvent(video_flush_done);
		}
		else if (ret == WAIT_OBJECT_0 + 4) // data from demuxer
		{
			FRAMEDATA *frame_data = 0;
			while (frame_data = coded_frames.PopProcessed())
			{
				DecodeVideo(frame_data);
				frame_data->Reset();
				coded_frames.PushFree(frame_data);
			}
		}		
	};

	if (video_opened && video_output)
		video_output->close();
	video_opened=false;

	return 0;
}

static DWORD CALLBACK VideoReaderProcedure(LPVOID param)
{
	VideoContext *ctx = (VideoContext *)param;
	nsavi::avi_reader *reader = ctx->reader;
	nsavi::Demuxer *demuxer = ctx->demuxer;
	SetThreadName(-1,"AVI_VideoProcedure");
	HANDLE wait_handles[] = { killswitch, video_break, video_flush, video_resume };
	int waitTime = 0;
	while (1)
	{
		int ret = WaitForMultipleObjects(4, wait_handles, FALSE, waitTime);

		if (ret == WAIT_OBJECT_0)
		{
			break;
		}
		if (ret == WAIT_OBJECT_0 + 1) // video break
		{
			ResetEvent(coded_frames_event); 
			ResetEvent(video_break);
			SetEvent(video_flush_done);
			waitTime = INFINITE;
		}
		else if (ret == WAIT_OBJECT_0 + 2) // video flush
		{
			if (video_decoder)
				video_decoder->Flush();
			ResetEvent(video_flush);
			coded_frames.Trim();
			SetEvent(video_flush_done);
			waitTime = 0;
		}
		else if (ret == WAIT_OBJECT_0 + 3) // video resume
		{
			ResetEvent(video_resume);
			SetEvent(coded_frames_event);
			SetEvent(video_flush_done);
			waitTime = 0;
		}
		else if (ret == WAIT_TIMEOUT)
		{
			void *data=0;
			uint32_t data_size = 0;
			uint32_t data_type = 0;
			int ret = demuxer->GetNextMovieChunk(reader, &data, &data_size, &data_type, video_stream_num);
			if (ret != nsavi::READ_OK)
			{
				break;
			}
			int stream_number = GetStreamNumber(data_type);
			uint16_t type = (data_type>>16);

			if (stream_number == video_stream_num)
			{
				FRAMEDATA frame_data;
				frame_data.data = data;
				frame_data.length = data_size;
				frame_data.type = type;
				DecodeVideo(&frame_data);
				frame_data.Reset();
			}
			else
				free(data);
		}
	};

	if (video_opened && video_output)
		video_output->close();
	video_opened=false;
	free(ctx);
	return 0;
}

bool CreateVideoReaderThread(nsavi::Demuxer *demuxer, nsavi::avi_reader *video_reader)
{
	if (!video_decoder || video_only)
		return false;

	VideoContext *context = (VideoContext *)calloc(1, sizeof(VideoContext));

	context->reader = video_reader;
	context->demuxer = demuxer;
	DWORD threadId = 0;
	video_thread = CreateThread(0, 0, VideoReaderProcedure, context, 0, &threadId);
	SetThreadPriority(video_thread, (INT)AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
	return true;
}

bool OnVideo(uint16_t type, void *data, size_t length)
{
	if (!video_decoder)
		return false;

	if (!video_only && !video_thread)
	{
		DWORD threadId;
		video_thread = CreateThread(0, 0, VideoProcedure, 0, 0, &threadId);
		SetThreadPriority(video_thread, (INT)AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
	}

	if (video_thread)
	{
		FRAMEDATA *new_frame = coded_frames.PopFree();
		if (new_frame)
		{
			new_frame->Set(type, data, length);
			coded_frames.PushProcessed(new_frame);
			SetEvent(coded_frames_event);
		}
		return true;
	}
	else if (video_only)
	{
		FRAMEDATA *new_frame = coded_frames.PopFree();
		if (new_frame)
		{
			new_frame->Set(type, data, length);
			DecodeVideo(new_frame);
			new_frame->Reset();
		}
		return true;
	}

	return false;
}

void Video_Stop()
{
	SetEvent(killswitch);
	if (video_only)
	{
		video_thread=0;

		ResetEvent(coded_frames_event);
		Nullsoft::Utility::AutoLock l(coded_frames_guard);
		coded_frames.Trim();
		if (video_opened && video_output)
			video_output->close();
		video_opened=false;
	}
	else
	{
		if (video_thread)
			WaitForSingleObject(video_thread, INFINITE);
		video_thread=0;

		ResetEvent(coded_frames_event);
		Nullsoft::Utility::AutoLock l(coded_frames_guard);
		coded_frames.Trim();
	}
}

void Video_Close()
{
	video_opened=false;

	if (video_decoder)
	{
		video_decoder->Close();
		video_decoder=0;
	}
}

void Video_Break()
{
	if (!video_only && video_decoder)
	{
		SetEvent(video_break);
		HANDLE events[2] = {video_flush_done, video_thread}; 
		WaitForMultipleObjects(2, events, FALSE, INFINITE);
	}
}

void Video_Flush()
{
	if (video_decoder)
	{
		if (video_only)
		{
			video_decoder->Flush();
		}
		else 
		{
			SetEvent(video_flush);
			HANDLE events[2] = {video_flush_done, video_thread}; 
			WaitForMultipleObjects(2, events, FALSE, INFINITE);
		}
	}
}

/*
bool Video_DecoderReady()
{
if (!video_decoder)
return true;

return !!video_decoder->Ready();
}
*/
