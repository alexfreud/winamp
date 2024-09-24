#ifndef NULLSOFT_FEEDPARSEH
#define NULLSOFT_FEEDPARSEH

#include "DownloadThread.h"
#include "ChannelSync.h"
class FeedParse : public DownloadThread
{
public:
	FeedParse(ChannelSync *_sync, bool doWinampSpecificTags = false)
			: sync(_sync), loadingOwnFeed(doWinampSpecificTags)
	{}

	~FeedParse()
	{
		sync = 0;
	}

	virtual void ReadNodes(const wchar_t *url);
private:
	ChannelSync *sync;
	bool loadingOwnFeed;
};

#endif