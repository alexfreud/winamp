/*
** Copyright (C) 2007-2011 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
** Author: Ben Allison benski@winamp.com
** Created: March 1, 2007
**
*/

#include "main.h"
#include "Metadata.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoChar.h"
#include "../Winamp/wa_ipc.h"
#include <shlwapi.h>
#include "resource.h"
#include "../Agave/Language/api_language.h"
#include "Stopper.h"
#include <strsafe.h>

static int FillFileInfo(wchar_t *infoStr, size_t len, FLACMetadata &metadata)
{
	const FLAC__StreamMetadata_StreamInfo *info = metadata.GetStreamInfo();
	if (info)
	{
		unsigned __int64 length = info->total_samples / info->sample_rate;
		StringCchPrintfExW(infoStr, len, &infoStr, &len, 0, WASABI_API_LNGSTRINGW(IDS_LENGTH_IN_SECONDS), length);
		StringCchPrintfExW(infoStr, len, &infoStr, &len, 0, WASABI_API_LNGSTRINGW(IDS_CHANNELS), info->channels);
		StringCchPrintfExW(infoStr, len, &infoStr, &len, 0, WASABI_API_LNGSTRINGW(IDS_BITS_PER_SAMPLE), info->bits_per_sample);
		StringCchPrintfExW(infoStr, len, &infoStr, &len, 0, WASABI_API_LNGSTRINGW(IDS_SAMPLE_RATE), info->sample_rate);
		__int64 filesize = metadata.GetFileSize();
		StringCchPrintfExW(infoStr, len, &infoStr, &len, 0, WASABI_API_LNGSTRINGW(IDS_FILE_SIZE_IN_BYTES), filesize);
		if (info->total_samples)
		{
			StringCchPrintfExW(infoStr, len, &infoStr, &len, 0, WASABI_API_LNGSTRINGW(IDS_AVERAGE_BITRATE), filesize / (125*info->total_samples / (__int64)info->sample_rate)); // (125 is 1000/8)
			int percent = (int)((100*filesize) / (info->total_samples * (info->bits_per_sample/8) * info->channels));
			StringCchPrintfExW(infoStr, len, &infoStr, &len, 0, WASABI_API_LNGSTRINGW(IDS_COMPRESSION_RATIO), percent);
		}
		return 1;
	}
	return 0;
}

bool KeywordMatch(const char *mainString, const char *keyword)
{
	return !_stricmp(mainString, keyword);
}

Info *info = 0;
FLACMetadata *getMetadata = 0;
wchar_t *getFileInfoFn = 0;
static FILETIME ftLastWriteTime;

// is used to determine if the last write time of the file has changed when
// asked to get the metadata for the same cached file so we can update things
BOOL HasFileTimeChanged(const wchar_t *fn)
{
	WIN32_FILE_ATTRIBUTE_DATA fileData = {0};
	if (GetFileAttributesExW(fn, GetFileExInfoStandard, &fileData) == TRUE)
	{
		if(CompareFileTime(&ftLastWriteTime, &fileData.ftLastWriteTime))
		{
			ftLastWriteTime = fileData.ftLastWriteTime;
			return TRUE;
		}
	}
	return FALSE;
}

void UpdateFileTimeChanged(const wchar_t *fn)
{
	WIN32_FILE_ATTRIBUTE_DATA fileData;
	if (GetFileAttributesExW(fn, GetFileExInfoStandard, &fileData) == TRUE)
	{
		ftLastWriteTime = fileData.ftLastWriteTime;
	}
}

void ResetMetadataCache()
{
	// cheap way to trigger a metadata reset in a thread-safe manner
	wchar_t d[10] = {0};
	extendedFileInfoStructW reset_info = {0};
	reset_info.filename=L".flac";
	reset_info.metadata=L"artist";
	reset_info.ret = d;
	reset_info.retlen=10;
	SendMessage(plugin.hMainWindow, WM_WA_IPC, (WPARAM)&reset_info, IPC_GET_EXTENDED_FILE_INFOW);
}

#define START_TAG_ALIAS(name, alias) if (KeywordMatch(data, name)) lookup=alias
#define TAG_ALIAS(name, alias) else if (KeywordMatch(data, name)) lookup=alias
extern "C" __declspec( dllexport ) int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, int destlen)
{
	if (KeywordMatch(data, "type"))
	{
		dest[0] = '0'; 
		dest[1] = 0;
		return 1;
	}
	if (KeywordMatch(data, "rateable"))
	{
		dest[0] = '1'; 
		dest[1] = 0;
		return 1;
	}
	else if (KeywordMatch(data, "lossless"))
	{
		dest[0] = '1'; 
		dest[1] = 0;
		return 1;
	}

	if (!fn || (fn && !fn[0]))
		return 0;

	if (KeywordMatch(data, "family"))
	{  
		LPCWSTR e;
		int pID = -1;
		DWORD lcid;
		e = PathFindExtensionW(fn);
		if (L'.' != *e) return 0;
		e++;
		lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
		if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"FLAC", -1) ||  
			CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"FLA", -1)) pID = IDS_FAMILY_STRING;
		
		if (pID != -1 && S_OK == StringCchCopyW(dest, destlen, WASABI_API_LNGSTRINGW(pID))) return 1;
		return 0;
	}

	if (KeywordMatch(data, "mime"))
	{
		StringCchCopyW(dest, destlen, L"audio/flac");
		return 1;
	}

	if (!getMetadata || !getFileInfoFn || _wcsicmp(fn, getFileInfoFn) || HasFileTimeChanged(fn))
	{
		if (getMetadata)
			getMetadata->Reset();
		else
			getMetadata = new FLACMetadata;
	
		if (!getMetadata->Open(fn))
		{
			delete getMetadata;
			getMetadata = 0;		
			dest[0]=0;
			return 0;
		}
		free(getFileInfoFn);
		getFileInfoFn = _wcsdup(fn);
	}

	FLACMetadata &metadata = *getMetadata;

	if(KeywordMatch(data, "formatinformation"))
		return FillFileInfo(dest,destlen,metadata);

	const char *lookup=0;
	if (KeywordMatch(data, "length"))
	{
		unsigned __int64 length_in_msec;
		if (metadata.GetLengthMilliseconds(&length_in_msec))
			StringCchPrintfW(dest, destlen, L"%d", length_in_msec);
		else
			dest[0]=0;
		return 1;
	}
	else if (KeywordMatch(data, "bitrate"))
	{
		// TODO: move this into FLACMetadata
		const FLAC__StreamMetadata_StreamInfo *streaminfo = metadata.GetStreamInfo();
		if (streaminfo)
		{
			if (streaminfo->total_samples == 0 || streaminfo->sample_rate == 0) // prevent divide-by-zero
				dest[0]=0;
			else
				StringCchPrintfW(dest, destlen, L"%I64d", metadata.GetFileSize() / (125*streaminfo->total_samples / (__int64)streaminfo->sample_rate)); // (125 is 1000/8)
		}
		else
			dest[0]=0;
		return 1;
	}
	TAG_ALIAS("title", "TITLE");
	TAG_ALIAS("artist", "ARTIST");
	TAG_ALIAS("album", "ALBUM");
	TAG_ALIAS("genre", "GENRE");
	TAG_ALIAS("comment", "COMMENT");
	TAG_ALIAS("year", "DATE");
	TAG_ALIAS("track", "TRACKNUMBER");
	TAG_ALIAS("albumartist", "ALBUM ARTIST");
	TAG_ALIAS("composer", "COMPOSER");
	TAG_ALIAS("disc", "DISCNUMBER");
	TAG_ALIAS("publisher", "PUBLISHER");
	TAG_ALIAS("conductor", "CONDUCTOR");
	TAG_ALIAS("tool", "ENCODED-BY");
	TAG_ALIAS("replaygain_track_gain", "REPLAYGAIN_TRACK_GAIN");
	TAG_ALIAS("replaygain_track_peak", "REPLAYGAIN_TRACK_PEAK");
	TAG_ALIAS("replaygain_album_gain", "REPLAYGAIN_ALBUM_GAIN");
	TAG_ALIAS("replaygain_album_peak", "REPLAYGAIN_ALBUM_PEAK");
	TAG_ALIAS("GracenoteFileID", "GRACENOTEFILEID");
	TAG_ALIAS("GracenoteExtData", "GRACENOTEEXTDATA");
	TAG_ALIAS("bpm", "BPM");
	TAG_ALIAS("remixing", "REMIXING");
	TAG_ALIAS("subtitle", "VERSION");
	TAG_ALIAS("isrc", "ISRC");
	TAG_ALIAS("category", "CATEGORY");
	TAG_ALIAS("rating", "RATING");
	TAG_ALIAS("producer", "PRODUCER");
	
	if (!lookup)
		return 0;

	const char *value = metadata.GetMetadata(lookup);

	if(KeywordMatch("comment",data)) {
		if(!value || !*value) value = metadata.GetMetadata("DESCRIPTION");
	}

	if(KeywordMatch("year",data)) {
		if(!value || !*value) value = metadata.GetMetadata("YEAR");
	}

	if(KeywordMatch("track",data)) {
		if(!value || !*value) value = metadata.GetMetadata("TRACK");
	}

	if(KeywordMatch("albumartist",data)) {
		if(!value || !*value) value = metadata.GetMetadata("ALBUMARTIST");
		if(!value || !*value) value = metadata.GetMetadata("ENSEMBLE");
	}

	if(KeywordMatch("publisher",data)) {
		if(!value || !*value) value = metadata.GetMetadata("ORGANIZATION");
	}

	if(KeywordMatch("category",data)) {
		if(!value || !*value) value = metadata.GetMetadata("CONTENTGROUP");
		if(!value || !*value) value = metadata.GetMetadata("GROUPING");
	}

	if(KeywordMatch(data, "rating")) {
		if(!value || !*value) value = metadata.GetMetadata("RATING");
		if(value && *value) {
			int rating = atoi(value);

			// keeps things limited to our range of 0-100
			if (rating >= 100) {
				rating = 5;
			}
			// 1-100 case
			else if (rating > 5 && rating < 100) {
				rating = (rating /= 20);
				// shift up by one rating when in next band
				// 1-20 = 1, 21-40 = 2, 41-60 = 3, 61-80 = 4, 81-100 = 5
				rating += ((atoi(value) - (rating * 20)) > 0);
			}
			else if (rating > 0 && rating <= 5) {
			}
			// otherwise just make sure and set zero
			else {
				rating = 0;
			}

			StringCchPrintfW(dest, destlen, L"%u", rating);
			return 1;
		}
	}

	MultiByteToWideCharSZ(CP_UTF8, 0, value, -1, dest, destlen);
	return 1;
}

FLACMetadata *setMetadata=0;
wchar_t *setFn=0;
extern "C" __declspec( dllexport ) int winampSetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *val)
{
	if (!setMetadata || !setFn || lstrcmpiW(fn, setFn))
	{
		free(setFn);
		setFn=_wcsdup(fn);
		if (!setMetadata)
			setMetadata = new FLACMetadata;
		if (setMetadata->Open(setFn, true) == false)
		{
			delete setMetadata;
			setMetadata=0;
			return 0;
		}
	}

	const char *lookup=0;
	START_TAG_ALIAS("artist", "ARTIST");
	TAG_ALIAS("title", "TITLE");
	TAG_ALIAS("album", "ALBUM");
	TAG_ALIAS("genre", "GENRE");
	TAG_ALIAS("comment", "COMMENT");
	TAG_ALIAS("year", "DATE");
	TAG_ALIAS("track", "TRACKNUMBER");
	TAG_ALIAS("albumartist", "ALBUM ARTIST");
	TAG_ALIAS("composer", "COMPOSER");
	TAG_ALIAS("disc", "DISCNUMBER");
	TAG_ALIAS("publisher", "PUBLISHER");
	TAG_ALIAS("conductor", "CONDUCTOR");
	TAG_ALIAS("tool", "ENCODED-BY");
	TAG_ALIAS("replaygain_track_gain", "REPLAYGAIN_TRACK_GAIN");
	TAG_ALIAS("replaygain_track_peak", "REPLAYGAIN_TRACK_PEAK");
	TAG_ALIAS("replaygain_album_gain", "REPLAYGAIN_ALBUM_GAIN");
	TAG_ALIAS("replaygain_album_peak", "REPLAYGAIN_ALBUM_PEAK");
	TAG_ALIAS("GracenoteFileID", "GRACENOTEFILEID");
	TAG_ALIAS("GracenoteExtData", "GRACENOTEEXTDATA");
	TAG_ALIAS("bpm", "BPM");
	TAG_ALIAS("remixing", "REMIXING");
	TAG_ALIAS("subtitle", "VERSION");
	TAG_ALIAS("isrc", "ISRC");
	TAG_ALIAS("category", "CATEGORY");
	TAG_ALIAS("rating", "RATING");
	TAG_ALIAS("producer", "PRODUCER");

	if (!lookup)
		return 0;

	if (val && *val)
	{
		if(KeywordMatch("rating",data)) 
		{
			char temp[128] = {0};
			StringCchPrintfA(temp, 128, "%u", _wtoi(val)*20);
			setMetadata->SetMetadata(lookup, temp);
		}
		else
		{
			setMetadata->SetMetadata(lookup, AutoChar(val, CP_UTF8));
		}
	}
	else
	{
		setMetadata->RemoveMetadata(lookup);
		if(KeywordMatch("comment",data)) 
		{
			// need to remove this one also, or else it's gonna look like delete doesn't work
			// if the file was tagged using this alternate field
			setMetadata->RemoveMetadata("DESCRIPTION");
		}
		else if(KeywordMatch("year",data)) 
		{
			// need to remove this one also, or else it's gonna look like delete doesn't work
			// if the file was tagged using this alternate field
			setMetadata->RemoveMetadata("YEAR");
		}
		else if(KeywordMatch("track",data)) 
		{
			// need to remove this one also, or else it's gonna look like delete doesn't work
			// if the file was tagged using this alternate field
			setMetadata->RemoveMetadata("TRACK");
		}
		else if(KeywordMatch("albumartist",data)) 
		{
			// need to remove these two, also, or else it's gonna look like delete doesn't work
			// if the file was tagged using these alternate fields
			setMetadata->RemoveMetadata("ALBUMARTIST");
			setMetadata->RemoveMetadata("ENSEMBLE");
		}
		else if(KeywordMatch("publisher",data)) 
		{
			// need to remove this one also, or else it's gonna look like delete doesn't work
			// if the file was tagged using this alternate field
			setMetadata->RemoveMetadata("ORGANIZATION");
		}
		else if(KeywordMatch("category",data)) 
		{
			// need to remove these two also, or else it's gonna look like delete doesn't work
			// if the file was tagged using these alternate fields
			setMetadata->RemoveMetadata("CONTENTGROUP");
			setMetadata->RemoveMetadata("GROUPING");
		}
	}

	return 1;
}

extern "C" __declspec(dllexport) int winampWriteExtendedFileInfo()
{
	if (setFn && setMetadata)
	{
		Stopper stopper;
		if (lastfn && !_wcsicmp(lastfn, setFn))
			stopper.Stop();
		bool success = setMetadata->Save(setFn);
		stopper.Play();
		setMetadata->Reset();
		free(setFn);
		setFn=0;

		delete getMetadata;
		getMetadata=0;

		// update last modified so we're not re-queried on our own updates
		UpdateFileTimeChanged(setFn);

		return !!success;
	}
	return 1;
}

extern "C" __declspec(dllexport) const wchar_t *winampWriteExtendedGetLastError()
{
	return 0;
}