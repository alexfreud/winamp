#include "Main.h"
#include "AudioThread.h"
#include "AudioLayer.h"
#include <assert.h>

extern unsigned long endTime;

DWORD WINAPI AudThread_stub(void *ptr)
{
	((AudioThread *)ptr)->AudThread();
	return 0;
}

void AudioThread::Start(WMHandler *_output)
{
	assert(_output);
	output = _output;
	eof=0;
	ResetEvent(stopped);
	QueueUserAPC(MediaThread_StartAPC, thread, reinterpret_cast<ULONG_PTR>(static_cast<MediaThread *>(this)));
}

AudioThread::AudioThread(AudioLayer *audio) :  output(0), audioLayer(audio)
{
	DWORD id;
	thread = CreateThread(NULL, 256*1024, AudThread_stub, (void *)this, NULL, &id);
	SetThreadPriority(thread, AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
}

void AudioThread::AudThread()
{
	int endbreak=0;
	while (true)
	{
		switch (WaitForSingleObjectEx(killEvent, wait, TRUE))
		{
			case WAIT_OBJECT_0:
				//StopAPC();
				return;

			case WAIT_TIMEOUT:
			{
				if (buffers.empty() || endbreak)
				{
					SetEvent(bufferFreed);

					if (eof==1)
					{
						eof=2;
						output->EndOfFile();
					}
					endbreak = 0;
					continue;
				}

				MediaBuffer *buffer = buffers.front();
				DWORD length;
				void *data;
				buffer->buffer->GetBufferAndLength((BYTE **)&data, &length);

				//if (out->CanWrite() >= length)
				{
					QWORD timestamptemp = buffer->timestamp/10000LL;
					DWORD timestamp = static_cast<DWORD>(timestamptemp);

					if (buffer->flags & WM_SF_DISCONTINUITY)
					{
						// fill with silence!
						int msToFill = timestamp - out->GetWrittenTime(); // TODO: maybe use microsoft's time resolution?
						if (msToFill > 0 && msToFill < 2000)
						{
							int bytes = audioLayer->AudioMillisecondsToBytes(msToFill);
							__int8 *zeroes = (__int8 *)calloc(bytes, 1);
							if (zeroes)
							{
								output->AudioDataReceived(zeroes, bytes, timestamp);
								free(zeroes);
							}
							else
							{
								out->Flush(timestamp);
							}
						}
						else if (msToFill > 0)
						{
							out->Flush(timestamp);
						}
					}

					output->AudioDataReceived(data, length, timestamp);

					// TODO seen a few crash reports failing around here
					//		might be the cause of the random wma fails
					//		but crash dump doesn't help too much afaict
					try {
						buffer->buffer->Release();
						delete buffer;
					} catch (...) {}
					
					//buffers.pop_front();
					if (buffers.size())
					{
						buffers.erase(buffers.begin());
					}

					unsigned long x =  endTime;
					if (x && timestamp > x)
					{
						eof = 1;  // reached the end baby....
						endbreak = 1;
					}
				}				
				if (buffers.size() < config_audio_cache_frames)
					SetEvent(bufferFreed);
				}
				continue;

			default:
				continue;
		}
	}
}

void AudioThread::AddAPC(MediaBuffer *buffer)
{
	OrderedInsert(buffer);
	if (buffers.size() >= config_audio_cache_frames)
		ResetEvent(bufferFreed);
}