#include <windows.h>
#include "../../General/gen_ml/ml.h"
#include "pmp.h"
#include <strsafe.h>
#include "api__ml_pmp.h"
#include "resource1.h"
extern winampMediaLibraryPlugin plugin;

wchar_t * FixReplacementVars(wchar_t *str, int str_size, Device * dev, songid_t song);
BOOL RecursiveCreateDirectory(wchar_t* buf1);
void doFormatFileName(wchar_t out[MAX_PATH], wchar_t *fmt, int trackno, wchar_t *artist, wchar_t *album, wchar_t *title, wchar_t *genre, wchar_t *year, wchar_t *trackartist);

static void removebadchars(wchar_t *s) {
	while (s && *s)
	{
		if (*s == L'?' || *s == L'/' || *s == L'\\' || *s == L':' || *s == L'*' || *s == L'\"' || *s == L'<' || *s == L'>' || *s == L'|') 
			*s = L'_';
		s = CharNextW(s);
	}
}

// Skip_Root: removes drive/host/share name in a path
wchar_t * Skip_Root(wchar_t *path) {
	wchar_t *p = CharNext(path);
	wchar_t *p2 = CharNext(p);
	if (*path && *p == L':' && *p2 == L'\\') return CharNext(p2);
	else if (*path == L'\\' && *p == L'\\') {
		// skip host and share name
		int x = 2;
		while (x--) {
		while (p2 && *p2 != L'\\') {
			if (!p2 || !*p2) return NULL;
			p2 = CharNext(p2);
		}
		p2 = CharNext(p2);
		}
		return p2;
	}
	return NULL;
}

// RecursiveCreateDirectory: creates all non-existent folders in a path
BOOL RecursiveCreateDirectory(wchar_t* buf1) {
	wchar_t *p=buf1;
	int errors = 0;
	if (*p) {
		p = Skip_Root(buf1);
		if (!p) return true ;

		wchar_t ch='c';
		while (ch) {
			while (p && *p != '\\' && *p) p=CharNext(p);
			ch=*p;
			if (p) *p=0;
			int pp = wcslen(buf1)-1;

			while(buf1[pp] == '.' || 
					buf1[pp] == ' ' || 
					(buf1[pp] == '\\' && (buf1[pp-1] == '.' || buf1[pp-1] == ' ' || buf1[pp-1] == '/'))
					|| buf1[pp] == '/' && buf1)
			{
				if(buf1[pp] == '\\')
				{
					buf1[pp-1] = '_';
					pp -= 2;
				}else{
					buf1[pp] = '_';
					pp--;
				}
			}

			HANDLE h;
			WIN32_FIND_DATA fd;
			// Avoid a "There is no disk in the drive" error box on empty removable drives
			UINT prevErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
			h = FindFirstFile(buf1,&fd);
			SetErrorMode(prevErrorMode);
			if (h == INVALID_HANDLE_VALUE)
			{ 
				if (!CreateDirectory(buf1,NULL)) errors++;
			} else {
				FindClose(h);
				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) errors++;
			}
			*p++ = ch;
		}
	}
	return errors != 0;
}

// FixReplacementVars: replaces <Artist>, <Title>, <Album>, and #, ##, ##, with appropriate data
// DOES NOT add a file extention!!
wchar_t * FixReplacementVars(wchar_t *str, int str_size, Device * dev, songid_t song)
{
	wchar_t artist[256]=L"",album[256]=L"",albumartist[256]=L"",title[256]=L"",genre[256]=L"",year[10]=L"",dest[MAX_PATH]=L"";
	dev->getTrackArtist(song,artist,256);
	dev->getTrackAlbum(song,album,256);
	dev->getTrackAlbumArtist(song,albumartist,256);
	if(!albumartist[0]) lstrcpyn(albumartist,artist,256);
	dev->getTrackTitle(song,title,256);
	dev->getTrackGenre(song,genre,256);
	int y = dev->getTrackYear(song);
	if(y>0) StringCchPrintfW(year,10,L"%d",y);
	wchar_t unknown[32] = {0}, unknownartist[32] = {0}, unknownalbum[32] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN,unknown,32);
	WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN_ARTIST,unknownartist,32);
	WASABI_API_LNGSTRINGW_BUF(IDS_UNKKNOWN_ALBUM,unknownalbum,32);
	doFormatFileName(dest,str,
					 dev->getTrackTrackNum(song),
					 *albumartist ? albumartist : unknownartist,
					 *album ? album : unknownalbum,
					 *title ? title : L"0",
					 *genre ? genre : unknown,
					 *year ? year : unknown,
					 *artist ? artist : unknown
	);
	bool c = (str[1]==':');
	lstrcpyn(str,dest,str_size);
	if(c) str[1]=L':';
	return str;
}

static void CleanDirectory(wchar_t *str)
{
	if (!str)
		return ;
	int l = wcslen(str);

	while (l--)
	{
		if (str[l] == L' '
		        || str[l] == L'.')
			str[l] = 0;
		else
			break;
	}
}

void doFormatFileName(wchar_t out[MAX_PATH], wchar_t *fmt, int trackno, wchar_t *artist, wchar_t *album, wchar_t *title, wchar_t *genre, wchar_t *year, wchar_t *trackartist)
{
	CleanDirectory(artist);
	CleanDirectory(album);
	CleanDirectory(title);
	CleanDirectory(genre);
	CleanDirectory(year);
	while (fmt && *fmt)
	{
		int whichstr = 0;
		if (*fmt == L'#' && trackno != 0xdeadbeef)
		{
			int cnt = 0;
			while (fmt && *fmt == L'#')
			{
				fmt++;
				cnt++;
			}
			if (cnt > 8) cnt = 8;
			wchar_t specstr[32] = {0};
			StringCchPrintf(specstr, 32, L"%%%02dd", cnt);
			wchar_t tracknostr[32] = {0};
			StringCchPrintf(tracknostr, 32, specstr, trackno);
			StringCchCat(out, MAX_PATH, tracknostr);
		}
		else if (artist && !_wcsnicmp(fmt, L"<artist>", 8)) whichstr = 1;
		else if (album && !_wcsnicmp(fmt, L"<album>", 7)) whichstr = 2;
		else if (title && !_wcsnicmp(fmt, L"<title>", 7)) whichstr = 3;
		else if (genre && !_wcsnicmp(fmt, L"<genre>", 7)) whichstr = 4;
		else if (year && !_wcsnicmp(fmt, L"<year>", 6)) whichstr = 5;
		else if (year && !_wcsnicmp(fmt, L"<trackartist>", 13)) whichstr=6;
		else
		{
			wchar_t p[2] = {0};
			p[0] = *fmt++;
			if (p[0] == L'?' || p[0] == L'*' || p[0] == L'|') p[0] = L'_';
			else if (p[0] == L':') p[0] = L'-';
			else if (p[0] == L'\"') p[0] = L'\'';
			else if (p[0] == L'<') p[0] = L'(';
			else if (p[0] == L'>') p[0] = L')';
			p[1] = 0;
			StringCchCat(out, MAX_PATH, p);
		}
		if (whichstr > 0)
		{
			int islow = IsCharLowerW(fmt[1]) && IsCharLowerW(fmt[2]);
			int ishi = IsCharUpperW(fmt[1]) && IsCharUpperW(fmt[2]);
			wchar_t *src;
			if (whichstr == 1) { src = artist; fmt += 8; }
			else if (whichstr == 2) { src = album; fmt += 7; }
			else if (whichstr == 3) { src = title; fmt += 7; }
			else if (whichstr == 4) { src = genre; fmt += 7; }
			else if (whichstr == 5) { src = year; fmt += 6; }
			else if (whichstr == 6) { src= trackartist; fmt+=13; }
			else break;

			while (src && *src)
			{
				wchar_t p[2] = {src[0], 0};
				if (ishi) CharUpperBuffW(p, 1);
				else if (islow) CharLowerBuffW(p, 1);

				if (p[0] == L'?' || p[0] == L'*' || p[0] == L'|') p[0] = L'_';
				else if (p[0] == L'/' || p[0] == L'\\' || p[0] == L':') p[0] = L'-';
				else if (p[0] == L'\"') p[0] = L'\'';
				else if (p[0] == L'<') p[0] = L'(';
				else if (p[0] == L'>') p[0] = L')';

				src++;
				p[1] = 0;
				StringCchCat(out, MAX_PATH, p);
			}
		}
	}
}