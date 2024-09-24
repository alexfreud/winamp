#define STRICT
#include <windows.h>
#include <stdio.h>
#include "string_unicode.h"

void string_a::s_GetWindowText(HWND w)
{
	reset();
	int len=GetWindowTextLengthA(w)+1;
	GetWindowTextA(w,string_buffer_a(*this,len),len);
	
}

void string_a::s_SetWindowText(HWND w)
{
	SetWindowTextA(w,*this);
}


/*bool string_a::reg_read(HKEY hk,const char * name)
{
	DWORD sz=0,t=0;
	if (RegQueryValueExA(hk,name,0,&t,0,&sz)!=ERROR_SUCCESS) return 0;
	if (sz==0 || t!=REG_SZ) return 0;
	RegQueryValueExA(hk,name,0,0,(BYTE*)buffer_get(sz),&sz);
	buffer_done();
	return 1;
}

void string_a::reg_write(HKEY hk,const char * name)
{
	RegSetValueExA(hk,name,0,REG_SZ,(const BYTE*)(const char*)*this,(length()+1));
}
*/