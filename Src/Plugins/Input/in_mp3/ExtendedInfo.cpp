#include "main.h"
#include "Metadata.h"
#include "../Winamp/wa_ipc.h"
#include "../nu/ns_wc.h"
#include "uvox_3901.h"
#include "uvox_3902.h"
#include "Stopper.h"
#include <shlwapi.h>
#include "../Agave/Language/api_language.h"
#include <strsafe.h>

extern CGioFile *g_playing_file;
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

Metadata *m_ext_set_mp3info = NULL;
Metadata *m_ext_get_mp3info = NULL;
extern "C" __declspec(dllexport)
int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, size_t destlen)
{
	if (!_stricmp(data, "type"))
	{
		dest[0] = '0';
		dest[1] = 0;
		return 1;
	}
	if (!_stricmp(data, "rateable"))
	{
		dest[0] = '1';
		dest[1] = 0;
		return 1;
	}
	else if (!_stricmp(data, "family"))
	{
		if (!fn || !fn[0]) return 0;
		int len = lstrlenW(fn);
		if (len < 4 || L'.' != fn[len - 4])  return 0;
		const wchar_t *p = &fn[len - 3];
		if (!_wcsicmp(p, L"MP3")) { WASABI_API_LNGSTRINGW_BUF(IDS_FAMILY_STRING_MP3, dest, destlen); return 1; }
		if (!_wcsicmp(p, L"MP2")) { WASABI_API_LNGSTRINGW_BUF(IDS_FAMILY_STRING_MP2, dest, destlen); return 1; }
		if (!_wcsicmp(p, L"MP1")) { WASABI_API_LNGSTRINGW_BUF(IDS_FAMILY_STRING_MP1, dest, destlen); return 1; }
		if (!_wcsicmp(p, L"AAC")) { WASABI_API_LNGSTRINGW_BUF(IDS_FAMILY_STRING_MPEG2_AAC, dest, destlen); return 1; }
		if (!_wcsicmp(p, L"VLB")) { WASABI_API_LNGSTRINGW_BUF(IDS_FAMILY_STRING_DOLBY, dest, destlen); return 1; }
		return 0;
	}
	else if (!_stricmp(data, "mime"))
	{
		if (!fn || !fn[0]) return 0;
		int len = lstrlenW(fn);
		if (len < 4 || L'.' != fn[len - 4])  return 0;
		const wchar_t *p = &fn[len - 3];
		if (!_wcsicmp(p, L"MP3")) { StringCchCopyW(dest, destlen, L"audio/mpeg"); return 1; }
		if (!_wcsicmp(p, L"MP2")) { StringCchCopyW(dest, destlen, L"audio/mpeg"); return 1; }
		if (!_wcsicmp(p, L"MP1")) { StringCchCopyW(dest, destlen, L"audio/mpeg"); return 1; }
		if (!_wcsicmp(p, L"AAC")) { StringCchCopyW(dest, destlen, L"audio/aac"); return 1; }
		if (!_wcsicmp(p, L"VLB")) { StringCchCopyW(dest, destlen, L"audio/vlb"); return 1; }
		return 0;
	}
	else if (!_strnicmp(data, "uvox/", 5))
	{
		EnterCriticalSection(&streamInfoLock);
		if (g_playing_file)
		{
			if (g_playing_file->uvox_3901) // check again now that we've acquired the lock
			{
				Ultravox3901 uvox_metadata;
				if (uvox_metadata.Parse(g_playing_file->uvox_3901) != API_XML_FAILURE)
				{
					LeaveCriticalSection(&streamInfoLock);
					return uvox_metadata.GetExtendedData(data, dest, (int)destlen);
				}
			}
			else if (g_playing_file->uvox_3902)
			{
				Ultravox3902 uvox_metadata;
				if (uvox_metadata.Parse(g_playing_file->uvox_3902) != API_XML_FAILURE)
				{
					LeaveCriticalSection(&streamInfoLock);
					return uvox_metadata.GetExtendedData(data, dest, (int)destlen);
				}
			}
		}
		LeaveCriticalSection(&streamInfoLock);
		return 0;
	}
	else if (_stricmp(data, "0x3901") == 0)
	{
		EnterCriticalSection(&streamInfoLock);
		if (g_playing_file && g_playing_file->uvox_3901) // check again now that we've acquired the lock
		{
			if (dest == NULL)   // It's empty, he's looking for the size of the 0x3901
			{
				int size = MultiByteToWideChar(CP_UTF8, 0, g_playing_file->uvox_3901, -1, 0, 0);
				LeaveCriticalSection(&streamInfoLock);
				return size;
			}
			else
			{
				MultiByteToWideCharSZ(CP_UTF8, 0, g_playing_file->uvox_3901, -1, dest, (int)destlen);
				LeaveCriticalSection(&streamInfoLock);
				return 1;
			}
		}
		LeaveCriticalSection(&streamInfoLock);
		return 0;
	}
	else if (!_stricmp(data, "streamtype"))
	{
		if (lstrcmpW(lastfn, fn))
			return 0;

		if (g_playing_file)
		{
			EnterCriticalSection(&streamInfoLock);
			if (g_playing_file) // check again now that we've acquired the lock
			{
				StringCchPrintfW(dest, destlen, L"%d", g_playing_file->IsStream());
				LeaveCriticalSection(&streamInfoLock);
				return 1;
			}
			LeaveCriticalSection(&streamInfoLock);
		}

		return 0;
	}
	else if (!_stricmp(data, "streammetadata"))
	{
		if (lstrcmpW(lastfn, fn))
			return 0;

		if (g_playing_file)
		{
			uint32_t len=0;
			EnterCriticalSection(&streamInfoLock);
			if (g_playing_file && g_playing_file->GetID3v2(&len) && len > 0) // check again now that we've acquired the lock
			{
				lstrcpynW(dest, L"1", (int)destlen);
				LeaveCriticalSection(&streamInfoLock);
				return 1;	// always return 1 to ensure we can do title lookups
			}
			LeaveCriticalSection(&streamInfoLock);
		}
		return 0;
	}
	else if (!_stricmp(data, "streamtitle"))
	{
		EnterCriticalSection(&streamInfoLock);
		if (g_playing_file) // check again now that we've acquired the lock
			ConvertTryUTF8(g_playing_file->stream_current_title, dest, destlen);
		else
			dest[0]=0;
		LeaveCriticalSection(&streamInfoLock);
		return 1;
	}
	else if (!_stricmp(data, "streamname"))
	{
		EnterCriticalSection(&streamInfoLock);
		if (g_playing_file) // check again now that we've acquired the lock
			ConvertTryUTF8(g_playing_file->stream_name, dest, destlen);
		else
			dest[0]=0;
		LeaveCriticalSection(&streamInfoLock);
		return 1;
	}
	else if (!_stricmp(data, "streamurl"))
	{
		EnterCriticalSection(&streamInfoLock);
		if (g_playing_file) // check again now that we've acquired the lock
			ConvertTryUTF8(g_playing_file->stream_url, dest, destlen);
		else
			dest[0]=0;
		LeaveCriticalSection(&streamInfoLock);
		return 1;
	}	
	else if (!_stricmp(data, "streamgenre"))
	{
		EnterCriticalSection(&streamInfoLock);
		if (g_playing_file) // check again now that we've acquired the lock
			ConvertTryUTF8(g_playing_file->stream_genre, dest, destlen);
		else
			dest[0]=0;
		LeaveCriticalSection(&streamInfoLock);
		return 1;
	}
	else if (!_stricmp(data, "streaminformation"))
 	{
 		EnterCriticalSection(&streamInfoLock);
 		if (g_playing_file)
 			g_playing_file->GetStreamInfo(dest, destlen);
 		else
 			dest[0]=0;
 		LeaveCriticalSection(&streamInfoLock);
 		return 1;
 	}

	if (!fn || !fn[0]) return 0;

	if (!_wcsnicmp(fn, L"uvox://", 7))
		return 0;

	if (g_playing_file && PathIsURL(fn) && !lstrcmpW(lastfn, fn))
	{
		EnterCriticalSection(&streamInfoLock);
		uint32_t len = 0;
		if (g_playing_file && g_playing_file->GetID3v2(&len) && len > 0) // check again now that we've acquired the lock
		{
			Metadata meta(g_playing_file, fn);
			int ret = meta.GetExtendedData(data, dest, (int)destlen);
			LeaveCriticalSection(&streamInfoLock);
			return ret;
		}
		LeaveCriticalSection(&streamInfoLock);
	}

	if (PathIsURL(fn))
		return 0;

	if (m_ext_get_mp3info && (!m_ext_get_mp3info->IsMe(fn) || HasFileTimeChanged(fn)))
	{
		m_ext_get_mp3info->Release();
		m_ext_get_mp3info=0;
	}

	if (!m_ext_get_mp3info)
	{
		m_ext_get_mp3info = new Metadata;
		m_ext_get_mp3info->Open(fn);
	}

	return m_ext_get_mp3info->GetExtendedData(data, dest, (int)destlen);
}

extern "C"
	__declspec(dllexport) int winampSetExtendedFileInfoW(const wchar_t *fn, const char *data, const wchar_t *val)
{
	if (!m_ext_set_mp3info || (m_ext_set_mp3info && !m_ext_set_mp3info->IsMe(fn)))
	{
		if(m_ext_set_mp3info) m_ext_set_mp3info->Release();
		m_ext_set_mp3info = new Metadata;
		m_ext_set_mp3info->Open(fn);
	}
	return m_ext_set_mp3info->SetExtendedData(data, val);
}

extern "C"
	__declspec(dllexport) int winampWriteExtendedFileInfo()
{
	// flush our read cache too :)
	if(m_ext_get_mp3info) m_ext_get_mp3info->Release();
	m_ext_get_mp3info = NULL;

	if (!m_ext_set_mp3info) return 0;

	Stopper stopper;
	if (m_ext_set_mp3info->IsMe(lastfn))
		stopper.Stop();

	// just in-case something changed
	if (!m_ext_set_mp3info) return 0;

	int ret = m_ext_set_mp3info->Save();
	stopper.Play();
	m_ext_set_mp3info->Release();
	m_ext_set_mp3info = NULL;

	// update last modified so we're not re-queried on our own updates
	UpdateFileTimeChanged(lastfn);

	return !ret;
}

extern "C" __declspec(dllexport)
	int winampClearExtendedFileInfoW(const wchar_t *fn)
{
	Metadata meta;
	if (meta.Open(fn)==METADATA_SUCCESS)
	{
		meta.id3v2.Clear();
		Stopper stopper;
		if (meta.IsMe(lastfn))
			stopper.Stop();
		meta.Save();
		stopper.Play();

		// update last modified so we're not re-queried on our own updates
		UpdateFileTimeChanged(fn);

		return 1;
	}
	return 0;
}