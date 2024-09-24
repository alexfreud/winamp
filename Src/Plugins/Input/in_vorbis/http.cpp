#include "api__in_vorbis.h"
#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"
#include "rf.h"
#include "main.h"
#include "../Winamp/wa_ipc.h"
#include <api/service/waservicefactory.h>
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
extern CfgInt cfg_fix0r,cfg_httpseek2,cfg_proxy_mode,cfg_prebuf1,cfg_prebuf2,cfg_fsave,cfg_http_bsize;

#define CANSEEK 

WORD *wdup(const char * src);//info.c

#define zeromem(x) memset(&x,0,sizeof(x))

class StreamSave
{
  private:
	ogg_sync_state   oy_src;
	ogg_stream_state os_src;
	ogg_stream_state os_dst;
	ogg_page         og_src;
	ogg_page         og_dst;
	ogg_packet       op;
	StringW tmp_fn;
	BOOL is_temp;
	BOOL got_streams,got_delta,use_fix0r;
	ogg_int64_t pcm_delta;
	int packets,serial;
	HANDLE hFile;
  public:

	StreamSave()
	{
		zeromem(oy_src);
		zeromem(os_src);
		zeromem(os_dst);
		zeromem(og_src);
		zeromem(og_dst);
		zeromem(op);
		got_streams=0;
		got_delta=0;
		pcm_delta=0;
		hFile=0;
		packets=0;
		serial=0;
		is_temp=1;

		tmp_fn=cfg_dumpdir;
		if (tmp_fn[tmp_fn.Length()-1]!='\\') tmp_fn.AddChar('\\');

		tmp_fn+=StringPrintfW(L"oggtemp%u.foo",GetTickCount64()&0xFFFF);

		hFile=CreateFileW(tmp_fn,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_HIDDEN,0);
		if (hFile==INVALID_HANDLE_VALUE) hFile=0;
		else
		{
		ogg_sync_init(&oy_src);
		use_fix0r=cfg_fix0r;
		}
	};

	void Write(void * ptr,UINT siz)
	{
		if (!hFile) return;

		void * b=ogg_sync_buffer(&oy_src,siz);
		memcpy(b,ptr,siz);
		ogg_sync_wrote(&oy_src,siz);

		while(ogg_sync_pageout(&oy_src,&og_src)>0)
		{
			if (!got_streams)
			{
				serial=ogg_page_serialno(&og_src);
				ogg_stream_init(&os_src,serial);
				ogg_stream_init(&os_dst,serial);
				got_streams=1;
				packets=0;
				got_delta=0;
			}
			else if (serial!=ogg_page_serialno(&og_src))
			{
				if (got_streams)
				{
					/*while(ogg_stream_flush(&os_dst,&og_dst))
					{
						write_page(dst,&og_dst,&wb);
					}*/

					ogg_stream_clear(&os_src);
					ogg_stream_clear(&os_dst);
				}
				serial=ogg_page_serialno(&og_src);
				ogg_stream_init(&os_src,serial);
				ogg_stream_init(&os_dst,serial);

				packets=0;
				got_delta=0;
			}

			ogg_stream_pagein(&os_src,&og_src);
			while(ogg_stream_packetout(&os_src,&op)>0)
			{
				if (use_fix0r && !got_delta && packets>2 && op.granulepos>=0) //hack to fix saved streams
				{
					got_delta=1;
					if (op.granulepos>4096*(packets-2)) pcm_delta=op.granulepos;
				}
				if (got_delta)
				{
					if (op.granulepos>=pcm_delta) op.granulepos-=pcm_delta;
					else if (op.granulepos>0) op.granulepos=0;
				}
				ogg_stream_packetin(&os_dst,&op);
				packets++;
			}

			while((packets==3 ? ogg_stream_flush(&os_dst,&og_dst) :  ogg_stream_pageout(&os_dst,&og_dst))>0)
			{
				DWORD bw = 0;
				WriteFile(hFile,og_dst.header,og_dst.header_len,&bw,0);
				bw = 0; WriteFile(hFile,og_dst.body,og_dst.body_len,&bw,0);
			}
		}
	}

	void FixName(VorbisFile * vf,const char * streamname)
	{
	    if (!hFile) return;
		CloseHandle(hFile);
		StringW fn(cfg_dumpdir);

		if (fn[fn.Length()-1]!='\\') fn.AddChar('\\');

		UINT n=fn.Length();
		fn+=(wchar_t *)AutoWide(vf->get_meta("TITLE", 0), CP_UTF8);
		UINT m=fn.Length();

		while(n<m)
		{
			char * b="/\\:*?\"<>|";
			while(b && *b)
			{
				if (fn[n]==*b) {fn.SetChar(n,'_');break;}
				b++;
			}
			n++;
		};
		fn.AddStringA(".ogg");
		if (!MoveFileW(tmp_fn,fn))
		{
			DeleteFileW(fn);
			MoveFileW(tmp_fn,fn);
		}
		SetFileAttributesW(fn,FILE_ATTRIBUTE_NORMAL);
		hFile=CreateFileW(fn,GENERIC_WRITE,0,0,OPEN_EXISTING,0,0);

		if (hFile==INVALID_HANDLE_VALUE) {hFile=0;}
		else SetFilePointer(hFile,0,0,FILE_END);
		is_temp=0;
	}

	~StreamSave()
	{
		if (hFile)
		{
			/*if (got_streams)
			{
				while(ogg_stream_flush(&os_dst,&og_dst))
				{
					write_page(dst,&og_dst,&wb);
				}
			}*/

			ogg_stream_clear(&os_src);
			ogg_stream_clear(&os_dst);

			SetFilePointer(hFile,0,0,FILE_CURRENT);
			CloseHandle(hFile);
			if (is_temp) DeleteFileW(tmp_fn);
		}
		ogg_sync_clear(&oy_src);
	}
};

static const char * do_proxy(const char * url)
{
	switch(cfg_proxy_mode)
	{
	default:
		return 0;
	case 1:
		{
			const char * p=strstr(url,"://");
			if (!p) p=url;
			while(p && *p && *p!=':' && *p!='/') p++;
			if (p && *p==':')
			{
				if (atoi(p+1)!=80) return 0;
			}
		}
	case 2:
		{
			char *x = (char *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GET_PROXY_STRING);
			if (x == (char *)1 || !x || !*x)
				return 0;
			return x;
		}
	}
}

class VorbisFile_HTTP : public VorbisFile
{
  protected:
	api_httpreceiver *get;
	UINT bsize;
	uint64_t len;
	UINT pos; 
	UINT seekpos;
	BOOL can_seek;
	StreamSave * saver;
	virtual void Idle();
	virtual int f_seek(__int64 offset,int whence);
	virtual size_t f_read(UINT siz,void * ptr);
	virtual UINT f_tell();
	virtual UINT FileSize() {return len;}
	bool is_live;

  public:
	virtual int GetType() {return TYPE_HTTP;}
	virtual bool IsLive() {return is_live;}
	bool http_init();

	void do_prebuf() {VorbisFile::do_prebuf();fillbuf(bsize * cfg_prebuf1 / 100,0);}

	VorbisFile_HTTP(UINT s, const wchar_t *u,bool is_info, bool hasauth) : VorbisFile(u,is_info), usedauth(hasauth)
	{
		get=0;
		can_seek=0;
		len=pos=seekpos=0;
		bsize=s;
		saver=0;
		m_needs_auth=0;
		lpinfo[0]=0;
		force_lpinfo[0]=0;
		is_live = false;
		memset(dlg_realm, 0, sizeof(dlg_realm));
	}

	~VorbisFile_HTTP()
	{
		if (get)
		{
			waServiceFactory *sf = mod.service->service_getServiceByGuid(httpreceiverGUID);
			if (sf)  
				sf->releaseInterface(get);
			get=0;
		}
		if (saver) delete saver;
	}

	void fillbuf(UINT max,bool shutup);

	size_t _http_read(char* ptr,size_t total);
	int reconnect(UINT ofs);
	virtual void post_init()
	{
		if (saver) saver->FixName(this,get->getheader("ice-name"));
	}

	static BOOL CALLBACK httpDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
	int m_needs_auth;
	char dlg_realm[256];
	char lpinfo[256];
	char force_lpinfo[256];
	bool usedauth;
};

int VorbisFile_HTTP::reconnect(UINT ofs)
{
	//	get.reset_headers();
	get->addheader("User-Agent: WinampOGG/5.24(MPEG stream compatible)");
	get->addheader("Accept:*/*");
	if (ofs>0) get->addheader(StringPrintf("Range: bytes=%u-",ofs));
	get->connect(AutoChar(url));

	Status(WASABI_API_LNGSTRINGW(IDS_CONNECTING));

	int st=get->run();
	if (st<0)
	{
		return 1;
	}
	return 0;
}

void VorbisFile_HTTP::fillbuf(UINT max,bool shutup)
{
	if (len>0 && pos+max>len) max=len-pos;
	while(!Aborting() && !abort_prebuf)	//stop prebuffering if we wanna seek
	{
		if (!shutup)
		{
			Status(StringPrintfW(WASABI_API_LNGSTRINGW(IDS_PREBUFFERING), get->bytes_available()*100/bsize));
		}
		if (get->run()) break;
		if (Aborting() || abort_prebuf || get->bytes_available()>=(int)max) break;
		Sleep(2);
	}
	if (!shutup)
	{
		Status(0);
	}
}

size_t VorbisFile_HTTP::_http_read(char* ptr,size_t total)
{
#ifdef CANSEEK
	if (seekpos!=-1)
	{
		UINT sp=seekpos;
		seekpos=-1;
		if (sp!=pos)
		{
			if (sp>pos && sp<=pos+get->bytes_available())
			{
				get->get_bytes(0,sp-pos);
			}
			else
			{
				if (reconnect(sp))
				{
				return 0;//oh well...
				}
			}
			pos=sp;
		}
	}
#endif
	UINT wr=0;
	while(!Aborting() && wr<total)
	{
		int st=get->run();
		int d=get->get_bytes(ptr,(int)total-wr);
		if (st && !d) break;
		wr+=d;
		ptr+=d;
		pos+=d;
		if ((len>0 && pos>=len) || wr>=total || Aborting()) break;
		if (use_prebuf) fillbuf(bsize * cfg_prebuf2 / 100,0);
		else Sleep(1);
	}
	return wr;
}

void VorbisFile_HTTP::Idle()
{
	get->run();
	Sleep(1);
	get->run();
	Sleep(1);
}

size_t VorbisFile_HTTP::f_read(UINT siz,void* ptr)
{
	if (Aborting()) return 0;//fixme
	int i=(int)_http_read((char*)ptr,siz);
	if (i>0 && saver) saver->Write(ptr,i);
	return i;
}

int VorbisFile_HTTP::f_seek(ogg_int64_t offset,int whence)
{
#ifdef CANSEEK
	if (can_seek)
	{
		switch(whence)
		{
		case FILE_BEGIN:
			seekpos=(int)offset;
			break;
		case FILE_END:
			seekpos=len+(int)offset;
			break;
		case FILE_CURRENT:
			seekpos=pos+(int)offset;
			break;
		}
		if (seekpos>len) seekpos=len;
		return 0;
	}
	else 
#endif
    return -1;
}

UINT VorbisFile_HTTP::f_tell()
{
#ifdef CANSEEK
	if (can_seek)
	{
		if (seekpos!=-1) return seekpos;
		else return pos;
	}
	else
#endif
    return -1;
}

HWND GetDialogBoxParent()
{
	HWND parent = (HWND)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETDIALOGBOXPARENT);
	if (!parent || parent == (HWND)1)
		return mod.hMainWindow;
	return parent;
}

bool VorbisFile_HTTP::http_init()
{
	if (mod.service)
	{
		waServiceFactory *sf = mod.service->service_getServiceByGuid(httpreceiverGUID);
		if (sf)  get = (api_httpreceiver *)sf->getInterface();
	}
	if (!get) return false;
	get->open(API_DNS_AUTODNS, bsize, do_proxy(AutoChar(url)));

	if (reconnect(0))
	{
		return 0;
	}

#ifdef CANSEEK
	//	if (cfg_httpseek)
	{
		//need to get http headers first
		while(!memcmp(get->getallheaders(),"\0\0",2))
		{
			if (get->run()<0 || Aborting())
			{
				int reply = get->getreplycode();
				if ( reply == 401 )
				{
					api_connection *mcon=get->GetConnection();
					if ( mcon && mcon->GetReceiveBytesAvailable())
					{
						char p[1024]="";
						while ( mcon->GetReceiveBytesAvailable() )
						{
							char b[2048]="";
							mcon->ReceiveLine(b,2048);
							if ( *b )
							{
								char *t= strstr(b,"WWW-Authenticate:");
								if ( t && *t )
								{
									char *y = strstr(t,"\"");
									if ( y && *y )
									{
										y++;
										if ( *y )
										{
											char *u = strstr(y,"\"");
											if ( u && *u )
											{
												*u = 0;
												wsprintfA(p,"%s",y);
											}
										}
									}
								}
							}
						}

						if ( *p )  // found our realm
						{
							if (!force_lpinfo[0]) GetPrivateProfileStringA("HTTP-AUTH",p,"",force_lpinfo,sizeof(force_lpinfo),INI_FILE);
							if (!force_lpinfo[0] || lpinfo[0] || usedauth )
							{
								lstrcpynA(dlg_realm,p,sizeof(dlg_realm));
								if (!WASABI_API_DIALOGBOXPARAM(IDD_HTTPAUTH, GetDialogBoxParent(), httpDlgProc, (LPARAM)this))
								{
									force_lpinfo[0]=0;
								}
								else
								{
									WritePrivateProfileStringA("HTTP-AUTH",p,force_lpinfo,INI_FILE);
								}
							}
							Status(WASABI_API_LNGSTRINGW(IDS_AUTH_REQUIRED));
							m_needs_auth=1;
						}
					}
				}
				return 0;
			}
			//hg->get.wait(10);
			Sleep(1);
		}
		len=get->content_length();
		const char* poo=get->getheader("icy-name");
		if (poo) stream_title=poo;
		if (cfg_httpseek2 && len) can_seek=1;
		is_live=(len<=0);
	}
#endif

	//if (hg->len==0 || hg->len==-1) hg->can_seek=0;	
	seekpos=-1;

	if (cfg_fsave && !can_seek)
	{
		saver=new StreamSave;
	}
	return 1;
}

VorbisFile * VorbisFile::Create_HTTP(const char * url,bool is_info)
{
	VorbisFile_HTTP * r=new VorbisFile_HTTP(cfg_http_bsize,AutoWide(url),is_info, false);
	if (r)
	{
		if (!r->http_init())
		{
			int trys=0;
			while ( r && r->m_needs_auth && trys++ < 2)
			{
				const char *p=strstr(url,"://");
				if (p && *p)
				{
					p += 3;
					if (p && *p)
					{
						char lurl[4096] = {0};
						wsprintfA(lurl, "http://%s@%s", r->force_lpinfo, p);
						delete r;
						r = new VorbisFile_HTTP(cfg_http_bsize,AutoWide(lurl),is_info, true);
						if (r && r->http_init())
						{
							return r;
						}
					}
				}
			}
			delete r;
			r=0;
		}
	}
	return r;
}

BOOL CALLBACK VorbisFile_HTTP::httpDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	VorbisFile_HTTP *_this;
	switch (uMsg)
	{
	case WM_INITDIALOG:
#ifdef WIN64
		SetWindowLong(hwndDlg, GWLP_USERDATA, (LONG)lParam);
		_this = (VorbisFile_HTTP*)(GetWindowLong(hwndDlg, GWLP_USERDATA));
#else
		SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
		_this = (VorbisFile_HTTP*)GetWindowLong(hwndDlg, GWL_USERDATA);
#endif
		if (_this->force_lpinfo[0])
			SetDlgItemTextA(hwndDlg,IDC_EDITAUTH,_this->force_lpinfo);
		else SetDlgItemTextA(hwndDlg,IDC_EDITAUTH,_this->lpinfo);
			SetDlgItemTextA(hwndDlg,IDC_REALM,_this->dlg_realm);
		return 1;

	case WM_COMMAND:
#ifdef WIN64
		_this = (VorbisFile_HTTP*)GetWindowLong(hwndDlg, GWLP_USERDATA);
#else
		_this = (VorbisFile_HTTP*)GetWindowLong(hwndDlg, GWL_USERDATA);
#endif
		if (LOWORD(wParam) == IDOKAUTH)
		{
			char *p;
			GetDlgItemTextA(hwndDlg,IDC_EDITAUTH,_this->force_lpinfo,sizeof(_this->force_lpinfo));
			p = strstr(_this->force_lpinfo,"\r");
			if ( p && *p ) *p=0;
			p = strstr(_this->force_lpinfo,"\n");
			if ( p && *p ) *p=0;
			EndDialog(hwndDlg,1);
		}
		else if (LOWORD(wParam) == IDCANCELAUTH)
		{
			EndDialog(hwndDlg,0);
		}
		break;
	}
	return 0;
}