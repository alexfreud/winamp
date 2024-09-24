#ifndef NULLSOFT_BUFFERLAYERH
#define NULLSOFT_BUFFERLAYERH

#include "WMHandler.h"

class BufferLayer : public WMHandler
{
public:
	BufferLayer(IWMReader *reader);
	~BufferLayer();

protected:
	void BufferingStarted();
	void BufferingStopped();
	void OpenFailed();

private:
	static DWORD WINAPI BufThread_stub(void *ptr);
	void BufThread();
	int Wait();
	HANDLE events[2]; 
	IWMReaderAdvanced2 *reader2;
	HANDLE thread;
	volatile bool buffering;
};
#endif