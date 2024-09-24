#include <windows.h>
#include "api.h"
#include "resource.h"
#include "../Winamp/wa_ipc.h"
#include "../Winamp/in2.h"

extern In_Module mod;			// the output module (filled in near the bottom of this file)

static char app_name[] = "Nullsoft NSV Decoder2";

//char config_http_proxynonport80=1;

int config_padtag=1024;
int config_bufms=10000;
int config_prebufms=2000;
int config_underunbuf=3000;
int config_bufms_f=1000;
int config_prebufms_f=1000;
int config_underunbuf_f=1000;
int config_vidoffs=0;
int config_precseek=3;
int config_subtitles=1;

void config_write();

char INI_FILE[512] = {0};

static int _r_i(char *name, int def)
{
	if (!_strnicmp(name,"config_",7)) name += 7;
	return GetPrivateProfileIntA(app_name,name,def,INI_FILE);
}
#define RI(x) (( x ) = _r_i(#x,( x )))
static void _w_i(char *name, int d)
{
	char str[120] = {0};
	wsprintfA(str,"%d",d);
	if (!_strnicmp(name,"config_",7)) name += 7;
	WritePrivateProfileStringA(app_name,name,str,INI_FILE);
}
#define WI(x) _w_i(#x,( x ))

static void _r_s(char *name,char *data, int mlen)
{
	char buf[2048] = {0};
	strncpy(buf, data, 2048);
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



static void config_init()
{
char *p;
	if (mod.hMainWindow &&
		(p = (char *)SendMessageA(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILE))
		&& p!= (char *)1)
	{
		strncpy(INI_FILE, p, MAX_PATH);
	}
	else
	{
		GetModuleFileNameA(NULL,INI_FILE,sizeof(INI_FILE));
		p=INI_FILE+strlen(INI_FILE);
		while (p >= INI_FILE && *p != '.') p--;
		strcpy(++p,"ini");
	}
}

static INT_PTR CALLBACK configProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      // show config
      SetDlgItemInt(hwndDlg,IDC_BUF1,config_bufms,FALSE);
      SetDlgItemInt(hwndDlg,IDC_BUF2,config_prebufms,FALSE);
      SetDlgItemInt(hwndDlg,IDC_BUF3,config_underunbuf,FALSE);
      SetDlgItemInt(hwndDlg,IDC_BUF4,config_bufms_f,FALSE);
      SetDlgItemInt(hwndDlg,IDC_BUF5,config_prebufms_f,FALSE);
      SetDlgItemInt(hwndDlg,IDC_BUF6,config_underunbuf_f,FALSE);
      SetDlgItemInt(hwndDlg,IDC_OFFS,config_vidoffs,TRUE);
      SetDlgItemInt(hwndDlg,IDC_TAGPAD,config_padtag,FALSE);
      if (config_precseek&1)CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
      if (config_precseek&2)CheckDlgButton(hwndDlg,IDC_CHECK2,BST_CHECKED);
	  if (config_subtitles)CheckDlgButton(hwndDlg,IDC_CHECK3,BST_CHECKED);
//      if (!config_http_proxynonport80) CheckDlgButton(hwndDlg,IDC_CHECK5,BST_CHECKED);
    return 0;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_CHECK1:
        case IDC_CHECK2:
          config_precseek=(IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0)|
                          (IsDlgButtonChecked(hwndDlg,IDC_CHECK2)?2:0);

        break;
		case IDC_CHECK3:
			config_subtitles=(IsDlgButtonChecked(hwndDlg,IDC_CHECK3)?1:0);

			break;
        case IDC_OFFS:
          if (HIWORD(wParam) == EN_CHANGE)
          {
            BOOL t;
            config_vidoffs=GetDlgItemInt(hwndDlg,IDC_OFFS,&t,TRUE);
          }
        break;
        case IDCANCEL: EndDialog(hwndDlg,0); break;
        case IDOK: 
          // save config
//          config_http_proxynonport80=!IsDlgButtonChecked(hwndDlg,IDC_CHECK5);
          {
            BOOL t;
            config_bufms=GetDlgItemInt(hwndDlg,IDC_BUF1,&t,FALSE);
            if (config_bufms < 100) config_bufms=100;
            if (config_bufms > 100000) config_bufms=100000;

            config_prebufms=GetDlgItemInt(hwndDlg,IDC_BUF2,&t,FALSE);
            if (config_prebufms < 100) config_prebufms=100;
            if (config_prebufms > config_bufms) config_prebufms=config_bufms;

            config_underunbuf=GetDlgItemInt(hwndDlg,IDC_BUF3,&t,FALSE);
            if (config_underunbuf < 100) config_underunbuf=100;
            if (config_underunbuf > config_bufms) config_underunbuf=config_bufms;

            config_bufms_f=GetDlgItemInt(hwndDlg,IDC_BUF4,&t,FALSE);
            if (config_bufms_f < 100) config_bufms_f=100;
            if (config_bufms_f > 100000) config_bufms_f=100000;

            config_prebufms_f=GetDlgItemInt(hwndDlg,IDC_BUF5,&t,FALSE);
            if (config_prebufms_f < 100) config_prebufms_f=100;
            if (config_prebufms_f > config_bufms_f) config_prebufms_f=config_bufms_f;

            config_underunbuf_f=GetDlgItemInt(hwndDlg,IDC_BUF6,&t,FALSE);
            if (config_underunbuf_f < 100) config_underunbuf_f=100;
            if (config_underunbuf_f > config_bufms_f) config_underunbuf_f=config_bufms_f;

            config_vidoffs=GetDlgItemInt(hwndDlg,IDC_OFFS,&t,TRUE);

            config_padtag=GetDlgItemInt(hwndDlg,IDC_TAGPAD,&t,FALSE);
          }
          config_write();
          EndDialog(hwndDlg,1); 
        break;
      }
    break;
  }
  return 0;
}

void config(HWND hwndParent)
{
	WASABI_API_DIALOGBOXW(IDD_DIALOG1,hwndParent,configProc);
}

void config_read()
{
	config_init();
	RI(config_bufms);
	RI(config_prebufms);
	RI(config_underunbuf);
	RI(config_bufms_f);
	RI(config_prebufms_f);
	RI(config_underunbuf_f);
	RI(config_vidoffs);
	RI(config_padtag);
//  RI(allow_uvox);
//	RI(config_http_proxynonport80);
	RI(config_precseek);
	RI(config_subtitles);
}

void config_write()
{
	WI(config_bufms);
	WI(config_prebufms);
	WI(config_underunbuf);
	WI(config_bufms_f);
	WI(config_prebufms_f);
	WI(config_underunbuf_f);
	WI(config_vidoffs);
	WI(config_padtag);
	WI(config_precseek);
	WI(config_subtitles);
//  WI(allow_uvox);
//	WI(config_http_proxynonport80);
}