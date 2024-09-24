#include "main.h"
#include "FeedUtil.h"
#include "ChannelCheck.h"
#include "FeedParse.h"
#include "errors.h"
#include "./defaults.h"


int DownloadFeedInformation(Channel &newFeed)
{
	ChannelCheck check;
	FeedParse downloader(&check, false);

	int ret = downloader.DownloadURL(newFeed.url);
	if (ret != DOWNLOAD_SUCCESS)
		return ret;		

	if (!check.channel.title || !check.channel.title[0])
		return DOWNLOAD_NOTRSS;

	newFeed.SetTitle(check.channel.title);
	if (check.channel.ttl)
	{
		newFeed.updateTime = check.channel.ttl * 60;
		newFeed.autoUpdate = true;
	}
	else
	{
		newFeed.updateTime = ::updateTime;
		newFeed.autoUpdate = ::autoUpdate;
	}

	if (check.channel.url && check.channel.url[0])
		newFeed.SetURL(check.channel.url);

	return DOWNLOAD_SUCCESS;
}