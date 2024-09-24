#include "main.h"
#include "ChannelRefresher.h"
#include <algorithm>

#include "./subscriptionView.h"

using namespace Nullsoft::Utility;
void ChannelRefresher::BeginChannelSync()
{}

void ChannelRefresher::NewChannel(const Channel &newChannel)
{
	AutoLock lock (channels LOCKNAME("ChannelRefresher::NewChannel"));
	ChannelList::iterator found;
	for (found=channels.begin();found!=channels.end(); found++)
	{
		if (!wcscmp(found->url, newChannel.url))
			break;
	}
	if (found != channels.end())
	{
		// todo, redo category indexing as necessary.
		found->UpdateFrom(newChannel);
		found->lastUpdate = _time64(0);
		found->needsRefresh = false;
	}
}

void ChannelRefresher::EndChannelSync()
{
	HWND wnd = SubscriptionView_FindWindow();
	SubscriptionView_RefreshChannels(wnd, TRUE);
}