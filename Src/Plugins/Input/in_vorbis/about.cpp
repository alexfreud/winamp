#include "main.h"
#include "api__in_vorbis.h"
#include "resource.h"
#include <strsafe.h>

/*static UINT xiphframes_ids[12]={IDB_BITMAP1,IDB_BITMAP2,IDB_BITMAP3,IDB_BITMAP4,IDB_BITMAP5,IDB_BITMAP6,IDB_BITMAP7,IDB_BITMAP8,IDB_BITMAP9,IDB_BITMAP10,IDB_BITMAP11,IDB_BITMAP12};
static HBITMAP xiphframes[12];*/

static UINT xiphframes_ids[12]={IDB_PNG1,IDB_PNG2,IDB_PNG3,IDB_PNG4,IDB_PNG5,IDB_PNG6,IDB_PNG7,IDB_PNG8,IDB_PNG9,IDB_PNG10,IDB_PNG11,IDB_PNG12};
static ARGB32 *xiphframes[12] = {0};
static HBITMAP xiphframesBmp[12] = {0};

static void slap(HWND wnd,int v)
{
	long hi=GetWindowLong(wnd,4);
	if (v) hi+=v*1000;
	else hi=0;
	SetWindowLong(wnd,4,hi);
}

static CfgInt cfg_rpm("rpm",0);

static int visible_rpm,visible_max_rpm;
static char show_rpm=0;
static DWORD last_visible_rpm;

ARGB32 * loadImg(const void * data, int len, int *w, int *h, bool ldata=false)
{
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = (int)mod.service->service_getNumServices(imgload);
	for(int i=0; i<n; i++)
	{
		waServiceFactory *sf = mod.service->service_enumService(imgload,i);
		if(sf)
		{
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if(l)
			{
				if(l->testData(data,len))
				{
					ARGB32* ret;
					if(ldata) ret = l->loadImageData(data,len,w,h);
					else ret = l->loadImage(data,len,w,h);
					sf->releaseInterface(l);
					return ret;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}

ARGB32 * loadRrc(int id, wchar_t * sec, int *w, int *h, bool data=false)
{
	DWORD size=0;
	HGLOBAL resourceHandle = WASABI_API_LOADRESFROMFILEW(sec, MAKEINTRESOURCEW(id), &size);
	if(resourceHandle)
	{
		ARGB32* ret = loadImg(resourceHandle,size,w,h,data);
		UnlockResource(resourceHandle);
		return ret;
	}
	return NULL;
}

static LRESULT WINAPI XiphProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
		case WM_CREATE:
			SetWindowLong(wnd,8,last_visible_rpm=GetTickCount());
			SetTimer(wnd,666,10,0);
			visible_rpm=-1;
			visible_max_rpm=-1;
			show_rpm=0;
			break;
		case WM_TIMER:
			if (wp==666)
			{
				long low=GetWindowLong(wnd,0);
				long hi=GetWindowLong(wnd,4);

				long org=low&~0xFFFF;

				int rpm=MulDiv(abs(hi),1000*60,12*0x10000);

				DWORD t=GetTickCount();
				DWORD ot=(DWORD)SetWindowLong(wnd,8,t);
				bool redraw=0;

				if (rpm>25) show_rpm=1;
				if (cfg_rpm<rpm) cfg_rpm=rpm;

				if (show_rpm && (t&~0x3F)!=(ot&~0x3F))
				{
					wchar_t foo[128] = {0};
					if (visible_rpm<rpm || (visible_rpm>rpm && (t-last_visible_rpm)>333))
					{
						last_visible_rpm=t;
						visible_rpm=rpm;
						StringCchPrintfW(foo,128,WASABI_API_LNGSTRINGW(IDS_GAME_SPEED),rpm);
						SetDlgItemTextW(GetParent(wnd),IDC_RPM,foo);
					}
					if (visible_max_rpm!=cfg_rpm)
					{
						visible_max_rpm=cfg_rpm;
						StringCchPrintfW(foo,128,WASABI_API_LNGSTRINGW(IDS_BEST_RPM),(int)cfg_rpm);
						SetDlgItemTextW(GetParent(wnd),IDC_RPM2,foo);
					}
				}

				low+=hi*(t-ot);
				while(low<0) low+=12*0x10000;
				while(low>=12*0x10000) low-=12*0x10000;

				{
					int z=hi>>6;
					if (z) hi-=z;
					else if (hi>0) hi--;
					else if (hi<0) hi++;
				}

				SetWindowLong(wnd,0,low);
				SetWindowLong(wnd,4,hi);
				if (redraw || (low&~0xFFFF)!=org)
				{
					RedrawWindow(wnd,0,0,RDW_INVALIDATE);
				}
				KillTimer(wnd,666);
				SetTimer(wnd,666,10,0);
			}
			break;
		case WM_LBUTTONDOWN:
			slap(wnd,-1);
			break;
		case WM_RBUTTONDOWN:
			slap(wnd,1);
			break;
		case WM_MBUTTONDOWN:
			slap(wnd,0);
			break;
		case WM_PAINT:
			{
				int i=(GetWindowLong(wnd,0))>>16;
				HDC dc = CreateCompatibleDC(0);

				if (!xiphframesBmp[i])
				{
					int cur_w = 0, cur_h = 0;
					xiphframes[i] = loadRrc(xiphframes_ids[i], L"PNG", &cur_w, &cur_h, true);

					BITMAPINFO bmi = {0};
					bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					bmi.bmiHeader.biWidth = cur_w;
					bmi.bmiHeader.biHeight = -cur_h;
					bmi.bmiHeader.biPlanes = 1;
					bmi.bmiHeader.biBitCount = 32;
					bmi.bmiHeader.biCompression = BI_RGB;
					void *bits = 0;
					if(xiphframesBmp[i]) DeleteObject(xiphframesBmp[i]);
					xiphframesBmp[i] = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
					memcpy(bits, xiphframes[i], cur_w * cur_h * 4);
				}

				if (xiphframesBmp[i])
				{
					HGDIOBJ foo = SelectObject(dc, xiphframesBmp[i]);
					HDC wdc = GetDC(wnd);
					RECT r = {0};
					GetClientRect(wnd, &r);
					FillRect(wdc, &r, GetSysColorBrush(COLOR_3DFACE));

					BLENDFUNCTION blendFn = {0};
					blendFn.BlendOp = AC_SRC_OVER;
					blendFn.SourceConstantAlpha = 255;
					blendFn.AlphaFormat = AC_SRC_ALPHA;
					AlphaBlend(wdc, 2, 2, r.right - 2, r.bottom - 2, dc, 0, 0, 63, 63, blendFn);

					ReleaseDC(wnd, wdc);
					SelectObject(dc, foo);
				}
				DeleteDC(dc);
			}
			break;
		case WM_DESTROY:
		{
			for (int i = 0; i < ARRAYSIZE(xiphframes_ids); i++)
			{
				if(xiphframesBmp[i]) DeleteObject(xiphframesBmp[i]); xiphframesBmp[i] = 0;
				if(xiphframes[i] && WASABI_API_MEMMGR) WASABI_API_MEMMGR->sysFree((void *)xiphframes[i]); xiphframes[i] = 0;
			}
			KillTimer(wnd,666);
			break;
		}
	};
	return DefWindowProc(wnd,msg,wp,lp);
}

static BOOL CALLBACK AboutProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
	{
		wchar_t tmp[1024] = {0}, tmp2[1024] = {0}, *t1 = tmp, *t2 = tmp2, text[1024] = {0};
		SetWindowTextW(wnd,WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_VORBIS_DECODER_OLD,text,1024));
		StringCchPrintfW(tmp,1024,WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),mod.description,__DATE__);
		// due to quirks with the more common resource editors, is easier to just store the string
		// internally only with \n and post-process to be \r\n (as here) so it will appear correctly
		// on new lines as is wanted (silly multiline edit controls)
		while(t1 && *t1 && (t2 - tmp2 < 1024))
		{
			if(*t1 == L'\n')
			{
				*t2 = L'\r';
				t2 = CharNextW(t2);
			}
			*t2 = *t1;
			t1 = CharNextW(t1);
			t2 = CharNextW(t2);
		}

		SetDlgItemTextW(wnd,IDC_ABOUT_TEXT,tmp2);
		// fixes the incorrect selection of the text on dialog opening
		PostMessage(GetDlgItem(wnd,IDC_ABOUT_TEXT),EM_SETSEL,-1,0);
		return 1;
	}
	case WM_COMMAND:
		if (wp==IDOK || wp==IDCANCEL)
		{
			do_cfg(1);
			EndDialog(wnd,0);
		}
		break;
	}
	return 0;
}

void About(HWND hwndParent)
{
	static char got_xiph;
	if (!got_xiph)
	{
		WNDCLASS wc=
		{
			0,
			XiphProc,
			0,
			12,
			WASABI_API_LNG_HINST,
			0,
			LoadCursor(0,IDC_ARROW),
			0,
			0,
			L"XIPH_CLASS",
		};

		RegisterClassW(&wc);
		got_xiph=1;
	}

	WASABI_API_DIALOGBOXW(IDD_ABOUT,hwndParent,AboutProc);
}