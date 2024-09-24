#include "main.h"
#include "MediaThread.h"
#include "config.h"

MediaThread::MediaThread() :  wait(INFINITE), thread(0)
{
  killEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	stopped = CreateEvent(NULL, TRUE, TRUE, NULL);
	bufferFreed = CreateEvent(NULL, TRUE, TRUE, NULL);
}

MediaThread::~MediaThread()
{
	Kill();
	if (thread)
		CloseHandle(thread);
}

VOID CALLBACK MediaThread_StartAPC(ULONG_PTR param)
{
  reinterpret_cast<MediaThread *>(param)->StartAPC();
}

void MediaThread::StartAPC()
{
  wait=config_video_jitter;
}

void MediaThread::StopAPC()
{
  BufferList::iterator itr;
  for (itr = buffers.begin();itr != buffers.end();itr++)
  {
    (*itr)->buffer->Release();
    delete (*itr);
  }

  buffers.clear();
  SetEvent(stopped);
  SetEvent(bufferFreed);
  wait=INFINITE;
}

static VOID CALLBACK MediaThread_StopAPC(ULONG_PTR param)
{
  reinterpret_cast<MediaThread *>(param)->StopAPC();
}

void MediaThread::Stop()
{
	ResetEvent(stopped);
  QueueUserAPC(MediaThread_StopAPC, thread, reinterpret_cast<ULONG_PTR>(this));
  WaitForSingleObject(stopped, INFINITE);
}

void MediaThread::WaitForStop()
{
	 WaitForSingleObject(stopped, INFINITE);
}

void MediaThread::SignalStop()
{
	ResetEvent(stopped);
  QueueUserAPC(MediaThread_StopAPC, thread, reinterpret_cast<ULONG_PTR>(this));
}

void MediaThread::Kill()
{
  SetEvent(killEvent);
  WaitForSingleObject(stopped, INFINITE);
}

void MediaThread::OrderedInsert(MediaBuffer *buffer)
{
	BufferList::iterator itr;
	for (itr = buffers.begin();itr != buffers.end(); itr++)
	{
		if ((*itr)->timestamp > buffer->timestamp)
		{
			buffers.insert(itr, buffer);
			break;
		}
	}
	if (itr == buffers.end())
  	buffers.push_back(buffer);
  
}


VOID CALLBACK MediaThread_AddAPC(ULONG_PTR param)
{
  MediaBufferAPC *apc = reinterpret_cast<MediaBufferAPC *>(param);
  apc->thread->AddAPC(apc->buffer);
  delete apc;
}

bool MediaThread::AddBuffer(INSSBuffer *buff, QWORD ts, unsigned long flags, bool drmProtected)
{
  if (WaitForSingleObject(bufferFreed, 0) == WAIT_TIMEOUT)
    return false;

  buff->AddRef();
  MediaBuffer *buffer = new MediaBuffer(buff, ts, flags, drmProtected);
  MediaBufferAPC *apc = new MediaBufferAPC;
  apc->buffer = buffer;
  apc->thread = this;
  QueueUserAPC(MediaThread_AddAPC, thread, reinterpret_cast<ULONG_PTR>(apc));
  Sleep(config_video_jitter); // sleep for a bit to keep the thread from going nuts
  return true; // added
}
