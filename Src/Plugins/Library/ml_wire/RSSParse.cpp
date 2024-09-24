#include "Main.h"
#include "RSSParse.h"

#include "RFCDate.h"
#include "../xml/XMLNode.h"
#include "Feeds.h"
#include "Defaults.h"
#include "ChannelSync.h"
#include "ParseUtil.h"
#include <strsafe.h>

static void ReadWinampSpecificItem(const XMLNode *item, RSS::Item &newItem)
{
	newItem.listened = PropertyIsTrue(item, L"winamp:listened");
	newItem.downloaded = PropertyIsTrue(item, L"winamp:downloaded");
}

static void ReadRSSItem(XMLNode *item, Channel &channel, bool doWinampSpecificTags)
{
	RSS::MutableItem newItem;
	if (doWinampSpecificTags)
		ReadWinampSpecificItem(item, newItem);

	const wchar_t *pubdate = GetContent(item, L"pubDate");
	if (pubdate && pubdate[0])
	{
		newItem.publishDate = MakeRFCDate(pubdate);
		if (newItem.publishDate <= 0)
		{
			newItem.publishDate = _time64(0);
			newItem.generatedDate = true;
		}
		else
			newItem.generatedDate = false;
	}
	else
	{
		newItem.publishDate = _time64(0);
		newItem.generatedDate = true;
	}

	const wchar_t *itemName = GetContent(item, L"title");
	if (itemName && itemName[0])
		newItem.SetItemName(itemName);
	else
	{
		wchar_t date_str[128] = {0};
		MakeRFCDateString(newItem.publishDate, date_str, 128);
		newItem.SetItemName(date_str);
	}

	newItem.SetLink(GetContent(item, L"link"));
	newItem.SetURL(GetProperty(item, L"enclosure", L"url"));
	const wchar_t* src = GetProperty(item, L"source", L"src");
	if (!src || !src[0])
	{
		newItem.SetSourceURL(GetProperty(item, L"source", L"url"));
	}
	else
	{
		newItem.SetSourceURL(src/*GetProperty(item, L"source", L"src")*/);
	}
	newItem.SetSize(GetProperty(item, L"enclosure", L"length"));

	const wchar_t *guid = GetContent(item, L"guid");
	if (!guid || !guid[0])
		guid = GetProperty(item, L"enclosure", L"url");

	if (guid && guid[0])
	{
		newItem.SetGUID(guid);
	}
	else
	{
		wchar_t generated_guid[160] = {0};
		StringCbPrintf(generated_guid, sizeof(generated_guid), L"%s%s", channel.title, newItem.itemName);
		newItem.SetGUID(generated_guid);
	}

	const wchar_t *description = GetContent(item, L"description");
	if (!description || !description[0])
		description =  GetContent(item, L"content:encoded");
	newItem.SetDescription(description);

	const wchar_t *duration = GetContent(item, L"itunes:duration");
	if (duration && duration[0])
		newItem.SetDuration(duration);

	if (newItem.itemName && newItem.itemName[0])
	{
		channel.items.push_back(newItem);
	}
}

void ReadWinampSpecificChannel(const XMLNode *node, Channel &newChannel)
{
	const XMLNode *curNode = 0;
	const wchar_t *lastupdate = node->GetProperty(L"winamp:lastupdate");
	if (lastupdate && lastupdate[0])
		newChannel.lastUpdate = MakeRFCDate(lastupdate);

	const wchar_t *winamp_url = GetContent(node, L"winamp:url");
	newChannel.SetURL(winamp_url);

	// set to preference value first
	newChannel.updateTime = updateTime;
	newChannel.autoUpdate = autoUpdate;
	newChannel.autoDownload = autoDownload;
	newChannel.autoDownloadEpisodes = autoDownloadEpisodes;

	curNode = node->Get(L"winamp:update");
	if (curNode)
	{
		newChannel.useDefaultUpdate = PropertyIsTrue(curNode, L"usedefaultupdate");
		newChannel.needsRefresh = PropertyIsTrue(curNode, L"needsrefresh"); 
		if (!newChannel.useDefaultUpdate)
		{
			newChannel.updateTime = _wtoi64(curNode->GetProperty(L"updatetime"));
			newChannel.autoUpdate = PropertyIsTrue(curNode, L"autoupdate");
		}
	}

	curNode = node->Get(L"winamp:download");
	if (curNode) 
	{
		newChannel.autoDownload = PropertyIsTrue(curNode, L"autodownload");
		if (newChannel.autoDownload) 
		{
			const wchar_t *prop = curNode->GetProperty(L"autoDownloadEpisodes");
			if (prop)
				newChannel.autoDownloadEpisodes = _wtoi(prop);
		}
	}
}

static void ReadRSSChannel(const XMLNode *node, Channel &newChannel, bool doWinampSpecificTags)
{
	XMLNode::NodeList::const_iterator itemItr;
	if (doWinampSpecificTags)
		ReadWinampSpecificChannel(node, newChannel);

	const wchar_t *title = GetContent(node, L"title");
	newChannel.SetTitle(title);

	const wchar_t *link = GetContent(node, L"link");
	newChannel.SetLink(link);

	const wchar_t *description = GetContent(node, L"description");
	newChannel.SetDescription(description);

	const wchar_t *ttl = GetContent(node, L"ttl");
	if (ttl)
		newChannel.ttl = _wtoi(ttl);

	const XMLNode::NodeList *items = node->GetList(L"item");
	if (items)
	{
		for (itemItr = items->begin(); itemItr != items->end(); itemItr++)
			ReadRSSItem(*itemItr, newChannel, doWinampSpecificTags);
	}
}

void ReadRSS(const XMLNode *rss, ChannelSync *sync, bool doWinampSpecificTags, const wchar_t *url)
{
	XMLNode::NodeList::const_iterator itr;

	sync->BeginChannelSync();
	const XMLNode::NodeList *channelList = rss->GetList(L"channel");
	if (channelList)
	{
		for (itr = channelList->begin(); itr != channelList->end(); itr ++)
		{
			Channel newChannel;
			ReadRSSChannel(*itr, newChannel, doWinampSpecificTags);
			if (!newChannel.url || !newChannel.url[0])
				newChannel.SetURL(url);
			sync->NewChannel(newChannel);
		}
	}
	sync->EndChannelSync();
}