#include "Main.h"
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

static void BuildXMLDownloads(FILE *fp, DownloadList &downloads)
{
	fputws(L"<downloads", fp);
	InstaProp(fp, L"downloadsTitleWidth",    downloadsTitleWidth);
	InstaProp(fp, L"downloadsProgressWidth", downloadsProgressWidth);
	InstaProp(fp, L"downloadsDateWidth",     downloadsDateWidth);
	InstaProp(fp, L"downloadsSourceWidth",   downloadsSourceWidth);
	InstaProp(fp, L"downloadsPathWidth",     downloadsPathWidth);
	InstaProp(fp, L"downloadsItemSort",      downloadsItemSort);
	InstaProp(fp, L"downloadsSortAscending", downloadsSortAscending);
	fputws(L">\r\n", fp);

	AutoLock lock (downloads);
	DownloadList::const_iterator itr;
	for ( itr = downloads.begin(); itr != downloads.end(); itr++ )
	{
		fputws(L"<download>", fp);
		InstaTag(fp, L"source",       itr->source);
		InstaTag(fp, L"title",        itr->title);
		InstaTag(fp, L"url",          itr->url);
		InstaTag(fp, L"path",         itr->path);
		InstaTag(fp, L"size",         (unsigned int)itr->totalSize);
		InstaTag(fp, L"downloadDate", itr->downloadDate);
		wchar_t status[64] = {0};
		_itow(itr->downloadStatus, status, 10);
		InstaTag(fp, L"downloadStatus", status);
		fputws(L"</download>\r\n", fp);
	}

	fputws(L"</downloads>", fp);
}

void SaveDownloads(const wchar_t *fileName, DownloadList &downloads)
{
	FILE *fp = _wfopen(fileName, L"wb");
	if (fp)
	{
		wchar_t BOM = 0xFEFF;
		fwrite(&BOM, sizeof(BOM), 1, fp);
		fputws(L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n", fp);
		BuildXMLDownloads(fp, downloads);
		fclose(fp);
	}
}