#include <windows.h>
#include "resource.h"
#include "../Agave/Language/api_language.h"
#include "main.h"
#include <strsafe.h>

extern const char *INI_FILE;

static char app_name[] = "in_dshow";

static char default_extlist[]="MPG;MPEG;M2V;AVI";

char config_extlist[129] = {0};

static int _r_i(char *name, int def)
{
	if (!_strnicmp(name,"config_",7)) name += 7;
	return GetPrivateProfileIntA(app_name,name,def,INI_FILE);
}
#define RI(x) (( x ) = _r_i(#x,( x )))
static void _w_i(char *name, int d)
{
	char str[120] = {0};
	StringCchPrintfA(str, 120, "%d",d);
	if (!_strnicmp(name,"config_",7)) name += 7;
	WritePrivateProfileStringA(app_name,name,str,INI_FILE);
}
#define WI(x) _w_i(#x,( x ))

static void _r_s(char *name,char *data, int mlen)
{
	char buf[2048] = {0};
	lstrcpynA(buf,data, 2048);
	if (!_strnicmp(name,"config_",7)) name += 7;
	GetPrivateProfileStringA(app_name,name,buf,data,mlen,INI_FILE);
}
#define RS(x) (_r_s(#x,x,sizeof(x)))

static void _w_s(char *name, char *data)
{
	if (!_strnicmp(name,"config_",7)) name += 7;
	WritePrivateProfileStringA(app_name,name,data,INI_FILE);
}
#define WS(x) (_w_s(#x,x))

#define ISSEP(x) ((x) == ' ' || (x) == ';' || (x) == ',' || (x) == ':' || (x) == '.')
char *getfileextensions()
{
	static char list[512];
	char *op=list;
	//  char *g_fileassos="MP3;MP2;MP1\0MPEG Audio Files (*.MP3;*.MP2;*.MP1)\0";

	char *p=config_extlist;
	int s=0;
	while (p && *p)
	{
		while (ISSEP(*p)) p++;
		if (!*p) break;
		if (s) *op++=';';
		s=1;
		while (p && *p && !ISSEP(*p)) *op++=*p++;
	}
	*op++=0;
	lstrcpynA(op,WASABI_API_LNGSTRING(IDS_VIDEO_FILES_OFD),512);
	while (op && *op) op++;
	p=config_extlist;
	s=0;
	while (p && *p)
	{
		while (ISSEP(*p)) p++;
		if (!p || !*p) break;
		if (s) *op++=';';
		s=1;
		*op++='*';
		*op++='.';
		while (p && *p && !ISSEP(*p)) *op++=*p++;     
	}
	*op++=')';
	*op++=0;
	*op++=0;
	return list;
}

void config_read()
{
  lstrcpynA(config_extlist,default_extlist, 129);
  RS(config_extlist);
}

void config_write()
{
  WS(config_extlist);
}

INT_PTR CALLBACK configProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
  case WM_INITDIALOG:
    SetDlgItemTextA(hwndDlg,IDC_TYPES,config_extlist);
    SendDlgItemMessage(hwndDlg,IDC_TYPES,EM_LIMITTEXT,128,0);
    return TRUE;
  case WM_COMMAND:
    switch(LOWORD(wParam))
    {
    case IDC_DEFAULTTYPES:
      SetDlgItemTextA(hwndDlg,IDC_TYPES,default_extlist);
      break;
    case IDOK:
      GetDlgItemTextA(hwndDlg,IDC_TYPES,config_extlist,128);
      config_write();
    case IDCANCEL:
      EndDialog(hwndDlg,0);
      break;
    }
    break;
  }
  return FALSE;
}

void doConfig(HINSTANCE hInstance, HWND hwndParent)
{
	WASABI_API_DIALOGBOXW(IDD_CONFIG,hwndParent,configProc);
}