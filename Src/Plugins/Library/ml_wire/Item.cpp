#include "Item.h"
#include "util.h"
#include "defaults.h"
#include <shlwapi.h>
#include <strsafe.h>

void RSS::Item::Reset()
{
	free(itemName);
	free(url);
	free(sourceUrl);
	free(guid);
	free(description);
	free(link);
	free(duration);
}

void RSS::Item::Init()
{
	listened = false;
	publishDate = 0;
	generatedDate = true;
	downloaded=false;
	itemName=0;
	url=0;
	sourceUrl=0;
	guid=0;
	description=0;
	link=0;
	duration=0;
	size=0;
}

RSS::Item::Item()
{
	Init();
}

RSS::Item::~Item()
{
	Reset();
}

RSS::Item::Item(const RSS::Item &copy)
{
	Init();
	operator =(copy);
}

const RSS::Item &RSS::Item::operator =(const RSS::Item &copy)
{
	Reset();
	Init();
	listened=copy.listened;
	publishDate = copy.publishDate;
	generatedDate = copy.generatedDate;
	downloaded=copy.downloaded;
	itemName=_wcsdup(copy.itemName);
	url=_wcsdup(copy.url);
	sourceUrl=_wcsdup(copy.sourceUrl);
	guid=_wcsdup(copy.guid);
	description=_wcsdup(copy.description);
	link=_wcsdup(copy.link);
	duration=wcsdup(copy.duration);
	size = copy.size;
	return *this;
}

HRESULT RSS::Item::GetDownloadFileName(const wchar_t *channelName, wchar_t *buffer, int bufferMax, BOOL fValidatePath) const
{
	if (NULL == buffer || NULL == channelName) return E_INVALIDARG;
	buffer[0] = L'\0';

	WCHAR szBuffer[MAX_PATH] = {0};
	
	if (FAILED(StringCchCopyN(szBuffer, ARRAYSIZE(szBuffer), channelName, 100)))
		return E_UNEXPECTED;
	
	Plugin_CleanDirectory(szBuffer);
	Plugin_ReplaceBadPathChars(szBuffer);

	if (L'\0' == *szBuffer)
		StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), L"UnknownChannel");

	if (FALSE == PathCombine(buffer, defaultDownloadPath, szBuffer))
		return E_FAIL;
	
	if (FALSE != fValidatePath && FAILED(Plugin_EnsurePathExist(buffer)))
		return E_FAIL;
	
	LPWSTR cursor = szBuffer;
	size_t remaining = ARRAYSIZE(szBuffer);

	tm* time = _localtime64(&publishDate);
	if(NULL != time && publishDate > 0) 
	{
		StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE,  
			L"%04d-%02d-%02d - ", time->tm_year+1900, time->tm_mon+1, time->tm_mday);
	}

	LPWSTR t = cursor;
	if (FAILED(StringCchCopyNEx(cursor, remaining, itemName, 100, &cursor, &remaining, 0)))
		return E_UNEXPECTED;

	INT offset = Plugin_CleanDirectory(t);
	if (0 != offset)
	{
		remaining += offset;
		cursor -= offset;
	}

	if (t == cursor)
		StringCchCopyEx(cursor, remaining, L"UnknownItem", &cursor, &remaining, 0);
	else
		Plugin_ReplaceBadPathChars(t);

	if (FAILED(Plugin_FileExtensionFromUrl(cursor, (INT)remaining, url, L".mp3")))
		return E_UNEXPECTED;

	if (FALSE == PathAppend(buffer, szBuffer))
		return E_FAIL;
	
	return S_OK;	
}

void RSS::MutableItem::SetLink(const wchar_t *value)
{
	free(link);
	link = _wcsdup(value);
}

void RSS::MutableItem::SetItemName(const wchar_t *value)
{
	free(itemName);
	itemName = _wcsdup(value);
}

void RSS::MutableItem::SetURL(const wchar_t *value)
{
	free(url);
	url = _wcsdup(value);
}

void RSS::MutableItem::SetSourceURL(const wchar_t *value)
{
	free(sourceUrl);
	sourceUrl = _wcsdup(value);
}

void RSS::MutableItem::SetGUID(const wchar_t *value)
{
	free(guid);
	guid = _wcsdup(value);
}

void RSS::MutableItem::SetDescription(const wchar_t *value)
{
	free(description);
	description = _wcsdup(value);
}

void RSS::MutableItem::SetDuration(const wchar_t *value)
{
	free(duration);
	duration = _wcsdup(value);
}

void RSS::MutableItem::SetSize(const wchar_t * _size)
{
	if (_size)
		size = _wtoi64(_size);
	else
		size=0;
}