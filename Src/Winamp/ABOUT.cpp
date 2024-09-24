/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename: 
** Project:
** Description:
** Author:
** Created:
**/

#include "Main.h"
#include "language.h"
#include <math.h>
#include "../nu/threadname.h"
#include "resource.h"
#include <tataki/export.h>
#include "api.h"
#include "../nu/threadpool/TimerHandle.hpp"
#include "../nu/AutoWide.h"

int img_w[2] = {400, 100}, img_h[2] = {189, 34};
int about_lastpage;
HWND about_hwnd;
#define M_PI 3.14159265358979323846

static BOOL CALLBACK tipsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK whatsnewProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK about1Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK creditProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK translationProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static LRESULT WINAPI aboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

static unsigned const char sq_table[]=
{0, 16, 22, 27, 32, 35, 39, 42, 45, 48, 50, 53, 55, 57, 59, 61, 64, 65, 
67, 69, 71, 73, 75, 76, 78, 80, 81, 83, 84, 86, 87, 89, 90, 91, 93, 94, 
96, 97, 98, 99, 101, 102, 103, 104, 106, 107, 108, 109, 110, 112, 113, 
114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 128,  
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 
142, 143, 144, 144, 145, 146, 147, 148, 149, 150, 150, 151, 152, 153, 
154, 155, 155, 156, 157, 158, 159, 160, 160, 161, 162, 163, 163, 164, 
165, 166, 167, 167, 168, 169, 170, 170, 171, 172, 173, 173, 174, 175,  
176, 176, 177, 178, 178, 179, 180, 181, 181, 182, 183, 183, 184, 185,  
185, 186, 187, 187, 188, 189, 189, 190, 191, 192, 192, 193, 193, 194, 
195, 195, 196, 197, 197, 198, 199, 199, 200, 201, 201, 202, 203, 203, 
204, 204, 205, 206, 206, 207, 208, 208, 209, 209, 210, 211, 211, 212, 
212, 213, 214, 214, 215, 215, 216, 217, 217, 218, 218, 219, 219, 220,
221, 221, 222, 222, 223, 224, 224, 225, 225, 226, 226, 227, 227, 228,
229, 229, 230, 230, 231, 231, 232, 232, 233, 234, 234, 235, 235, 236, 
236, 237, 237, 238, 238, 239, 240, 240, 241, 241, 242, 242, 243, 243, 
244, 244, 245, 245, 246, 246, 247, 247, 248, 248, 249, 249, 250, 250, 
251, 251, 252, 252, 253, 253, 254, 254, 255};

static __inline unsigned long isqrt(unsigned long n)
{
	if (n >= 0x10000)
		if (n >= 0x1000000)
			if (n >= 0x10000000)
				if (n >= 0x40000000) return(sq_table[n >> 24] << 8);
				else return(sq_table[n >> 22] << 7);
			else
				if (n >= 0x4000000) return(sq_table[n >> 20] << 6);
				else return(sq_table[n >> 18] << 5);
		else
			if (n >= 0x100000)
				if (n >= 0x400000) return(sq_table[n >> 16] << 4);
				else return(sq_table[n >> 14] << 3);
			else
				if (n >= 0x40000) return(sq_table[n >> 12] << 2);
				else return(sq_table[n >> 10] << 1);
	else
		if (n >= 0x100)
			if (n >= 0x1000)
				if (n >= 0x4000) return(sq_table[n >> 8]);
				else return(sq_table[n >> 6] >> 1);
			else
				if (n >= 0x400) return(sq_table[n >> 4] >> 2);
				else return(sq_table[n >> 2] >> 3);
		else
			if (n >= 0x10)
				if (n >= 0x40) return(sq_table[n] >> 4);
				else return(sq_table[n << 2] << 5);
			else
				if (n >= 0x4) return(sq_table[n >> 4] << 6);
				else return(sq_table[n >> 6] << 7);
}

void about_dialog(void)
{
	if (about_hwnd)
	{
		SetForegroundWindow(about_hwnd);
		return;
	}

	about_hwnd=(HWND)LPCreateDialogW(IDD_NEWABOUT, hMainWindow, aboutProc);

	// show about window and restore last position as applicable
	POINT pt = {about_rect.left, about_rect.top};
	if (!windowOffScreen(about_hwnd, pt))
		SetWindowPos(about_hwnd, HWND_TOP, about_rect.left, about_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
	else
		ShowWindow(about_hwnd, SW_SHOW);
}

static LRESULT WINAPI aboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	static HWND cur_wnd;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			int t=about_lastpage;
			about_lastpage=-1;
			// make sure not to show the localised page if not under a language pack
			if (t==4 && config_langpack[0]) SendMessageW(hwndDlg, WM_COMMAND, IDC_BUTTON2, 0);
			else if (t==3) SendMessageW(hwndDlg, WM_COMMAND, IDC_BUTTON6, 0);
			else if (t==2) SendMessageW(hwndDlg, WM_COMMAND, IDC_BUTTON5, 0);
			else if (t==1) SendMessageW(hwndDlg, WM_COMMAND, IDC_BUTTON4, 0);
			else
			{
				if (t==4) t = 0;
				SendMessageW(hwndDlg, WM_COMMAND, IDC_BUTTON3, 0);
			}
		}
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
		case IDOK:
			DestroyWindow(hwndDlg);
			return FALSE;
		case IDC_BUTTON3:
		case IDC_BUTTON4:
		case IDC_BUTTON5:
		case IDC_BUTTON6:
		case IDC_BUTTON2:
			{
				int id=-1;
				int t = 0;
				void *proc = 0;
				if (LOWORD(wParam)==IDC_BUTTON3) t=0;
				if (LOWORD(wParam)==IDC_BUTTON4) t=1;
				if (LOWORD(wParam)==IDC_BUTTON5) t=2;
				if (LOWORD(wParam)==IDC_BUTTON6) t=3;
				if (LOWORD(wParam)==IDC_BUTTON2) t=4;
				if (t == about_lastpage) return 0;
				about_lastpage=t;
				switch (t)
				{
					case 0: id=IDD_NEWABOUT1;	proc=about1Proc; break;
					case 1: id=IDD_NEWABOUT2;	proc=creditProc; break;
					case 2: id=IDD_NEWABOUT3;	proc=tipsProc; break;
					case 3: id=IDD_NEWABOUT4;	proc=whatsnewProc; break;
					case 4: id=IDD_ABOUT_TRANSLATION; proc=translationProc; break;
				}

				if (IsWindow(cur_wnd)) 
				{
					DestroyWindow(cur_wnd); 
					cur_wnd=0;
				}

				if (id != -1)
				{
					cur_wnd = CreateDialogW(language_pack_instance, MAKEINTRESOURCEW(id), hwndDlg, (DLGPROC)proc);
					// handles cases where there's no localised info page in a language pack / under en-us
					if(!IsWindow(cur_wnd)) cur_wnd = LPCreateDialogW(id, hwndDlg, (DLGPROC)proc);

					{
						RECT r;
						GetWindowRect(GetDlgItem(hwndDlg,IDC_RECT),&r);
						ScreenToClient(hwndDlg,(LPPOINT)&r);
						SetWindowPos(cur_wnd,0,r.left,r.top,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
					}
					ShowWindow(cur_wnd,SW_SHOWNA);
					{
						RECT r,r2;
						GetWindowRect(GetDlgItem(hwndDlg,IDC_BUTTON3),&r);
						GetWindowRect(GetDlgItem(hwndDlg,IDC_BUTTON6),&r2);
						r.right=r2.right;
						ScreenToClient(hwndDlg,(LPPOINT)&r);
						ScreenToClient(hwndDlg,((LPPOINT)&r)+1);
						InvalidateRect(hwndDlg,&r,FALSE);
					}
				}
			}
			return FALSE;
		}
		break;

	case WM_DESTROY:
		GetWindowRect(hwndDlg, &about_rect);
		if (IsWindow(cur_wnd)) DestroyWindow(cur_wnd); 
		cur_wnd=0;
		about_hwnd=NULL;
		break;
	}

	if (uMsg == WM_DRAWITEM)
	{
		DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
		if (di->CtlType == ODT_BUTTON) 
		{
			wchar_t wt[123] = {0};
			int sel=0;
			RECT r;
			GetDlgItemTextW(hwndDlg,wParam,wt,123);

			if (di->CtlID==IDC_BUTTON3 && about_lastpage==0) sel=1;
			else if (di->CtlID==IDC_BUTTON4 && about_lastpage==1) sel=1;
			else if (di->CtlID==IDC_BUTTON5 && about_lastpage==2) sel=1;
			else if (di->CtlID==IDC_BUTTON6 && about_lastpage==3) sel=1;
			else if (di->CtlID==IDC_BUTTON2 && about_lastpage==4) sel=1;

			if (di->CtlID != IDC_BUTTON6)
			{
				MoveToEx(di->hDC,di->rcItem.right-1,di->rcItem.top,NULL);
				LineTo(di->hDC,di->rcItem.right-1,di->rcItem.bottom);
			}

			// draw text
			if (!sel && (di->itemState & (ODS_SELECTED)))
				SetTextColor(di->hDC,RGB(0,40,255));
			r=di->rcItem;
			DrawTextW(di->hDC,wt,-1,&r,DT_VCENTER|DT_SINGLELINE|DT_CENTER);
			if (sel)
			{
				r=di->rcItem;
				r.left+=2;
				SetBkMode(di->hDC,TRANSPARENT);
				DrawTextW(di->hDC,wt,-1,&r,DT_VCENTER|DT_SINGLELINE|DT_CENTER);
			}
		}	
	}
	return 0;
}

/* Window Proc for the 'keyboard shorcuts' tab of about screen */
static BOOL CALLBACK tipsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static HGLOBAL hResource=0;
	static DWORD hResourceSize=0;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			char *b = NULL, *p = 0, *op = 0;
			DWORD size = 0;
			if (!hResource)
			{
				hResource = langManager->LoadResourceFromFileW(language_pack_instance, hMainInstance,TEXT("TEXT"), TEXT("TIPSTEXT"),&size);
				hResourceSize = size;
			}
			p = (char*)hResource;
			if (p && (op = strstr(p, "!!End")))  // if there's "!!End" in the resource, than copy everything before it
			{
				b = (char*)GlobalAlloc(GPTR, op-p+1);
				memcpy(b, p, op-p);
				b[op-p] = 0;
			} else {
				b = (char*)GlobalAlloc(GPTR, hResourceSize+1);
				memcpy(b, p, hResourceSize);
				b[hResourceSize] = 0;
			}

			SetDlgItemTextA(hwndDlg, IDC_TIPS, (b ? b : p)); // send it to the text control to display
			if (b) GlobalFree(b);
		}
		break;

	case WM_COMMAND:
		if(LOWORD(wParam) == IDCANCEL)
		{
			DestroyWindow(about_hwnd);
		}
		break;
	}

	if (FALSE != IsDirectMouseWheelMessage(uMsg))
	{
		HWND textWindow;
		RECT windowRect;
		textWindow = GetDlgItem(hwndDlg, IDC_TIPS);
		if (NULL != textWindow && 
			FALSE != GetClientRect(textWindow, &windowRect))
		{
			POINT pt;
			POINTSTOPOINT(pt, lParam);
			
			MapWindowPoints(HWND_DESKTOP, textWindow, &pt, 1);
			if (FALSE != PtInRect(&windowRect, pt))
			{			
				SendMessageW(textWindow, WM_MOUSEWHEEL, wParam, lParam);
				SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, TRUE);
			}
			return TRUE;
		}
	}
	return 0;
}

WNDPROC whatedproc = 0;
static LRESULT CALLBACK whatseditproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_KEYDOWN)
	{
		if(wParam == 'F' && (GetAsyncKeyState(VK_CONTROL)&0x8000) && !(GetAsyncKeyState(VK_SHIFT)&0x8000))
		{
			if(IsWindowEnabled(GetDlgItem(GetParent(hwndDlg), IDC_ABOUT_SEARCH)))
			{
				SendMessageW(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(IDC_ABOUT_SEARCH,0), 0);
			}
		}
	}
	return CallWindowProcW(whatedproc,hwndDlg,uMsg,wParam,lParam);
}

#define WHATSNEW_BUFFER_SIZE 262144
static wchar_t* ver_buf;

void GetWhatsNewFromFile(FILE *fp)
{
	bool utf8=false, utf16=false;
	unsigned char BOM[3] = {0, 0, 0};
	if (fread(BOM, 3, 1, fp) == 1 && BOM[0] == 0xEF && BOM[1] == 0xBB && BOM[2] == 0xBF)
		utf8 = true;
	else
	{
		fseek(fp, 0, SEEK_SET);
		if (fread(BOM, 2, 1, fp) == 1 && BOM[0] == 0xFF && BOM[1] == 0xFE)
			utf16=true;
		else
			fseek(fp, 0, SEEK_SET);
	}

	if (utf16)
	{
		wchar_t buffer[WHATSNEW_BUFFER_SIZE]={0}, *p=buffer;
		for (;;)
		{
			fgetws(p,1024,fp);
			if (feof(fp)) break;
			if (p[wcslen(p)-1]==L'\n')
				p[wcslen(p)-1]=0;
			StringCchCatW(p,WHATSNEW_BUFFER_SIZE,L"\r\n");
			p=p+wcslen(p);
			if (p-buffer > WHATSNEW_BUFFER_SIZE) break;
		}

		ver_buf = wcsdup(buffer);
	}
	else
	{
		char buffer[WHATSNEW_BUFFER_SIZE]={0},*p=buffer;
		for (;;)
		{
			fgets(p,1024,fp);
			if (feof(fp)) break;
			if (p[lstrlenA(p)-1]=='\n')
				p[lstrlenA(p)-1]=0;
			StringCchCatA(p,WHATSNEW_BUFFER_SIZE,"\r\n");
			p=p+lstrlenA(p);
			if (p-buffer > WHATSNEW_BUFFER_SIZE) break;
		}

		if (utf8)
		{
			ver_buf = AutoWideDup(buffer, CP_UTF8);
		}
		else
		{
			ver_buf = AutoWideDup(buffer);
		}
	}
}

static std::map<int,wchar_t*> versions;
/* Window Proc for the 'version history' tab of about screen */
static BOOL CALLBACK whatsnewProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static UINT fm_msg;
	static FINDREPLACEW find;
	static wchar_t fromstring[256];
	static HWND finder;

	if(uMsg == fm_msg)
	{
		LPFINDREPLACEW lpFindReplace = (LPFINDREPLACEW) lParam;
		if(lpFindReplace->Flags & FR_FINDNEXT)
		{
			int len = GetWindowTextLengthW(GetDlgItem(hwndDlg,IDC_TIPS))+1,
				flen = lstrlenW(lpFindReplace->lpstrFindWhat), start = 0, end = 0;
			wchar_t *buffer = (wchar_t*)calloc((len+1),sizeof(wchar_t)),
					*search = (wchar_t*)calloc((flen+2),sizeof(wchar_t)),
					*result = 0;

			lstrcpynW(search,lpFindReplace->lpstrFindWhat,flen+1);
			SendDlgItemMessageW(hwndDlg,IDC_TIPS,EM_GETSEL,(WPARAM)&start,(LPARAM)&end);

			GetDlgItemTextW(hwndDlg,IDC_TIPS,buffer,len);

			// handles the match case option
			if(!(lpFindReplace->Flags & FR_MATCHCASE))
			{
				CharUpperBuffW(buffer, len);
				CharUpperBuffW(search, flen+1);
			}

			if((result = wcsstr(buffer+end,search)))
			{
				SendDlgItemMessage(hwndDlg,IDC_TIPS,EM_SETSEL,result-buffer,(result-buffer)+flen);
				SendDlgItemMessage(hwndDlg,IDC_TIPS,EM_SCROLLCARET,0,0);
			}
			else
			{
				MessageBoxW(finder,getStringW(IDS_NO_MATCHES_FOUND,NULL,0),L"Winamp",MB_ICONINFORMATION);
			}

			free(buffer);
			free(search);
		}
	}

	switch (uMsg) {
		case WM_INITDIALOG:
			{
				wchar_t fn[MAX_PATH] = {0};
				FILE *fp = NULL;
				int last_add = 0;

				whatedproc = (WNDPROC)SetWindowLongPtrW(GetDlgItem(hwndDlg,IDC_TIPS),GWLP_WNDPROC,(LONG_PTR)whatseditproc);

				SendDlgItemMessage(hwndDlg, IDC_TIPS, EM_SETLIMITTEXT, 0, 0);

				// attempt to pull a localised whatsnew.txt
				// before reverting to the english default
				if (lang_directory[0])
				{
					lstrcpynW(fn,lang_directory,MAX_PATH);
				}
				else
				{
					GetModuleFileNameW(NULL,fn,MAX_PATH);
					PathRemoveFileSpecW(fn);
				}
				PathAppendW(fn, L"whatsnew.txt");

				// we don't set the buffer position so that it all works on the fallback code
				// and make sure there's a selection no matter what happens with other things
				SendDlgItemMessageW(hwndDlg,IDC_VER_COMBO,CB_ADDSTRING,0,(LPARAM)getStringW(IDS_SHOW_ALL_HISTORY,NULL,0));
				SendDlgItemMessageW(hwndDlg,IDC_VER_COMBO,CB_SETCURSEL,0,0);

				versions.clear();
				fp = _wfopen(fn,L"rb");

				// if there isn't a localised whatsnew.txt then revert to the old ways
				if(!fp)
				{
					GetModuleFileNameW(NULL,fn,MAX_PATH);
					PathRemoveFileSpecW(fn);
					PathAppendW(fn,L"whatsnew.txt");
					fp = _wfopen(fn,L"rb");
				}

				if (fp)
				{
					GetWhatsNewFromFile(fp);
					fclose(fp);
					wchar_t* p = ver_buf;
					while(p && *p)
					{
						// populate the version combobox
						if(!wcsncmp(p,L"Winamp 5.",9))
						{
							wchar_t* pp = p, ver[64] = {0};
							while(pp && *pp && *pp != L'\r') pp = CharNextW(pp);
							// need to make sure that we've actually found a valid part otherwise
							// just skip over things and don't fill in the combobox (attempted exploit)
							if(pp && *pp && *pp == L'\r'){
								pp = CharNextW(pp);
								// make sure that we keep within the buffer size as some people have
								// tried to make a buffer overflow vulnerability with this on XP SP3
								lstrcpynW(ver,p,(pp-p<64?pp-p:64));
								wchar_t* v = ver, *vn = 0, *vne = 0;
								while(v && *v && *v != L'\t') v = CharNextW(v);
								if(v && *v == L'\t'){
									vn = vne = CharNextW(v);
									*v = 0;
									if(vn && *vn == L'[') vn = CharNextW(vn);
									while(vne && *vne && *vne != L']') vne = CharNextW(vne);
									if(vne && *vne == L']') *vne = 0;
								}
								last_add = SendDlgItemMessageW(hwndDlg,IDC_VER_COMBO,CB_ADDSTRING,0,(LPARAM)ver);
								SendDlgItemMessageW(hwndDlg,IDC_VER_COMBO,CB_SETITEMDATA,last_add,(LPARAM)p);
								versions[last_add] = wcsdup(vn);
							}
						}
						p = CharNextW(p);
					}

					// reset the selection to the last view and force an update to that
					// would be better to do it straight away on load but this ensures all synchs up
					SendDlgItemMessage(hwndDlg,IDC_VER_COMBO,CB_SETCURSEL,_r_i("whtsnewlp",1),0);
					SendMessageW(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_VER_COMBO,CBN_SELCHANGE),(LPARAM)GetDlgItem(hwndDlg,IDC_VER_COMBO));
				}
				else
				{
					EnableWindow(GetDlgItem(hwndDlg,IDC_VER_COMBO),0);
					EnableWindow(GetDlgItem(hwndDlg,IDC_ABOUT_SEARCH),0);
					SetDlgItemTextW(hwndDlg,IDC_TIPS,getStringW(IDS_WHATSNEW_FAIL,ver_buf,WHATSNEW_BUFFER_SIZE));
				}
			}
			return FALSE;

		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_VER_COMBO && HIWORD(wParam) == CBN_SELCHANGE)
			{
				int cur = SendMessageW((HWND)lParam,CB_GETCURSEL,0,0);
				wchar_t* position = (wchar_t*)SendMessageW((HWND)lParam,CB_GETITEMDATA,cur,0);
				if(cur != CB_ERR && position){
					wchar_t* p = position, *out_buf = (wchar_t*)calloc(WHATSNEW_BUFFER_SIZE,sizeof(wchar_t)), *out = out_buf;
					while(p && *p)
					{
						if(*p == L'\t')
						{
							if(*CharNextW(p) == L'[')
							{
								wchar_t* pb = p;
								while(pb && *pb && *pb != L']')
								{
									pb = CharNextW(pb);
								}
								if(pb && *pb == L']') p = CharNextW(pb);
							}
						}

						// look for 2 empty lines to indicate end of version block
						if(*p == L'\r')
						{
							wchar_t* n = CharNextW(p);
							if(n && *n == L'\n')
							{
								n = CharNextW(n);
								if(n && *n == L'\r')
								{
									break;
								}
							}
						}
						*out = *p;
						p = CharNextW(p);
						out = CharNextW(out);
					}
					*out = 0;

					wchar_t released[128] = {0};
					if(cur == 1 || cur > 1 && versions[cur] != 0)
						StringCchPrintfW(released,128,getStringW(IDS_RELEASED,NULL,0), (cur == 1 ? AutoWide(__DATE__) : versions[cur]));
					else
						released[0] = 0;
					SendMessageW(GetDlgItem(hwndDlg,IDC_RELEASED),WM_SETTEXT,0,(LPARAM)released);
					SendMessageW(GetDlgItem(hwndDlg,IDC_TIPS),WM_SETTEXT,0,(LPARAM)out_buf);
					free(out_buf);
				}
				else{
					SendMessageW(GetDlgItem(hwndDlg,IDC_RELEASED),WM_SETTEXT,0,(LPARAM)L"");
					SendMessageW(GetDlgItem(hwndDlg,IDC_TIPS),WM_SETTEXT,0,(LPARAM)ver_buf);
				}
			}
			else if(LOWORD(wParam) == IDC_ABOUT_SEARCH)
			{
				if(!IsWindow(finder))
				{
					if(!fm_msg) fm_msg = RegisterWindowMessageW(FINDMSGSTRINGW);

					find.lStructSize = sizeof(find);
					find.hwndOwner = hwndDlg;
					find.Flags = FR_DOWN|FR_HIDEWHOLEWORD|FR_HIDEUPDOWN;
					find.lpstrFindWhat = fromstring;
					find.wFindWhatLen = ARRAYSIZE(fromstring);
					finder = FindTextW(&find);
					ShowWindow(finder,SW_SHOW);
				}
				else
				{
					SetActiveWindow(finder);
				}
			}

			else if(LOWORD(wParam) == IDCANCEL)
			{
				DestroyWindow(about_hwnd);
			}
			break;

		case WM_DESTROY:
			free(ver_buf);
			ver_buf = 0;

			//for (int i=0; i!=versions.size(); i++)
			//	free(versions[i]);
			for (auto& version : versions)
			{
				if (version.second)
				{
					free(version.second);
				}
			}

			versions.clear();
			_w_i("whtsnewlp",SendDlgItemMessage(hwndDlg,IDC_VER_COMBO,CB_GETCURSEL,0,0));
			if(IsWindow(finder)) DestroyWindow(finder);
			break;
	} 

	if (FALSE != IsDirectMouseWheelMessage(uMsg))
	{
		HWND textWindow;
		RECT windowRect;
		textWindow = GetDlgItem(hwndDlg, IDC_TIPS);
		if (NULL != textWindow && 
			FALSE != GetClientRect(textWindow, &windowRect))
		{
			POINT pt;
			POINTSTOPOINT(pt, lParam);
			
			MapWindowPoints(HWND_DESKTOP, textWindow, &pt, 1);
			if (FALSE != PtInRect(&windowRect, pt))
			{			
				SendMessageW(textWindow, WM_MOUSEWHEEL, wParam, lParam);
				SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, TRUE);
			}
			return TRUE;
		}
	}
	return 0;
}

static volatile int aboutThread_kill, aboutThread_mode;
static HPALETTE m_haboutpal;

/* This does the 'zooming' effect on the image in the 'winamp' tab of the about screen */
class AboutContext
{
public:
	bool Init(HWND _hwndDlg);
	bool Tick();
	void Quit();

private:
	int mode;
	static int m_wt,m_wait;
	static int a;
	HDC m_hdc;
	HBITMAP m_hbm,m_oldhbm;

	int m_pitch,m_height;
	char *m_source,*m_dib;
	int m_wmul[200];
	RECT r;
	HBITMAP m_imgbm, m_imgoldbm;
	HDC m_imgdc;

	struct
	{
		BITMAPINFO bmi;
		RGBQUAD more_bmiColors[256];
		LPVOID data;
	} m_bitmap;
	int c,use_palette;
	HWND hwndDlg;
};

int AboutContext::m_wt = 0;
int AboutContext::m_wait = 0;
int AboutContext::a = 0;
bool AboutContext::Init(HWND _hwndDlg)
{
	hwndDlg = _hwndDlg;
	mode=(GetAsyncKeyState(VK_SHIFT)&0x8000);
	GetClientRect(GetDlgItem(hwndDlg,IDC_ABOUTIMG),&r);
	HDC hdc=GetWindowDC(hwndDlg);
	if (!hdc) 
		return false;
	use_palette = (GetDeviceCaps(hdc,RASTERCAPS)&RC_PALETTE)?1:0;
	m_hdc = CreateCompatibleDC(NULL);
	m_imgdc = CreateCompatibleDC(NULL);
	ReleaseDC(hwndDlg,hdc);

	if (!m_imgdc) 
		return false;

	m_imgbm= (HBITMAP)LoadImage(hMainInstance,MAKEINTRESOURCE((!aboutThread_mode ? IDB_SPLASH : IDB_PLATE)),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
	m_imgoldbm=(HBITMAP)SelectObject(m_imgdc,m_imgbm);
	m_source=(char *)GlobalAlloc(GPTR,img_w[aboutThread_mode]*img_h[aboutThread_mode]);

	if (m_imgbm && m_source) 
	{
		memset(&m_bitmap, 0, sizeof(m_bitmap));
		m_bitmap.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		m_bitmap.bmi.bmiHeader.biPlanes = 1;
		m_bitmap.bmi.bmiHeader.biBitCount = 8;
		m_bitmap.bmi.bmiHeader.biCompression = BI_RGB;
		m_bitmap.bmi.bmiHeader.biSizeImage = 0;
		m_bitmap.bmi.bmiHeader.biClrUsed = 128;
		m_bitmap.bmi.bmiHeader.biClrImportant = 128;
		m_bitmap.bmi.bmiHeader.biWidth = img_w[aboutThread_mode];
		m_bitmap.bmi.bmiHeader.biHeight = -img_h[aboutThread_mode];
		m_bitmap.bmi.bmiHeader.biSizeImage = img_w[aboutThread_mode]*img_h[aboutThread_mode];

		GetDIBits(m_imgdc,m_imgbm,0,img_h[aboutThread_mode],m_source,(BITMAPINFO *)&m_bitmap,DIB_RGB_COLORS);
		GetDIBColorTable(m_imgdc,0,256,m_bitmap.bmi.bmiColors);

		SelectObject(m_imgdc, m_imgoldbm);
		DeleteDC(m_imgdc);
		DeleteObject(m_imgbm);
	}
	else
	{
		DeleteDC(m_imgdc);
		if (m_imgbm) DeleteObject(m_imgbm);
		if (m_source) GlobalFree(m_source);
		return false;
	}

	if (use_palette) 
	{
		struct
		{
			LOGPALETTE logpal;
			PALETTEENTRY palentries[255];
		} palette;
		palette.logpal.palVersion = 0x300;
		palette.logpal.palNumEntries = 128;

		for (c = 0; c < 128; c ++) 
		{
			palette.logpal.palPalEntry[c].peRed = m_bitmap.bmi.bmiColors[c].rgbRed;
			palette.logpal.palPalEntry[c].peGreen = m_bitmap.bmi.bmiColors[c].rgbGreen;
			palette.logpal.palPalEntry[c].peBlue = m_bitmap.bmi.bmiColors[c].rgbBlue;
			palette.logpal.palPalEntry[c].peFlags = 0;
		}
		m_haboutpal = CreatePalette((LOGPALETTE *)&palette.logpal);
	}

	m_pitch=((r.right-r.left+3)&~3);
	m_height=r.bottom-r.top;
	m_bitmap.bmi.bmiHeader.biSizeImage = 0;
	m_bitmap.bmi.bmiHeader.biClrUsed = 0;
	m_bitmap.bmi.bmiHeader.biClrImportant = 0;
	m_bitmap.bmi.bmiHeader.biWidth = m_pitch;
	m_bitmap.bmi.bmiHeader.biHeight = -m_height;
	m_bitmap.bmi.bmiHeader.biSizeImage = m_pitch*m_height;
	m_hbm = CreateDIBSection(m_hdc,&m_bitmap.bmi,DIB_RGB_COLORS, &m_bitmap.data, NULL, 0);

	if (!m_hbm)
	{
		if (m_imgbm) DeleteObject(m_imgbm);
		if (m_source) GlobalFree(m_source);
		return 0;
	} 
	m_oldhbm = (HBITMAP) SelectObject(m_hdc, m_hbm);
	m_dib=(char*)m_bitmap.data;
	{
		for (int x = 0; x < img_h[aboutThread_mode]; x ++)
			m_wmul[x]=x*img_w[aboutThread_mode];
	}

	return true;
}


bool AboutContext::Tick()
{
	if (aboutThread_kill)
		return false;

	int tab[512] = {0};
	//		Sleep(50);
	{
		int offs;
		int x;
		double max_d=sqrt((double)((m_pitch*m_pitch+m_height*m_height))/4.0f);
		//int imax_d2=(int)(255.0/max_d*256.0);
		int imax_d=(int)max_d;
		double dpos;

		int thiswt=m_wt;
		if (thiswt > 63) thiswt = 126-thiswt;

		dpos=1.0+sin(thiswt*M_PI/32.0);

		if (dpos < 1.0) dpos=0.5+dpos/2.0;
		if (thiswt < 32) offs=thiswt*24;
		else offs=(64-thiswt)*24;       
		if (imax_d > 512) return false;

		for (x = 0; x < imax_d; x ++)
		{
			tab[x]=(int) (pow(sin(x/(max_d-1)),dpos)*1.7*256.0*max_d/(x+1)) - offs;
			if (tab[x]<0)tab[x]=-tab[x];
		}

		if (m_wt == 0)
		{
			if (m_wait++>=40)
			{
				m_wt++;
			}
		}
		else
		{
			m_wait=0;
			m_wt++;
		}
		m_wt&=127;
	}

	{
		int xsc=((!aboutThread_mode ? img_w[0]*180 : img_w[1]*360))/m_pitch;
		int ysc=((!aboutThread_mode ? img_h[0]*200 : img_h[1]*500))/m_height;
		int w2=m_pitch/2;
		int h2=m_height/2;
		char *out=m_dib;
		int y;
		if (m_wt)
		{
			a=!a;
			// weird interlace shit
			if (a && mode)
			{
				out+=m_pitch;
			}
			for (y = -h2+(mode?a:0); y < h2; y ++)
			{
				int x2=w2*w2+w2+y*y+256;
				int dx2=-2*w2;
				int yysc=y*ysc;
				int xxsc=-w2*xsc;
				int x=m_pitch;
				while (x--)
				{
					int qd=tab[isqrt(x2)];
					int ow,oh;
					x2+=dx2;
					dx2+=2;
					ow = img_w[aboutThread_mode]/2 + (qd*xxsc)/65536;
					xxsc+=xsc;
					oh = img_h[aboutThread_mode]/2 + (qd*yysc)/65536;

					if (ow < 0) ow=0;
					else if (ow >= img_w[aboutThread_mode]) ow=img_w[aboutThread_mode]-1;
					if (oh < 0) oh=0;
					else if (oh >= img_h[aboutThread_mode]) oh=img_h[aboutThread_mode]-1;

					*out++=m_source[ow+m_wmul[oh]];
				}

				// weird interlace shit
				if (mode) 
				{
					y++;
					out+=m_pitch;
				}
			}
		}
		else // copy, with optional rescale
		{
			int x;
			int skipw=0;
			int skiph=0;

			int dxp,dyp,xp,yp;

			skiph=(m_height-img_h[aboutThread_mode])/2;
			skipw=(m_pitch-img_w[aboutThread_mode])/2;

			if (skipw <0) skipw=0;
			if (skiph <0) skiph=0;

			dxp=(img_w[aboutThread_mode]<<16)/(m_pitch-skipw*2);
			dyp=(img_h[aboutThread_mode]<<16)/(m_height-skiph*2);
			yp=0;
			for (y = 0; y < m_height; y ++)
			{
				if (y < skiph || y >= m_height - skiph)
				{
					memset(out,0,m_pitch);
					out+=m_pitch;
				}
				else
				{
					char *in=m_source+(yp>>16)*img_w[aboutThread_mode];
					xp=0;
					for (x = 0; x < m_pitch; x ++)
					{
						if (x < skipw || x >=m_pitch-skipw)
						{
							*out++=0;
						}
						else
						{
							*out++=in[xp>>16];
							xp+=dxp;
						}
					}
					yp+=dyp;
				}
			}
		}
	}

	{
		HWND hwnd=GetDlgItem(hwndDlg,IDC_ABOUTIMG);
		if (hwnd)
		{
			HDC h=GetDC(hwnd);
			if (h)
			{
				if (m_haboutpal)
				{
					SelectPalette(h,m_haboutpal,FALSE);
					RealizePalette(h);
				}
				BitBlt(h,0,0,m_pitch,m_height,m_hdc,0,0,SRCCOPY);
				ReleaseDC(hwnd,h);
			}
		}
	}

	return true;
}

void AboutContext::Quit()
{
	if (m_hbm)
	{
		if (m_hdc) SelectObject(m_hdc, m_oldhbm);
		DeleteObject(m_hbm);
		m_hbm = NULL;
		m_oldhbm=NULL;
	}
	if (m_haboutpal) 
	{
		DeleteObject(m_haboutpal);
		m_haboutpal = NULL;
	}
	if (m_hdc) 
	{
		DeleteDC(m_hdc);
		m_hdc=NULL;
	}
	if (m_source)
	{
		GlobalFree((HGLOBAL)m_source);
		m_source=NULL;
	}
}

static int AboutTickThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id)
{
	AboutContext *context = (AboutContext *)user_data;
	TimerHandle t(handle);
	if (context->Tick())
	{
		t.Wait(50);
		return 0;
	}
	else
	{
		context->Quit();
		delete context;
		HANDLE event = (HANDLE)id;
		SetEvent(event);
		WASABI_API_THREADPOOL->RemoveHandle(0, handle);
		t.Close();
		return 0;
	}
}

static int AboutThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id)
{
	AboutContext *context = new AboutContext;
	if (context->Init((HWND)user_data))
	{
		TimerHandle t;
		WASABI_API_THREADPOOL->AddHandle(0, t, AboutTickThreadPoolFunc, context, id, 0);
		t.Wait(50);
	}
	else
	{
		delete context;
		HANDLE event = (HANDLE)id;
		SetEvent(event);
	}
	return 0;
}
/* Window Proc for the 'winamp' tab of the about screen */
static INT_PTR CALLBACK about1EggProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
#if 0
			static char ascii[]="\n\n  ______  ____  ___/\\  _/\\_ _____  dro  _____   <3  ______  /\\____   _______\n\
													\\_    \\/    \\/     \\_\\__/_\\    \\___  _\\  _ \\___  _\\     \\/      \\__\\  _   \\_\n\
													/    /     /     /     /          \\/   |/     \\/    /  /  /     /   |/    /\n\
  													/    /  /  /     /     /     /     /           /    /     /     /     ____/\n\
  													\\______/\\________\\____/\\____/\\_____\\____/\\_____\\____\\____/\\____/\\____/\n\
  													_________________________________________________________________  _________\n\
 													(_________________________________________________________________\\\\\\_WINAMP_)\n\n\n\
													supplied by deadbeef\n\n\
													cracked by rOn\n\n\
													32kb cool intro by lone";

			int i=0;
			FILE *fh;
			fh=fopen("c:\\blah.c","wt");
			for(i=0;ascii[i];i++)
			{
				ascii[i]^=0x65;
				fprintf(fh,"0x%x,",ascii[i]);
				if((i&31)==31) fprintf(fh,"\n");
			}
			fclose(fh);
#else
			static char ascii[]={
					0x6f,0x6f,0x6f,0x45,0x45,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x45,0x45,0x3a,0x3a,0x3a,0x3a,0x45,0x45,0x3a,0x3a,0x3a,0x4a,0x39,0x45,0x45,0x3a,0x4a,0x39,0x3a,0x45,
					0x3a,0x3a,0x3a,0x3a,0x3a,0x45,0x45,0x1,0x17,0xa,0x45,0x45,0x3a,0x3a,0x3a,0x3a,0x3a,0x45,0x45,0x45,0x59,0x56,0x45,0x45,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x45,0x45,
					0x4a,0x39,0x3a,0x3a,0x3a,0x3a,0x45,0x45,0x45,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x6f,0x45,0x45,0x39,0x3a,0x45,0x45,0x45,0x45,0x39,0x4a,0x45,0x45,0x45,0x45,0x39,
					0x4a,0x45,0x45,0x45,0x45,0x45,0x39,0x3a,0x39,0x3a,0x3a,0x4a,0x3a,0x39,0x45,0x45,0x45,0x45,0x39,0x3a,0x3a,0x3a,0x45,0x45,0x3a,0x39,0x45,0x45,0x3a,0x45,0x39,0x3a,
					0x3a,0x3a,0x45,0x45,0x3a,0x39,0x45,0x45,0x45,0x45,0x45,0x39,0x4a,0x45,0x45,0x45,0x45,0x45,0x45,0x39,0x3a,0x3a,0x39,0x45,0x45,0x3a,0x45,0x45,0x45,0x39,0x3a,0x6f,
					0x45,0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x45,
					0x45,0x45,0x45,0x45,0x45,0x39,0x4a,0x45,0x45,0x45,0x19,0x4a,0x45,0x45,0x45,0x45,0x45,0x39,0x4a,0x45,0x45,0x45,0x45,0x4a,0x45,0x45,0x4a,0x45,0x45,0x4a,0x45,0x45,
					0x45,0x45,0x45,0x4a,0x45,0x45,0x45,0x19,0x4a,0x45,0x45,0x45,0x45,0x4a,0x6f,0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x4a,0x45,0x45,0x4a,0x45,0x45,0x4a,0x45,0x45,0x45,
					0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,
					0x4a,0x45,0x45,0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x45,0x4a,0x45,0x45,0x45,0x45,0x45,0x3a,0x3a,0x3a,0x3a,0x4a,0x6f,0x45,0x45,0x39,
					0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x4a,0x39,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x39,0x3a,0x3a,0x3a,0x3a,0x4a,0x39,0x3a,0x3a,0x3a,0x3a,0x4a,0x39,0x3a,0x3a,0x3a,
					0x3a,0x3a,0x39,0x3a,0x3a,0x3a,0x3a,0x4a,0x39,0x3a,0x3a,0x3a,0x3a,0x3a,0x39,0x3a,0x3a,0x3a,0x3a,0x39,0x3a,0x3a,0x3a,0x3a,0x4a,0x39,0x3a,0x3a,0x3a,0x3a,0x4a,0x39,
					0x3a,0x3a,0x3a,0x3a,0x4a,0x6f,0x45,0x45,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,
					0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,
					0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x45,0x45,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x6f,0x45,0x4d,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,
					0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,
					0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x3a,0x39,0x39,0x39,0x3a,0x32,0x2c,0x2b,0x24,
					0x28,0x35,0x3a,0x4c,0x6f,0x6f,0x6f,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,
					0x45,0x45,0x45,0x16,0x10,0x15,0x15,0x9,0xc,0x0,0x1,0x45,0x7,0x1c,0x45,0x1,0x0,0x4,0x1,0x7,0x0,0x0,0x3,0x6f,0x6f,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,
					0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x6,0x17,0x4,0x6,0xe,0x0,0x1,0x45,0x7,0x1c,0x45,
					0x17,0x2a,0xb,0x6f,0x6f,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,
					0x56,0x57,0xe,0x7,0x45,0x6,0xa,0xa,0x9,0x45,0xc,0xb,0x11,0x17,0xa,0x45,0x7,0x1c,0x45,0x9,0xa,0xb,0x0,
			};
			static int dexored=0;
			if(!dexored)
			{
				int i=0;
				for(i=0;i<sizeof(ascii);i++) ascii[i]^=0x65;
				dexored=1;
			}
#endif
			SetDlgItemTextA(hwndDlg,IDC_ASCII,ascii);
		}
		return 1;
	case WM_LBUTTONDBLCLK:
		{
			EndDialog(hwndDlg,0);
			ShowWindow(GetDlgItem(GetParent(hwndDlg),IDC_ABOUTIMG),SW_NORMAL);
		}
		break;
	}
	return 0;
}

/* Window proc for the about screen (i.e. this one handles the tab changes */
static BOOL CALLBACK about1Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static HANDLE hThread;
	static HWND egg_wnd;
	if (uMsg == WM_INITDIALOG)
	{
		if (!hThread)
		{
			aboutThread_kill=0;
			aboutThread_mode=0;
			hThread = CreateEvent(NULL, TRUE, FALSE, NULL);
			WASABI_API_THREADPOOL->RunFunction(0, AboutThreadPoolFunc, (void *)hwndDlg, (intptr_t)hThread, 0);
		}
		{
			wchar_t buf[2048] = {0}, buf2[2048] = {0}, buf3[256] = {0},
					*t1 = 0, *t2 = 0, *t3 = 0;
			GetDlgItemTextW(hwndDlg,IDC_ABOUTTEXT,buf,ARRAYSIZE(buf));
			StringCchPrintfW(buf2,2048,buf,(buf3[0] ? buf3 : (t1 = AutoWideDup(app_version_string))),
							 (t2 = AutoWideDup(APP_VERSION_PLATFORM)), (t3 = AutoWideDup(app_date)));
			SetDlgItemTextW(hwndDlg, IDC_ABOUTTEXT, buf2);
			link_startsubclass(hwndDlg, IDC_WINAMPLINK);
			if (t1) free(t1);
			if (t2) free(t2);
			if (t3) free(t3);
		}
		return TRUE;
	}
	if (uMsg == WM_QUERYNEWPALETTE)
	{
		if (m_haboutpal) 
		{
			HDC hdc = GetWindowDC(hwndDlg);
			SelectPalette(hdc,m_haboutpal,FALSE);
			RealizePalette(hdc);
			InvalidateRect(hwndDlg,NULL,FALSE);
			ReleaseDC(hwndDlg,hdc);
			return 1;
		}
		return 0;
	}
	if (uMsg == WM_PALETTECHANGED)
	{
		if (m_haboutpal)
		{
			HDC hdc = GetWindowDC(hwndDlg);
			SelectPalette(hdc,m_haboutpal,FALSE);
			RealizePalette(hdc);
			UpdateColors(hdc);
			ReleaseDC(hwndDlg,hdc);
			return 1;
		}
		return 0;
	}
	if (uMsg == WM_DESTROY)
	{
		if (hThread)
		{
			aboutThread_kill=1;
			WaitForSingleObject(hThread,INFINITE);
			CloseHandle(hThread);
			hThread=NULL;
		}
	}
	if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
			case IDC_WINAMPLINK:
			{
				wchar_t homepage[1024] = {0};
				if (config_langpack[0])
				{
					getStringW(IDS_LOCALIZED_HOMEPAGE, homepage, 1024);
				}
				if (homepage[0])
					myOpenURL(hwndDlg, homepage);
				else
					myOpenURL(hwndDlg,L"http://www.winamp.com/");
			}
			return 0;
		}
	}
	if (uMsg == WM_LBUTTONDBLCLK)
	{
		if ((GetAsyncKeyState(VK_SHIFT)&0x8000) && !(GetAsyncKeyState(VK_CONTROL)&0x8000))
		{
			ShowWindow(GetDlgItem(hwndDlg,IDC_ABOUTIMG),SW_HIDE);
			if (!IsWindow(egg_wnd))
			{
				egg_wnd = LPCreateDialogW(IDD_NEWABOUT1EGG,hwndDlg,about1EggProc);
			}
			ShowWindow(egg_wnd, SW_SHOW);
		}
		else
		{
			if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && !(GetAsyncKeyState(VK_SHIFT)&0x8000))
			{
				if (hThread)
				{
					aboutThread_kill=1;
					WaitForSingleObject(hThread,INFINITE);
					CloseHandle(hThread);
					hThread=NULL;
					aboutThread_kill=0;
					aboutThread_mode=!aboutThread_mode;
					hThread = CreateEvent(NULL, TRUE, FALSE, NULL);
					WASABI_API_THREADPOOL->RunFunction(0, AboutThreadPoolFunc, (void *)hwndDlg, (intptr_t)hThread, 0);
				}
			}
			ShowWindow(GetDlgItem(hwndDlg,IDC_ABOUTIMG),SW_SHOW);
		}
	}
	link_handledraw(hwndDlg,uMsg,wParam,lParam);
	return 0;
}

static volatile int creditThread_kill;
static HPALETTE m_hcreditspal;

/* this thread handles the 3d credits rendering (this one does some busywork and housecleaning, actual rendering done in creditsrend.c) */
class CreditsContext
{
public:
	bool Init(HWND _hwndDlg);
	bool Tick(int &wait_time);
	void Quit();

private:	
	HDC m_hdc;
	HBITMAP m_hbm,m_oldhbm;
	int m_pitch,m_height;
	char *m_dib;
char pal[768];
	struct
	{
		BITMAPINFO bmi;
		RGBQUAD more_bmiColors[255];
	} m_bitmap;
	int c,use_palette;
	HWND hwndDlg;
};

bool CreditsContext::Init(HWND _hwndDlg)
{
	hwndDlg = _hwndDlg;
	Tataki::Init(WASABI_API_SVC);
	m_hcreditspal = 0;
	m_hdc = 0;
	m_hbm = 0;

	RECT r={0,};
	memset(&m_bitmap, 0, sizeof(m_bitmap));

	GetWindowRect(hwndDlg,&r);
	r.right=r.right-r.left;
	r.bottom=r.bottom-r.top;
		HDC hdc=GetWindowDC(hwndDlg);
		if (!hdc) 
			return false;
		use_palette = (GetDeviceCaps(hdc,RASTERCAPS)&RC_PALETTE)?1:0;
		m_hdc = CreateCompatibleDC(NULL);
		ReleaseDC(hwndDlg,hdc);

		m_pitch=((r.right+3)&~3);
	m_height=r.bottom;
	if (m_pitch < 4) m_pitch=4;
	if (m_height < 4) m_height=4;
	//  if (m_pitch > GetSystemMetrics(SM_CXSCREEN)) m_pitch=GetSystemMetrics(SM_CXSCREEN);
	//  if (m_height > GetSystemMetrics(SM_CYSCREEN)) m_height=GetSystemMetrics(SM_CYSCREEN);
	render_init(m_pitch,m_height,pal);
	
	{
		struct 
		{
			LOGPALETTE logpal;
			PALETTEENTRY palentries[255];
		} palette;
		palette.logpal.palVersion = 0x300;
		palette.logpal.palNumEntries = 220;
		for (c = 0; c < 256; c ++) 
		{
			palette.logpal.palPalEntry[c].peRed = pal[c*3];
			palette.logpal.palPalEntry[c].peGreen = pal[c*3+1];
			palette.logpal.palPalEntry[c].peBlue = pal[c*3+2];
			palette.logpal.palPalEntry[c].peFlags = PC_NOCOLLAPSE;
			m_bitmap.bmi.bmiColors[c].rgbRed=pal[c*3];
			m_bitmap.bmi.bmiColors[c].rgbGreen=pal[c*3+1];
			m_bitmap.bmi.bmiColors[c].rgbBlue=pal[c*3+2];
			m_bitmap.bmi.bmiColors[c].rgbReserved=0;
		}
		if (use_palette) 
			m_hcreditspal = CreatePalette((LOGPALETTE *)&palette.logpal);
	}
		

	m_bitmap.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bitmap.bmi.bmiHeader.biPlanes = 1;
	m_bitmap.bmi.bmiHeader.biBitCount = 8;
	m_bitmap.bmi.bmiHeader.biCompression = BI_RGB;
	m_bitmap.bmi.bmiHeader.biSizeImage = 0;
	m_bitmap.bmi.bmiHeader.biClrImportant = 0;
	m_bitmap.bmi.bmiHeader.biClrUsed = 256;
	m_bitmap.bmi.bmiHeader.biWidth = m_pitch;
	m_bitmap.bmi.bmiHeader.biHeight = -m_height;
	m_bitmap.bmi.bmiHeader.biSizeImage = m_pitch*m_height;
	m_hbm = CreateDIBSection(m_hdc,&m_bitmap.bmi,DIB_RGB_COLORS, (void**)&m_dib, NULL, 0);
	m_oldhbm = (HBITMAP) SelectObject(m_hdc, m_hbm);
	return (m_hbm && m_dib);
}

void CreditsContext::Quit()
{
		if (m_hbm)
	{
		if (m_hdc) SelectObject(m_hdc, m_oldhbm);
		DeleteObject(m_hbm);
		m_hbm = NULL;
		m_oldhbm=NULL;
	}
	if (m_hcreditspal) 
	{
		DeleteObject(m_hcreditspal);
		m_hcreditspal = NULL;
	}
	if (m_hdc) 
	{
		DeleteDC(m_hdc);
		m_hdc=NULL;
	}
	render_quit();
	Tataki::Quit();
}

bool CreditsContext::Tick(int &wait_time)
{
	if (creditThread_kill)
		return false;

		unsigned int thist=GetTickCount();
		render_render((unsigned char*)m_dib, m_hdc);
		{
			HDC h=GetDC(hwndDlg);
			if (h)
			{
				if (m_hcreditspal)
				{
					SelectPalette(h,m_hcreditspal,FALSE);
					RealizePalette(h);
				}
				BitBlt(h,0,0,m_pitch,m_height,m_hdc,0,0,SRCCOPY);
				ReleaseDC(hwndDlg,h);
			}
		}
		thist=GetTickCount()-thist;
		if (thist > 28) thist=28;
		wait_time = 30-thist;
		return true;
}

static int CreditsTickThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id)
{
	CreditsContext *context = (CreditsContext *)user_data;
	TimerHandle t(handle);
	int wait_time=30;
	if (context->Tick(wait_time))
	{
		t.Wait(wait_time);
		return 0;
	}
	else
	{
		context->Quit();
		delete context;
		HANDLE event = (HANDLE)id;
		SetEvent(event);
		WASABI_API_THREADPOOL->RemoveHandle(0, handle);
		t.Close();
		return 0;
	}
}

static int CreditThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id)
{
	CreditsContext *context = new CreditsContext;
	if (context->Init((HWND)user_data))
	{
		TimerHandle t;
		WASABI_API_THREADPOOL->AddHandle(0, t, CreditsTickThreadPoolFunc, context, id, api_threadpool::FLAG_LONG_EXECUTION);
		t.Wait(30);
	}
	else
	{
		delete context;
		HANDLE event = (HANDLE)id;
		SetEvent(event);
	}
	return 0;
}

/* Window Proc for the 'credits' tab of the about screen */
static BOOL CALLBACK creditProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static int g_fullscreen=0;
	static RECT rsave;
	static HWND hOldParent;
	static int oldstyle;
	static HANDLE hThread;
	if (uMsg == WM_LBUTTONDBLCLK)
	{
		render_togglecredits();
		return 0;
	}
	if (uMsg == WM_KEYDOWN && g_fullscreen)
	{
		if (wParam == VK_ESCAPE)
		{
			PostMessageW(hwndDlg,WM_LBUTTONDOWN,0,0);
		}
		else PostMessageW(hMainWindow,uMsg,wParam,lParam);
	}
	if ((uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK) && g_fullscreen)
	{
		if (hThread)
		{
			creditThread_kill=1;
			WaitForSingleObject(hThread,INFINITE);
			CloseHandle(hThread);
			hThread = 0;
			if (g_fullscreen)
			{
				RECT r=rsave;
				ScreenToClient(hOldParent,(LPPOINT)&r);
				ScreenToClient(hOldParent,(LPPOINT)&r+1);
				SetWindowLong(hwndDlg,GWL_STYLE,oldstyle);
				SetParent(hwndDlg,hOldParent);
				SetWindowPos(hwndDlg,0,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOZORDER);
			}
		}
		g_fullscreen=0;
		creditThread_kill=0;
		hThread = CreateEvent(NULL, TRUE, FALSE, NULL);
		WASABI_API_THREADPOOL->RunFunction(0, CreditThreadPoolFunc, (void *)hwndDlg, (intptr_t)hThread, api_threadpool::FLAG_LONG_EXECUTION);
	}
	else if (uMsg == WM_RBUTTONDBLCLK && !g_fullscreen)
	{
		RECT r;
		if (hThread)
		{
			creditThread_kill=1;
			WaitForSingleObject(hThread,INFINITE);
			CloseHandle(hThread);
			hThread = 0;
		}
		g_fullscreen=1;
		GetWindowRect(hwndDlg,&rsave);
		oldstyle=GetWindowLongW(hwndDlg,GWL_STYLE);
		hOldParent=SetParent(hwndDlg,NULL);

		SetWindowLong(hwndDlg,GWL_STYLE,WS_POPUP|WS_VISIBLE);

		getViewport(&r,hwndDlg,1,NULL);

		SetWindowPos(hwndDlg, HWND_TOPMOST, r.left, r.top, r.right, r.bottom, SWP_DRAWFRAME);
		creditThread_kill=0;
		hThread = CreateEvent(NULL, TRUE, FALSE, NULL);
		WASABI_API_THREADPOOL->RunFunction(0, CreditThreadPoolFunc, (void *)hwndDlg, (intptr_t)hThread, api_threadpool::FLAG_LONG_EXECUTION);
		// go fullscreen
	}
	if (uMsg == WM_INITDIALOG)
	{
		if (!hThread)
		{
			g_fullscreen=0;
			creditThread_kill=0;
			hThread = CreateEvent(NULL, TRUE, FALSE, NULL);
			WASABI_API_THREADPOOL->RunFunction(0, CreditThreadPoolFunc, (void *)hwndDlg, (intptr_t)hThread, api_threadpool::FLAG_LONG_EXECUTION);
		}
		return TRUE;
	}
	if (uMsg == WM_QUERYNEWPALETTE)
	{
		if (m_hcreditspal) 
		{
			HDC hdc = GetWindowDC(hwndDlg);
			SelectPalette(hdc,m_hcreditspal,FALSE);
			RealizePalette(hdc);
			InvalidateRect(hwndDlg,NULL,FALSE);
			ReleaseDC(hwndDlg,hdc);
			return 1;
		}
		return 0;
	}
	if (uMsg == WM_PALETTECHANGED)
	{
		if (m_hcreditspal)
		{
			HDC hdc = GetWindowDC(hwndDlg);
			SelectPalette(hdc,m_hcreditspal,FALSE);
			RealizePalette(hdc);
			UpdateColors(hdc);
			ReleaseDC(hwndDlg,hdc);
			return 1;
		}
		return 0;
	}
	if (uMsg == WM_DESTROY)
	{
		if (hThread)
		{
			creditThread_kill=1;
			WaitForSingleObject(hThread,INFINITE);
			CloseHandle(hThread);
			hThread=NULL;
		}
	}
	return 0;
}

static BOOL CALLBACK translationProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			// only attempt set it to show a hand cursor if it's an ownerdraw button
			if(GetWindowLongPtrW(GetDlgItem(hwndDlg, IDC_AUTHOR_HOMEPAGE), GWL_STYLE) & BS_OWNERDRAW)
			{
				link_startsubclass(hwndDlg, IDC_AUTHOR_HOMEPAGE);
			}
			if(GetWindowLongPtrW(GetDlgItem(hwndDlg, IDC_AUTHOR_HOMEPAGE2), GWL_STYLE) & BS_OWNERDRAW)
			{
				link_startsubclass(hwndDlg, IDC_AUTHOR_HOMEPAGE2);
			}
		}
		break;

	case WM_COMMAND:
		if(LOWORD(wParam) == IDCANCEL)
		{
			DestroyWindow(about_hwnd);
		}
		else if (LOWORD(wParam) == IDC_AUTHOR_HOMEPAGE)
		{
			wchar_t homepage[1024] = {0};
			getStringW(IDS_AUTHOR_HOMEPAGE, homepage, 1024);
			myOpenURL(hwndDlg, homepage);
		}
		else if (LOWORD(wParam) == IDC_AUTHOR_HOMEPAGE2)
		{
			wchar_t homepage[1024] = {0};
			getStringW(IDS_AUTHOR_HOMEPAGE2, homepage, 1024);
			myOpenURL(hwndDlg, homepage);
		}
		break;
	} 
	link_handledraw(hwndDlg,uMsg,wParam,lParam);
	return 0;
}
