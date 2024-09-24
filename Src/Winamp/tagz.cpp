/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "tagz.h"
#include "../tagz/ifc_tagprovider.h"
#include "../tagz/ifc_tagparams.h"
#include "api.h"
#include "TagProvider.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"
extern "C"
{
	typedef char * (*GetTagFunc)(const char * tag, void * p); 
	typedef void (*FreeTagFunc)(char * tag,void * p);
	typedef wchar_t * (*GetTagFuncW)(const wchar_t * tag, void * p); 
	typedef void (*FreeTagFuncW)(wchar_t * tag,void * p);
}

class TagParameters : public ifc_tagparams
{
public:
	TagParameters(const wchar_t *_filename) :filename(_filename){}
	void *GetParameter(const GUID *parameterID)
	{
		if (*parameterID == filenameParameterID)
			return (void *)filename;
		return 0;
	}
protected:
	RECVS_DISPATCH;
private:
	const wchar_t *filename;
};

class ExternalTagProvider : public ifc_tagprovider
{
public:
	ExternalTagProvider(GetTagFunc getter, FreeTagFunc freer, void *context);
	wchar_t *GetTag(const wchar_t *name, ifc_tagparams *parameters);
	void FreeTag(wchar_t *Tag);
protected:
	RECVS_DISPATCH;
private:
	GetTagFunc externalGetTag;
	FreeTagFunc externalFreeTag;
	void *externalContext;
};

class ExtendedTagProvider : public ifc_tagprovider
{
public:
	ExtendedTagProvider(const wchar_t *_filename, GetTagFuncW getter, FreeTagFuncW freer, void *context, bool _useExtendedInfo);
	wchar_t *GetTag(const wchar_t *name, ifc_tagparams *parameters);
	void FreeTag(char *Tag);
protected:
	RECVS_DISPATCH;
private:
	GetTagFuncW externalGetTag;
	FreeTagFuncW externalFreeTag;
	void *externalContext;
	const wchar_t *filename;
	bool useExtendedInfo;
};

static void CleanupTitle(char *title)
{
	while (title && *title)
	{
		if (*title == '\n' || *title == '\r')
			*title = ' ';
		title = CharNextA(title);
	}
}

static void CleanupTitleW(wchar_t *title)
{
	while (title && *title)
	{
		if (*title == L'\n' || *title == L'\r')
			*title = L' ';
		title = CharNextW(title);
	}
}

void FormatTitle(waFormatTitle *format)
{
	AutoWide wideSpec(format->spec);
	wchar_t *spec = (format->spec ? wideSpec : config_titlefmt);

	if (format->out && format->out_len)
	{
		memset(format->out, 0, format->out_len);
		if (WINAMP5_API_TAGZ)
		{
			if (format->TAGFUNC)
			{
				ExternalTagProvider provider(format->TAGFUNC, format->TAGFREEFUNC, format->p);
				wchar_t *tempOut = (wchar_t *)calloc(format->out_len, sizeof(wchar_t));
				if (tempOut)
				{
					WINAMP5_API_TAGZ->format(spec, tempOut, format->out_len, &provider, 0 /*&parameters*/);
					WideCharToMultiByteSZ(CP_ACP, 0, tempOut, -1, format->out, format->out_len, 0, 0);
					free(tempOut);
				}
			}
			else
			{
				AutoWide wideFn((char *)format->p); // we'd better hope it's the filename!
				TagParameters parameters(wideFn);
				wchar_t *tempOut = (wchar_t *)calloc(format->out_len, sizeof(wchar_t));
				if (tempOut)
				{
					WINAMP5_API_TAGZ->format(spec, tempOut, format->out_len, tagProvider, &parameters);
					WideCharToMultiByteSZ(CP_ACP, 0, tempOut, -1, format->out, format->out_len, 0, 0);
					free(tempOut);
				}
			}
			CleanupTitle(format->out);
		}
	}
}

void FormatTitleExtended(waFormatTitleExtended *format)
{
	if (!format->filename || format->filename && !*format->filename) return;

	const wchar_t *spec = (format->spec ? format->spec : config_titlefmt);
	if (format->out && format->out_len)
	{
		if (WINAMP5_API_TAGZ)
		{
			ExtendedTagProvider provider(format->filename, format->TAGFUNC, format->TAGFREEFUNC, format->p, !!format->useExtendedInfo);
			WINAMP5_API_TAGZ->format(spec, format->out, format->out_len, &provider, 0 /*&parameters*/);
			CleanupTitleW(format->out);
		}
		else
		{
			lstrcpynW(format->out, format->filename, format->out_len);
			PathStripPathW(format->out);
		}
	}
}

int FormatTitle(waHookTitleStructW *hts)
{
	if (hts && hts->title && 
		((hts->force_useformatting&1) || !(PathIsURLW(hts->filename) && StrCmpNIW(hts->filename, L"cda://", 6))))
	{
		wchar_t buf[32] = {0};
		wchar_t buf2[32] = {0};
		hts->length = -1;
		hts->title[0] = 0;
		if (in_get_extended_fileinfoW(hts->filename, L"length", buf, 32) ||
			in_get_extended_fileinfoW(hts->filename, L"type", buf2, 32))
		{
			hts->length = StrToIntW(buf);
			if (hts->length <= 0)
				hts->length = -1;
			else 
				hts->length /= 1000;
			if (WINAMP5_API_TAGZ)
			{
				TagParameters parameters(hts->filename);

				WINAMP5_API_TAGZ->format(config_titlefmt, hts->title, 2048, tagProvider, &parameters);
			}
			CleanupTitleW(hts->title);
			if (hts->title[0]) return 1;
		}
	}
	return 0;
}

namespace Winamp
{
	/* our tag getting functions */
	wchar_t *GetTag(const wchar_t *tag, const wchar_t *filename) // for simple tags
	{
		if (!_wcsicmp(tag, L"filename"))
			return _wcsdup(filename);

		else if (!_wcsicmp(tag, L"folder"))
		{
			wchar_t *folder = (wchar_t *)calloc(MAX_PATH, sizeof(wchar_t));
			if (folder)
			{
				lstrcpynW(folder, filename, MAX_PATH);
				PathRemoveFileSpecW(folder);
				PathRemoveBackslashW(folder);
				PathStripPathW(folder);
				return folder;
			}
		}

		return 0;
	}

	wchar_t *GetExtendedTag(const wchar_t *tag, const wchar_t *filename) //return 0 if not found
	{
		wchar_t buf[1024] = {0};

		if (!_wcsicmp(tag, L"tracknumber")) 
			tag = L"track";

#if 0
		if (in_get_extended_fileinfo(filename, tag, buf, sizeof(buf)) && buf[0])
			return _strdup(buf);
#else
		extendedFileInfoStructW s = {filename, tag, buf, 1024};
		if (SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&s, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE) && buf[0])
			return _wcsdup(buf);
#endif
		return 0;
	}
} // namespace Winamp

/* --- ExternalTagProvider ---

maybe we'll move this to a separate file once it's done 
*/
ExternalTagProvider::ExternalTagProvider(GetTagFunc getter, FreeTagFunc freer, void *context)
: externalGetTag(getter), externalFreeTag(freer), externalContext(context)
{
}

wchar_t *ExternalTagProvider::GetTag(const wchar_t *name, ifc_tagparams *parameters)
{
	if (externalGetTag)
	{
		char *tag = externalGetTag(AutoChar(name), externalContext);
		if (tag == reinterpret_cast<char *>(-1))
			return 0;

		int size = MultiByteToWideChar(CP_ACP, 0, tag, -1, 0, 0);
		if (!size) 
		{
			if (externalFreeTag)
				externalFreeTag(tag, externalContext);
			return 0;
		}

		wchar_t *wide = (wchar_t *)calloc(size, sizeof(wchar_t));
		if (!MultiByteToWideChar(CP_ACP, 0, tag, -1, wide, size))
		{
			if (wide) free(wide);
			if (externalFreeTag)
				externalFreeTag(tag, externalContext);
			return 0;
		}

		if (externalFreeTag)
			externalFreeTag(tag, externalContext);
		return wide;
	}

	return 0;
}

void ExternalTagProvider::FreeTag(wchar_t *tag)
{
	if (tag) free(tag);
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS ExternalTagProvider
START_DISPATCH;
CB(IFC_TAGPROVIDER_GET_TAG, GetTag);
VCB(IFC_TAGPROVIDER_FREE_TAG, FreeTag);
END_DISPATCH;
#undef CBCLASS


/* --- ExtendedTagProvider ---

maybe we'll move this to a separate file once it's done 
*/
ExtendedTagProvider::ExtendedTagProvider(const wchar_t *_filename, GetTagFuncW getter, FreeTagFuncW freer, void *context, bool _useExtendedInfo)
: externalGetTag(getter), externalFreeTag(freer), externalContext(context), filename(_filename), useExtendedInfo(_useExtendedInfo)
{
}	

wchar_t *ExtendedTagProvider::GetTag(const wchar_t *name, ifc_tagparams *parameters)
{
	wchar_t *tag=0;
	if (externalGetTag)
		tag=externalGetTag(name, externalContext);

	if (tag) // if we got a tag, we'll need to make a copy, because we won't be able to tell where it came from when it's time to free it
	{
		if (tag == reinterpret_cast<wchar_t *>(-1))
			return 0;
		wchar_t *temp = *tag?_wcsdup(tag):0;
		if (externalFreeTag)
			externalFreeTag(tag, externalContext);
		return temp;
	}
	else
	{
		tag = Winamp::GetTag(name, filename);
		if (tag == reinterpret_cast<wchar_t *>(-1))
			return 0;
		if (!tag && useExtendedInfo)
			tag = Winamp::GetExtendedTag(name, filename);
		if (tag == reinterpret_cast<wchar_t *>(-1))
			return 0;
		return tag;		
	}
}

void ExtendedTagProvider::FreeTag(char *tag)
{
	if (tag) free(tag); 
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS ExtendedTagProvider
START_DISPATCH;
CB(IFC_TAGPROVIDER_GET_TAG, GetTag);
VCB(IFC_TAGPROVIDER_FREE_TAG, FreeTag);
END_DISPATCH;
#undef CBCLASS

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS TagParameters
START_DISPATCH;
CB(IFC_TAGPARAMS_GETPARAMETER, GetParameter);
END_DISPATCH;
#undef CBCLASS