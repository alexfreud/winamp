#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

wchar_t *guessTitles(const wchar_t *filename,
					 int *tracknum,
					 wchar_t **artist,
					 wchar_t **album,
					 wchar_t **title) // should free the result of this function after using artist/album/title
{
	*tracknum=0;
	*artist=0;
	*album=0;
	*title=0;

	if (!filename[0]) return 0;

	wchar_t *_artist=NULL;
	wchar_t *_album=NULL;
  
	const wchar_t *f=filename;
	while (f && *f) f++;
	while (f && (f > filename) && *f != L'/' && *f != L'\\') f--;

	if (f == filename) return 0;

	int dirlen = f-filename;

	wchar_t *fullfn = (wchar_t*)_wcsdup(filename);
	fullfn[dirlen]=0;

	wchar_t *n=fullfn+dirlen;
	while (n >= fullfn && *n != L'/' && *n != L'\\') n--;
	n++;

	// try to guess artist and album from the dirname
	if (!wcsstr(n,L"-")) // assume dir name is album name, artist name unknown
	{
		*album=n;
		_album=n;
	}
	else 
	{
		*artist=_artist=n;
		_album=wcsstr(n,L"-")+1;
		wchar_t *t=_album-2;
		while (t >= n && *t == L' ') t--;
		t[1]=0;

		while (_album && *_album == L' ') _album++;
		*album=_album;
	}

	// get filename shizit
	wcsncpy(fullfn+dirlen+1,filename+dirlen+1,wcslen(filename) - (dirlen + 1));

	n=fullfn+dirlen+1;
	while (n && *n) n++;
	while (n > fullfn+dirlen+1 && *n != L'.') n--;
	if (*n == L'.') *n=0;
	n=fullfn+dirlen+1;

	while (n && *n == L' ') n++;

	// detect XX. filename
	if (wcsstr(n,L".") && _wtoi(n) && _wtoi(n) < 130)
	{
		wchar_t *tmp=n;
		while (tmp && *tmp >= L'0' && *tmp <= L'9') tmp++;
		while (tmp && *tmp == L' ') tmp++;
		if (tmp && *tmp == L'.') 
		{ 
			*tracknum=_wtoi(n); 
			n=tmp+1; 
			while (n && *n == L'.') n++;
			while (n && *n == L' ') n++;
		}
	}

	// detect XX- filename
	if (!*tracknum && wcsstr(n,L"-") && _wtoi(n) && _wtoi(n) < 130)
	{
		wchar_t *tmp=n;
		while (tmp && *tmp >= L'0' && *tmp <= L'9') tmp++;
		while (tmp && *tmp == L' ') tmp++;
		if (tmp && *tmp == L'-') 
		{ 
			*tracknum=_wtoi(n); 
			n=tmp+1; 
			while (n && *n == L'-') n++;
			while (n && *n == L' ') n++;
		}
	}
 
	while (wcsstr(n,L"-"))
	{
		wchar_t *a=n;
		n=wcsstr(n,L"-");
		{
			wchar_t *t=n-1;
			while (t >= a && *t == L' ') t--;
			t[1]=0;
		}
		*n=0;
		n++;
		while (n && *n == L'-') n++;
		while (n && *n == L' ') n++;

		// a is the next token.
		if (!*tracknum && !_wcsnicmp(a,L"Track ",6) && _wtoi(a+6)) *tracknum=_wtoi(a+6);
		else if (*artist== _artist)
		{
			*artist=a;
		}
		if (*artist != _artist && *tracknum) break;
	}
	*title=n;
   
	return fullfn;  
}