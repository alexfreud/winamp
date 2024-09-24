#ifndef NULLSOFT_WIREH
#define NULLSOFT_WIREH

#include "Feeds.h"
#include "../nu/AutoLock.h"
#include "api_podcasts.h"
#include <vector>

class ChannelList : public api_podcasts
{
public:

	ChannelList() : channelGuard( "Feed Guard" )                      {}

	typedef std::vector<Channel> ChannelContainer;
	typedef ChannelContainer::iterator iterator;

	size_t size()                                                     { return channelList.size(); }
	bool empty()                                                      { return channelList.empty();  }

	void push_back( const Channel &channel );
	bool AddChannel( Channel &channel );

	iterator begin()                                                  { return channelList.begin(); }

	iterator end()                                                    { return channelList.end(); }

	void RemoveChannel( int index )                                   { channelList.erase( channelList.begin() + index ); }

	Channel &operator[]( size_t index )                               { return channelList[ index ]; }

	ChannelContainer channelList;

	operator Nullsoft::Utility::LockGuard &( )                        { return channelGuard; }

	void SortByTitle();

	Nullsoft::Utility::LockGuard channelGuard;

public: // api_podcasts interface
	size_t GetNumPodcasts();
	ifc_podcast *EnumPodcast( size_t i );

protected:
	RECVS_DISPATCH;
};

extern ChannelList channels;
//extern CategoryIndex sourceByCategory;
#include "ChannelSync.h"

class WireManager : public ChannelSync
{
public:
	void BeginChannelSync();
	void NewChannel( const Channel &newChannel );
	void EndChannelSync();
};

extern WireManager channelMgr;

#endif