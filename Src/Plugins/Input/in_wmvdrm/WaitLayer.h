#ifndef NULLSOFT_WAITLAYERH
#define NULLSOFT_WAITLAYERH

#include "WMHandler.h"

class WaitLayer : public WMHandler
{
public:
	WaitLayer(IWMReader *_reader);
	~WaitLayer();

	void ResetForOpen();
	bool WaitForOpen(int time_ms);
	bool IsOpen();
protected:
	/* inherited from WMCallback */
	void OpenCalled();
	void OpenFailed();
	void Opened();
	
	IWMReader *reader; // not ours 
	HANDLE stopEvent, openEvent;
};

#endif
