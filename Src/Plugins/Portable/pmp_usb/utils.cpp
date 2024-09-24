#include <windows.h>
#include "../../General/gen_ml/ml.h"
#include "../../Library/ml_pmp/pmp.h"
#include "USBDevice.h"
#include <strsafe.h>
#include <shlwapi.h>

BOOL RecursiveCreateDirectory(wchar_t* buf1);
__int64 fileSize(wchar_t * filename);
bool supportedFormat(wchar_t * file, wchar_t * supportedFormats);
DeviceType detectDeviceType(wchar_t drive);
void removebadchars(wchar_t *s);
wchar_t * FixReplacementVars(wchar_t *str, int str_size, Device * dev, songid_t song);

void removebadchars(wchar_t *s) {
	while (s && *s)
	{
		if (*s == L'?' || *s == L'/' || *s == L'\\' || *s == L':' || *s == L'*' || *s == L'\"' || *s == L'<' || *s == L'>' || *s == L'|') 
			*s = L'_';
		s = CharNextW(s);
	}
}

DeviceType detectDeviceType(wchar_t drive) {
	// Check for device being a PSP
	{
		wchar_t memstickind[] = {drive, L":\\MEMSTICK.IND"};
		FILE * f = _wfopen(memstickind,L"rb");
		if(f) {
			fclose(f);
			WIN32_FIND_DATA FindFileData = {0};
			wchar_t pspdir[] = {drive,L":\\PSP"};
			HANDLE hFind = FindFirstFile(pspdir, &FindFileData);
			if(hFind != INVALID_HANDLE_VALUE) {
				FindClose(hFind);
				if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				return TYPE_PSP;
			}
		}
	}
	// Cannot recognise device type
	return TYPE_OTHER;
}

__int64 fileSize(wchar_t * filename)
{
	WIN32_FIND_DATA f={0};
	HANDLE h = FindFirstFileW(filename,&f);
	if(h == INVALID_HANDLE_VALUE) return -1;
	FindClose(h);
	ULARGE_INTEGER i;
	i.HighPart = f.nFileSizeHigh;
	i.LowPart = f.nFileSizeLow;
	return i.QuadPart;
}

// RecursiveCreateDirectory: creates all non-existent folders in a path
BOOL RecursiveCreateDirectory(wchar_t* buf1) {
	wchar_t *p=buf1;
	int errors = 0;
	if (*p) {
		p = PathSkipRoot(buf1);
		if (!p) return true ;

	wchar_t ch='c';
	while (ch) {
		while (p && *p != '\\' && *p) p = CharNext(p);
		ch = (p ? *p : 0);
		if (p) *p = 0;
		int pp = (int)wcslen(buf1)-1;

		while(buf1[pp] == '.' || buf1[pp] == ' ' || 
			  (buf1[pp] == '\\' && (buf1[pp-1] == '.' || buf1[pp-1] == ' ' || buf1[pp-1] == '/')) ||
			  buf1[pp] == '/' && buf1)
		{
			if(buf1[pp] == '\\')
			{
				buf1[pp-1] = '_';
				pp -= 2;
			} else {
				buf1[pp] = '_';
				pp--;
			}
		}

		WIN32_FIND_DATA fd = {0};
		// Avoid a "There is no disk in the drive" error box on empty removable drives
		UINT prevErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
		HANDLE h = FindFirstFile(buf1,&fd);
		SetErrorMode(prevErrorMode);
		if (h == INVALID_HANDLE_VALUE)
		{
			if (!CreateDirectory(buf1,NULL)) errors++;
			} else {
				FindClose(h);
				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) errors++;
			}
			if (p) *p++ = ch;
		}
	}

	return errors != 0;
}

bool supportedFormat(wchar_t * file, wchar_t * supportedFormats) {
	wchar_t * ext = wcsrchr(file,'.');
	if(!ext) return false;
	ext++;
	wchar_t * p = supportedFormats;
	while(p && *p) {
		bool ret=false;
		wchar_t * np = wcschr(p,L';');
		if(np) *np = 0;
		if(!_wcsicmp(ext,p)) ret=true;
		if(np) { *np = L';'; p=np+1; } 
		else return ret;
		if(ret) return true; 
	}
	return false;
}

// FixReplacementVars: replaces <Artist>, <Title>, <Album>, and #, ##, ##, with appropriate data
// DOES NOT add a file extention!!
wchar_t * fixReplacementVars(wchar_t *str, int str_size, Device * dev, songid_t song)
{
	#define ADD_STR(x) { x(song,outp,str_size-1-(outp-str)); removebadchars(outp); while (outp && *outp) outp++; }
	#define ADD_STR_U(x) { x(song,outp,str_size-1-(outp-str)); removebadchars(outp); while (outp && *outp) { *outp=towupper(*outp); outp++; } }
	#define ADD_STR_L(x) { x(song,outp,str_size-1-(outp-str)); removebadchars(outp); while (outp && *outp) { *outp=towlower(*outp); outp++; } }

	wchar_t tmpsrc[4096] = {0};
	lstrcpyn(tmpsrc,str,sizeof(tmpsrc)/sizeof(wchar_t)); //lstrcpyn is nice enough to make sure it's null terminated.

	wchar_t *inp = tmpsrc;
	wchar_t *outp = str;
	int slash = 0;

	while (inp && *inp && outp-str < str_size-2)
	{
		if (*inp == L'<')
		{
			if (!wcsncmp(inp,L"<TITLE>",7))
			{
				ADD_STR_U(dev->getTrackTitle);
				inp+=7;
			}
			else if (!wcsncmp(inp,L"<title>",7))
			{
				ADD_STR_L(dev->getTrackTitle);
				inp+=7;
			}
			else if (!_wcsnicmp(inp,L"<Title>",7))
			{
				ADD_STR(dev->getTrackTitle);
				inp+=7;
			}
			else if (!wcsncmp(inp,L"<ALBUM>",7))
			{
				ADD_STR_U(dev->getTrackAlbum);
				inp+=7;
			}
			else if (!wcsncmp(inp,L"<album>",7))
			{
				ADD_STR_L(dev->getTrackAlbum);
				inp+=7;
			}
			else if (!_wcsnicmp(inp,L"<Album>",7))
			{
				ADD_STR(dev->getTrackAlbum);
				inp+=7;
			}
			else if (!wcsncmp(inp,L"<GENRE>",7))
			{
				ADD_STR_U(dev->getTrackGenre);
				inp+=7;
			}
			else if (!wcsncmp(inp,L"<genre>",7))
			{
				ADD_STR_L(dev->getTrackGenre);
				inp+=7;
			}
			else if (!_wcsnicmp(inp,L"<Genre>",7))
			{
				ADD_STR(dev->getTrackGenre);
				inp+=7;
			}
			else if (!wcsncmp(inp,L"<ARTIST>",8))
			{
				ADD_STR_U(dev->getTrackArtist);
				inp+=8;
			}
			else if (!wcsncmp(inp,L"<artist>",8))
			{
				ADD_STR_L(dev->getTrackArtist);
				inp+=8;
			}
			else if (!_wcsnicmp(inp,L"<Artist>",8))
			{
				ADD_STR(dev->getTrackArtist);
				inp+=8;
			}
			else if (!wcsncmp(inp,L"<ALBUMARTIST>",13))
			{
				wchar_t temp[128] = {0};

				dev->getTrackAlbumArtist(song, temp, 128);
				if (temp[0] == 0)
					dev->getTrackArtist(song, temp, 128);

				lstrcpyn(outp,temp,str_size-1-(int)(outp-str));
				removebadchars(outp);
				while (outp && *outp) { *outp=towupper(*outp); outp++; }

				inp+=13;
			}
			else if (!wcsncmp(inp,L"<albumartist>",13))
			{
				wchar_t temp[128] = {0};

				dev->getTrackAlbumArtist(song, temp, 128);
				if (temp[0] == 0)
					dev->getTrackArtist(song, temp, 128);

				lstrcpyn(outp,temp,str_size-1-(int)(outp-str));
				removebadchars(outp);
				while (outp && *outp) { *outp=towlower(*outp); outp++; }
				inp+=13;
			}
			else if (!_wcsnicmp(inp,L"<Albumartist>",13))
			{
				wchar_t temp[128] = {0};

				dev->getTrackAlbumArtist(song, temp, 128);
				if (temp[0] == 0)
					dev->getTrackArtist(song, temp, 128);

				lstrcpyn(outp,temp,str_size-1-(int)(outp-str));
				removebadchars(outp);
				while (outp && *outp) outp++;
				inp+=13;
			}
			else if (!_wcsnicmp(inp,L"<year>",6))
			{
				wchar_t year[64] = {0};
				int y = dev->getTrackYear(song);
				if (y) StringCchPrintf(year, ARRAYSIZE(year), L"%d", y);
				lstrcpyn(outp,year,str_size-1-(int)(outp-str)); while (outp && *outp) outp++;
				inp+=6;
			}
			else if (!_wcsnicmp(inp,L"<disc>",6))
			{
				wchar_t disc[16] = {0};
				int d = dev->getTrackDiscNum(song);
				if (d) StringCchPrintf(disc, ARRAYSIZE(disc), L"%d", d);
				lstrcpyn(outp,disc,str_size-1-(int)(outp-str)); while (outp && *outp) outp++;
				inp+=6;
			}
			else if (!_wcsnicmp(inp,L"<filename>",10))
			{
				wchar_t tfn[MAX_PATH], *ext, *fn;
				StringCchCopy(tfn,MAX_PATH,((UsbSong*)song)->filename);
				ext = wcsrchr(tfn, L'.');
				*ext = 0; //kill extension since its added later
				fn = wcsrchr(tfn, L'\\');
				fn++;
				lstrcpyn(outp,fn,str_size-1-(int)(outp-str));
				while (outp && *outp) outp++;
				inp+=10;
			}
			else
			{
				// use this to skip over unknown tags
				while (inp && *inp && *inp != L'>') inp++;
				if (inp) inp++;
			}
		}
		else if (*inp == L'#')
		{
			int nd=0;
			wchar_t tmp[64] = {0};
			while (inp && *inp == L'#') nd++,inp++;
			int track = dev->getTrackTrackNum(song);
			if (!track)
			{
				while (inp && *inp == L' ') inp++;
				while (inp && *inp == L'\\') inp++; 
				if (inp && (*inp == L'-' || *inp == L'.' || *inp == L'_')) // separator
				{
					inp++;
					while (inp && *inp == L' ') inp++;
				}
			}
			else
			{
				if (nd > 1)
				{
					wchar_t tmp2[32] = {0};
					if (nd > 5) nd=5;
					StringCchPrintf(tmp2, ARRAYSIZE(tmp2), L"%%%02dd",nd);
					StringCchPrintf(tmp, ARRAYSIZE(tmp), tmp2,track);
				}
				else StringCchPrintf(tmp, ARRAYSIZE(tmp), L"%d",track);
			}
			lstrcpyn(outp,tmp,str_size-1-(int)(outp-str)); while (outp && *outp) outp++;
		}
		else
		{
			if (*inp == L'\\') slash += 1;
			*outp++=*inp++;
		}
	}

	if (outp) *outp = 0;

	// if we end up with something like U:\\\ then this
	// will set it to be just <filename> so it'll not
	// end up making a load of bad files e.g. U:\.mp3
	int out_len = lstrlen(str);
	if (out_len)
	{
		if (out_len - 2 == slash)
		{
			outp = str + 3;
			wchar_t tfn[MAX_PATH], *ext, *fn;
			StringCchCopy(tfn,MAX_PATH,((UsbSong*)song)->filename);
			ext = wcsrchr(tfn, L'.');
			*ext = 0; //kill extension since its added later
			fn = wcsrchr(tfn, L'\\');
			fn++;
			lstrcpyn(outp,fn,str_size-1-(int)(outp-str));
			while (outp && *outp) outp++;
			if (outp) *outp = 0;
		}
	}

	inp=str;
	outp=str;
	wchar_t lastc=0;
	while (inp && *inp) 
	{
		wchar_t ch=*inp++;
		if (ch == L'\t') ch=L' ';

		if (ch == L' ' && (lastc == L' ' || lastc == L'\\' || lastc == L'/')) continue; // ignore space after slash, or another space
    
		if ((ch == L'\\' || ch == L'/') && lastc == L' ') outp--;  // if we have a space then slash, back up to write the slash where the space was
		*outp++=ch;
		lastc=ch;
	}
	if (outp) *outp=0;
	#undef ADD_STR
	#undef ADD_STR_L
	#undef ADD_STR_U

	return str;
}