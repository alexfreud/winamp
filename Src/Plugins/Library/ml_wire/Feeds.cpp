#include "main.h"
#include "Feeds.h"
#include "./util.h"
#include "./defaults.h"
#include <algorithm>
#include <shlwapi.h>
#include "BackgroundDownloader.h"
#include <strsafe.h>

static bool operator == (const RSS::Item &a, const RSS::Item &b)
{
	if(a.guid && a.guid[0] && b.guid && b.guid[0])
		return !wcscmp(a.guid, b.guid);

	if(a.publishDate && b.publishDate)
		return a.publishDate == b.publishDate;

	if (a.url && a.url[0] && b.url && b.url[0])
		return !wcscmp(a.url, b.url);

	return a.url == b.url;
}

Channel::Channel()
{
	Init();
}

Channel::Channel(const Channel &copy)
{
	Init();
	operator =(copy);
}

const Channel &Channel::operator =(const Channel &copy)
{
	Reset();
	Init();
	url = _wcsdup(copy.url);
	title = _wcsdup(copy.title);
	link = _wcsdup(copy.link);
	description = _wcsdup(copy.description);
	ttl=copy.ttl;
	updateTime=copy.updateTime;
	lastUpdate=copy.lastUpdate;
	autoDownloadEpisodes=copy.autoDownloadEpisodes;
	autoDownload=copy.autoDownload;
	autoUpdate=copy.autoUpdate;
	useDefaultUpdate=copy.useDefaultUpdate;
	needsRefresh=copy.needsRefresh;
	items=copy.items;
	return *this;
}

Channel::~Channel()
{
	Reset();
}

void Channel::Init()
{
	url =0;
	title = 0;
	link = 0;
	description = 0;
	ttl=0;
	lastUpdate=0;
	autoUpdate = ::autoUpdate;
	updateTime = ::updateTime;
	autoDownload = ::autoDownload; 
	autoDownloadEpisodes = ::autoDownloadEpisodes;
	useDefaultUpdate=true;
	needsRefresh=false;
}

void Channel::Reset()
{
	free(url);
	free(title);
	free(link);
	free(description);
}

void Channel::UpdateFrom(const Channel &copy)
{
	if (copy.url && copy.url[0])
		SetURL(copy.url);

	SetTitle(copy.title);
	SetLink(copy.link);
	SetDescription(copy.description);
	if (copy.ttl)
		ttl=copy.ttl;

	ItemList::const_iterator itr;

	for (itr=copy.items.begin();itr!=copy.items.end();itr++)
	{
		const RSS::Item &b = *itr;
		if ( b.url && b.url[0] ) 
		{
			((RSS::Item*)&b)->downloaded = IsPodcastDownloaded(b.url);
		}
	}

	// update to the latest default setting
	if (useDefaultUpdate)
	{
		autoUpdate = ::autoUpdate;
		updateTime = ::updateTime;
		autoDownload = ::autoDownload;
		autoDownloadEpisodes = ::autoDownloadEpisodes;
	}

	items.clear(); // benski> added for 5.23
	for (itr=copy.items.begin();itr!=copy.items.end();itr++)
	{
		items.insert(items.begin(), *itr);
	}
	
	if(autoDownload)
	{
		SortByDate();
		size_t idx = items.size();
		if (idx)
		{
			int episodeCount = 0;
			do
			{
				idx--;
				const RSS::Item &b = items[idx];
				if(b.url && b.url[0]) 
				{
					episodeCount++;
					if (!b.downloaded)
					{
						WCHAR szPath[MAX_PATH *2] = {0};
						if (SUCCEEDED(((RSS::Item*)&b)->GetDownloadFileName(title, szPath, ARRAYSIZE(szPath), TRUE)))
						{
							wchar_t* url = urlencode(b.url);
							downloader.Download(url, szPath, title, b.itemName, b.publishDate);
							((RSS::Item*)&b)->downloaded = true;
							free(url);
						}
					}

				}
			} while (episodeCount<autoDownloadEpisodes && idx);
		}
	}
}

bool Channel::operator == (const Channel &compare)
{
	// changed from basing on the title as this allows for podcasts
	// with the same name to still work instead of being mangled as
	// was able to happen when this based things on the title value
	if (!compare.url || !compare.url[0])
		return false;
	return !wcscmp(url, compare.url);
}

bool TitleMediaSort(const RSS::Item &item1, const RSS::Item &item2)
{
	return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, item1.itemName, -1,  item2.itemName, -1));
}

void Channel::SortByTitle()
{
	std::sort(items.begin(), items.end(), TitleMediaSort);
}

static bool ItemMediaSort(const RSS::Item &item1, const RSS::Item &item2)
{
	if (!item1.url || !item1.url[0])
		return false;

	if (!item2.url || !item2.url[0])
		return true;

	if (!item2.listened)
		return false;

	if (item1.listened)
		return false;
	return true;
}

bool ParseDuration(const wchar_t *duration, int *out_hours, int *out_minutes, int *out_seconds);
static bool ItemMediaTimeSort(const RSS::Item &item1, const RSS::Item &item2)
{
	if (!item1.duration || !item1.duration[0])
		return false;

	if (!item2.duration || !item2.duration[0])
		return true;

	int h1, h2, m1, m2, s1, s2;
	if (!ParseDuration(item1.duration, &h1, &m1, &s1))
		return false;

	if (!ParseDuration(item2.duration, &h2, &m2, &s2))
		return true;

	if (h1 < h2)
		return true;
	else if (h1 > h2)
		return false;

	if (m1 < m2)
		return true;
	else if (m1 > m2)
		return false;

	if (s1 < s2)
		return true;
	else 
		return false;
}

static bool ItemMediaSizeSort(const RSS::Item &item1, const RSS::Item &item2)
{
	if (!item1.size)
		return false;

	if (!item2.size)
		return true;

	return item1.size < item2.size;
}

void Channel::SortByMedia()
{
	std::sort(items.begin(), items.end(), ItemMediaSort);
}

void Channel::SortByMediaTime()
{
	std::sort(items.begin(), items.end(), ItemMediaTimeSort);
}

void Channel::SortByMediaSize()
{
	std::sort(items.begin(), items.end(), ItemMediaSizeSort);
}

bool ItemDateSort(const RSS::Item &item1, const RSS::Item &item2)
{
	return (item1.publishDate < item2.publishDate);
}

void Channel::SortByDate() 
{
	std::sort(items.begin(), items.end(), ItemDateSort);
}

int Channel::GetTitle(wchar_t *str, size_t len)
{
	if (str && len)
	{
		str[0]=0;
		if (title && title[0])
			StringCchCopyW(str, len, title);
		return 0;
	}
	return 1;
}

void Channel::SetURL(const wchar_t *val)
{
	free(url);
	url = _wcsdup(val);
}

void Channel::SetTitle(const wchar_t *val)
{
	free(title);
	title = _wcsdup(val);
}

void Channel::SetLink(const wchar_t *val)
{
	free(link);
	link = _wcsdup(val);
}

void Channel::SetDescription(const wchar_t *val)
{
	free(description);
	description = _wcsdup(val);
}

#undef CBCLASS
#define CBCLASS Channel
START_DISPATCH;
CB(IFC_PODCAST_GETTITLE, GetTitle)
END_DISPATCH;
#undef CBCLASS