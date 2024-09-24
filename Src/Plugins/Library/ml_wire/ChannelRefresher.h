#ifndef NULLSOFT_CHANNELREFRESHERH
#define NULLSOFT_CHANNELREFRESHERH

#include "ChannelSync.h"
class ChannelRefresher: public ChannelSync
{
public:
	void BeginChannelSync();
	void NewChannel(const Channel &newChannel);
	void EndChannelSync();

};

#endif