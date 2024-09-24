#include "Main.h"
#include "Feeds.h"
#include "RFCDate.h"
#include "Downloaded.h"
#include "../nu/AutoLock.h"
#include "Defaults.h"
#include "../Agave/Language/api_language.h"

using namespace Nullsoft::Utility;

static void XMLWriteString(FILE *fp, const wchar_t *str)
{
	for (const wchar_t *itr = str; *itr; itr=CharNextW(itr))
	{
		switch (*itr)
		{
			case '<':	fputws(L"&lt;", fp);	break;
			case '>':	fputws(L"&gt;", fp);	break;
			case '&':	fputws(L"&amp;", fp);	break;
			case '\"':	fputws(L"&quot;", fp);	break;
			case '\'':	fputws(L"&apos;", fp);	break;
			default:	fputwc(*itr, fp);		break;
		}
	}
}

static void InstaProp(FILE *fp, const wchar_t *property, const wchar_t *value)
{
	fwprintf(fp, L" %s=\"", property);
	XMLWriteString(fp, value);
	fputws(L"\"", fp);
}

static void InstaProp(FILE *fp, const wchar_t *property, int value)
{
	fwprintf(fp, L" %s=\"%i\"", property, value);
}

static void InstaPropI64(FILE *fp, const wchar_t *property, int64_t value)
{
	fwprintf(fp, L" %s=\"%I64d\"", property, value);
}

static void InstaPropTime(FILE *fp, const wchar_t *property, __time64_t value)
{
	fwprintf(fp, L" %s=\"%I64u\"", property, value);
}

static void InstaProp(FILE *fp, const wchar_t *property, float value)
{
	_fwprintf_l(fp, L" %s=\"%3.3f\"", WASABI_API_LNG->Get_C_NumericLocale(), property, value);
}

static void InstaProp(FILE *fp, const wchar_t *property, bool value)
{
	fwprintf(fp, L" %s=\"", property);
	if (value)
		fputws(L"true\"", fp);
	else
		fputws(L"false\"", fp);
}

static void InstaTag(FILE *fp, const wchar_t *tag, const wchar_t *content)
{
	if (content && !((INT_PTR)content < 65536) && *content)
	{
		fwprintf(fp, L"<%s>", tag);
		XMLWriteString(fp, content);
		fwprintf(fp, L"</%s>\r\n", tag);
	}
}

static void InstaTag(FILE *fp, const wchar_t *tag, unsigned int uint)
{
	fwprintf(fp, L"<%s>%u</%s>", tag, uint, tag);
}

static void InstaTag(FILE *fp, const wchar_t *tag, __time64_t uint)
{
	fwprintf(fp, L"<%s>%I64u</%s>", tag, uint, tag);
}

static void BuildXMLString(FILE *fp, const RSS::Item &item)
{
	fputws(L"<item", fp);
	InstaProp(fp, L"winamp:listened", item.listened);
	InstaProp(fp, L"winamp:downloaded", item.downloaded);
	fputws(L">\r\n", fp);

	InstaTag(fp, L"title", item.itemName);

	if (item.url && item.url[0])
	{
		fputws(L"<enclosure", fp);
		InstaProp(fp, L"url", item.url);
		InstaPropI64(fp, L"length", item.size);
		fputws(L"/>\r\n", fp);
	}

	InstaTag(fp, L"guid",        item.guid);
	InstaTag(fp, L"description", item.description);
	InstaTag(fp, L"link",        item.link);
	
	if (item.duration && item.duration[0])
	{
		InstaTag(fp, L"itunes:duration", item.duration);
	}

	if (item.publishDate)
	{
		wchar_t date_str[128] = {0};
		MakeRFCDateString(item.publishDate, date_str, 128);
		InstaTag(fp, L"pubDate", date_str);
	}

	if (item.sourceUrl && item.sourceUrl[0])
	{
		fputws(L"<source",fp);
		InstaProp(fp, L"src", item.sourceUrl);
		fputws(L"/>\r\n", fp);
	}

	fputws(L"</item>\r\n", fp);
}

static void BuildXMLString(FILE *fp, Channel &channel)
{
	fputws(L"<channel", fp);

	wchar_t date_str[128] = {0};
	MakeRFCDateString(channel.lastUpdate, date_str, 128);

	InstaProp(fp, L"winamp:lastupdate", date_str);
	fputws(L">\r\n", fp);

	// write update settings for this channel
	fputws(L"<winamp:update", fp);
	InstaProp(fp, L"usedefaultupdate", channel.useDefaultUpdate);
	InstaProp(fp, L"needsrefresh",     channel.needsRefresh);


	if (!channel.useDefaultUpdate)
	{
		InstaProp(fp, L"autoupdate", channel.autoUpdate);
		InstaPropTime(fp, L"updatetime", channel.updateTime);
	}
	fputws(L"/>\r\n", fp);

	if (!channel.useDefaultUpdate)
	{
		fputws(L"<winamp:download", fp);
		InstaProp(fp, L"autodownload", channel.autoDownload);
		
		if (channel.autoDownload)
		{
			InstaProp(fp, L"autoDownloadEpisodes", channel.autoDownloadEpisodes);
		}
		fputws(L"/>\r\n", fp);
	}

	InstaTag(fp, L"winamp:url",  channel.url);
	InstaTag(fp, L"title",       channel.title);
	InstaTag(fp, L"link",        channel.link);
	InstaTag(fp, L"description", channel.description);

	if (channel.ttl)
		InstaTag(fp, L"ttl", channel.ttl);
	fputws(L"\r\n", fp);

	Channel::ItemList::iterator itemItr;
	for (itemItr = channel.items.begin();itemItr != channel.items.end(); itemItr++)
		BuildXMLString(fp, *itemItr);

	fputws(L"</channel>\r\n", fp);
}

void SaveChannels(const wchar_t *fileName, ChannelList &channels)
{
	FILE *fp = _wfopen(fileName, L"wb");
	if (fp)
	{
		ChannelList::iterator itr;
		wchar_t BOM = 0xFEFF;
		fwrite(&BOM, sizeof(BOM), 1, fp);
		fputws(L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n", fp);
		fputws(L"<rss version=\"2.0\" xmlns:winamp=\"http://www.winamp.com\">\r\n", fp);

		for (itr = channels.begin();itr != channels.end();itr++)
			BuildXMLString(fp, *itr);

		fputws(L"</rss>", fp);
		fclose(fp);
	}
}

static void BuildXMLPreferences(FILE *fp)
{
	fputws(L"<download", fp);
	InstaProp(fp, L"downloadpath",           defaultDownloadPath);
	InstaProp(fp, L"autodownload",           autoDownload);
	InstaProp(fp, L"autoDownloadEpisodes",   autoDownloadEpisodes);
	InstaProp(fp, L"needToMakePodcastsView", needToMakePodcastsView);
	fputws(L"/>\r\n", fp);

	fputws(L"<update", fp);
	InstaPropTime(fp, L"updatetime", updateTime);
	InstaProp(fp, L"autoupdate",             autoUpdate);
	InstaProp(fp, L"updateonlaunch",         updateOnLaunch);
	fputws(L"/>\r\n", fp);

	fputws(L"<subscriptions", fp);
	InstaProp(fp, L"htmldivider",            htmlDividerPercent);
	InstaProp(fp, L"channeldivider",         channelDividerPercent);
	InstaProp(fp, L"itemtitlewidth",         itemTitleWidth);
	InstaProp(fp, L"itemdatewidth",          itemDateWidth);
	InstaProp(fp, L"itemmediawidth",         itemMediaWidth);
	InstaProp(fp, L"itemsizewidth",          itemSizeWidth);
	InstaProp(fp, L"currentitemsort",        currentItemSort);
	InstaProp(fp, L"itemsortascending",      itemSortAscending);
	InstaProp(fp, L"channelsortascending",   channelSortAscending);
	InstaProp(fp, L"channelLastSelection",   channelLastSelection);
	fputws(L"/>\r\n", fp);

	fputws(L"<downloadsView", fp);
	InstaProp(fp, L"downloadsChannelWidth",  downloadsChannelWidth);
	InstaProp(fp, L"downloadsItemWidth",     downloadsItemWidth);
	InstaProp(fp, L"downloadsProgressWidth", downloadsProgressWidth);
	InstaProp(fp, L"downloadsPathWidth",     downloadsPathWidth);

	InstaProp(fp, L"downloadsItemSort",      downloadsItemSort);
	InstaProp(fp, L"downloadsSortAscending", downloadsSortAscending);
	fputws(L"/>\r\n", fp);

	fputws(L"<service", fp);
	InstaProp(fp, L"url", serviceUrl);
	fputws(L"/>\r\n", fp);
}

static void BuildXMLDownloads(FILE *fp, DownloadList &downloads)
{
	fputws(L"<downloads>", fp);
	AutoLock lock (downloads);
	DownloadList::const_iterator itr;
	for ( itr = downloads.begin(); itr != downloads.end(); itr++ )
	{
		fputws(L"<download>", fp);
		InstaTag(fp, L"channel",     itr->channel);
		InstaTag(fp, L"item",        itr->item);
		InstaTag(fp, L"url",         itr->url);
		InstaTag(fp, L"path",        itr->path);
		InstaTag(fp, L"publishDate", itr->publishDate);
		fputws(L"</download>\r\n", fp);
	}

	fputws(L"</downloads>", fp);
}

void SaveSettings(const wchar_t *fileName, DownloadList &downloads)
{
	FILE *fp = _wfopen(fileName, L"wb");
	if (fp)
	{
		wchar_t BOM = 0xFEFF;
		fwrite(&BOM, sizeof(BOM), 1, fp);
		fputws(L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n", fp);
		fputws(L"<winamp:preferences xmlns:winamp=\"http://www.winamp.com\" version=\"2\">\r\n", fp);
		BuildXMLPreferences(fp);
		BuildXMLDownloads(fp, downloads);
		fputws(L"</winamp:preferences>", fp);
		fclose(fp);
	}
}