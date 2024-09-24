#include <windows.h>
#include "genres.h"
#include <shlwapi.h>

extern const wchar_t *INI_DIRECTORY;

static void file_init(wchar_t *file_path, wchar_t *fn)
{
	PathCombineW(file_path, INI_DIRECTORY, fn);
}

static char eol[2]={13,10};

static char get_char(HANDLE f,BOOL * eof)
{
	DWORD br=0;
	char r=0;
	ReadFile(f,&r,1,&br,0);
	if (!br) *eof=1;
	return r;
}

void genres_read(HWND wnd, wchar_t* fn)
{
	char temp[MAX_GENRE] = {0};
	char add[MAX_GENRE] = {0};
	BOOL eof=0;
	char c = 0;
	wchar_t file_path[MAX_PATH] = {0};
	HANDLE f;

	file_init(file_path, fn);
	
	f = CreateFileW(file_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (f==INVALID_HANDLE_VALUE) return;
	GetWindowTextA(wnd,add,MAX_GENRE);
	while(!eof)
	{
		UINT ptr=0;
		BOOL start=1;
		while(ptr<MAX_GENRE-1)
		{
			c=get_char(f,&eof);
			if (eof) break;
			if (c==10 || c==13)
			{
				if (start) continue;
				else break;
			}
			start=0;
			temp[ptr++]=c;
		}
		if (ptr) 
		{
			temp[ptr]=0;
			SendMessage(wnd,CB_ADDSTRING,0, (LPARAM)temp);
			if (add[0])
			{
				if (!_stricmp(add,temp)) add[0]=0;
			}
		}
	}
	CloseHandle(f);
	if (add[0]) SendMessage(wnd,CB_ADDSTRING,0,(LPARAM)add);
}

void genres_write(HWND wnd, wchar_t* fn)
{
	wchar_t file_path[MAX_PATH] = {0};
	char temp[MAX_GENRE] = {0};
	UINT max = 0,n = 0;
	DWORD bw = 0;
	HANDLE f;
	{
		char add[MAX_GENRE] = {0};
		GetWindowTextA(wnd,add,MAX_GENRE);
		if (!add[0]) return;
		max=(UINT)SendMessage(wnd,CB_GETCOUNT,0,0);
		for(n=0;n<max;n++)
		{
			SendMessage(wnd,CB_GETLBTEXT,n,(LPARAM)temp);
			if (!_stricmp(temp,add)) return;
		}
		SendMessage(wnd,CB_ADDSTRING,0,(LPARAM)add);
	}
	file_init(file_path, fn);
	f = CreateFileW(file_path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (f==INVALID_HANDLE_VALUE) return;
	max=(UINT)SendMessage(wnd,CB_GETCOUNT,0,0);
	for(n=0;n<max;n++)
	{
		SendMessage(wnd,CB_GETLBTEXT,n,(LPARAM)temp);
		bw = 0; WriteFile(f,temp,(DWORD)strlen(temp),&bw,0);
		bw = 0; WriteFile(f,eol,2,&bw,0);
	}
	CloseHandle(f);
}