#define STRICT
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include "c_string.h"
#include "../nu/ns_wc.h"

extern BOOL is_nt;

template<class myChar>
void string_base<myChar>::makespace(UINT s)
{
	if (size<s)
	{
		int oldSize = size;
		do size<<=1; while(size<s);
		myChar *newPtr = (myChar*)realloc(ptr,size*sizeof(myChar));
		if (!newPtr)
		{
			newPtr = (myChar*)malloc(size*sizeof(myChar));
			if (newPtr)
			{
				memcpy(newPtr, ptr, oldSize*sizeof(myChar));
				free(ptr);
				ptr = newPtr;
			}
			else return ;
		}
		else ptr = newPtr;
	}
}

void String::s_GetWindowText(HWND w)
{
	Reset();
	int len=GetWindowTextLengthA(w)+1;
	GetWindowTextA(w,StringTempA(*this,len),len);
}

void StringW::s_GetWindowText(HWND w)
{
	Reset();
	int len=GetWindowTextLengthW(w)+1;
	GetWindowTextW(w,StringTempW(*this,len),len);
}

void String::SetStringW(const WCHAR * c)
{
	UINT len=(lstrlenW(c)+1)*2;
	WideCharToMultiByteSZ(CP_ACP,0,c,-1,StringTempA(*this,len),len,0,0);
}

void StringW::SetStringA(const char * c)
{
	UINT len=(UINT)strlen(c)+1;
	MultiByteToWideCharSZ(CP_ACP,0,c,-1,StringTempW(*this,len),len);
}

void String::AddStringW(const WCHAR * c)
{
	AddString(String(c));
}

void StringW::AddStringA(const char * c)
{
	AddString(StringW(c));
}

void String::s_SetWindowText(HWND w)
{
	SetWindowTextA(w,*this);
}

void StringW::s_SetWindowText(HWND w)
{
	SetWindowTextW(w,*this);
}


StringPrintf::StringPrintf(const char * fmt,...)
{
	va_list list;
	va_start(list,fmt);
	vsprintf(StringTempA(*this,1024),fmt,list);
	va_end(list);
}

StringPrintfW::StringPrintfW(const WCHAR * fmt,...)
{
	va_list list;
	va_start(list,fmt);
	vswprintf(StringTempW(*this,1024),1024,fmt,list);
	va_end(list);
}

String::String(const StringW & z) {AddStringW(z);}