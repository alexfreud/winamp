#include "main.h"
#include "./pcasturihandler.h"
#include "./Feeds.h"
#include "./FeedUtil.h"
#include "../nu/AutoLock.h"
#include "./wire.h"
#include "./errors.h"
//#include "../Agave/URIHandler/svc_urihandler.h"
//#include <api/service/waservicefactory.h>
#include "api__ml_wire.h"
#include "./cloud.h"
#include "./SubscriptionView.h"
#include "./resource.h"
#include "navigation.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include <strsafe.h>

using namespace Nullsoft::Utility;

extern ChannelList channels;
extern Cloud cloud;


static uint8_t quickhex(wchar_t c)
{
	int hexvalue = c;
	if (hexvalue & 0x10)
		hexvalue &= ~0x30;
	else
	{
		hexvalue &= 0xF;
		hexvalue += 9;
	}
	return hexvalue;
}

static uint8_t DecodeEscape(const wchar_t *&str)
{
	uint8_t a = quickhex(*++str);
	uint8_t b = quickhex(*++str);
	str++;
	return a * 16 + b;
}

static void DecodeEscapedUTF8(wchar_t *&output, const wchar_t *&input)
{
	uint8_t utf8_data[1024] = {0}; // hopefully big enough!!
	int num_utf8_words=0;
	bool error=false;

	while (input && *input == '%' && num_utf8_words < sizeof(utf8_data))
	{
		if (iswxdigit(input[1]) && iswxdigit(input[2]))
		{
			utf8_data[num_utf8_words++]=DecodeEscape(input);
		}
		else if (input[1] == '%')
		{
			input+=2;
			utf8_data[num_utf8_words++]='%';
		}
		else
		{
			error = true;
			break;
		}
	}

	int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)utf8_data, num_utf8_words, 0, 0);
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)utf8_data, num_utf8_words, output, len);
	output += len;

	if (error)
	{
		*output++ = *input++;
	}
}

static void UrlDecode(const wchar_t *input, wchar_t *output, size_t len)
{
	const wchar_t *stop = output+len-4; // give ourself a cushion large enough to hold a full UTF-16 sequence
	const wchar_t *itr = input;
	while (itr && *itr)
	{
		if (output >= stop)
		{
			*output=0;
			return;
		}

		switch (*itr)
		{
			case '%':
				DecodeEscapedUTF8(output, itr);
				break;
			case '&':
				*output = 0;
				return;
			default:
				*output++ = *itr++;
				break;
		}
	}
	*output = 0;
}
// first parameter has param name either null or = terminated, second is null terminated
static bool ParamCompare(const wchar_t *url_param, const wchar_t *param_name)
{
	while (url_param && *url_param && *param_name && *url_param!=L'=')
	{
		if (*url_param++ != *param_name++)
			return false;
	}
	return true;
}

static bool get_request_parm(const wchar_t *params, const wchar_t *param_name, wchar_t *value, size_t value_len)
{
	size_t param_name_len = wcslen(param_name);
	const wchar_t *t=params;
	while (t && *t && *t != L'?')  // find start of parameters
		t++;

	while (t && *t)
	{
		t++; // skip ? or &
		if (ParamCompare(t, param_name))
		{
			while (t && *t && *t != L'=' && *t != '&')  // find start of value
				t++;
			switch(*t)
			{
			case L'=':
				UrlDecode(++t, value, value_len);
					return true;
					case 0:
			case L'&': // no value
				*value=0;
				return true;
			default: // shouldn't get here
				return false;
			}
		}
	while (t && *t && *t != L'&')  // find next parameter
		t++;
  }
	
  return false;
}

int PCastURIHandler::ProcessFilename(const wchar_t *filename)
{
	if (
		(wcsnicmp(filename, L"pcast://", 8)) == 0 ||
		(wcsnicmp(filename, L"feed://", 7) == 0) ||
		(wcsnicmp(filename, L"winamp://Podcast/Subscribe", 26) == 0) ||
		(wcsnicmp(filename, L"winamp://Podcast/Search", 23) == 0)
		)
	{
		wchar_t *tempFilename = NULL;
		wchar_t url[1024] = {0};
		if (wcsnicmp(filename, L"winamp://Podcast/Subscribe", 26) == 0)
		{
			// extract/decode and use the url= parameter		
			if (get_request_parm(filename, L"url", url, 1024) && url[0])
			{
				tempFilename = wcsdup(url);
			}
			else
			{
				// did not find a url parameter
				return NOT_HANDLED;
			}
		}
		else if (wcsnicmp(filename, L"winamp://Podcast/Search", 23) == 0)
		{
// TODO: maybe:			if (get_request_parm(filename, L"url", url, 1024) && url[0])
			{
				HNAVITEM hItem = Navigation_FindService(SERVICE_PODCAST, NULL, NULL);
				MLNavItem_Select(plugin.hwndLibraryParent, hItem);
				return HANDLED;
			}
			/*
			else
			{
				// did not find a url parameter
				return NOT_HANDLED;
			}
			*/
		}
		else
		{
			// Use the full filename
			tempFilename = wcsdup(filename);
		}

		// subscription confirmation
		WCHAR szText[1024] = {0}, szBuffer[1024] = {0};
		WASABI_API_LNGSTRINGW_BUF(IDS_PODCAST_SUBSCRIPTION_PROMP, szBuffer, ARRAYSIZE(szBuffer));
		StringCchPrintf(szText, ARRAYSIZE(szText), szBuffer, tempFilename);
		
		WASABI_API_LNGSTRINGW_BUF(IDS_PODCAST_SUBSCRIPTION_HEADER, szBuffer, ARRAYSIZE(szBuffer));

		if (IDYES == MessageBox(plugin.hwndLibraryParent, szText, szBuffer, MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND) )
		{
			// add feed to channels, pulse the cloud, refresh the UI pane.
			Channel newFeed;
			newFeed.SetURL(tempFilename);
			if (DownloadFeedInformation(newFeed)==DOWNLOAD_SUCCESS)
			{
				channels.channelGuard.Lock();
				channels.AddChannel(newFeed);
				channels.channelGuard.Unlock();
				cloud.Pulse();
				HWND hView = SubscriptionView_FindWindow();
				if (NULL != hView)
				{
					SubscriptionView_RefreshChannels(hView, TRUE);
				}
				else
				{
					HNAVITEM myItem = Navigation_FindService(SERVICE_PODCAST, NULL, NULL);
					HNAVITEM podcastItem = MLNavItem_GetChild(plugin.hwndLibraryParent, myItem); 
					HNAVITEM subscriptionItem = Navigation_FindService(SERVICE_SUBSCRIPTION, podcastItem, NULL);
					MLNavItem_Select(plugin.hwndLibraryParent, subscriptionItem);
				}
			}
			free(tempFilename);

			return HANDLED;

		}
		else
			free(tempFilename);
	}
	return NOT_HANDLED;
}

int PCastURIHandler::IsMine(const wchar_t *filename)
{
	int i = 0;
	if (
		(wcsnicmp(filename, L"pcast://", 8)) == 0 ||
		(wcsnicmp(filename, L"feed://", 7) == 0) ||
		(wcsnicmp(filename, L"winamp://Podcast/Subscribe", 26) == 0) ||
		(wcsnicmp(filename, L"winamp://Podcast/Search", 23) == 0)
		)
		return HANDLED;
	else
		return NOT_HANDLED;
}

#define CBCLASS PCastURIHandler
START_DISPATCH;
CB(PROCESSFILENAME, ProcessFilename);
CB(ISMINE, IsMine);
END_DISPATCH;
#undef CBCLASS