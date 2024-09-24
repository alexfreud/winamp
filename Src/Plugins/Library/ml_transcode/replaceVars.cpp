#include <windows.h>
#include <ctype.h>
#include <string.h>
#include "..\..\General\gen_ml/ml.h"

#include <shlwapi.h>
#include <strsafe.h>

static void removebadchars(wchar_t *s, wchar_t *&end, size_t &str_size)
{
	const wchar_t *start = s;
	while (s && *s)
	{
		if (*s == L'?' || *s == L'/' || *s == L'\\' || *s == L':' || *s == L'*' || *s == L'\"' || *s == L'<' || *s == L'>' || *s == L'|') 
			*s = L'_';
		s = CharNextW(s);
	}

	// cut off trailing periods
	if (s != start)
	{
		do
		{
			s--;
			if (*s == '.')
			{
				*s = '_';
				end--;
				str_size++;
			}
			else
				break;
		}
		while (s != start);
	}
}

void RecursiveCreateDirectory(wchar_t *str)
{
	wchar_t *p = str;
	if ((p[0] ==L'\\' || p[0] ==L'/') && (p[1] ==L'\\' || p[1] ==L'/'))
	{
		p += 2;
		while (p &&*p && *p !=L'\\' && *p !=L'/') p++;
		if (!p || !*p) return ;
		p++;
		while (p && *p && *p !=L'\\' && *p !=L'/') p++;
	}
	else
	{
		while (p && *p && *p !=L'\\' && *p !=L'/') p++;
	}

	while (p && *p)
	{
		while (p && *p && *p !=L'\\' && *p !=L'/') p = CharNextW(p);
		if (p && *p)
		{
			wchar_t lp = *p;
			*p = 0;
			CreateDirectoryW(str, NULL);
			*p++ = lp;
		}
	}
}

void FixFileLength(wchar_t *str)
{
	size_t len = wcslen(str);
	if (len >= MAX_PATH) // no good
	{
		size_t extensionPosition=len;
		while (extensionPosition--)
		{
			if (str[extensionPosition]=='.' || str[extensionPosition] == '/' || str[extensionPosition] == '\\')
				break;
		}
		if (str[extensionPosition]=='.') // found an extension? good, let's relocate it
		{
			ptrdiff_t diff = len - extensionPosition;
			diff++;
			while (diff)
			{
  				str[MAX_PATH-diff]=str[len-diff+1];
				diff--;
			}
		}
		str[MAX_PATH-1]=0;
	}
}

static void ADD_STR(const wchar_t *x, wchar_t *&outp, size_t &str_size)
{
	wchar_t *end = outp; 
	StringCchCopyExW(outp, str_size, (x) ? (x) : L"", &end, &str_size, 0);
	removebadchars(outp, end, str_size); 
	outp = end;
}

static void ADD_STR_U(const wchar_t *x, wchar_t *&outp, size_t &str_size)
{
	wchar_t *end = outp; 
	StringCchCopyExW(outp, str_size, (x) ? (x) : L"", &end, &str_size, 0); 
	removebadchars(outp, end, str_size);
	CharUpperW(outp);
	outp = end;
}

static void ADD_STR_L(const wchar_t *x, wchar_t *&outp, size_t &str_size)
{
	wchar_t *end = outp;
	StringCchCopyExW(outp, str_size, (x)?(x):L"", &end, &str_size,0); 
	removebadchars(outp, end, str_size);  
	CharLowerW(outp);
	outp=end; 
}

// FixReplacementVars: replaces <Artist>, <Title>, <Album>, and #, ##, ##, with appropriate data
wchar_t *FixReplacementVars(wchar_t *str, size_t str_size, itemRecordW * song)
{
	wchar_t tmpsrc[4096] = {0};
	lstrcpyn(tmpsrc,str,sizeof(tmpsrc)/sizeof(wchar_t)); //lstrcpyn is nice enough to make sure it's null terminated.

	wchar_t *inp = tmpsrc;
	wchar_t *outp = str;

	while (inp && *inp && str_size)
	{
		if (*inp == L'<')
		{
			if (!wcsncmp(inp,L"<TITLE>",7))
			{
				ADD_STR_U(song->title, outp, str_size);
				inp+=7;
			}
			else if (!wcsncmp(inp,L"<title>",7))
			{
				ADD_STR_L(song->title, outp, str_size);
				inp+=7;
			}
			else if (!_wcsnicmp(inp,L"<Title>",7))
			{
				ADD_STR(song->title, outp, str_size);
				inp+=7;
			}
			else if (!wcsncmp(inp,L"<ALBUM>",7))
			{
				ADD_STR_U(song->album, outp, str_size);
				inp+=7;
			}
			else if (!wcsncmp(inp,L"<album>",7))
			{
				ADD_STR_L(song->album, outp, str_size);
				inp+=7;
			}
			else if (!_wcsnicmp(inp,L"<Album>",7))
			{
				ADD_STR(song->album, outp, str_size);
				inp+=7;
			}
			else if (!wcsncmp(inp,L"<GENRE>",7))
			{
				ADD_STR_U(song->genre, outp, str_size);
				inp+=7;
			}
			else if (!wcsncmp(inp,L"<genre>",7))
			{
				ADD_STR_L(song->genre, outp, str_size);
				inp+=7;
			}
			else if (!_wcsnicmp(inp,L"<Genre>",7))
			{
				ADD_STR(song->genre, outp, str_size);
				inp+=7;
			}
			else if (!wcsncmp(inp,L"<ARTIST>",8))
			{
				ADD_STR_U(song->artist, outp, str_size);
				inp+=8;
			}
			else if (!wcsncmp(inp,L"<artist>",8))
			{
				ADD_STR_L(song->artist, outp, str_size);
				inp+=8;
			}
			else if (!_wcsnicmp(inp,L"<Artist>",8))
			{
				ADD_STR(song->artist, outp, str_size);
				inp+=8;
			}
			else if (!wcsncmp(inp,L"<ALBUMARTIST>",13))
			{
				if (song->albumartist && song->albumartist[0])
					ADD_STR_U(song->albumartist, outp, str_size);
				else
					ADD_STR_U(song->artist, outp, str_size);
				inp+=13;
			}
			else if (!wcsncmp(inp,L"<albumartist>",13))
			{
				if (song->albumartist && song->albumartist[0])
					ADD_STR_L(song->albumartist, outp, str_size);
				else
					ADD_STR_L(song->artist, outp, str_size);
				inp+=13;
			}
			else if (!_wcsnicmp(inp,L"<Albumartist>",13))
			{
				if (song->albumartist && song->albumartist[0])
					ADD_STR(song->albumartist, outp, str_size);
				else
					ADD_STR(song->artist, outp, str_size);
				inp+=13;
			}
			else if (!_wcsnicmp(inp,L"<year>",6))
			{
				wchar_t year[64] = {0};
				if(song->year==0) year[0]=0;
				else StringCchPrintf(year, 64, L"%d", song->year);
				ADD_STR(year, outp, str_size);
				inp+=6;
			}
			else if (!_wcsnicmp(inp,L"<disc>",6))
			{
				wchar_t disc[16] = {0};
				if(song->disc==0) disc[0]=0;
				else StringCchPrintf(disc, 16, L"%d", song->disc);
				ADD_STR(disc, outp, str_size);
				inp+=6;
			}
			else if (!_wcsnicmp(inp,L"<discs>",7))
			{
				wchar_t discs[32] = {0};
				if(song->disc==0) discs[0]=0;
				else StringCchPrintf(discs, 32, L"%d", song->discs);
				ADD_STR(discs, outp, str_size);
				inp+=7;
			}
			else if(!_wcsnicmp(inp,L"<filename>",10))
			{
				wchar_t *fn = _wcsdup(PathFindFileName(song->filename));
				const wchar_t *insert;

				if (NULL != fn &&
					FALSE != PathIsFileSpec(fn))
				{
					PathRemoveExtension(fn);
					insert = fn;
				}
				else
					insert = NULL;

				if (NULL == insert || L'\0' == *insert)
					insert = L"<filename>";

				ADD_STR(insert, outp, str_size);

				free(fn);
				inp+=10;
			}
			else *outp++=*inp++;
		}
		else if (*inp == '#')
		{
			int nd = 0;
			wchar_t tmp[64] = {0};
			while (inp && *inp =='#') nd++,inp++;

			if (!song->track)
			{
				tmp[0] = 0;
				while (inp && *inp == ' ') inp++;
				if (inp && (*inp == '-' || *inp == '.' || *inp == '_')) // separator
				{
					inp++;
					while (inp && *inp == ' ') inp++;
				}
			}
			else
			{
				if (nd > 1)
				{
					wchar_t tmp2[32] = {0};
					if (nd > 5) nd=5;
					StringCchPrintf(tmp2, 32, L"%%%02dd",nd);
					StringCchPrintf(tmp, 64, tmp2,song->track);
				}
				else StringCchPrintf(tmp, 64, L"%d", song->track);
			}
			ADD_STR(tmp, outp, str_size);
		}
		else *outp++ = *inp++;
	}
	if (outp) *outp = 0;

	inp = str;
	outp = str;
	wchar_t lastc=0;
	while (inp && *inp) 
	{
		wchar_t c=*inp++;
		if (c == '\t') c=' ';

		if (c == ' ' && (lastc == ' ' || lastc == '\\' || lastc == '/')) continue; // ignore space after slash, or another space
	    
		if ((c == '\\' || c == '/') && lastc == ' ') outp--;  // if we have a space then slash, back up to write the slash where the space was
		*outp++ = c;
		lastc = c;
	}
	if (outp) *outp=0;

	#undef ADD_STR
	#undef ADD_STR_L
	#undef ADD_STR_U

	return(str);
}