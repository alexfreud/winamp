#ifndef NULLSOFT_CHANNELCHECKH
#define NULLSOFT_CHANNELCHECKH
#include "ChannelSync.h"
class ChannelCheck : public ChannelSync
{
public:
	void NewChannel(const Channel &newChannel) 
	{
		channel=newChannel;
	}
	Channel channel;
};

#endif