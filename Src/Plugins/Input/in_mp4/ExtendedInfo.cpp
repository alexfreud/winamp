#include "main.h"
#include "VirtualIO.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoChar.h"
#include <vector>
#include "../Winamp/wa_ipc.h"
#include "api__in_mp4.h"
#include "Stopper.h"
#include <shlwapi.h>
#include <strsafe.h>
#include "resource.h"

static inline wchar_t *IncSafe(wchar_t *val, int x)
{
	while (x--)
	{
		if (*val)
			val++;
	}
	return val;
}

bool KeywordMatch(const char *mainString, const char *keyword)
{
	return !_stricmp(mainString, keyword);
}

bool GetCustomMetadata(MP4FileHandle mp4, char *metadata, wchar_t *dest, int destlen, const char *owner)
{
	u_int8_t *value;
	u_int32_t size;
	bool success = MP4GetMetadataFreeForm(mp4, metadata, &value, &size, owner);
	if (success)
	{
		int cnt = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)value, size, dest, destlen - 1);
		dest[cnt] = 0;
		MP4Free(value);
	}
	else
		dest[0] = 0;

	return success;
}

bool HasVideo(MP4FileHandle infile)
{
	/* find Video track */
	int numTracks = MP4GetNumberOfTracks(infile, NULL,       /* subType */ 0);

	for (int i = 0; i < numTracks; i++)
	{
		MP4TrackId trackId = MP4FindTrackId(infile, i, NULL,       /* subType */ 0);
		const char* trackType = MP4GetTrackType(infile, trackId);

		if (!lstrcmpA(trackType, MP4_VIDEO_TRACK_TYPE))
			return true;

	}

	/* can't decode this */
	return false;
}
typedef struct __INFOKEYWORD
{
	LPCWSTR	buffer;
	INT		len;
} INFOKEYWORD;

typedef std::vector<INFOKEYWORD> KeywordList;

/*static void KeywordList_PushBack(KeywordList *list, LPCWSTR buffer, INT len)
{
	if (NULL == list) return;
	
	INFOKEYWORD kw = { buffer, len };
	list->push_back(kw);
}

static LPCWSTR ParseFormatInfoLine(LPCWSTR line, KeywordList *list)
{
	if (NULL == line || L'\0' == *line) return line;
	LPCWSTR cursor = line;
	
	for(;;)
	{
		switch(*cursor)
		{
			case L'\r': 	
				if (L'\n' == *(cursor + 1))
				{
					KeywordList_PushBack(list, line, (INT)(cursor - line));
					cursor += 2;
					return cursor;
				}
			case L'\0':
				KeywordList_PushBack(list, line, (INT)(cursor - line));
				return NULL;
			case L'\t':
				KeywordList_PushBack(list, line, (INT)(cursor - line));
				line = ++cursor;
				break;
			default:
				cursor++;
				break;
		}
	}
}

static HRESULT FormatInformationString(LPWSTR pszBuffer, INT cchBufferMax, LPCSTR formatInfo)
{
	if (NULL == pszBuffer) return E_POINTER;
	*pszBuffer = L'\0';

	if (NULL == formatInfo) 
		return S_OK;

	INT cchFormat = lstrlenA(formatInfo);
	if (0 == cchFormat) return S_OK;

	LPWSTR pszFormat = (LPWSTR)malloc(sizeof(WCHAR) * (cchFormat + 1));
	if (NULL == pszFormat) return E_OUTOFMEMORY;

	INT result = MultiByteToWideCharSZ(CP_UTF8, 0, formatInfo, cchFormat, pszFormat, cchFormat + 1);
	if (0 == result)
	{
		DWORD errorCode = GetLastError();
		return HRESULT_FROM_WIN32(errorCode);
	}

	HRESULT hr(S_OK);
	KeywordList keys;
	KeywordList values;
	LPCWSTR line = pszFormat;
	line = ParseFormatInfoLine(line, &keys);
	line = ParseFormatInfoLine(line, &values);

	size_t count = keys.size();
	if (0 != count && count == values.size())
	{	
		LPWSTR cursor = pszBuffer;
		size_t remaining = cchBufferMax;
		for (size_t index = 0; index < count; index++)
		{
			if (cursor != pszBuffer)
				hr = StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, 0);

			if (SUCCEEDED(hr) && 0 != keys[index].len)
			{
				hr = StringCchCopyNEx(cursor, remaining, keys[index].buffer, keys[index].len, &cursor, &remaining, 0);
				if (SUCCEEDED(hr)) 
					hr = StringCchCopyEx(cursor, remaining, L":", &cursor, &remaining, 0);
			}

			if (SUCCEEDED(hr) && 0 != values[index].len)
			{
				if (SUCCEEDED(hr) && 0 != keys[index].len) 
					hr = StringCchCopyEx(cursor, remaining, L" ", &cursor, &remaining, 0);
				
				hr = StringCchCopyNEx(cursor, remaining, values[index].buffer, values[index].len, &cursor, &remaining, 0);
			}
			
			if (FAILED(hr)) 
				break;
		}
	}
	else
	{
		hr = StringCchCopy(pszBuffer, cchBufferMax, pszFormat);
	}

	if (NULL != pszFormat) 
		free(pszFormat);

	return hr;
}*/

static MP4FileHandle getFileInfoMP4 = 0;
static wchar_t getFileInfoFn[MAX_PATH]=L"";
static void *getFileInfoReader=0;
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

extern "C"
{
	__declspec( dllexport ) int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, size_t destlen)
	{
		if (KeywordMatch(data, "type"))
		{
			if (!fn || (fn && !fn[0]) || !PathFileExistsW(fn))
			{
				const wchar_t *e = PathFindExtensionW(fn);
				DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
				// check known video extensions
				if ((CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L".M4V", -1)) 
					|| (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L".MOV", -1)) 
					|| (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L".F4V", -1))
					|| (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L".MP4", -1)))
					dest[0] = '1';
				else
					dest[0] = '0'; 
				dest[1] = 0;
				return 1;
			}
		}
		else if (KeywordMatch(data, "rateable"))
		{
			dest[0] = '1';
			dest[1] = 0;
			return 1;
		}

		if (!fn || (fn && !fn[0])) return 0;

		if (KeywordMatch(data, "family"))
		{  
			LPCWSTR e;
			int pID = -1;
			e = PathFindExtensionW(fn);
			if (L'.' != *e) return 0;
			e++;
			DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
			if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"M4A", -1)) pID = IDS_FAMILY_STRING_M4A;
			else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"MP4", -1)) pID = IDS_FAMILY_STRING_MPEG4;
			else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"M4V", -1)) pID = IDS_FAMILY_STRING_M4V;
			else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"MOV", -1)) pID = IDS_FAMILY_STRING_QUICKTIME;
			else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"3GP", -1)) pID = IDS_FAMILY_STRING_3GPP;
			else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"F4V", -1)) pID = IDS_FAMILY_STRING_FLV;
			if (pID != -1 && S_OK == StringCchCopyW(dest, destlen, WASABI_API_LNGSTRINGW(pID))) return 1;
			return 0;
		}

		if (KeywordMatch(data, "mime"))
		{  
			LPCWSTR e;
			e = PathFindExtensionW(fn);
			if (L'.' != *e) return 0;
			e++;
			DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
			if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"M4A", -1)) { StringCchCopyW(dest, destlen, L"audio/mp4"); return 1; }
			else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"MP4", -1)) { StringCchCopyW(dest, destlen, L"audio/mp4"); return 1; }
			else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"M4V", -1)) { StringCchCopyW(dest, destlen, L"video/mp4"); return 1; }
			else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"MOV", -1)) { StringCchCopyW(dest, destlen, L"video/quicktime"); return 1; }
			else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"3GP", -1)) { StringCchCopyW(dest, destlen, L"video/3gp"); return 1; }
			else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"F4V", -1)) { StringCchCopyW(dest, destlen, L"video/f4v"); return 1; }
			return 0;
		}

		if (!getFileInfoMP4 || lstrcmpi(getFileInfoFn, fn) || HasFileTimeChanged(fn)) // different file than last time?
		{
			lstrcpyn(getFileInfoFn, fn, MAX_PATH);

			if (getFileInfoMP4)
				MP4Close(getFileInfoMP4);
			getFileInfoMP4=0;

			if (getFileInfoReader)
				DestroyUnicodeReader(getFileInfoReader);

			getFileInfoReader = CreateUnicodeReader(fn);
			if (!getFileInfoReader)
			{
				getFileInfoFn[0]=0;
				return 0;
			}

			getFileInfoMP4 = MP4ReadEx(fn, getFileInfoReader, &UnicodeIO);
			if (!getFileInfoMP4)
			{
				DestroyUnicodeReader(getFileInfoReader);
				getFileInfoReader=0;
				getFileInfoFn[0]=0;
				return 0;
			}
			else
			{
				UnicodeClose(getFileInfoReader); // go ahead and close the file so we don't lock it
			}
		}

		bool success = false;
		char *pVal = NULL;
		uint16_t *pVal_utf16 = NULL;

		if (KeywordMatch(data, "type"))
		{
			if (GetVideoTrack(getFileInfoMP4) != MP4_INVALID_TRACK_ID) // check for a video track
				dest[0] = '1';
			else // assume no video mean audio (not necessarily true, for weird mp4 stuff like systems & text)
				dest[0] = '0';
			dest[1] = 0;
			success = true;
		}
		else if (KeywordMatch(data, "lossless"))
		{
			dest[0]='0';
			dest[1]=0;
			MP4TrackId track = GetAudioTrack(getFileInfoMP4);
			if (track != MP4_INVALID_TRACK_ID)
			{
				// TODO: benski> We should check more than just ALAC
				// potentially asking the mpeg4audio services
				// but for now this should suffice.
				const char *media_data_name = MP4GetTrackMediaDataName(getFileInfoMP4, track);
					if (media_data_name && KeywordMatch (media_data_name, "alac"))
						dest[0]='1';
			}
			success=true;
		}
		else if (KeywordMatch(data, "title"))
		{
			success = MP4GetMetadataName(getFileInfoMP4, &pVal);
			if (!success)
				success = MP4Get3GPMetadata(getFileInfoMP4, "titl", &pVal_utf16);
		}
		else if (KeywordMatch(data, "album"))
		{
			success = MP4GetMetadataAlbum(getFileInfoMP4, &pVal);
			if (!success)
				success = MP4Get3GPMetadata(getFileInfoMP4, "albm", &pVal_utf16);
		}
		else if (KeywordMatch(data, "artist"))
		{
			success = MP4GetMetadataArtist(getFileInfoMP4, &pVal);
			if (!success)
				success = MP4Get3GPMetadata(getFileInfoMP4, "perf", &pVal_utf16);
		}
		else if (KeywordMatch(data, "rating"))
		{
			success = MP4GetMetadataRating(getFileInfoMP4, &pVal);
			// add a /* to enable reading from 3GP metadata, otherwise is only used on normal MP4
			if (success)/*/
			if (!success)
			{
				if ((success = MP4Get3GPMetadata(getFileInfoMP4, "rate", &pVal_utf16)))
				{
					wchar_t *value = (wchar_t*)pVal_utf16;
					if(value && *value) {
						int rating = _wtoi(value);

						// keeps things limited to our range of 0-100
						if (rating >= 100) {
							rating = 5;
						}
						// 1-100 case
						else if (rating > 0 && rating < 100) {
							rating = (rating /= 20);
							// shift up by one rating when in next band
							// 1-20 = 1, 21-40 = 2, 41-60 = 3, 61-80 = 4, 81-100 = 5
							rating += ((_wtoi(value) - (rating * 20)) > 0);
						}
						// otherwise just make sure and set zero
						else {
							rating = 0;
						}

						StringCchPrintfW(dest, destlen, L"%u", rating);
						MP4Free(pVal_utf16);
						return 1;
					}
				}
			}
			else/**/
			{
				char *value = (char*)pVal;
				if(value && *value) {
					int rating = atoi(value);

					// keeps things limited to our range of 0-100
					if (rating >= 100) {
						rating = 5;
					}
					// 1-100 case
					else if (rating > 0 && rating < 100) {
						rating = (rating /= 20);
						// shift up by one rating when in next band
						// 1-20 = 1, 21-40 = 2, 41-60 = 3, 61-80 = 4, 81-100 = 5
						rating += ((atoi(value) - (rating * 20)) > 0);
					}
					// otherwise just make sure and set zero
					else {
						rating = 0;
					}

					StringCchPrintfW(dest, destlen, L"%u", rating);
					MP4Free(pVal);
					return 1;
				}
			}
		}
		else if (KeywordMatch(data, "comment"))
			success = MP4GetMetadataComment(getFileInfoMP4, &pVal);
		else if (KeywordMatch(data, "albumartist"))
			success = MP4GetMetadataAlbumArtist(getFileInfoMP4, &pVal);
		else if (KeywordMatch(data, "gain"))
		{
			StringCchPrintfW(dest, destlen, L"%+-.2f dB", (float)log10f(GetGain(getFileInfoMP4, false))*20.0f);
			success = true;
		}
		else if (KeywordMatch(data, "replaygain_track_gain"))
		{
			GetCustomMetadata(getFileInfoMP4, "replaygain_track_gain", dest, destlen);
			success = true;
		}
		else if (KeywordMatch(data, "replaygain_album_gain"))
		{
			GetCustomMetadata(getFileInfoMP4, "replaygain_album_gain", dest, destlen);
			success = true;
		}
		else if (KeywordMatch(data, "replaygain_track_peak"))
		{
			GetCustomMetadata(getFileInfoMP4, "replaygain_track_peak", dest, destlen);
			success = true;
		}
		else if (KeywordMatch(data, "replaygain_album_peak"))
		{
			GetCustomMetadata(getFileInfoMP4, "replaygain_album_peak", dest, destlen);
			success = true;
		}
		else if (KeywordMatch(data, "bpm"))
		{
			unsigned __int16 tempo = 0;
			success = MP4GetMetadataTempo(getFileInfoMP4, &tempo);
			if (success && tempo)
				StringCchPrintf(dest, destlen, L"%u", tempo);
		}
		else if (KeywordMatch(data, "year"))
		{
			success = MP4GetMetadataYear(getFileInfoMP4, &pVal);
			if (!success)
			{
				uint64_t val = 0;
				success = MP4Get3GPMetadataInteger(getFileInfoMP4, "yrrc", &val);
				if (success)
				{
					StringCchPrintf(dest, destlen, L"%I64u", val);
				}
			}
		}
		else if (KeywordMatch(data, "bitrate"))
		{
			uint32_t audio_bitrate = 0, video_bitrate = 0;
			MP4TrackId track = GetAudioTrack(getFileInfoMP4);
			if (track != MP4_INVALID_TRACK_ID)
				audio_bitrate = MP4GetTrackBitRate(getFileInfoMP4, track) / 1000;

			track = GetVideoTrack(getFileInfoMP4);
			if (track != MP4_INVALID_TRACK_ID)
				video_bitrate = MP4GetTrackBitRate(getFileInfoMP4, track) / 1000;


			if (audio_bitrate || video_bitrate)
				StringCchPrintf(dest, destlen, L"%u", audio_bitrate+video_bitrate);
			else
				dest[0] = 0;
			success = true;
		}
		else if (KeywordMatch(data, "height"))
		{
			MP4TrackId track = GetVideoTrack(getFileInfoMP4);
			if (track != MP4_INVALID_TRACK_ID)
			{
				uint16_t height = MP4GetTrackVideoHeight(getFileInfoMP4, track);
				if (height)
				{
					StringCchPrintf(dest, destlen, L"%u", height);
				}
				else
					dest[0]=0;
							success=true;
			}
		}
		else if (KeywordMatch(data, "width"))
		{
			
			MP4TrackId track = GetVideoTrack(getFileInfoMP4);
			if (track != MP4_INVALID_TRACK_ID)
			{
				uint16_t width = MP4GetTrackVideoWidth(getFileInfoMP4, track);
				if (width)
				{
					StringCchPrintf(dest, destlen, L"%u", width);
				}
				else
					dest[0]=0;
				success=true;
			}
		}
		//else if(KeywordMatch(data,"srate")) wsprintf(dest,"%d",srate);
		//else if(KeywordMatch(data,"stereo")) wsprintf(dest,"%d",is_stereo);
		//else if(KeywordMatch(data,"vbr")) wsprintf(dest,"%d",is_vbr);
		else if (KeywordMatch(data, "genre"))
		{
			success = MP4GetMetadataGenre(getFileInfoMP4, &pVal);
			if (!success)
				success = MP4Get3GPMetadata(getFileInfoMP4, "gnre", &pVal_utf16);
		}
		else if (KeywordMatch(data, "disc"))
		{
			unsigned __int16 dummy = 0, dummy2 = 0;
			success = MP4GetMetadataDisk(getFileInfoMP4, &dummy, &dummy2);
			if (success && dummy)
			{
				if (dummy2)
					StringCchPrintf(dest, destlen, L"%u/%u", dummy, dummy2);
				else
					StringCchPrintf(dest, destlen, L"%u", dummy);
			}
			else
				dest[0] = 0;
		}
		else if (KeywordMatch(data, "track"))
		{
			unsigned __int16 dummy = 0, dummy2 = 0;
			success = MP4GetMetadataTrack(getFileInfoMP4, &dummy, &dummy2);
			if (success && dummy)
			{
				if (dummy2)
					StringCchPrintf(dest, destlen, L"%u/%u", dummy, dummy2);
				else
					StringCchPrintf(dest, destlen, L"%u", dummy);
			}
			else
				dest[0] = 0;
		}
		else if (KeywordMatch(data, "length"))
		{
			/* TODO: use sample rate and number of samples from iTunSMPB to get a more exact length */
			//MP4TrackId track = GetAudioTrack(getFileInfoMP4);
			//if (track != MP4_INVALID_TRACK_ID)
			//{
			//	int m_timescale = MP4GetTrackTimeScale(getFileInfoMP4, track);
			//	unsigned __int64 trackDuration = MP4GetTrackDuration(getFileInfoMP4, track);
			//	double m_length = (double)(__int64)trackDuration / (double)m_timescale;
			//	StringCchPrintf(dest, destlen, L"%d", (int)(m_length*1000));
			//	success = true;
			//}

			double lengthAudio = 0;
			double lengthVideo = 0;
			double m_length = 0;
			MP4TrackId audio_track = GetAudioTrack(getFileInfoMP4);
			if (audio_track != -1)
			{
				int timescale = MP4GetTrackTimeScale(getFileInfoMP4, audio_track);
				unsigned __int64 trackDuration = MP4GetTrackDuration(getFileInfoMP4, audio_track);
				lengthAudio = (double)(__int64)trackDuration / (double)timescale;
			}
			MP4TrackId video_track = GetVideoTrack(getFileInfoMP4);
			if (video_track != -1)
			{
				int timescale = MP4GetTrackTimeScale(getFileInfoMP4, video_track);
				unsigned __int64 trackDuration = MP4GetTrackDuration(getFileInfoMP4, video_track);
				lengthVideo = (double)(__int64)trackDuration / (double)timescale;
			}
			m_length = (max(lengthAudio, lengthVideo));
			StringCchPrintf(dest, destlen, L"%d", (int)(m_length * 1000));
			success = true;

		}
		else if (KeywordMatch(data, "tool"))
			success = MP4GetMetadataTool(getFileInfoMP4, &pVal);
		else if (KeywordMatch(data, "composer"))
			success = MP4GetMetadataWriter(getFileInfoMP4, &pVal);
		else if (KeywordMatch(data, "category"))
			success = MP4GetMetadataGrouping(getFileInfoMP4, &pVal);
		else if (KeywordMatch(data, "GracenoteFileID"))
		{
			GetCustomMetadata(getFileInfoMP4, "gnid", dest, destlen);
			success = true;
		}
		else if (KeywordMatch(data, "GracenoteExtData"))
		{
			GetCustomMetadata(getFileInfoMP4, "gnxd", dest, destlen);
			success = true;
		}
		else if (KeywordMatch(data, "publisher"))
		{
			GetCustomMetadata(getFileInfoMP4, "publisher", dest, destlen, "com.nullsoft.winamp");
			success = true;
		}
		else if (KeywordMatch(data, "pregap"))
		{
			wchar_t gap_data[128] = {0};
			if (GetCustomMetadata(getFileInfoMP4, "iTunSMPB", gap_data, 128) && gap_data[0])
			{
				wchar_t *itr = IncSafe(gap_data, 9);

				unsigned int pregap = wcstoul(itr, 0, 16);
				StringCchPrintfW(dest, destlen, L"%u",pregap);
				success=true;
			}
		}
		else if (KeywordMatch(data, "postgap"))
		{
			wchar_t gap_data[128] = {0};
			if (GetCustomMetadata(getFileInfoMP4, "iTunSMPB", gap_data, 128) && gap_data[0])
			{
				wchar_t *itr = IncSafe(gap_data, 18);

				unsigned int postgap = wcstoul(itr, 0, 16);
				StringCchPrintfW(dest, destlen, L"%u",postgap);
				success=true;
			}
		}
		else if (KeywordMatch(data, "numsamples"))
		{
			wchar_t gap_data[128] = {0};
			if (GetCustomMetadata(getFileInfoMP4, "iTunSMPB", gap_data, 128) && gap_data[0])
			{
				wchar_t *itr = IncSafe(gap_data,27);

				unsigned __int64 numsamples = _wcstoui64(itr, 0, 16);
				StringCchPrintfW(dest, destlen, L"%I64u", numsamples);
				success=true;
			}
		}
		else if (KeywordMatch(data, "formatinformation"))
		{
			MP4TrackId track = GetAudioTrack(getFileInfoMP4);
			if (track != MP4_INVALID_TRACK_ID)
			{
				char *track_type = MP4PrintAudioInfo(getFileInfoMP4, track);
				if (track_type)
				{
					uint32_t bitrate = MP4GetTrackBitRate(getFileInfoMP4, track);
					StringCchPrintfEx(dest, destlen, &dest, &destlen, 0, WASABI_API_LNGSTRINGW(IDS_AUDIO_INFO), track_type, (bitrate + 500) / 1000);
					MP4Free(track_type);
				}
			}

			track = GetVideoTrack(getFileInfoMP4);
			if (track != MP4_INVALID_TRACK_ID)
			{
				char *track_type = MP4PrintVideoInfo(getFileInfoMP4, track);
				if (track_type)
				{
					uint32_t bitrate = MP4GetTrackBitRate(getFileInfoMP4, track);
					StringCchPrintfEx(dest, destlen, &dest, &destlen, 0, WASABI_API_LNGSTRINGW(IDS_VIDEO_INFO), track_type, (bitrate + 500) / 1000);
					u_int16_t width = MP4GetTrackVideoWidth(getFileInfoMP4, track);
					u_int16_t height = MP4GetTrackVideoHeight(getFileInfoMP4, track);
					if (width && height)
					{
						// TODO: framerate, but the MP4GetTrackVideoFrameRate method just guesses based on duration and samples
						StringCchPrintfEx(dest, destlen, &dest, &destlen, 0, L"\t%ux%u\n", width, height);
					}
					MP4Free(track_type);
				}
			}

			success=true;
		}
		else // don't understand the name
		{
//			MP4Close(getFileInfoMP4);
			return 0;
		}

		if (pVal)
		{
			MultiByteToWideCharSZ(CP_UTF8, 0, pVal, -1, dest, destlen);
			MP4Free(pVal);
		}
		else if (pVal_utf16)
		{
			StringCchCopyW(dest, destlen, (const wchar_t *)pVal_utf16);
			MP4Free(pVal_utf16);
		}

		if (!success)
			dest[0] = 0;

		//MP4Close(getFileInfoMP4);

		return 1;
	}


	static MP4FileHandle setFileInfoMP4 = 0;
	static int m_last_err = 0;
	static wchar_t m_last_ext_fn[MAX_PATH] = L"";
	static void *setFileInfoReader = 0;
static bool setFile3GP=false;

/*static int SetExtendedInfo3GP(const char *data, const wchar_t *val)
{
	if (KeywordMatch(data, "title"))
	{
		if (val && *val) MP4Set3GPMetadata(setFileInfoMP4, "titl", (const uint16_t *)val);
		else MP4Delete3GPMetadata(setFileInfoMP4, "titl");
	}
	else if (KeywordMatch(data, "album"))
	{
		if (val && *val) MP4Set3GPMetadata(setFileInfoMP4, "albm", (const uint16_t *)val);
		else MP4Delete3GPMetadata(setFileInfoMP4, "albm");
	}
	else if (KeywordMatch(data, "genre"))
	{
		if (val && *val) MP4Set3GPMetadata(setFileInfoMP4, "gnre", (const uint16_t *)val);
		else MP4Delete3GPMetadata(setFileInfoMP4, "gnre");
	}
	else if (KeywordMatch(data, "artist"))
	{
		if (val && *val) MP4Set3GPMetadata(setFileInfoMP4, "perf", (const uint16_t *)val);
		else MP4Delete3GPMetadata(setFileInfoMP4, "perf");
	}
	else if (KeywordMatch(data, "year"))
	{
		if (val && *val) MP4Set3GPMetadataInteger(setFileInfoMP4, "yrrc", _wtoi64(val));
		else MP4Delete3GPMetadata(setFileInfoMP4, "yrrc");
	}
	else
		return 0;

		return 1;
}*/

static int SetExtendedInfoMP4(const char *data, const wchar_t *val)
{
		if (KeywordMatch(data, "title"))
		{
			if (val && *val) MP4SetMetadataName(setFileInfoMP4, AutoChar(val, CP_UTF8));
			else MP4DeleteMetadataName(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "album"))
		{
			if (val && *val)
				MP4SetMetadataAlbum(setFileInfoMP4, AutoChar(val, CP_UTF8));
			else MP4DeleteMetadataAlbum(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "artist"))
		{
			if (val && *val)
				MP4SetMetadataArtist(setFileInfoMP4, AutoChar(val, CP_UTF8));
			else MP4DeleteMetadataArtist(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "rating"))
		{
			if (val && *val)
			{
				char temp[128] = {0};
				StringCchPrintfA(temp, 128, "%u", _wtoi(val) * 20);
				MP4SetMetadataRating(setFileInfoMP4, temp);
			}
			else MP4DeleteMetadataRating(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "comment"))
		{
			if (val && *val)
				MP4SetMetadataComment(setFileInfoMP4, AutoChar(val, CP_UTF8));
			else MP4DeleteMetadataComment(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "year"))
		{
			if (val && *val)
				MP4SetMetadataYear(setFileInfoMP4, AutoChar(val, CP_UTF8));
			else MP4DeleteMetadataYear(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "genre"))
		{
			if (val && *val)
				MP4SetMetadataGenre(setFileInfoMP4, AutoChar(val, CP_UTF8));
			else MP4DeleteMetadataGenre(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "albumartist"))
		{
			if (val && *val)
				MP4SetMetadataAlbumArtist(setFileInfoMP4, AutoChar(val, CP_UTF8));
			else MP4DeleteMetadataAlbumArtist(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "replaygain_track_gain"))
		{
			if (val && *val)
			{
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "replaygain_track_gain", "com.apple.iTunes");
				AutoChar utf8(val, CP_UTF8);
				MP4SetMetadataFreeForm(setFileInfoMP4, "replaygain_track_gain", (u_int8_t *)(char *)utf8, lstrlenA(utf8), "org.hydrogenaudio.replaygain");
			}
			else
			{
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "replaygain_track_gain", "com.apple.iTunes");
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "replaygain_track_gain", "org.hydrogenaudio.replaygain");
			}
		}
		else if (KeywordMatch(data, "replaygain_track_peak"))
		{
			if (val && *val)
			{
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "replaygain_track_peak", "com.apple.iTunes");
				AutoChar utf8(val, CP_UTF8);
				MP4SetMetadataFreeForm(setFileInfoMP4, "replaygain_track_peak", (u_int8_t *)(char *)utf8, lstrlenA(utf8), "org.hydrogenaudio.replaygain");
			}
			else
			{
			MP4DeleteMetadataFreeForm(setFileInfoMP4, "replaygain_track_peak", "com.apple.iTunes");
			MP4DeleteMetadataFreeForm(setFileInfoMP4, "replaygain_track_peak", "org.hydrogenaudio.replaygain");
			}
		}
		else if (KeywordMatch(data, "replaygain_album_gain"))
		{
			if (val && *val)
			{
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "replaygain_album_gain", "com.apple.iTunes");
				AutoChar utf8(val, CP_UTF8);
				MP4SetMetadataFreeForm(setFileInfoMP4, "replaygain_album_gain", (u_int8_t *)(char *)utf8, lstrlenA(utf8), "org.hydrogenaudio.replaygain");
			}
			else
			{
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "replaygain_album_gain", "com.apple.iTunes");
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "replaygain_album_gain", "org.hydrogenaudio.replaygain");
			}
		}
		else if (KeywordMatch(data, "replaygain_album_peak"))
		{
			if (val && *val)
			{
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "replaygain_album_peak", "com.apple.iTunes");
				AutoChar utf8(val, CP_UTF8);
				MP4SetMetadataFreeForm(setFileInfoMP4, "replaygain_album_peak", (u_int8_t *)(char *)utf8, lstrlenA(utf8), "org.hydrogenaudio.replaygain");
			}
			else
			{
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "replaygain_album_peak", "com.apple.iTunes");
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "replaygain_album_peak", "org.hydrogenaudio.replaygain");				
			}
		}
		else if (KeywordMatch(data, "track"))
		{
			int track = _wtoi(val);
			if (track)
			{
				int tracks = 0;
				const wchar_t *_tracks = wcschr(val, L'/');
				if (_tracks) tracks = _wtoi(_tracks + 1);
				MP4SetMetadataTrack(setFileInfoMP4, track, tracks);
			}
			else
				MP4DeleteMetadataTrack(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "disc"))
		{
			int disc = _wtoi(val);
			if (disc)
			{
				int discs = 0;
				const wchar_t *_discs = wcschr(val, L'/');
				if (_discs) discs = _wtoi(_discs + 1);
				MP4SetMetadataDisk(setFileInfoMP4, disc, discs);
			}
			else
				MP4DeleteMetadataDisk(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "tool"))
		{
			if (val && *val)
				MP4SetMetadataTool(setFileInfoMP4, AutoChar(val, CP_UTF8));
			else MP4DeleteMetadataTool(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "bpm"))
		{
				if (val && *val && *val != '0')
					MP4SetMetadataTempo(setFileInfoMP4, _wtoi(val));
				else
					MP4DeleteMetadataTempo(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "composer"))
		{
			if (val && *val)
				MP4SetMetadataWriter(setFileInfoMP4, AutoChar(val, CP_UTF8));
			else MP4DeleteMetadataWriter(setFileInfoMP4);
		}
		else if (KeywordMatch(data, "GracenoteFileID"))
		{
			MP4DeleteMetadataFreeForm(setFileInfoMP4, "gnid", "com.apple.iTunes"); // delete obselete metadata storage scheme
			if (val && *val)
			{
				AutoChar utf8(val, CP_UTF8);
				MP4SetMetadataFreeForm(setFileInfoMP4, "gnid", (u_int8_t *)(char *)utf8, lstrlenA(utf8), "com.gracenote.cddb");
			}
			else
			{
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "gnid", "com.gracenote.cddb");
			}
		}
		else if (KeywordMatch(data, "GracenoteExtData"))
		{
			MP4DeleteMetadataFreeForm(setFileInfoMP4, "gnxd", "com.apple.iTunes");// delete obselete metadata storage scheme
			if (val && *val)
			{
				AutoChar utf8(val, CP_UTF8);
				MP4SetMetadataFreeForm(setFileInfoMP4, "gnxd", (u_int8_t *)(char *)utf8, lstrlenA(utf8), "com.gracenote.cddb");
			}
			else
			{
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "gnxd", "com.gracenote.cddb");
			}
		}
		else if (KeywordMatch(data, "publisher"))
		{
			if (val && *val)
			{
				AutoChar utf8(val, CP_UTF8);
				MP4SetMetadataFreeForm(setFileInfoMP4, "publisher", (u_int8_t *)(char *)utf8, lstrlenA(utf8), "com.nullsoft.winamp");
			}
			else
			{
				MP4DeleteMetadataFreeForm(setFileInfoMP4, "publisher", "com.nullsoft.winamp");
			}
		}
		else if (KeywordMatch(data, "category"))
		{
			if (val && *val)
				MP4SetMetadataGrouping(setFileInfoMP4, AutoChar(val, CP_UTF8));
			else MP4DeleteMetadataGrouping(setFileInfoMP4);
		}
		else
			return 0;

		return 1;
}

	static Stopper stopper;

	__declspec( dllexport ) int winampSetExtendedFileInfoW(const wchar_t *fn, const char *data, const wchar_t *val)
	{
		if (!setFileInfoMP4 || lstrcmpi(m_last_ext_fn, fn)) // different file than last time?
		{
			m_last_err = 0;
			lstrcpyn(m_last_ext_fn, fn, MAX_PATH);

			if (setFileInfoMP4)
				MP4Close(setFileInfoMP4);

			/* TODO: make MP4ModifyEx so we can use this
			if (setFileInfoReader)
				DestroyUnicodeReader(setFileInfoReader);

			setFileInfoReader = CreateUnicodeReader(fn);
			if (!setFileInfoReader)
				return 0;
			*/

			if (!_wcsicmp(m_last_ext_fn, lastfn))
				stopper.Stop();

			setFileInfoMP4 = MP4Modify(m_last_ext_fn, 0, 0); 
			if (!setFileInfoMP4)
			{
				stopper.ChangeTracking(1); // enable stats updating
				//DestroyUnicodeReader(setFileInfoReader);
				m_last_err = 1;
				return 0;
			}

			setFile3GP = false;
			const char *brand=0;
			if (MP4GetStringProperty(setFileInfoMP4, "ftyp.majorBrand", &brand))
			{
				if (!strncmp(brand, "3gp6", 4))
					setFile3GP=true;
			}
		}
		/*
		if (setFile3GP)
			return SetExtendedInfo3GP(data, val);
		else*/
			return SetExtendedInfoMP4(data, val);

	}

	__declspec(dllexport) int winampWriteExtendedFileInfo()
	{
		int err = m_last_err;
		m_last_err = 0;

		// clear this as well, so dirty info isn't read back
		if (getFileInfoMP4)
			MP4Close(getFileInfoMP4);
		getFileInfoMP4=0;

		if (getFileInfoReader)
			DestroyUnicodeReader(getFileInfoReader);
		getFileInfoReader=0;

		if (setFileInfoMP4)
		{
			MP4Close(setFileInfoMP4);
			MP4Optimize(m_last_ext_fn, NULL, 0); //put the fields at the beginning of the file 
			setFileInfoMP4 = 0;
			m_last_ext_fn[0] = 0;
			stopper.Play();
		}

		// update last modified so we're not re-queried on our own updates
		UpdateFileTimeChanged(m_last_ext_fn);

		return !err;
	}
}