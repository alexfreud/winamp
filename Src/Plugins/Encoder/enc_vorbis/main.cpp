/*
** enc_vorbis: main.cpp - Ogg Vorbis encoder plug-in
** 
** Copyright (C) 2001-2012 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty.  
** In no event will the authors be held liable for any damages arising from the use of this software.
**
** Permission is granted to anyone to use this software for any purpose, including commercial 
** applications, and to alter it and redistribute it freely, subject to the following restrictions:
**  1. The origin of this software must not be misrepresented; you must not claim that you wrote the 
**     original software. If you use this software in a product, an acknowledgment in the product 
**     documentation would be appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be misrepresented as 
**     being the original software.
**  3. This notice may not be removed or altered from any source distribution.
*/

#define ENC_VERSION "v1.58"

#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <strsafe.h>
#include "resource.h"
#include "../nsv/enc_if.h"
#include <vorbis/vorbisenc.h>

// wasabi based services for localisation support
#include <api/service/waServiceFactory.h>
#include "../Agave/Language/api_language.h"
#include <api/application/api_application.h>
#include "../winamp/wa_ipc.h"

HWND winampwnd = 0;
int isthemethere = 0;
api_service *WASABI_API_SVC = 0;
api_application *WASABI_API_APP = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
}

typedef struct 
{
  bool cfg_abr_use_max,cfg_abr_use_min;
  UINT cfg_mode;

  float cfg_vbrquality;
  UINT cfg_abr_nominal;
  UINT cfg_abr_max;
  UINT cfg_abr_min;
} configtype;

typedef struct
{
  configtype cfg;
  char *configfile;
} 
configwndrec;

void readconfig(char *configfile, configtype *cfg)
{
	cfg->cfg_abr_use_max=0;
	cfg->cfg_abr_use_min=0;
	cfg->cfg_mode=0; //VBR

	cfg->cfg_vbrquality=0.4f;

	cfg->cfg_abr_nominal=160;
	cfg->cfg_abr_max=352;
	cfg->cfg_abr_min=32;

	if (configfile) GetPrivateProfileStructA("audio_ogg","conf",cfg,sizeof(configtype),configfile);
		cfg->cfg_mode=0; // VBR, fuckers.
}

void writeconfig(char *configfile, configtype *cfg)
{
	if (configfile) WritePrivateProfileStructA("audio_ogg","conf",cfg,sizeof(configtype),configfile);
}

static HINSTANCE GetMyInstance()
{
	MEMORY_BASIC_INFORMATION mbi = {0};
	if(VirtualQuery(GetMyInstance, &mbi, sizeof(mbi)))
		return (HINSTANCE)mbi.AllocationBase;
	return NULL;
}

void GetLocalisationApiService(void)
{
	if(!WASABI_API_LNG)
	{
		// loader so that we can get the localisation service api for use
		if(!WASABI_API_SVC)
		{
			WASABI_API_SVC = (api_service*)SendMessage(winampwnd, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
			if (WASABI_API_SVC == (api_service*)1)
			{
				WASABI_API_SVC = NULL;
				return;
			}
		}

		if(!WASABI_API_APP)
		{
			waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
			if (sf) WASABI_API_APP = reinterpret_cast<api_application*>(sf->getInterface());
		}

		if(!WASABI_API_LNG)
		{
			waServiceFactory *sf;
			sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
			if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());
		}

		// need to have this initialised before we try to do anything with localisation features
		WASABI_API_START_LANG(GetMyInstance(),EncVorbisLangGUID);
	}
}

class AudioCoderOgg : public AudioCoder
{
  public:
	AudioCoderOgg(int nch, int srate, int bps, configtype *cfg);
	int Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail);
	~AudioCoderOgg()
	{
		ogg_stream_clear(&os);
		vorbis_block_clear(&vb);
		vorbis_dsp_clear(&vd);
		vorbis_comment_clear(&vc);
		vorbis_info_clear(&vi);
	}
	int GetLastError() { return m_err; };
  private:
	int m_err;
	int m_bps, m_nch;

	ogg_stream_state os; /* take physical pages, weld into a logical stream of packets */
	ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
	ogg_packet       op; /* one raw packet of data for decode */

	vorbis_info      vi; /* struct that stores all the static vorbis bitstream settings */
	vorbis_comment   vc; /* struct that stores all the user comments */

	vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
	vorbis_block     vb; /* local working space for packet->PCM decode */

	int m_inbuf;
};

int AudioCoderOgg::Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail)
{
	if (m_err) return -1;
	*in_used = 0;		//only necessary for flawed impl that dont reset this to zero each time (BAD rOn)

	int wrote=0;
	char *dest = (char*)out;

	if (!in_avail && !framepos) 
	{
		vorbis_analysis_wrote(&vd,0);
	}

	for (;;)
	{
		/* vorbis does some data preanalysis, then divvies up blocks for
		more involved (potentially parallel) processing.  Get a single
		block for encoding now */

		while (m_inbuf || vorbis_analysis_blockout(&vd,&vb)==1)
		{
			/* analysis */
			if (!m_inbuf)
			{
				vorbis_analysis(&vb,&op);
				vorbis_bitrate_addblock(&vb);
			}

			while (m_inbuf || vorbis_bitrate_flushpacket(&vd, &op))
			{
				/* weld the packet into the bitstream */
				if (!m_inbuf) ogg_stream_packetin(&os,&op);

				/* write out pages (if any) */
				while (m_inbuf || ogg_stream_pageout(&os,&og)) 
				{
					int l=og.header_len+og.body_len;
					if(out_avail<l)
					{
						m_inbuf=1;
						return wrote;
					}
					memcpy(dest,og.header,og.header_len);
					memcpy(dest+og.header_len,og.body,og.body_len);
					dest+=l;
					wrote+=l;
					out_avail-=l;
					m_inbuf=0;

					if (ogg_page_eos(&og)) break;
				}
			}
		}

		// if we used all our samples, or had output, flush it
		if (*in_used >= in_avail || wrote) return wrote; 

		// bring in more pcm samples
		if (in_avail > *in_used) 
		{
			UINT i;
			int c;
			int bytes=in_avail-*in_used;
			void *buf=(char*)in + *in_used;

			if (bytes > 1024) bytes=1024;

			*in_used+=bytes;

			UINT nsam=bytes/((m_bps>>3)*m_nch);
			float **buffer=vorbis_analysis_buffer(&vd,nsam);
			switch(m_bps)
			{
				case 8:
				{
					BYTE* rbuf=(BYTE*)buf;
					for(i=0;i<nsam;i++)
					{
						for(c=0;c<m_nch;c++)
						{
							buffer[c][i]=((float)(UINT)*rbuf)/128.f-1.f;
							rbuf++;					  
						}
					}
				}
				break;
				case 16:
				{
					short* rbuf=(short*)buf;
					for(i=0;i<nsam;i++)
					{
						for(c=0;c<m_nch;c++)
						{
							buffer[c][i]=(float)*rbuf/32768.f;
							rbuf++;
						}
					}
				}
				break;
				case 24:
				{
					char* rbuf=(char*)buf;
					for(i=0;i<nsam;i++)
					{
						for(c=0;c<m_nch;c++)
						{
							BYTE* b=(BYTE*)rbuf;
							long val = b[0] | (b[1]<<8) | (b[2]<<16);
							if (val&0x800000) val|=0xFF000000;
							buffer[c][i]=(float)((double)val/(double)0x800000);
							rbuf+=3;
						}
					}
				}
				break;
				case 32:
				{
					long* rbuf=(long*)buf;
					for(i=0;i<nsam;i++)
					{
						for(c=0;c<m_nch;c++)
						{
							buffer[c][i]=(float)((double)rbuf[i*m_nch+c]/(double)0x80000000);
						}
					}
				}
				break;
			}
    
			/* tell the library how much we actually submitted */
			vorbis_analysis_wrote(&vd,nsam);
		}
	}
}

AudioCoderOgg::AudioCoderOgg(int nch, int srate, int bps, configtype *cfg)
{
	m_err=0;

	m_bps=bps;
	m_nch=nch;
	m_inbuf=0;

	int poo;

	vorbis_info_init(&vi);
	if (cfg->cfg_mode==0) poo=vorbis_encode_init_vbr(&vi,nch,srate,cfg->cfg_vbrquality);
	else
	{
		UINT nominal,min,max;
		if (cfg->cfg_mode==1)
		{//abr
			nominal=cfg->cfg_abr_nominal * 1000;
			min=cfg->cfg_abr_use_min ? cfg->cfg_abr_min * 1000: -1;
			max=cfg->cfg_abr_use_max ? cfg->cfg_abr_max * 1000: -1;
		}
		else//cbr
		{
			nominal=min=max=cfg->cfg_abr_nominal*1000;
		}
		poo=vorbis_encode_init(&vi,nch,srate,max,nominal,min);
	}

	if (poo)
	{
		vorbis_info_clear(&vi);
		m_err++;
		return;
	}

	vorbis_comment_init(&vc);
	vorbis_comment_add(&vc, "ENCODEDBY=Winamp");

	/* set up the analysis state and auxiliary encoding storage */
	vorbis_analysis_init(&vd,&vi);
	vorbis_block_init(&vd,&vb);

	/* set up our packet->stream encoder */
	/* pick a random serial number; that way we can more likely build
	chained streams just by concatenation */

	ogg_stream_init(&os,GetTickCount()^(GetTickCount()<<16));//fixme : rand

	/* Vorbis streams begin with three headers; the initial header (with
	most of the codec setup parameters) which is mandated by the Ogg
	bitstream spec.  The second header holds any comment fields.  The
	third header holds the bitstream codebook.  We merely need to
	make the headers, then pass them to libvorbis one at a time;
	libvorbis handles the additional Ogg bitstream constraints */
	  
	{
		ogg_packet header;
		ogg_packet header_comm;
		ogg_packet header_code;
	    
		vorbis_analysis_headerout(&vd,&vc,&header,&header_comm,&header_code);
		ogg_stream_packetin(&os,&header); /* automatically placed in its own page */
		ogg_stream_packetin(&os,&header_comm);
		ogg_stream_packetin(&os,&header_code);
	}
}

extern "C"
{
	unsigned int __declspec(dllexport) GetAudioTypes3(int idx, char *desc)
	{
		if (idx==0)
		{
			GetLocalisationApiService();
			StringCchPrintfA(desc, 1024, "%s %s (aoTuV b6.03)", WASABI_API_LNGSTRING(IDS_ENC_VORBIS_DESC), ENC_VERSION);
			return mmioFOURCC('O','G','G',' ');
		}
		return 0;
	}

	AudioCoder __declspec(dllexport)  *CreateAudio3(int nch, int srate, int bps, unsigned int srct, unsigned int *outt, char *configfile)
	{
		if (srct == mmioFOURCC('P','C','M',' ') && *outt == mmioFOURCC('O','G','G',' ')) 
		{
			configtype cfg;
			readconfig(configfile,&cfg);
			AudioCoderOgg *t = new AudioCoderOgg(nch,srate,bps,&cfg);
			if (t->GetLastError())
			{
				delete t;
				return NULL;
			}
			return t;
		}
		return NULL;
	}

	void __declspec(dllexport) FinishAudio3(const char *filename, AudioCoder *coder)
	{
	}

	#define doshow(x,y) ShowWindow(x,(y)?SW_SHOWNA:SW_HIDE)

	static HCURSOR link_hand_cursor;
	LRESULT link_handlecursor(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT ret = CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"link_proc"), hwndDlg, uMsg, wParam, lParam);
		// override the normal cursor behaviour so we have a hand to show it is a link
		if(uMsg == WM_SETCURSOR)
		{
			if((HWND)wParam == hwndDlg)
			{
				if(!link_hand_cursor)
				{
					link_hand_cursor = LoadCursor(NULL, IDC_HAND);
				}
				SetCursor(link_hand_cursor);
				return TRUE;
			}
		}
		return ret;
	}

	void link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_DRAWITEM)
		{
			DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
			if (di->CtlType == ODT_BUTTON)
			{
				wchar_t wt[123] = {0};
				int y;
				RECT r;
				HPEN hPen, hOldPen;
				GetDlgItemTextW(hwndDlg, (int)wParam, wt, sizeof(wt)/sizeof(wt[0])); 

				// due to the fun of theming and owner drawing we have to get the background colour
				if(isthemethere){
					HTHEME hTheme = OpenThemeData(hwndDlg, L"Tab");
					if (hTheme) {
						DrawThemeParentBackground(di->hwndItem, di->hDC, &di->rcItem);
						CloseThemeData(hTheme);
					}
				}

				// draw text
				SetTextColor(di->hDC, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
				r = di->rcItem;
				r.left += 2;
				DrawTextW(di->hDC, wt, -1, &r, DT_VCENTER | DT_SINGLELINE);

				memset(&r, 0, sizeof(r));
				DrawTextW(di->hDC, wt, -1, &r, DT_SINGLELINE | DT_CALCRECT);

				// draw underline
				y = di->rcItem.bottom - ((di->rcItem.bottom - di->rcItem.top) - (r.bottom - r.top)) / 2 - 1;
				hPen = CreatePen(PS_SOLID, 0, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
				hOldPen = (HPEN) SelectObject(di->hDC, hPen);
				MoveToEx(di->hDC, di->rcItem.left + 2, y, NULL);
				LineTo(di->hDC, di->rcItem.right + 2 - ((di->rcItem.right - di->rcItem.left) - (r.right - r.left)), y);
				SelectObject(di->hDC, hOldPen);
				DeleteObject(hPen);
			}
		}
	}

	void link_startsubclass(HWND hwndDlg, UINT id)
	{
	HWND ctrl = GetDlgItem(hwndDlg, id);
		if(!GetPropW(ctrl, L"link_proc"))
		{
			SetPropW(ctrl, L"link_proc",
					(HANDLE)SetWindowLongPtrW(ctrl, GWLP_WNDPROC, (LONG_PTR)link_handlecursor));
		}
	}

	BOOL CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
	{
		if (uMsg == WM_USER+667)
		{
			int p=(int)SendDlgItemMessage(hwndDlg,IDC_VBRQUALITY,TBM_GETPOS,0,0)-10;
			char tmp[512] = {0};
			double dispVal = ((double)p)/10;
			vorbis_info vi = {0};
			int br = 128;

			vorbis_info_init(&vi);
			if(vorbis_encode_init_vbr(&vi, 2, 44100, (float) (p / 100.0))) 
				br=128; // Mode setup failed: go with a default. 
			else
			{
				br = vi.bitrate_nominal / 1000;
				vorbis_info_clear(&vi);
			}

			StringCchPrintfA(tmp, 512, WASABI_API_LNGSTRING(IDS_QUALITY_FACTOR_F_KBPS), dispVal, br);

			SetDlgItemTextA(hwndDlg,IDC_VBRVAL,tmp);

			link_startsubclass(hwndDlg, IDC_URL1);
			link_startsubclass(hwndDlg, IDC_URL2);
		}

		else if (uMsg == WM_INITDIALOG)
		{
#if defined (_WIN64)
			SetWindowLong(hwndDlg,GWLP_USERDATA,(LONG)lParam);
#else
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
#endif
			if (lParam)
			{
				configwndrec *wc=(configwndrec*)lParam;
				SendDlgItemMessage(hwndDlg,IDC_VBRQUALITY,TBM_SETRANGE,0,MAKELONG(0,110));
				SendDlgItemMessage(hwndDlg,IDC_VBRQUALITY,TBM_SETTICFREQ,5,0);
				SendDlgItemMessage(hwndDlg,IDC_VBRQUALITY,TBM_SETPAGESIZE,0,10);
				SendDlgItemMessage(hwndDlg,IDC_VBRQUALITY,TBM_SETPOS,TRUE,(int)(wc->cfg.cfg_vbrquality*100.0f + (wc->cfg.cfg_vbrquality < 0.0 ? - 0.5f : 0.5f)) + 10);
				SendMessage(hwndDlg,WM_USER+667,0,0);
			}
		}

		else if (uMsg == WM_HSCROLL)
		{
			HWND swnd = (HWND) lParam;
			if (swnd == GetDlgItem(hwndDlg, IDC_VBRQUALITY))
			{
				int p=(int)SendDlgItemMessage(hwndDlg,IDC_VBRQUALITY,TBM_GETPOS,0,0)-10;

#if defined (_WIN64)
				configwndrec* wc = (configwndrec*)GetWindowLong(hwndDlg, GWLP_USERDATA);
#else
				configwndrec* wc = (configwndrec*)GetWindowLong(hwndDlg, GWL_USERDATA);
#endif
				if (wc)
				{
					wc->cfg.cfg_vbrquality=(float)p/100.0f;
				}
				SendMessage(hwndDlg,WM_USER+667,0,0);
			}
		}

		else if (uMsg == WM_COMMAND)
		{
			if(LOWORD(wParam) == IDC_URL1)
			{
				SendMessage(winampwnd, WM_WA_IPC, (WPARAM)"http://xiph.org/vorbis/", IPC_OPEN_URL);
			}
			else if(LOWORD(wParam) == IDC_URL2)
			{
				SendMessage(winampwnd, WM_WA_IPC, (WPARAM)"https://ao-yumi.github.io/aotuv_web/index.html", IPC_OPEN_URL);
			}
		}

		else if (uMsg == WM_DESTROY)
		{
#if defined (_WIN64)
			configwndrec* wc = (configwndrec*)SetWindowLong(hwndDlg, GWLP_USERDATA, 0);
#else
			configwndrec* wc = (configwndrec*)SetWindowLong(hwndDlg, GWL_USERDATA, 0);
#endif
			if (wc)
			{
				wc->cfg.cfg_mode=0;
				writeconfig(wc->configfile,&wc->cfg);
				free(wc->configfile);
				free(wc);
			}
		}

		const int controls[] = 
		{
			IDC_VBRQUALITY,
		};
		if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		{
			return TRUE;
		}

		link_handledraw(hwndDlg, uMsg, wParam, lParam);
		return 0;
	}

	HWND __declspec(dllexport) ConfigAudio3(HWND hwndParent, HINSTANCE hinst, unsigned int outt, char *configfile)
	{
		if (outt == mmioFOURCC('O','G','G',' '))
		{
			configwndrec *wr=(configwndrec*)malloc(sizeof(configwndrec));
			if (configfile) wr->configfile=_strdup(configfile);
			else wr->configfile=0;

			readconfig(configfile,&wr->cfg);
			GetLocalisationApiService();
			return WASABI_API_CREATEDIALOGPARAMW(IDD_CONFIG,hwndParent,DlgProc,(LPARAM)wr);
		}
		return NULL;
	}

	void __declspec(dllexport) SetWinampHWND(HWND hwnd)
	{
		winampwnd = hwnd;
		isthemethere = !SendMessage(hwnd,WM_WA_IPC,IPC_ISWINTHEMEPRESENT,IPC_USE_UXTHEME_FUNC);
	}
};