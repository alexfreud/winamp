#include "BufferLayer.h"
#include "Main.h"
#include "resource.h"
#define killEvent events[0]
#define startEvent events[1]

enum
{
    KILL_EVENT = 0,
    START_EVENT = 1,
};

DWORD WINAPI BufferLayer::BufThread_stub(void *ptr)
{
	((BufferLayer *)ptr)->BufThread();
	return 0;
}

BufferLayer::BufferLayer(IWMReader *reader) : reader2(0), buffering(false)
{
	if (FAILED(reader->QueryInterface(&reader2)))
		reader2 = 0;

	startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	killEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	DWORD id;
	thread = CreateThread(NULL, 128*1024, BufThread_stub, (void *)this, NULL, &id);
}

BufferLayer::~BufferLayer()
{
	SetEvent(killEvent);
	ResetEvent(startEvent);
	WaitForSingleObject(thread, INFINITE);
	if (reader2) reader2->Release(); reader2 = 0;
}


void BufferLayer::BufferingStarted()
{
	winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_BUFFERING));
	buffering=true;
	SetEvent(startEvent);
	WMHandler::BufferingStarted();
}

void BufferLayer::BufferingStopped()
{
	winamp.SetStatus(L"");
	buffering=false;
	ResetEvent(startEvent);
	WMHandler::BufferingStopped();
}

int BufferLayer::Wait()
{
	if (WaitForSingleObject(killEvent, 0) == WAIT_OBJECT_0)
		return KILL_EVENT;

	return WaitForMultipleObjects(2, events, FALSE, INFINITE) - WAIT_OBJECT_0;

}

void BufferLayer::BufThread()
{
	do
	{
		switch (Wait())
		{
		case KILL_EVENT:
			return ;
		case START_EVENT:
			{
				if (reader2)
				{
					DWORD percent;
					QWORD throwAway;
					if (SUCCEEDED(reader2->GetBufferProgress(&percent, &throwAway)))
						winamp.Buffering(percent, WASABI_API_LNGSTRINGW(IDS_BUFFERING));

				}
				Sleep(10);
			}
			continue;
		}
	}
	while (true);
}

void BufferLayer::OpenFailed()
{
	ResetEvent(startEvent);
	WMHandler::OpenFailed();
}

