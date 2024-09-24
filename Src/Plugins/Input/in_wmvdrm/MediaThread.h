#ifndef NULLSOFT_MEDIATHREADH
#define NULLSOFT_MEDIATHREADH

#include <deque> 
#include <wmsdk.h>
#include <vector>

VOID CALLBACK MediaThread_StartAPC(ULONG_PTR param);
VOID CALLBACK MediaThread_AddAPC(ULONG_PTR param);
struct MediaBuffer
{
	MediaBuffer(INSSBuffer *b, QWORD t, unsigned long f, bool d) : buffer(b), timestamp(t), flags(f), drmProtected(d) {}
	INSSBuffer *buffer;
	QWORD timestamp;
	unsigned long flags;
	bool drmProtected;
};
struct MediaBufferAPC;

class MediaThread
{
public:
	MediaThread();
	~MediaThread();

	bool AddBuffer(INSSBuffer *buff, QWORD ts, unsigned long flags, bool drmProtected);

	void Stop();
	void SignalStop();
	void WaitForStop();
	void Kill();

public:
	void StopAPC();
	void StartAPC();

	virtual void AddAPC(MediaBuffer *buffer)=0;

protected:
	void OrderedInsert(MediaBuffer *buffer);

protected:
	int wait;
	HANDLE thread;
	HANDLE killEvent, stopped, bufferFreed;

	typedef std::vector<MediaBuffer*> BufferList;
	BufferList buffers;
};

struct MediaBufferAPC
{
	MediaBuffer *buffer;
	MediaThread *thread;
};
#endif