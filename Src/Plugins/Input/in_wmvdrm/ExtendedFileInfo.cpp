#include "main.h"
#include "../nu/AutoWide.h"
#include "WMPlaylist.h"
#include "resource.h"
#include <math.h>
#include <strsafe.h>

WMInformation *setFileInfo = 0;
static wchar_t *setFileInfoName=0;
static bool forcedStop = false;
static int outTime = 0;
float GetGain(WMInformation *info, bool allowDefault);

static const wchar_t *extension(const wchar_t *fn)
{
	const wchar_t *x = PathFindExtension(fn);

	if (*x)
		return CharNext(x);
	else
		return x;
}

static bool KeywordMatch(const char *mainString, const char *keyword)
{
	return !_stricmp(mainString, keyword);
}

static bool KeywordMatch(const wchar_t *mainString, const wchar_t *keyword)
{
	return !lstrcmpiW(mainString, keyword);
}

static bool StartsWith(const wchar_t *mainString, const wchar_t *substring)
{
	return !_wcsnicmp(mainString, substring, lstrlenW(substring));
}

static int Width(int dec)
{
	// there's probably a better way
	int width=3;
	while (width && (dec % 10) == 0)
	{
		dec/=10;
		width--;
	}
	return width;
}


int GetExtendedInformation(WMInformation *getExtendedFileInfo, const wchar_t *fn, const char *data, wchar_t *dest, int destlen)
{
	AutoWide tagNameW(data);
	const wchar_t *tagName = GetAlias(tagNameW);

	if (KeywordMatch(tagName, L"streammetadata"))
	{
		if (config_http_metadata)
		{
			lstrcpyn(dest, L"1", destlen);
			return 1;
		}
		return 0;
	}
	else if (KeywordMatch(tagName, L"type"))
	{
		if (!fn || !fn[0])
			lstrcpyn(dest, (config_no_video ? L"0" : L"1"), destlen);
		else if (getExtendedFileInfo->IsAttribute(g_wszWMHasVideo))
			lstrcpyn(dest, L"1", destlen);
		else if (getExtendedFileInfo->IsAttribute(g_wszWMHasAudio))
			lstrcpyn(dest, L"0", destlen);
		else
		{
			switch (fileTypes.GetAVType(extension(fn)))
			{
				case FileType::AUDIO:
					lstrcpyn(dest, L"0", destlen);
					break;
				case FileType::VIDEO:
					lstrcpyn(dest, L"1", destlen);
					break;
				default:
					return 0;
			}
		}
		return 1;
	}
	else if (KeywordMatch(tagName, L"rateable"))
	{
		dest[0] = '1';
		dest[1] = 0;
		return 1;
	}
	else if (StartsWith(tagName, L"WM/"))
	{
		getExtendedFileInfo->GetAttribute(tagName, dest, destlen);
		return 1;
	}
	/*else if (KeywordMatch(data, "burnable")) // note: this isn't supposed to be any kind of protection - just keeps the burner from misbehaving on protected tracks
	{
		if (getExtendedFileInfo->IsAttribute(g_wszWMProtected))
			lstrcpyn(dest, L"0", destlen);
		else
			lstrcpyn(dest, L"1", destlen);

		return 1;
	}*/
	else if (KeywordMatch(data, "noburnreason")) // note: this isn't supposed to be any kind of protection - just keeps the burner from misbehaving on protected tracks
	{
		if (getExtendedFileInfo->IsAttribute(g_wszWMProtected))
		{
			lstrcpyn(dest, L"DRM (copy protected) file", destlen);
			return 1;
		}
	}
	else if ((const wchar_t *)tagNameW != tagName) // if the tag was in the tag list
	{
		getExtendedFileInfo->GetAttribute(tagName, dest, destlen);
		return 1;
	}
	else if (KeywordMatch(data, "bitrate"))
	{
		StringCchPrintfW(dest, destlen, L"%u", getExtendedFileInfo->GetBitrate() / 1000);
		return 1;
	}
	else if (KeywordMatch(data, "vbr"))
	{
		if (getExtendedFileInfo->IsAttribute(g_wszWMIsVBR))
			StringCchCopyW(dest, destlen, L"1");
		else if (getExtendedFileInfo->IsNotAttribute(g_wszWMIsVBR))
			StringCchCopyW(dest, destlen, L"0");

		return 1;
	}
	//else if (KeywordMatch(data, "srate"))
	else if (KeywordMatch(data, "length"))
	{
		long length = getExtendedFileInfo->GetLengthMilliseconds();
		if (length == -1000)
			return 0;

		_itow(length, dest, 10);
		return 1;
	}
	else if (KeywordMatch(data, "rating"))
	{
		wchar_t rating_string[128] = {0};
		getExtendedFileInfo->GetAttribute(L"WM/SharedUserRating", rating_string, 128);
		int rating = _wtoi(rating_string);
		if (rating == 0)
			dest[0]=0;
		else if (rating >= 1 && rating <= 12)
			dest[0]=L'1';
		else if (rating >= 13 && rating <= 37)
			dest[0]=L'2';
		else if (rating >= 38 && rating <= 62)
			dest[0]=L'3';
		else if (rating >= 63 && rating <= 86)
			dest[0]=L'4';
		else
			dest[0]=L'5';
		dest[1]=0;
		return 1;
	}
	else if (KeywordMatch(data, "replaygain_track_gain")
	         || KeywordMatch(data, "replaygain_track_peak")
	         || KeywordMatch(data, "replaygain_album_gain")
	         || KeywordMatch(data, "replaygain_album_peak"))
	{
		getExtendedFileInfo->GetAttribute(tagName, dest, destlen);
		return 1;
	}
	else if (KeywordMatch(data, "gain"))
	{
		StringCchPrintfW(dest, destlen, L"%-+.2f dB", (float)log10f(GetGain(getExtendedFileInfo, false))*20.0f);
		return 1;
	}
	else if (KeywordMatch(data, "audiocodec"))
	{
		if (!getExtendedFileInfo->GetCodecName(dest, destlen))
			dest[0]=0;
		return 1;
	}
	else if (KeywordMatch(data, "lossless"))
	{
		wchar_t codecname[1024] = {0};
		if (!getExtendedFileInfo->GetCodecName(codecname, 1024))
			dest[0]=0;
		else
		{
			dest[0] = wcsstr(codecname, L"Lossless")?'1':'0';
			dest[1]=0;
		}
		return 1;
	}
	else if (KeywordMatch(data, "GracenoteFileID"))
	{
		getExtendedFileInfo->GetAttribute_BinString(L"GN/UniqueFileIdentifier", dest, destlen);
		return 1;
	}
	else if (KeywordMatch(data, "GracenoteExtData"))
	{
		getExtendedFileInfo->GetAttribute_BinString(L"GN/ExtData", dest, destlen);
		return 1;
	}
	else if (KeywordMatch(data, "formatinformation"))
	{
		// this is a bit of a clusterfuck, but it's safe and (hopefully) logically laid out.
		wchar_t codec[128]=L"", duration[64]=L"", bitrate[64]=L"", filesize[64]=L"", wmver[64]=L"", seekable[64]=L"";
		wchar_t stridable[64]=L"", broadcast[64]=L"", protect[64]=L"", trusted[64]=L"", contents[64]=L"";
		wchar_t buf[128]=L""; // temporary buffer

		// get the codec name string
		if (getExtendedFileInfo->GetCodecName(buf, 128) && buf[0])
			StringCchPrintf(codec,128,L"%s: %s\n",WASABI_API_LNGSTRINGW(IDS_CODEC),buf);

		// get the length string formatted h:mm:ss.tttt
		long t = getExtendedFileInfo->GetLengthMilliseconds();
		if (t)
		{
			long h = t/36000000;
			long m = (t/60000)%60;
			long s = (t/1000)%60;
			long ms = t%1000;
			if (h)
				StringCchPrintf(duration,64,L"%s: %u:%02u:%02u.%03u\n",WASABI_API_LNGSTRINGW(IDS_DURATION),h,m,s,ms);
			else if (m)
				StringCchPrintf(duration,64,L"%s: %u:%02u.%03u\n",WASABI_API_LNGSTRINGW(IDS_DURATION),m,s,ms);
			else
				StringCchPrintf(duration,64,L"%s: %u.%03u\n",WASABI_API_LNGSTRINGW(IDS_DURATION),s,ms);
		}

		// get the bitrate string formatted 128.235 kbps
		long br = getExtendedFileInfo->GetBitrate();
		wchar_t kbps[16] = {0};
		StringCchPrintf(bitrate,64,L"%s: %.*f %s\n",
						WASABI_API_LNGSTRINGW(IDS_BITRATE),
						Width(br%1000),
						br/1000.0,
						WASABI_API_LNGSTRINGW_BUF(IDS_KBPS,kbps,16));

		// get the filesize string, with commas grouping in threes
		buf[0]=0;
		getExtendedFileInfo->GetAttribute(L"FileSize",buf,64);
		uint64_t fs = _wcstoui64(buf, 0, 10);
		if (fs)
		{
			uint64_t fsgb = (fs/1000000000LL);
			uint64_t fsmb = (fs/1000000LL)%1000LL;
			uint64_t fskb = (fs/1000LL)%1000LL;
			uint64_t fsb = fs%1000LL;
			if (fsgb)
				StringCchPrintf(filesize,64,L"%s: %I64u,%03I64u,%03I64u,%03I64u\n",WASABI_API_LNGSTRINGW(IDS_FILESIZE),fsgb,fsmb,fskb,fsb);
			else if (fsmb)
				StringCchPrintf(filesize,64,L"%s: %I64u,%03I64u,%03I64u\n",WASABI_API_LNGSTRINGW(IDS_FILESIZE),fsmb,fskb,fsb);
			else if (fskb)
				StringCchPrintf(filesize,64,L"%s: %I64u,%03I64u\n",WASABI_API_LNGSTRINGW(IDS_FILESIZE),fskb,fsb);
			else
				StringCchPrintf(filesize,64,L"%s: %I64u\n",WASABI_API_LNGSTRINGW(IDS_FILESIZE),fsb);
		}

		// 4 boolean flags, compose their strings
		wchar_t yes[64] = {0}, no[64] = {0};
		WASABI_API_LNGSTRINGW_BUF(IDS_YES,yes,64);
		WASABI_API_LNGSTRINGW_BUF(IDS_NO,no,64);

		buf[0]=0;
		getExtendedFileInfo->GetAttribute(L"WMFSDKVersion",buf,128);
		if (buf[0])
			StringCchPrintf(wmver,64,L"%s: %s\n",WASABI_API_LNGSTRINGW(IDS_WMVER),buf);

		StringCchPrintf(seekable,64,L"%s: %s\n",WASABI_API_LNGSTRINGW(IDS_SEEKABLE),getExtendedFileInfo->IsAttribute(L"Seekable")?yes:no);
		StringCchPrintf(stridable,64,L"%s: %s\n",WASABI_API_LNGSTRINGW(IDS_STRIDABLE),getExtendedFileInfo->IsAttribute(L"Stridable")?yes:no);
		StringCchPrintf(broadcast,64,L"%s: %s\n",WASABI_API_LNGSTRINGW(IDS_BROADCAST),getExtendedFileInfo->IsAttribute(L"Broadcast")?yes:no);
		StringCchPrintf(protect,64,L"%s: %s\n",WASABI_API_LNGSTRINGW(IDS_PROTECTED),getExtendedFileInfo->IsAttribute(L"Is_Protected")?yes:no);
		StringCchPrintf(trusted,64,L"%s: %s\n",WASABI_API_LNGSTRINGW(IDS_TRUSTED),getExtendedFileInfo->IsAttribute(L"Is_Trusted")?yes:no);

		// file contents. bit gross i know.
		wchar_t cont[4][16]={L"",L"",L"",L""};
		int i=0;
		if (getExtendedFileInfo->IsAttribute(L"HasAudio"))
			WASABI_API_LNGSTRINGW_BUF(IDS_AUDIO,cont[i++],16);
		if (getExtendedFileInfo->IsAttribute(L"HasVideo"))
			WASABI_API_LNGSTRINGW_BUF(IDS_VIDEO,cont[i++],16);
		if (getExtendedFileInfo->IsAttribute(L"HasImage"))
			WASABI_API_LNGSTRINGW_BUF(IDS_IMAGE,cont[i++],16);
		if (getExtendedFileInfo->IsAttribute(L"HasScript"))
			WASABI_API_LNGSTRINGW_BUF(IDS_SCRIPT,cont[i++],16);

		WASABI_API_LNGSTRINGW_BUF(IDS_NONE,buf,64);
		if (i == 0)
			StringCchPrintf(contents,64,L"%s: %s",WASABI_API_LNGSTRINGW(IDS_CONTAINS), buf);
		else if (i == 1)
			StringCchPrintf(contents,64,L"%s: %s",WASABI_API_LNGSTRINGW(IDS_CONTAINS), cont[0]);
		else if (i == 2)
			StringCchPrintf(contents,64,L"%s: %s, %s",WASABI_API_LNGSTRINGW(IDS_CONTAINS), cont[0], cont[1]);
		else if (i == 3)
			StringCchPrintf(contents,64,L"%s: %s, %s, %s",WASABI_API_LNGSTRINGW(IDS_CONTAINS), cont[0], cont[1], cont[2]);
		else if (i == 4)
			StringCchPrintf(contents,64,L"%s: %s, %s, %s, %s",WASABI_API_LNGSTRINGW(IDS_CONTAINS), cont[0], cont[1], cont[2], cont[3]);

		// compose our string together!
		StringCchPrintf(dest,destlen,L"%s%s%s%s%s%s%s%s%s%s%s",codec, duration, bitrate, filesize, wmver, seekable, stridable, broadcast, protect, trusted, contents);
	}
	else
		return 0;

	return 1;
}

#if 0 // had to disable this because it was locking the file from being deleted
WMInformation *lastGetInfo = 0;
wchar_t *lastGetInfoFn;
#endif

extern "C" __declspec(dllexport)
	int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, int destlen)
{
	/* Check if there's a status message for this filename 
		 doing this forces Winamp to hit plugin.getfileinfo, which gives us better control
		 over adding things like [Individualizing] to the playlist title for local files
	*/
	if (winamp.HasStatus(fn))
		return 0;

	if ((!fn || !*fn) && KeywordMatch(data, "type"))
	{
		if (config_no_video)
			lstrcpyn(dest, L"0", destlen);
		else
			lstrcpyn(dest, L"1", destlen);
		return 1;
	}

	
	if (KeywordMatch(data, "mime"))
	{
		int len;
		const wchar_t *p;
		if (!fn || !fn[0]) return 0;
		len = lstrlenW(fn);
		if (len < 4 || L'.' != fn[len - 4])  return 0;
		p = &fn[len - 3];
		if (!_wcsicmp(p, L"WMA")) { StringCchCopyW(dest, destlen, L"audio/x-ms-wma"); return 1; }
		if (!_wcsicmp(p, L"WMV")) { StringCchCopyW(dest, destlen, L"video/x-ms-wmv"); return 1; }
		if (!_wcsicmp(p, L"ASF")) { StringCchCopyW(dest, destlen, L"video/x-ms-asf"); return 1; }
		
		return 0;
	}
	if (KeywordMatch(data, "family"))
	{
		LPCWSTR e, p(NULL);
		DWORD lcid;
		size_t i;
		int len2(0);

		if (!fn || !*fn) return 0;
		e = PathFindExtensionW(fn);
		if (L'.' != *e) return 0;
		e++;

		if (!*e) return 0;
		lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
		
		for (i = 0; i < fileTypes.types.size() && !p; i++)
		{
			if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, fileTypes.types.at(i).wtype, -1)) 
				p = fileTypes.types.at(i).description;
		}

		if (p)
		{
			wchar_t szTest[16];
			if (S_OK == StringCchPrintfW(szTest, sizeof(szTest)/sizeof(wchar_t), L" (*.%s)", e))
			{
				int len1 = lstrlenW(szTest);
				len2 = lstrlenW(p);
				if (len2 > len1 && CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, szTest, -1, (p + len2 - len1), -1))
				len2 -= len1;
			}
		}

		return (p && S_OK == StringCchCopyNW(dest, destlen, p, len2));
	}

	if (!config_http_metadata && PathIsURL(fn))
		return 0;

	if (KeywordMatch(data, "type") && !PathFileExistsW(fn))
	{
		switch (fileTypes.GetAVType(extension(fn)))
		{
		case FileType::AUDIO:
			lstrcpyn(dest, L"0", destlen);
			return 1;
		case FileType::VIDEO:
			lstrcpyn(dest, L"1", destlen);
			return 1;
		default:
			return 0;
		}
	}

	

	if (activePlaylist.IsMe(fn))
	{
		WMInformation getExtendedFileInfo(activePlaylist.GetFileName());

		return GetExtendedInformation(&getExtendedFileInfo, fn, data, dest, destlen);
	}
	else
	{
		WMInformation getExtendedFileInfo(fn);

		return GetExtendedInformation(&getExtendedFileInfo, fn, data, dest, destlen);
	}

	#if 0 // had to disable this because it was locking the file from being deleted
	if (lastGetInfo && lastGetInfoFn && !_wcsicmp(fn, lastGetInfoFn))
	{
		return GetExtendedInformation(lastGetInfo, fn, data, dest, destlen);
	}

	delete lastGetInfo;
	lastGetInfo=0;
	free(lastGetInfoFn);
	lastGetInfoFn=0;

	if (activePlaylist.IsMe(fn))
		lastGetInfoFn = _wcsdup(activePlaylist.GetFileName());
	else
		lastGetInfoFn = _wcsdup(fn);

	lastGetInfo = new WMInformation(lastGetInfoFn);
	if (lastGetInfo->ErrorOpening())
	{
		if (KeywordMatch(data, "type"))
		{
			switch (fileTypes.GetAVType(extension(fn)))
			{
			case FileType::AUDIO:
				lstrcpyn(dest, L"0", destlen);
				return 1;
			case FileType::VIDEO:
				lstrcpyn(dest, L"1", destlen);
				return 1;
			default:
				return 0;
			}
		}

		delete lastGetInfo;
		lastGetInfo=0;
		free(lastGetInfoFn);
		lastGetInfoFn=0;
		return 0;
	}

	return GetExtendedInformation(lastGetInfo, fn, data, dest, destlen);
	#endif
}

extern "C" __declspec(dllexport) int winampClearExtendedFileInfoW(const wchar_t *fn)
{
	// TODO: press stop if it's the currently playing file
	WMInformation wmInfo(fn);
	wmInfo.ClearAllAttributes();
	wmInfo.Flush();
	return 1;
}

extern "C" __declspec(dllexport) int winampSetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *val)
{
	//	if (!lastSetInfoFilename.empty() && lastSetInfoFilename != fn)
	//		dosomething();

	#if 0 // had to disable this because it was locking the file from being deleted
	if (lastGetInfoFn && !_wcsicmp(lastGetInfoFn,fn))
	{
		delete lastGetInfo;
		lastGetInfo=0;
		free(lastGetInfoFn);
		lastGetInfoFn=0;
	}
#endif

	if (!setFileInfo)
	{
		if (activePlaylist.IsMe(fn) && mod.playing)
		{
			forcedStop = true;
			outTime = mod.GetOutputTime();
			winamp.PressStop();
		}
		free(setFileInfoName);
		setFileInfoName = _wcsdup(fn);
		setFileInfo = new WMInformation(fn);
		if (!setFileInfo->MakeWritable(fn))
			return 0; // can't write
	}

	AutoWide tagNameW(data);
	const wchar_t *tagName = GetAlias(tagNameW);

	if (StartsWith(tagName, L"WM/"))
	{
		setFileInfo->SetAttribute(tagName, val);
		return 1;
	}

	else if ((const wchar_t *)tagNameW != tagName) // if the tag was in the tag list
	{
		setFileInfo->SetAttribute(tagName, val);
		return 1;
	}
	else if (KeywordMatch(data, "rating"))
	{
		int rating = _wtoi(val);
		if (rating == 0)
			setFileInfo->SetAttribute(L"WM/SharedUserRating", L"",WMT_TYPE_DWORD);
		else
		{
			switch(rating)
			{
			case 1:
				setFileInfo->SetAttribute(L"WM/SharedUserRating", L"1",WMT_TYPE_DWORD);
				break;
			case 2:
				setFileInfo->SetAttribute(L"WM/SharedUserRating", L"25",WMT_TYPE_DWORD);
				break;
			case 3:
				setFileInfo->SetAttribute(L"WM/SharedUserRating", L"50",WMT_TYPE_DWORD);
				break;
			case 4:
				setFileInfo->SetAttribute(L"WM/SharedUserRating", L"75",WMT_TYPE_DWORD);
				break;
			default:
				setFileInfo->SetAttribute(L"WM/SharedUserRating", L"99",WMT_TYPE_DWORD);
				break;

			}
		}
	}
	else if (KeywordMatch(data, "replaygain_track_gain")
	         || KeywordMatch(data, "replaygain_track_peak")
	         || KeywordMatch(data, "replaygain_album_gain")
	         || KeywordMatch(data, "replaygain_album_peak"))
	{
		setFileInfo->SetAttribute(tagName, val);
		return 1;
	}
	else if (KeywordMatch(data, "GracenoteFileID"))
	{
		setFileInfo->SetAttribute_BinString(L"GN/UniqueFileIdentifier", val);
		return 1;
	}
	else if (KeywordMatch(data, "GracenoteExtData"))
	{
		setFileInfo->SetAttribute_BinString(L"GN/ExtData", val);
		return 1;
	}

	//	else if (KeywordMatch(data, "bitrate"))
	//else if (KeywordMatch(data, "disc"))
	//	else if (KeywordMatch(data, "vbr"))
	//else if (KeywordMatch(data, "srate"))
	//	else if (KeywordMatch(data, "length"))

	return 0;
}

extern "C" __declspec(dllexport) int winampWriteExtendedFileInfo()
{
	if (setFileInfo)
	{
		bool flushOK = setFileInfo->Flush();
		delete setFileInfo;
		setFileInfo = 0;

		if (forcedStop)
		{
			mod.startAtMilliseconds = outTime;
			winamp.PressPlay();
		}
		forcedStop=false;

		if (flushOK)
			return 1;
		else
			return 0;
	}

	return 0;
}
