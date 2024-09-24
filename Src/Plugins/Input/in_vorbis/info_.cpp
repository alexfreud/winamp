//tag editor file i/o code, title formatting interface
#include "main.h"
#include "genres.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"
#include "api__in_vorbis.h"
#include <wchar.h>
#include <math.h>
#include <shlwapi.h>
#include "vcedit.h"
#include <strsafe.h>
#include "resource.h"

namespace ogg_helper	//chainedstream_parse
{
	int num_get_tracks(HANDLE hFile);
	int query_chained_stream_offset(HANDLE hFile, int idx, __int64 * out_beginning, __int64 * out_end);
}

/*static void xfer(HANDLE src, HANDLE dst, __int64 size)
{
	enum { BUFFER = 1024 * 1024 };
	void * buffer = malloc((int)(BUFFER > size ? size : BUFFER));
	while (size > 0)
	{
		int d = BUFFER;
		if ((__int64)d > size) d = (int)size;
		DWORD br = 0;
		ReadFile(src, buffer, d, &br, 0);
		WriteFile(dst, buffer, d, &br, 0);
		size -= d;
	}
}*/

static void seek64(HANDLE src, __int64 offset)
{
	__int64 temp = offset;
	SetFilePointer(src, *(DWORD*)&temp, ((long*)&temp + 1), FILE_BEGIN);
}

extern OSVERSIONINFO os_ver;
extern HANDLE hThread;

static DWORDLONG get_space(const wchar_t * url)
{
	ULARGE_INTEGER free_space;
	char zzz[4] = {(char)url[0], (char)url[1], (char)url[2], 0}; //"c:\";

	free_space.QuadPart = 0;

	if (os_ver.dwPlatformId == VER_PLATFORM_WIN32_NT || (os_ver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && LOWORD(os_ver.dwBuildNumber) > 1000))
	{
		static BOOL (WINAPI* pGetDiskFreeSpaceEx)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
		if (!pGetDiskFreeSpaceEx)
		{
			pGetDiskFreeSpaceEx = (BOOL (WINAPI*)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER))GetProcAddress(GetModuleHandle(L"kernel32.dll"), "GetDiskFreeSpaceExA");
		}
		if (pGetDiskFreeSpaceEx)
		{
			ULARGE_INTEGER blah1, blah2;
			pGetDiskFreeSpaceEx((LPCTSTR)zzz, &free_space, &blah1, &blah2);
		}
	}
	if (!free_space.QuadPart)
	{
		DWORD spc, bps, nfc, tnc;
		GetDiskFreeSpaceA(zzz, &spc, &bps, &nfc, &tnc);
		free_space.QuadPart = UInt32x32To64(spc * bps, nfc);
	}
	return free_space.QuadPart;
}

bool sync_movefile(const wchar_t * src, const wchar_t * dst);

struct vcedit_param
{
	HANDLE hFile;
	__int64 remaining;
};

static size_t callback_fread(void *ptr, size_t size, size_t nmemb, vcedit_param * param)
{
	int to_read = (int)(nmemb *size);
	if (to_read > param->remaining) to_read = (int)param->remaining;
	DWORD br = 0;
	ReadFile(param->hFile, ptr, to_read, &br, 0);
	param->remaining -= br;
	return br / size;
}

static size_t callback_write(const void *ptr, size_t size, size_t nmemb, HANDLE hFile)
{
	DWORD bw = 0;
	WriteFile(hFile, ptr, (DWORD)(size*nmemb), &bw, 0);
	return bw / size;
}

BOOL modify_file(const wchar_t* url, const vorbis_comment * comments, int links)
{	//also used for stream save fix
	HANDLE dst = INVALID_HANDLE_VALUE;
	int scream = 0;
	StringW tmp;

	winampGetExtendedFileInfoW_Cleanup();

	tmp = url;
	tmp += L".tmp";

	HANDLE src = CreateFileW(url, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (src != INVALID_HANDLE_VALUE)
	{
		ULARGE_INTEGER src_size;
		src_size.LowPart = GetFileSize(src, &src_size.HighPart);
		if (src_size.QuadPart > get_space(url))
		{ //shit happens... try default temp location
			StringW tempdir;
			GetTempPathW(MAX_PATH, StringTempW(tempdir, MAX_PATH));
			if (get_space(tempdir) < src_size.QuadPart)
			{ //oh well
				CloseHandle(src);
				src = INVALID_HANDLE_VALUE;
			}
			{
				tmp = tempdir;
				if (tmp[tmp.Length() - 1] != '\\') tmp.AddChar('\\');

				StringCchPrintfW(StringTempW(tempdir, MAX_PATH), MAX_PATH, L"ogg%u_%u.tmp", GetTickCount(), GetCurrentProcessId());
				tmp.AddString(tempdir);
			}
		}
		dst = CreateFileW(tmp, GENERIC_WRITE | GENERIC_READ, 0, 0, CREATE_ALWAYS, 0, 0);
	}

	if (dst != INVALID_HANDLE_VALUE && src != INVALID_HANDLE_VALUE)
	{
		{
			FILETIME ct;
			GetFileTime(src, &ct, 0, 0);
			SetFileTime(dst, &ct, 0, 0);
		}

		int num_links = ogg_helper::num_get_tracks(src);
		if (num_links < links) scream = 1;
		else
		{
			int cur_link;
			for (cur_link = 0; cur_link < links && !scream; cur_link++)
			{
				__int64 stream_beginning, stream_end;
				if (ogg_helper::query_chained_stream_offset(src, cur_link, &stream_beginning, &stream_end))
				{
					seek64(src, stream_beginning);
					vcedit_state *vs;
					vcedit_param param;
					param.hFile = src;
					param.remaining = stream_end - stream_beginning;
					vs = vcedit_new_state();
					if (vcedit_open_callbacks(vs, &param, (vcedit_read_func)callback_fread, (vcedit_write_func)callback_write) < 0)
					{
						scream = 1;
					}
					else
					{
						vorbis_comment * vc = vcedit_comments(vs);
						vorbis_comment_clear(vc);
						vorbis_comment_init(vc);
						const vorbis_comment * vc_src = comments + cur_link;

						int n;
						for (n = 0;n < vc_src->comments;n++)
						{
										if (vc_src->user_comments[n])
											vorbis_comment_add(vc, vc_src->user_comments[n]);
						}

						vcedit_write(vs, dst);
						vcedit_clear(vs);
					}
				}
			}
		}
	}
	else scream = 1;
	if (src != INVALID_HANDLE_VALUE) CloseHandle(src);
	if (dst != INVALID_HANDLE_VALUE)
	{
		CloseHandle(dst);
		if (scream)
		{
			DeleteFileW(tmp);
		}
	}

	if (!scream)
	{
		BOOL f_sync;
		EnterCriticalSection(&sync);

		f_sync = !_wcsicmp(url, cur_file) && hThread;	//check for i/o conflict with currently played file
		LeaveCriticalSection(&sync);
		if (f_sync)
		{ //drat, it's now playing
			scream = !sync_movefile(tmp, url);
		}
		else
		{
			if (!DeleteFileW(url)) scream = 1;
			else
			{
				if (!MoveFileW(tmp, url))
				{
					if (!CopyFileW(tmp, url, 0)) scream = 1;
					DeleteFileW(tmp);
				}
			}
		}
	}
	if (scream) return 0;
	else return 1;
}

wchar_t *wdup(const char * src)
{
	return _wcsdup(StringW(src));
}

extern StringW stat_disp;

void GetFileInfo(const wchar_t *file, wchar_t *title, int *len)
{
	VorbisFile* vf = 0;
	BOOL is_cur_file = 0;
	BOOL is_vf_local = 1;
	if (title) *title = 0;
	if (len) *len = -1;

	if (!file || !*file)
	{
		file = cur_file;
		is_cur_file = 1;
	}
	else if (!lstrcmpiW(file, cur_file))
	{
		is_cur_file = 1;
	}

	if (title && stat_disp.Length() > 0 && is_cur_file) 
	{
		lstrcpynW(title, stat_disp, 256);
		title = 0;
	}

	if (!len && !title) return ;

	if (is_cur_file)
	{
		EnterCriticalSection(&sync);
		if (theFile)
		{
			vf = theFile;
			is_vf_local = 0;
		}
		else
			LeaveCriticalSection(&sync);
	}

	if (!vf)
	{
		vf = VorbisFile::Create(file, 1);
		if (!vf)
		{
			if (title)
			{
				lstrcpynW(title, PathFindFileNameW(file), 256);
				wchar_t *blah = PathFindExtensionW(title);
				*blah=0;      
			}
			return ;
		}
	}

	if (len)
	{
		*len = (int)(vf->Length() * 1000);
	}

	if (title)
	{
		const char *t = vf->get_meta("ARTIST", 0);
		if (t)
		{
			MultiByteToWideCharSZ(CP_UTF8, 0, t, -1, title, 256);
			t = vf->get_meta("TITLE", 0);
			if (t)
			{
				StringCchCatW(title, 256, L" - ");
				StringCchCatW(title, 256, AutoWide(t, CP_UTF8));
			}
		}
		else
		{
			const char *t = vf->get_meta("TITLE", 0);
			if (t)
				MultiByteToWideCharSZ(CP_UTF8, 0, t, -1, title, 256);
			else
			{
				lstrcpynW(title, PathFindFileNameW(file), 256);
				wchar_t *blah = PathFindExtensionW(title);
				*blah=0;  
			}
		}
	}
	//q:
	if (is_vf_local)
		delete vf;
	else
		LeaveCriticalSection(&sync);
}

void w9x_itow(wchar_t *dest, int value, int destlen)
{
	StringCchPrintfW(dest, destlen, L"%d", value);

}
void w9x_utow(wchar_t *dest, int value, int destlen)
{
	StringCchPrintfW(dest, destlen, L"%u", value);
}
void w9x_htow(wchar_t *dest, int value, int destlen)
{
	StringCchPrintfW(dest, destlen, L"%08x", value);
}

static void print_misc(VorbisFile * _vf,int link,wchar_t * out, int outlen)
{
	OggVorbis_File * vf=&_vf->vf;
	double t=ov_time_total(vf,link);
	vorbis_info * vi=ov_info(vf,link);
	vorbis_comment * vc=ov_comment(vf,link);
	if (!vi || !vc) {WASABI_API_LNGSTRINGW_BUF(IDS_FILE_ERROR,out,outlen);return;}

	wchar_t kbps_str[16] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_KBPS, kbps_str, 16);

	wchar_t length[48]=L"", avgbitrate[48]=L"", filesize[48]=L"", nombitrate[48]=L"", maxbitrate[48]=L"", minbitrate[48]=L"";
	if (t>0)
	{
		int h = (int)(t/3600.0);
		int m = (int)(t/60.0)%60;
		int s = (int)t%60;
		
		if(h>0) StringCchPrintfW(length,48,L"%s%u:%02u:%02u\r\n",WASABI_API_LNGSTRINGW(IDS_LENGTH),h,m,s);
		else if(m>0) StringCchPrintfW(length,48,L"%s%u:%02u\r\n",WASABI_API_LNGSTRINGW(IDS_LENGTH),m,s);
		else if(s>0) StringCchPrintfW(length,48,L"%s%u\r\n",WASABI_API_LNGSTRINGW(IDS_LENGTH),s);

		UINT fs=_vf->FileSize();
		if (fs>0)
		{
			int kbps = (int)(((double)fs)/(t*125.0));
			wchar_t tmp[32] = {0};
			StringCchPrintfW(avgbitrate,48,L"%s%u %s\r\n",WASABI_API_LNGSTRINGW(IDS_AVERAGE_BITRATE),kbps,kbps_str);

			int fs1=fs/1000000;
			int fs2=(fs/1000)%1000;
			int fs3=fs%1000;
			if(fs1) 
				StringCchPrintfW(filesize,48,L"%s%u%03u%03u %s\r\n",WASABI_API_LNGSTRINGW(IDS_FILE_SIZE),fs1,fs2,fs3,WASABI_API_LNGSTRINGW_BUF(IDS_BYTES,tmp,32));
			else if(fs2) 
				StringCchPrintfW(filesize,48,L"%s%u%03u %s\r\n",WASABI_API_LNGSTRINGW(IDS_FILE_SIZE),fs2,fs3,WASABI_API_LNGSTRINGW_BUF(IDS_BYTES,tmp,32));
			else 
				StringCchPrintfW(filesize,48,L"%s%u %s\r\n",WASABI_API_LNGSTRINGW(IDS_FILE_SIZE),fs3,WASABI_API_LNGSTRINGW_BUF(IDS_BYTES,tmp,32));
		}
	}
	
	if (vi->bitrate_nominal>0)
		StringCchPrintfW(nombitrate,48,L"%s%u %s\r\n",WASABI_API_LNGSTRINGW(IDS_NOMINAL_BITRATE),vi->bitrate_nominal/1000,kbps_str);

	if (vi->bitrate_lower>0)
		StringCchPrintfW(minbitrate,48,L"%s%u %s\r\n",WASABI_API_LNGSTRINGW(IDS_MIN_BITRATE),vi->bitrate_lower/1000,kbps_str);

	if (vi->bitrate_nominal>0)
		StringCchPrintfW(maxbitrate,48,L"%s%u %s\r\n",WASABI_API_LNGSTRINGW(IDS_MAX_BITRATE),vi->bitrate_nominal/1000,kbps_str);
	
	wchar_t tmp[32] = {0}, tmp2[32] = {0}, tmp3[32] = {0}, tmp4[32] = {0}, tmp5[32] = {0}, hzStr[8] = {0};
	StringCchPrintfW(out,outlen,L"%s%s%s%s%s%s%s: %u\r\n%s: %u %s\r\n%s: %u\r\n%s: %u\r\n%s: \r\n%s",
			length, avgbitrate, filesize, nombitrate, maxbitrate, minbitrate,
			WASABI_API_LNGSTRINGW_BUF(IDS_CHANNELS,tmp,32),vi->channels,
			WASABI_API_LNGSTRINGW_BUF(IDS_SAMPLING_RATE,tmp2,32),vi->rate, WASABI_API_LNGSTRINGW_BUF(IDS_HZ,hzStr,8),
			WASABI_API_LNGSTRINGW_BUF(IDS_SERIAL_NUMBER,tmp3,32),ov_serialnumber(vf,link),
			WASABI_API_LNGSTRINGW_BUF(IDS_VERSION,tmp4,32),vi->version,
			WASABI_API_LNGSTRINGW_BUF(IDS_VENDOR,tmp5,32),(wchar_t*)AutoWide(vc->vendor,CP_UTF8));
}

static VorbisFile* last_vf = 0;
static wchar_t last_file[MAX_PATH] = {0};
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

// need to call this when we shut down just to make sure things are correctly cleaned up
//(the joys of caching for speed)
void winampGetExtendedFileInfoW_Cleanup(void)
{
	if (last_vf)
	{
		delete last_vf;
		last_vf = 0;
	}
	last_file[0] = 0;
}

static void CALLBACK winampGetExtendedFileInfoW_Timer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD elapsed)
{
	// TODO need to do a better way of getting and caching the metadata
	//		this is _just_ a temp fix for the file being locked when it
	//		it really needs 'class Info' to be able to cache and read.
	KillTimer(hwnd, eventId);
	winampGetExtendedFileInfoW_Cleanup();
}

bool KeywordMatch(const char *mainString, const char *keyword)
{
	return !_stricmp(mainString, keyword);
}

#define START_TAG_ALIAS(name, alias) if (KeywordMatch(data, name)) lookup=alias
#define TAG_ALIAS(name, alias) else if (KeywordMatch(data, name)) lookup=alias
extern "C" __declspec( dllexport ) int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, int destlen)
{
	if (!_stricmp(data, "type"))
	{
		dest[0] = '0';
		dest[1] = 0;
		return 1;
	}
	else if (!_stricmp(data, "rateable"))
	{
		dest[0] = '1';
		dest[1] = 0;
		return 1;
	}
	else if (!_stricmp(data, "streammetadata"))
	{
		return 0;
	}

	if (!fn || (fn && !fn[0])) return 0;

	if (!_stricmp(data, "family"))
	{
		LPCWSTR e;
		int pID = -1;
		DWORD lcid;
		e = PathFindExtensionW(fn);
		if (L'.' != *e) return 0;
		e++;
		lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
		if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"OGG", -1) ||  
			CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, e, -1, L"OGA", -1)) pID = IDS_FAMILY_STRING;
		if (pID != -1 && S_OK == StringCchCopyW(dest, destlen, WASABI_API_LNGSTRINGW(pID))) return 1;
		return 0;
	}

	if (!_stricmp(data, "mime"))
	{
		StringCchCopyW(dest, destlen, L"audio/ogg");
		return 1;
	}

	// attempt to cache/use a cached instance of VorbisFile to speed up metadata queries
	// which is especially needed with large ogg files (like with a 4Mb embedded image!)
	VorbisFile* vf = 0;
	if(last_file[0] && !_wcsicmp(last_file, fn) && !HasFileTimeChanged(fn))
	{
		vf = last_vf;
	}
	else
	{
		// different file so clean up if there's a cached instance
		if(last_vf)
		{
			delete last_vf;
			last_vf = 0;
		}
		// open the new file and cache the current filename for subsequent query checks
		vf = VorbisFile::Create(fn, 1);
		lstrcpynW(last_file, fn, MAX_PATH);
	}

	if (!vf) return 0;
	else last_vf = vf;

	// TODO need to do a better way of getting and caching the metadata
	//		this is _just_ a temp fix for the file being locked when it
	//		it really needs 'class Info' to be able to cache and read.
	SetTimer(mod.hMainWindow, 256, 2000, winampGetExtendedFileInfoW_Timer);

	const char *lookup = 0;
	if (!_stricmp(data, "length"))
	{
		int len = (int)(vf->Length() * 1000);
		w9x_itow(dest, len, destlen);
		return 1;
	}
	else if (!_stricmp(data, "bitrate"))
	{
		int br = vf->get_avg_bitrate();
		w9x_itow(dest, br, destlen);
		return 1;
	}
	else if (!_stricmp(data, "SERIALNUMBER"))
	{
		w9x_utow(dest, ov_serialnumber(&vf->vf, -1), destlen);
		return 1;
	}
	else if (!_stricmp(data, "SERIALNUMBER_HEX"))
	{
		w9x_htow(dest, ov_serialnumber(&vf->vf, -1), destlen);
		return 1;
	}
	else if (!_stricmp(data, "gain"))
	{
		float gain = 20.0f*(float)log10(vf->GetGain());
		StringCchPrintfW(dest, destlen, L"%-+.2f dB", gain);
		return 1;
	}
	else if(!_stricmp(data,"formatinformation"))
	{
		print_misc(vf,0,dest,destlen);
		return 1;
	}
	TAG_ALIAS("title", "TITLE");
	TAG_ALIAS("artist", "ARTIST");
	TAG_ALIAS("album", "ALBUM");
	TAG_ALIAS("genre", "GENRE");
	TAG_ALIAS("comment", "COMMENT");
	TAG_ALIAS("year", "DATE");
	TAG_ALIAS("track", "TRACKNUMBER");
	TAG_ALIAS("albumartist", "ALBUMARTIST");
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

	const char *value = vf->get_meta(lookup, 0);

	if(KeywordMatch("comment",data)) {
		if(!value || !*value) value = vf->get_meta("DESCRIPTION", 0);
	}

	if(KeywordMatch("year",data)) {
		if(!value || !*value) value = vf->get_meta("YEAR", 0);
	}

	if(KeywordMatch("track",data)) {
		if(!value || !*value) value = vf->get_meta("TRACK", 0);
	}

	if(KeywordMatch("albumartist",data)) {
		if(!value || !*value) value = vf->get_meta("ALBUM ARTIST", 0);
		if(!value || !*value) value = vf->get_meta("ENSEMBLE", 0);
	}

	if(KeywordMatch("publisher",data)) {
		if(!value || !*value) value = vf->get_meta("ORGANIZATION", 0);
	}

	if(KeywordMatch("category",data)) {
		if(!value || !*value) value = vf->get_meta("CONTENTGROUP", 0);
		if(!value || !*value) value = vf->get_meta("GROUPING", 0);
	}

	if(KeywordMatch(data, "rating")) {
		if(!value || !*value) value = vf->get_meta("RATING", 0);
		if(value && *value) {
			int rating = atoi(value);
			// keeps things limited to our range of 0-100
			if (rating >= 100) {
				rating = 5;
			}
			// 1-100 case
			else if (rating > 5 && rating < 100) {
				rating /= 20;
				// shift up by one rating when in next band
				// 1-20 = 1, 21-40 = 2, 41-60 = 3, 61-80 = 4, 81-100 = 5
				rating += ((atoi(value) - (rating * 20)) > 0);
			}
			// Remove support for old 1-10 range
			/* or maybe we're dealing with a 1-10 range
			else if (rating > 5) {
				rating /= 2;
			} */
			
			// otherwise it is hopefully in the 0-5 range
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

	if(value)
		MultiByteToWideCharSZ(CP_UTF8, 0, value, -1, dest, destlen);
	else
	{
		dest[0]=0;
		return 1;
	}

	return 1;
}