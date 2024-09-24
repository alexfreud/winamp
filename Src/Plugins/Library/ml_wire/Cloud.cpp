#include "main.h"
#include "api__ml_wire.h"
#include "Cloud.h"
#include "FeedParse.h"
#include "Defaults.h"
#include "./subscriptionView.h"
#include "ChannelRefresher.h"
#include "Util.h"
#include <algorithm>
#include <strsafe.h>

/* benski> TODO rewrite Callback() so we don't have to reserve a thread */
using namespace Nullsoft::Utility;

#define CLOUD_TICK_MS 60000
ChannelRefresher channelRefresher;

static bool kill = false;

static __time64_t RetrieveMinimalUpdateTime()
{
	__time64_t minUpdateTime = 0;

	AutoLock lock (channels LOCKNAME("RetrieveMinimalUpdateTime"));
	ChannelList::iterator itr;
	for (itr=channels.begin(); itr!=channels.end(); itr++)
	{
		if (itr->useDefaultUpdate)
		{
			if ( !updateTime ) autoUpdate = 0;
			if ( autoUpdate && (!minUpdateTime || minUpdateTime && (updateTime < minUpdateTime)) )
			{
				minUpdateTime = updateTime;
			}
		}
		else // use the custom values
		{
			if ( !itr->updateTime ) itr->autoUpdate = 0;
			if ( itr->autoUpdate && (!minUpdateTime || minUpdateTime && (itr->updateTime < minUpdateTime)) ) 
			{
				minUpdateTime = itr->updateTime;
			}		
		}
	}
	return minUpdateTime;
}

int Cloud::CloudThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id)
{
	Cloud *cloud = (Cloud *)user_data;
	if (kill)
	{
		WASABI_API_THREADPOOL->RemoveHandle(0, cloud->cloudEvent);
		CloseHandle(cloud->cloudEvent);
		WASABI_API_THREADPOOL->RemoveHandle(0, cloud->cloudTimerEvent);
		cloud->cloudTimerEvent.Close();
		SetEvent(cloud->cloudDone);
		return 0;
	}

	cloud->Callback();

	// set waitable timer, overwrite previouse value if any
	if (!kill)
	{
		__time64_t timeToWait = RetrieveMinimalUpdateTime();
		if ( timeToWait ) 
			cloud->cloudTimerEvent.Wait(timeToWait * 1000);
	}

	return 0;
}


Cloud::Cloud() : cloudThread(0), cloudEvent(0), statusText(0)
{}

Cloud::~Cloud()
{
	free(statusText);
}

void Cloud::Quit()
{
	cloudDone= CreateEvent(NULL, FALSE, FALSE, NULL);
	kill = true;
	SetEvent(cloudEvent);
	WaitForSingleObject(cloudDone, INFINITE);
	CloseHandle(cloudDone);
}

void Cloud::RefreshAll()
{
	AutoLock lock (channels LOCKNAME("RefreshAll"));
	ChannelList::iterator itr;
	for (itr = channels.begin();itr != channels.end();itr++)
	{
		itr->needsRefresh = true;
	}

}
void Cloud::Init()
{
	// setup a periodic callback so we can check on our times
	cloudEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	cloudThread = WASABI_API_THREADPOOL->ReserveThread(0);

	WASABI_API_THREADPOOL->AddHandle(cloudThread, cloudEvent, CloudThreadPoolFunc, this, 0, 0);

	WASABI_API_THREADPOOL->AddHandle(cloudThread, cloudTimerEvent, CloudThreadPoolFunc, this, 1, 0);
	__time64_t timeToWait = RetrieveMinimalUpdateTime();
	if ( timeToWait )
		cloudTimerEvent.Wait(timeToWait * 1000);
}

void Cloud::Refresh(Channel &channel)
{
	wchar_t lang_buf[1024] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_RECEIVING_UPDATES_FOR, lang_buf, 1024);
	if (channel.title)
		StringCbCat(lang_buf, sizeof(lang_buf), channel.title);
	else
		lang_buf[0]=0;
	SetStatus(lang_buf);
	size_t oldSize = channel.items.size();

	FeedParse downloader(&channelRefresher, false);
	downloader.DownloadURL(channel.url);

	if (channel.items.size() > oldSize)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_GOT_NEW_ITEMS_FOR, lang_buf, 1024);
		StringCbCat(lang_buf, sizeof(lang_buf), channel.title);
		SetStatus(lang_buf);
	}
	else
		SetStatus(L"");
}

void Cloud::GetStatus(wchar_t *status, size_t len)
{
		AutoLock lock (statusGuard);
		if (statusText)
			StringCchCopy(status, len, statusText);
		else
			status[0]=0;
}

void Cloud::SetStatus(const wchar_t *newStatus)
{
	AutoLock lock (statusGuard);
	free(statusText);
	statusText = _wcsdup(newStatus);

	HWND hView = SubscriptionView_FindWindow();
	if (NULL != hView)
		SubscriptionView_SetStatus(hView, statusText);
}

/* --- Private Methods of class Cloud --- */


static void ForceLastUpdate(const Channel &channel)
{
	AutoLock lock (channels LOCKNAME("ChannelRefresher::NewChannel"));
	ChannelList::iterator found;
	for (found=channels.begin();found!=channels.end(); found++)
	{
		if (!wcscmp(found->url, channel.url))
			break;
	}
	if (found != channels.end())
	{
		found->lastUpdate = _time64(0);
		found->needsRefresh = false;
	}
}
/*
@private
checks all channels and updates any that requiring refreshing.
*/
void Cloud::Callback()
{
	__time64_t curTime = _time64(0);

	size_t i = 0;
	Channel temp;
	bool refreshed = false;

	while (true) // we need to lock the channels object before we check its size, etc, so we can't just use a "for" loop.
	{
		{ // we want to minimize how long we have to lock, so we'll make a copy of the channel data
			AutoLock lock (channels LOCKNAME("Callback"));
			if (i >= channels.size())
				break;
			temp = channels[i]; // make a copy the data so we can safely release the lock
			channels[i].needsRefresh = false; // have to set this now.  if the site is down or 404, then the refresh will never "complete".
		} // end locking scope

		if (temp.needsRefresh) // need an immediate refresh? (usually set when the user clicks refresh or update-on-launch is on)
		{
			Refresh(temp);
			refreshed = true;
		}
		else if (temp.useDefaultUpdate) // this flag is set unless the user chose custom update values
		{
			if (!updateTime) autoUpdate = 0;
			if (autoUpdate && (temp.lastUpdate + updateTime) <= curTime)
			{
				Refresh(temp);
				ForceLastUpdate(temp);
				refreshed = true;
			}
		}
		else // use the custom values
		{
			if (temp.updateTime == 0) temp.autoUpdate = 0;
			if (temp.autoUpdate && (temp.lastUpdate + temp.updateTime) <= curTime)
			{
				Refresh(temp);
				ForceLastUpdate(temp);
				refreshed = true;
			}
		}

		i++;
	}

	// if we're refreshing then save out
	if (refreshed)
	{
		SaveAll();
	}
}