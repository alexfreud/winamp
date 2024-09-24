#include "urlmanager.h"

const wchar_t *URLManager::GetURL(const wchar_t *urlid)
{
	for (URLList::iterator itr=urls.begin();itr!=urls.end();itr++)
	{
		if (!_wcsicmp(urlid, itr->urlid))
		{
			return itr->url;
		}
	}
	return 0;
}

void URLManager::AddURL(const wchar_t *urlid, const wchar_t *url)
{
	for (URLList::iterator itr=urls.begin();itr!=urls.end();itr++)
	{
		if (!_wcsicmp(urlid, itr->urlid))
		{
			free(itr->url);
			itr->url=_wcsdup(url);
			return ;
		}
	}
	URLS newUrl;
	newUrl.urlid = _wcsdup(urlid);
	newUrl.url = _wcsdup(url);
	urls.push_back(newUrl);
}

#define CBCLASS URLManager
START_DISPATCH;
CB(API_URLMANAGER_GETURL, GetURL);
END_DISPATCH;
#undef CBCLASS