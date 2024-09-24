#define STRICT
#include <windows.h>

#include "genres.h"

static char file_path[MAX_PATH];

static void file_init()
{
	char * p;
	GetModuleFileName(0,file_path,MAX_PATH);
	p=strrchr(file_path,'\\');
	if (p) p++; else p=file_path;
	strcpy(p,"genres.txt");
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

void genres_read(HWND wnd)
{
	HANDLE f;
	char temp[MAX_GENRE] = {0};
	char add[MAX_GENRE] = {0};
	UINT ptr;
	BOOL eof=0;
	BOOL start;
	char c;

	if (!file_path[0]) file_init();
	

	f=CreateFile(file_path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	if (f==INVALID_HANDLE_VALUE) return;
	GetWindowText(wnd,add,MAX_GENRE);
	while(!eof)
	{
		ptr=0;
		start=1;
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
			SendMessage(wnd,CB_ADDSTRING,0,(LPARAM)temp);
			if (add[0])
			{
				if (!_stricmp(add,temp)) add[0]=0;
			}
		}
	}
	CloseHandle(f);
	if (add[0]) SendMessage(wnd,CB_ADDSTRING,0,(LPARAM)add);
}

void genres_write(HWND wnd)
{
	char temp[MAX_GENRE] = {0};
	UINT max = 0, n = 0;
	DWORD bw = 0;
	HANDLE f;
	{
		char add[MAX_GENRE] = {0};
		GetWindowText(wnd,add,MAX_GENRE);
		if (!add[0]) return;
		max=SendMessage(wnd,CB_GETCOUNT,0,0);
		for(n=0;n<max;n++)
		{
			SendMessage(wnd,CB_GETLBTEXT,n,(LPARAM)temp);
			if (!_stricmp(temp,add)) return;
		}
		SendMessage(wnd,CB_ADDSTRING,0,(LPARAM)add);
	}
	if (!file_path[0]) file_init();
	f=CreateFile(file_path,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
	if (f==INVALID_HANDLE_VALUE) return;
	max=SendMessage(wnd,CB_GETCOUNT,0,0);
	for(n=0;n<max;n++)
	{
		SendMessage(wnd,CB_GETLBTEXT,n,(LPARAM)temp);
		bw = 0; WriteFile(f,temp,strlen(temp),&bw,0);
		bw = 0; WriteFile(f,eol,2,&bw,0);
	}
	CloseHandle(f);
}
