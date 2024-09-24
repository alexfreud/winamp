/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "Main.h"
#include <cstdint>
#if 1
#include <sys/types.h>
#include <winsock.h>
#include "../nu/threadname.h"
#include "Wa_dlg.h"
#include "../nu/AutoWide.h"

static void createDirForFileW(wchar_t *str)
{
	wchar_t *p = str;
	if ((p[0] ==L'\\' || p[0] ==L'/') && (p[1] ==L'\\' || p[1] ==L'/'))
	{
		p += 2;
		while (p && *p && *p !=L'\\' && *p !=L'/') p++;
		if (!*p) return ;
		p++;
		while (p && *p && *p !=L'\\' && *p !=L'/') p++;
	}
	else
	{
		while (p && *p && *p !=L'\\' && *p !=L'/') p++;
	}

	while (p && *p)
	{
		while (p && *p !=L'\\' && *p !=L'/' && *p) p = CharNextW(p);
		if (*p)
		{
			wchar_t lp = *p;
			*p = 0;
			CreateDirectoryW(str, NULL);
			*p++ = lp;
		}
	}
}

static const wchar_t *rf_file, *rf_dlgtitle;
static const char *rf_url;
static int rf_in;
static HWND rf_dlgWnd, rf_statusWnd;
static volatile int rf_rv=-1;
static volatile int rf_abort;
static int g_rv;

static int _rftabort(int r, char *s)
{
	if (s) SetWindowTextA(rf_statusWnd,s);
	if (rf_dlgWnd) SendMessageW(rf_dlgWnd,WM_USER+1,r,0);
	else rf_rv=r;
	return r;
}

#define rfta(s) return _rftabort(!success,s)
#define sets(s) SetWindowTextA(rf_statusWnd,s)

typedef int (__stdcall *waP_RECV)(SOCKET s, char FAR* buf, int len, int flags);
waP_RECV		p_recv;

static void encodeMimeStr(char *in, char *out)
{
	char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int shift = 0;
	int accum = 0;

	while (in && *in)
	{
		if (*in)
		{
			accum <<= 8;
			shift += 8;
			accum |= *in++;
		}
		while ( shift >= 6 )
		{
			shift -= 6;
			*out++ = alphabet[(accum >> shift) & 0x3F];
		}
	}
	if (shift == 4)
	{
		*out++ = alphabet[(accum & 0xF)<<2];
		*out++='=';  
	}
	else if (shift == 2)
	{
		*out++ = alphabet[(accum & 0x3)<<4];
		*out++='=';  
		*out++='=';  
	}

	*out++=0;
}

static int recv_string(SOCKET s, char *str, int maxlen)
{
	int p=0;
	do
	{
		int t=0;
		while (t!=1)
		{
			t=p_recv(s,str+p,1,0);
			if (t != 1)
			{
				if (rf_abort || !t) { str[0]=0; return -1; }
				Sleep(100);
			}
			if (str[p] == '\r') t=0;
		}
	} while (str[p] != '\n' && ++p < maxlen-1);
	str[p--]=0;
	while (str[p] == '\n' && p > 0)
	{
		str[p--]=0;
	}
	if (p < 0) p = 0;
	return p;
}

static int g_in_resolve;

static DWORD WINAPI rf_ThreadProc(LPVOID p666)
{
	char locbuf[1024]={0};
	int redirect=0;
	HINSTANCE hws = LoadLibraryA("wsock32.dll");
	int total_bytes;
	uint64_t content_length;
	SOCKET sock;
	char proxytmp[256]={0};
	char *proxy;
	char connect_host[MAX_PATH]={0};
	unsigned short connect_port;
	int success=0;
	
	typedef int (__stdcall *waSELECT)(int nfds,fd_set FAR * readfds,fd_set FAR * writefds,fd_set FAR * exceptfds,const struct timeval FAR * timeout);
	typedef int (__stdcall *waWSAGETLASTERROR)(void);
	typedef int (__stdcall *waWSACLEANUP)(void);
	typedef int (__stdcall *waWSASTARTUP)(WORD wVersionRequested,LPWSADATA lpWSAData);
	typedef int (__stdcall *waCLOSESOCKET)(SOCKET s);
	typedef int (__stdcall *waSEND)(SOCKET s,const char FAR *buf,int len,int flags);
	typedef SOCKET (__stdcall *waSOCKET)(int af, int type,int protocol);	
	typedef int (__stdcall *waCONNECT)( SOCKET s, const struct sockaddr FAR *name, int namelen  );
	typedef unsigned long (__stdcall *waINET_ADDR)(const char FAR *cp );	
	typedef struct hostent FAR * (__stdcall *waGETHOSTBYNAME)(const char FAR *name); 
	typedef int (__stdcall *waIOCTLSOCKET)(SOCKET s,long cmd,u_long FAR *argp);
	typedef u_short (__stdcall *waHTONS)(u_short hostshort);

	waSELECT			select;
	waWSAGETLASTERROR	WSAGetLastError;
	waWSACLEANUP		WSACleanup;
	waWSASTARTUP		WSAStartup;
	waCLOSESOCKET		closesocket;
	waSEND				send;
	waSOCKET			socket;
	waCONNECT			connect;
	waINET_ADDR			inet_addr;
	waGETHOSTBYNAME		gethostbyname;
	waIOCTLSOCKET		ioctlsocket;
	waHTONS				htons;

	char  buf[ 4096 ]      = { 0 };
	char  errorStr[ 1024 ] = { 0 };
	char *p;
	char  url_buf[ 1024 ]  = { 0 };
	char *url_lp           = NULL;
	char *url_host         = NULL;
	int   url_port         = 80;
	char *url_url          = NULL;
	char  authstr[ 1024 ]  = { 0 };
	char  proxy_lp[ 1024 ] = { 0 };
	const char *t;

	SetThreadName((DWORD)-1, "HTTP Retrieve File");
	if ( hws )
	{
		WSAGetLastError = (waWSAGETLASTERROR)GetProcAddress( hws, "WSAGetLastError" );
		WSACleanup      = (waWSACLEANUP)GetProcAddress( hws, "WSACleanup" );
		WSAStartup      = (waWSASTARTUP)GetProcAddress( hws, "WSAStartup" );
		closesocket     = (waCLOSESOCKET)GetProcAddress( hws, "closesocket" );
		send            = (waSEND)GetProcAddress( hws, "send" );
		p_recv          = (waP_RECV)GetProcAddress( hws, "recv" );
		select          = (waSELECT)GetProcAddress( hws, "select" );
		connect         = (waCONNECT)GetProcAddress( hws, "connect" );
		socket          = (waSOCKET)GetProcAddress( hws, "socket" );
		inet_addr       = (waINET_ADDR)GetProcAddress( hws, "inet_addr" );
		gethostbyname   = (waGETHOSTBYNAME)GetProcAddress( hws, "gethostbyname" );
		ioctlsocket     = (waIOCTLSOCKET)GetProcAddress( hws, "ioctlsocket" );
		htons           = (waHTONS)GetProcAddress( hws, "htons" );
	}

	if (!hws || !p_recv || !WSACleanup || 
		!WSAStartup || !closesocket || !send || 
		!connect || !socket || !inet_addr || 
		!gethostbyname || !ioctlsocket || !htons || !select || !WSAGetLastError)
	{
		if (hws)
			FreeLibrary(hws);

		rfta(getString(IDS_HTTP_LOAD_ERROR,errorStr,1024));
	}

	sets(getString(IDS_HTTP_INIT_SOCKET,errorStr,1024));
	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(1, 1), &wsaData)) 
		{
			FreeLibrary(hws);
			rfta(getString(IDS_HTTP_INIT_SOCKET_ERROR,errorStr,1024));
		}
	}

_redirect_goto:
	total_bytes=0;
	content_length=0;

	// parse out l/p, host, port, and url from loc
	authstr[0]=0;
	url_lp=NULL;
	url_host=NULL;
	url_port=80;
	url_url=NULL;

	t=  strstr(rf_url,"://");
	if (t) 
	{
		StringCchCopyA(url_buf,1024,t+3);
	}
	else 
	{
		StringCchCopyA(url_buf,1024,rf_url);
	}

	p=url_buf;
	while (p && *p != '/' && *p) p++; 
	if (p && *p) *p++=0;
	url_url=p;

	p=url_buf;
	while (p && *p) p++;
	while (p>=url_buf && *p != '@') p--;
	if (p>=url_buf) 
	{
	    *p++=0;
		url_host=p;
		url_lp=url_buf;
		if (lstrlenA(url_lp)>0)
		{
			StringCchCopyA(authstr,1024,"Authorization: Basic ");
			encodeMimeStr(url_lp,authstr+lstrlenA(authstr));
			StringCchCatA(authstr,1024,"\r\n");
		}
	}
	else url_host=url_buf;

	p=url_host;
	while (p && *p != ':' && *p) p++;
	if (p && *p)
	{
		*p++=0;
		if (*p) url_port=atoi(p);
	}

	// determine if proxy server used
	{
		StringCchCopyA(proxytmp,256,config_proxy);
		proxy=proxytmp;
		while (proxy && (*proxy == ' ' || *proxy == '\t')) proxy++;

		if (url_port != 80 && GetPrivateProfileIntW(L"Winamp",L"proxyonly80",0,INI_FILE))
			proxy="";
	}

	if (*proxy) 
	{
		p=proxy;
		while (p && *p && *p != '@') p++;
		if (p && *p)
		{
			*p++=0;
			StringCchCopyA(proxy_lp,1024,"Proxy-Authorization: Basic ");
			encodeMimeStr(proxy,proxy_lp+lstrlenA(proxy_lp));
			StringCchCatA(proxy_lp,1024,"\r\n");
			proxy=p;
		}

		lstrcpynA(connect_host,proxy,sizeof(connect_host)/sizeof(*connect_host));
		p=connect_host;
		while (p && *p && *p != ':') p++;
		if (p && *p)
		{
			*p++=0;
			if (*p) connect_port=(unsigned short)atoi(p);
			else connect_port=80;
		}
	}
	else
	{
		lstrcpynA(connect_host,url_host,sizeof(connect_host)/sizeof(*connect_host));
		connect_port=(unsigned short)url_port;
	}
	
	sets(getString(IDS_HTTP_SOCKET_CREATE,errorStr,1024));
    if (rf_abort || (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==INVALID_SOCKET) 
	{
		WSACleanup();
		FreeLibrary(hws);
		rfta(getString(IDS_HTTP_SOCKET_ERROR,errorStr,1024));
	}

	{
		int t;
		struct sockaddr_in blah;
		memset((char *)&blah,0,sizeof(blah));
		blah.sin_family=AF_INET;
		blah.sin_addr.s_addr=inet_addr(connect_host);
		blah.sin_port=htons(connect_port);

		if ( blah.sin_addr.s_addr == INADDR_NONE )
		{
			struct hostent *he;
			g_in_resolve = 1;
			sets( *proxy ? getString( IDS_HTTP_RESOLVE_PROXY, errorStr, 1024 ) : getString( IDS_HTTP_RESOLVE_HOST, errorStr, 1024 ) );
			if ( ( he = gethostbyname( connect_host ) ) != NULL )
				memcpy( (char *)&blah.sin_addr, he->h_addr, he->h_length );
			else if ( ( blah.sin_addr.s_addr = inet_addr( connect_host ) ) == INADDR_NONE )
			{
				g_in_resolve = 0;
				closesocket( sock );
				WSACleanup();
				FreeLibrary( hws );
				rfta( *proxy ? getString( IDS_HTTP_RESOLVE_PROXY_ERROR, errorStr, 1024 ) : getString( IDS_HTTP_RESOLVE_HOST_ERROR, errorStr, 1024 ) );
			}

			g_in_resolve = 0;
		}

		sets(*proxy?getString(IDS_HTTP_CONNECT_PROXY,errorStr,1024):getString(IDS_HTTP_CONNECT_HOST,errorStr,1024));
		{
			unsigned long arg=1;
			ioctlsocket(sock,FIONBIO,&arg);
		}

		t=connect(sock,(struct sockaddr *)&blah,16);
		if (t == -1 && WSAGetLastError()==WSAEWOULDBLOCK)
		{
			int a=0;
			while (!rf_abort && t==-1)
			{
				TIMEVAL to={0,250*1000};
				fd_set f;
				FD_ZERO(&f);
				FD_SET(sock,&f);
				switch (select(0,NULL,&f,NULL,&to))
				{
					case 1: t=0; break;
					case 0: if (a++ > 40) rf_abort =1; break;
					case -1: rf_abort =1; break;
				}
			}
		}
		if (rf_abort || t==-1) 
		{
			closesocket(sock);
			WSACleanup();
			FreeLibrary(hws);
			rfta(*proxy?getString(IDS_HTTP_CONNECT_PROXY_ERROR,errorStr,1024):getString(IDS_HTTP_CONNECT_HOST_ERROR,errorStr,1024));
		}
	}

	sets(getString(IDS_HTTP_SEND_REQUEST,errorStr,1024));
	{
		if ( *proxy )
			StringCchPrintfA( buf, 4096, "GET http://%s:%d/%s", url_host, url_port, url_url );
		else
			StringCchPrintfA( buf, 4096, "GET /%s", url_url );

		if (url_port != 80)
			StringCchPrintfA( buf + lstrlenA( buf ), 4096 - lstrlenA( buf ), " HTTP/1.0\r\nHost: %s:%d\r\n", url_host, url_port );
		else
			StringCchPrintfA( buf + lstrlenA( buf ), 4096 - lstrlenA( buf ), " HTTP/1.0\r\nHost: %s\r\n", url_host );

		StringCchPrintfA( buf + lstrlenA( buf ), 4096 - lstrlenA( buf ),
						  "User-Agent: Winamp/%s%s\r\n"
						  "%s%s"
						  "Accept: */*\r\n\r\n",
						  app_version,
						  ( redirect == 2 ? " (Mozilla)" : "" ),
						  proxy_lp, authstr );

		//MessageBox(NULL,buf,"SENDING:",0);
		{
			unsigned long arg = 0;
			ioctlsocket( sock, FIONBIO, &arg );
		}

		send(sock,buf,lstrlenA(buf),0);

		{
			unsigned long arg = 1;
			ioctlsocket( sock, FIONBIO, &arg );
		}
	}

	sets( getString( IDS_HTTP_READ_REQUEST, errorStr, 1024 ) );

	{ // get the standard HTTP 1.0 200 OK
		char buf[1024] = {0};
		int x = recv_string(sock,buf,sizeof(buf));
		//MessageBox(0, buf, "RECEIVING:", 0);
		if (x < 0 || rf_abort)
		{
			closesocket(sock);
			WSACleanup();
			FreeLibrary(hws);
			rfta(getString(IDS_HTTP_CONNECTION_LOST,errorStr,1024));
		}

		if (strstr(buf," 302") || strstr(buf,"301"))
		{
			redirect=1;
		}
		else
		{
			// this is a very specific handling to allow for /listen.m3u with v1.x DNAS to work correctly
			// as we need alter the user-agent so it will provide us with the needed response (a DNAS bug)
			if ((redirect != 2) && strstr(buf,"ICY 404 Resource Not Found") && strstr(url_url,"listen.m3u")) {
				redirect=2;
				closesocket(sock);
				goto _redirect_goto;
			}
			else 
			{
				redirect=0;
			}
		}

		if (!strstr(buf,"OK") && !redirect)
		{
			StringCchCatA(buf,1024,getString(IDS_HTTP_CONNECTION_CLOSED,errorStr,1024));
			closesocket(sock);
			WSACleanup();
			FreeLibrary(hws);
			rfta(buf);
		}

		sets(buf);
	}

	while (1)
	{
		char buf[1024] = {0}, *p;
		int x = recv_string(sock,buf,sizeof(buf));
		if (x < 0 || rf_abort)
		{
			closesocket(sock);
			WSACleanup();
			FreeLibrary(hws);
			rfta(getString(IDS_HTTP_CONNECTION_LOST,errorStr,1024));
		}

		if (buf[0] == '\r' || !buf[0])
			break;

		{
			p=buf;
			while (p && *p && *p != ':') p++;
			if (p && *p == ':')
			{
				*p++=0;
				while (p && (*p == ' ' || *p == '\t'))
					p++;
			}
			else
				p=NULL;
		}

		if (!lstrcmpiA(buf,"Content-Length") && (*p >= '0' && *p <= '9'))
		{
			content_length=0;
			while (p && *p >= '0' && *p <= '9') 
			{
				content_length *= 10;
				content_length += *p++-'0';
			}
		}

		if (!lstrcmpiA(buf,"Location") && redirect)
		{
			StringCchCopyA(locbuf,1024,p);
			rf_url=locbuf;
			closesocket(sock);

			//blah
			goto _redirect_goto;
		}
	}

	{
		createDirForFileW((wchar_t *)rf_file);
		HANDLE h = CreateFileW(rf_file,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
		if (h == INVALID_HANDLE_VALUE)
		{
			closesocket(sock);
			WSACleanup();
			FreeLibrary(hws);
			rfta(getString(IDS_HTTP_ERROR_OPEN_FILE,errorStr,1024));
		}

		{
			unsigned int start_time=GetTickCount();
			char buf[4096] = {0};
			while (1)
			{
				int x=p_recv(sock,buf,sizeof(buf),0);
				if (x == 0 || rf_abort)
					break;
				else if (x < 0)
				{
					if (WSAGetLastError()!=WSAEWOULDBLOCK) break;
					Sleep(50);
				}
				else // x > 0
				{
					DWORD t = 0;
					int lb=total_bytes;
					WriteFile(h,buf,x,&t,NULL);

					total_bytes += x;
					if ( lb / 16384 != total_bytes / 16384 )
					{
						int bps;
						int t = ( GetTickCount() - start_time );
						if ( t < 1000 ) t = 1000;
						bps = total_bytes / ( ( t + 999 ) / 1000 );

						if ( content_length )
							StringCchPrintfA( buf, 4096, getString( IDS_HTTP_RETRIEVE_FILE_LENGTH, errorStr, 1024 ), ( total_bytes * 100 ) / content_length, total_bytes, content_length, bps / 1000, ( bps / 10 ) % 100 );
						else
							StringCchPrintfA( buf, 4096, getString( IDS_HTTP_RETRIEVE_FILE, errorStr, 1024 ), total_bytes, bps / 1000, ( bps / 10 ) % 100 );

						sets( buf );
					}
				}
			}
		}
		CloseHandle(h);
	}

	if (!content_length || total_bytes == content_length)
		success=1;
	else
		sets(getString(IDS_HTTP_FILE_INCOMPLETE,errorStr,1024));

	closesocket(sock);
	WSACleanup();
	FreeLibrary(hws);
	rfta(success?getString(IDS_HTTP_SUCCESS,errorStr,1024):NULL);
}

#undef rfta
#undef sets

static BOOL CALLBACK rf_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static HANDLE hThread;
	static int r;

	if (WADlg_initted())
	{
		INT_PTR a = WADlg_handleDialogMsgs(hwndDlg, uMsg, wParam, lParam);
		if (a)
			return a;
	}

	switch (uMsg)
	{
		case WM_USER+1:
			if (hThread != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hThread);
				hThread=INVALID_HANDLE_VALUE;
			}

			if (!wParam) 
			{
				g_rv=0;
				PostMessageW(hwndDlg,WM_USER,0,0); // make it go quick
			}
			else 
			{
				SetDlgItemTextA(hwndDlg,IDCANCEL,getString(IDS_HTTP_CLOSE,NULL,0));
				r=5;
				SetTimer(hwndDlg,123,1000,NULL);
			}

			return 0;
		case WM_TIMER:
			if (wParam == 123)
			{
				if ( r == 0 )
				{
					KillTimer( hwndDlg, wParam );
					g_rv = 1;
				}
				else
				{
					char s[ 32 ] = { 0 };
					StringCchPrintfA( s, 32, getString( IDS_CLOSE_COUNTDOWN, NULL, 0 ), r-- );
					SetDlgItemTextA( hwndDlg, IDCANCEL, s );
				}
			}
			return 0;
		case WM_ERASEBKGND:
			if (WADlg_initted())
				return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT
			break;
		case WM_PAINT:
			{
				if (WADlg_initted())
				{
					int tab[] = { IDC_STATUS | DCW_SUNKENBORDER};
					WADlg_DrawChildWindowBorders(hwndDlg, tab, sizeof(tab) / sizeof(tab[0]));
					return 0;
				}
			}
			break;
		case WM_INITDIALOG:
		{
			DWORD id;
			char errorStr[ 1024 ] = { 0 };

			if ( WADlg_initted() )
				SetWindowLong( GetDlgItem( hwndDlg, IDCANCEL ), GWL_STYLE, GetWindowLongW( GetDlgItem( hwndDlg, IDCANCEL ), GWL_STYLE ) | BS_OWNERDRAW );

			rf_dlgWnd = hwndDlg;
			rf_statusWnd = GetDlgItem( hwndDlg, IDC_STATUS );
			SetWindowTextW( hwndDlg, rf_dlgtitle );

			if ( strstr( rf_url, "client.winamp.com/update" ) )
				SetDlgItemTextA( hwndDlg, IDC_URL, getString( IDS_HTTP_WINAMP_UPDATE_SITE, errorStr, 1024 ) );
			else
				SetDlgItemTextA( hwndDlg, IDC_URL, rf_url );

			SetDlgItemTextA( hwndDlg, IDC_STATUS, getString( IDS_HTTP_INIT, errorStr, 1024 ) );
			rf_abort = 0;
			hThread = CreateThread( NULL, 0, rf_ThreadProc, 0, 0, &id );
		}
			return FALSE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
					if (hThread!=INVALID_HANDLE_VALUE)
					{
						char errorStr[1024] = {0};
						rf_abort=1;
#if 0
						if (g_in_resolve) g_in_resolve++;
						if (0 && g_in_resolve==3)  // lame terminatethread shouldnt be used
						{
							TerminateThread(hThread,0);
							CloseHandle(hThread);
							hThread=INVALID_HANDLE_VALUE;
							g_rv=1;
						}
						else 
#endif
							SetDlgItemTextA(hwndDlg,IDCANCEL,getString(IDS_HTTP_ABORT,errorStr,1024));
					}
					else
					{
						g_rv=1;
					}

					return 0;
			}

			return 0;
	}

	return 0;
}

int httpRetrieveFileW(HWND hwnd, const char *url, const wchar_t *file, const wchar_t *dlgtitle)
{
	int i;
	int activated=0;
	RECT r;
	HWND dlgWnd;
	if (rf_in) return 1;
	rf_in=1;
	g_rv=-1;
	rf_url=url;
	rf_file=file;
	rf_dlgtitle=dlgtitle;
	g_in_resolve=0;
	if (!hwnd) hwnd=hMainWindow;

	if (hwnd == hMainWindow && g_dialog_box_parent)
		hwnd=g_dialog_box_parent;

	{
		if (_strnicmp(url,"http://",7))
		{
//			MessageBox(hwnd,getString(IDS_ONLYHTTP,NULL,0),"Winamp",MB_OK|MB_ICONSTOP);
			rf_in=0;
			return 1;
		}
	}

	GetWindowRect(hwnd,&r);
	dlgWnd=LPCreateDialogW(IDD_HTTPGET, hwnd, (WNDPROC)rf_DlgProc);
	if (r.bottom > GetSystemMetrics(SM_CXSCREEN)/2 && r.bottom-r.top < 100)
	{
		RECT r2;
		GetWindowRect(dlgWnd,&r2);
		r.top=r.bottom-(r2.bottom-r2.top);
	}

	SetWindowPos(dlgWnd,NULL,r.left,r.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

	if (GetForegroundWindow()==hwnd)
	{
		activated=1;
		ShowWindow(dlgWnd,SW_SHOW);
	}
	else
		ShowWindow(dlgWnd,SW_SHOWNA);

	if (hwnd == hMainWindow) 
	{
		if (hMainWindow) EnableWindow(hMainWindow,0);
		if (hPLWindow) EnableWindow(hPLWindow,0);
		if (hEQWindow) EnableWindow(hEQWindow,0);
		//if (hMBWindow) EnableWindow(hMBWindow,0);
	}
	else 
		EnableWindow(hwnd,0);

	while (1)
	{	
		MSG msg;
		if (g_rv != -1) break;
		GetMessage(&msg,NULL,0,0);
		DispatchMessage(&msg);
	}

	if ( activated && GetForegroundWindow() == dlgWnd )
	{
	}
	else
		activated = 0;

	if ( hwnd == hMainWindow )
	{
		if ( hMainWindow )
			EnableWindow( hMainWindow, 1 );

		if ( hPLWindow )
			EnableWindow( hPLWindow, 1 );

		if ( hEQWindow )
			EnableWindow( hEQWindow, 1 );
		//if (hMBWindow) EnableWindow(hMBWindow,1);
	}
	else
		EnableWindow( hwnd, 1 );

	DestroyWindow(dlgWnd);
	if (activated)
		SetForegroundWindow(hwnd);

	i = g_rv;
	rf_in=0;

	return i;
}

int httpRetrieveFile(HWND hwnd, const char *url, char *file, char *dlgtitle)
{
	return httpRetrieveFileW(hwnd, url, AutoWide(file), AutoWide(dlgtitle));
}

#endif