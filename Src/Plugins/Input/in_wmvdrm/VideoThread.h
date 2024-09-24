#ifndef NULLSOFTVIDEOTHREADH
#define NULLSOFTVIDEOTHREADH

#include "VideoDataConverter.h"
#include "WMHandler.h"
#include <deque> 
#include <wmsdk.h>
#include "MediaThread.h"

class VideoThread  : public MediaThread
{
public:
	VideoThread();
	void Start(VideoDataConverter *_converter, WMHandler *_clock);

	/* AddBuffers put a video buffer in the queue
	it returns true if it was added 
	it returns false if it was NOT added.  it is up to YOU (the caller) to sleep for a while and call again
	*/
	void VidThread();

	void OpenVideo(bool drm, int width, int height, bool flip, double aspect, int fourcc);
	void CloseVideo(bool drm);
private:
	static VOID CALLBACK VideoThread_VideoOpenAPC(ULONG_PTR params);
	static VOID CALLBACK VideoThread_VideoCloseAPC(ULONG_PTR params);
	
	
  void AddAPC(MediaBuffer *);
	VideoDataConverter *converter;
	WMHandler *clock;
	bool drm;
	
};


#endif