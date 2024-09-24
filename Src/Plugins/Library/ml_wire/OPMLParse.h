#ifndef NULLSOFT_OPMLPARSEH
#define NULLSOFT_OPMLPARSEH

#include "DownloadThread.h"
#include "ChannelSync.h"
class OPMLParse : public DownloadThread
{
public:
	OPMLParse(ChannelSync *_sync)  
			: sync(_sync)
	{

	}


	~OPMLParse()
	{
		sync = 0;
	}

	virtual void ReadNodes(const wchar_t *url);

private:

	ChannelSync *sync;
};

#endif