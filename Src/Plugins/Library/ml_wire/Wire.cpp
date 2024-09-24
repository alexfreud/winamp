#include "Main.h"

#include "./subscriptionView.h"
#include "FeedsDialog.h"
#include "Feeds.h"

#include <algorithm>


ChannelList channels;

//CategoryIndex sourceByCategory;
using namespace Nullsoft::Utility;
//LockGuard /*feedGuard, */channelGuard, categoryGuard;

void WireManager::BeginChannelSync()
{
	// TODO something better than this =)
	// like making 'touched' flags and delete untouched ones
	//sources.clear();
	//sourceByCategory.clear();
}

void WireManager::NewChannel(const Channel &newChannel)
{
	AutoLock lock (channels LOCKNAME("NewChannel"));
	for (ChannelList::iterator itr=channels.begin();itr!=channels.end(); itr++)
	{
		if (*itr == newChannel)
		{
			itr->UpdateFrom(newChannel);
			return;
		}
	}
	channels.push_back(newChannel);
			
}

void WireManager::EndChannelSync()
{
	//SubscriptionView_RefreshChannels(NULL, TRUE);
}

bool ChannelTitleSort(Channel &channel1, Channel &channel2)
{
	return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, channel1.title, -1,  channel2.title, -1));
}

void ChannelList::SortByTitle()
{
	AutoLock lock (channelGuard LOCKNAME("SortByTitle"));
	std::sort(channelList.begin(), channelList.end(), ChannelTitleSort);
}

void ChannelList::push_back(const Channel &channel)
{
	channelList.push_back(channel);
}

bool ChannelList::AddChannel(Channel &channel)
{
	ChannelList::iterator found;
	for (found=channels.begin(); found!=channels.end(); found++)
	{
		if (!wcscmp(found->url, channel.url))
			break;
	}

	if (found == channels.end()){
		channel.needsRefresh=true;
		push_back(channel);
		SaveChannels(*this);
		return true;
	}

	return false;
}

size_t ChannelList::GetNumPodcasts()
{
	return channels.size();
}

ifc_podcast *ChannelList::EnumPodcast(size_t i)
{
	if (i < channels.size())
		return &channels[i];
	else
		return 0;
}

#undef CBCLASS
#define CBCLASS ChannelList
START_DISPATCH;
CB(API_PODCASTS_GETNUMPODCASTS, GetNumPodcasts)
CB(API_PODCASTS_ENUMPODCAST, EnumPodcast)
END_DISPATCH;
#undef CBCLASS

