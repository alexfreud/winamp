#include "main.h"
#include "navigation.h"
#include "servicehelper.h"
#include "handler.h"
#include "ifc_omservice.h"
#include "../Agave/URIHandler/svc_urihandler.h"
#include <api/service/waservicefactory.h>
#include "api.h"

#include <shlwapi.h>

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

int OnlineServicesURIHandler::ProcessFilename(const wchar_t *filename)
{
	if (HANDLED != IsMine(filename))
		return NOT_HANDLED;

	UINT serviceId = 0;
	wchar_t szBuffer[512]=L"";
	if (get_request_parm(filename, L"id", szBuffer, ARRAYSIZE(szBuffer)) && 
		L'\0' != szBuffer[0])
	{
		if (FALSE == StrToIntEx(szBuffer, STIF_SUPPORT_HEX, (INT*)&serviceId))
			serviceId = 0;
	}

	ServiceHelper_ShowService(serviceId, SHOWMODE_ENSUREVISIBLE);
	return HANDLED_EXCLUSIVE;
}

int OnlineServicesURIHandler::IsMine(const wchar_t *filename)
{
	if (!_wcsnicmp(filename, L"winamp://Online Services", 24) || !_wcsnicmp(filename, L"winamp://Online%20Services", 26))
		return HANDLED;
	else
		return NOT_HANDLED;
}

#define CBCLASS OnlineServicesURIHandler
START_DISPATCH;
CB(PROCESSFILENAME, ProcessFilename);
CB(ISMINE, IsMine);
END_DISPATCH;
#undef CBCLASS