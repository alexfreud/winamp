/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "Main.h"
#if 0
static int (__stdcall *p_recv)(SOCKET s, char FAR* buf, int len, int flags);


static int g_nvck;

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
				str[0]=0; 
				return -1;
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


static DWORD WINAPI _Thread(void *p666)
{
	char *rf_url = (char *)p666;
	HINSTANCE hws = LoadLibrary("wsock32.dll");
	SOCKET sock;
	char *proxy;
	char connect_host[MAX_PATH];
	unsigned short connect_port;
	int success=0;
	int (__stdcall *select)(int nfds,fd_set FAR * readfds,fd_set FAR * writefds,fd_set FAR * exceptfds,const struct timeval FAR * timeout);
	int (__stdcall *WSAGetLastError)(void);
	int (__stdcall *WSACleanup)(void);
	int (__stdcall *WSAStartup)(WORD wVersionRequested,LPWSADATA lpWSAData);
	int (__stdcall *closesocket)(SOCKET s);
	int (__stdcall *send)(SOCKET s,const char FAR *buf,int len,int flags);
	SOCKET (__stdcall *socket)(int af, int type,int protocol);	
	int (__stdcall *connect)( SOCKET s, const struct sockaddr FAR *name, int namelen  );

  unsigned long (__stdcall *inet_addr)(const char FAR *cp );	
	struct hostent FAR * (__stdcall *gethostbyname)(const char FAR *name); 
	int (__stdcall *ioctlsocket)(SOCKET s,long cmd,u_long FAR *argp);
	u_short (__stdcall *htons)(u_short hostshort);

	if (hws)
	{
		WSAGetLastError=(void*)GetProcAddress(hws,"WSAGetLastError");
		WSACleanup=(void*)GetProcAddress(hws,"WSACleanup");
		WSAStartup=(void*)GetProcAddress(hws,"WSAStartup");
		closesocket=(void*)GetProcAddress(hws,"closesocket");
		send=(void*)GetProcAddress(hws,"send");
		p_recv=(void*)GetProcAddress(hws,"recv");
		select=(void*)GetProcAddress(hws,"select");
		connect=(void*)GetProcAddress(hws,"connect");
		socket=(void*)GetProcAddress(hws,"socket");
		inet_addr=(void*)GetProcAddress(hws,"inet_addr");
		gethostbyname=(void*)GetProcAddress(hws,"gethostbyname");
		ioctlsocket=(void*)GetProcAddress(hws,"ioctlsocket");
		htons=(void*)GetProcAddress(hws,"htons");
	}

	if (!hws || !p_recv || !WSACleanup || 
		!WSAStartup || !closesocket || !send || 
		!connect || !socket || !inet_addr || 
		!gethostbyname || !ioctlsocket || !htons || !select || !WSAGetLastError)
	{
		if (hws) FreeLibrary(hws);
		return 0;
	}

	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(1, 1), &wsaData)) 
		{
			FreeLibrary(hws);
			return 0;
		}
	}

	// determine if proxy server used
	proxy=config_proxy;
	while (*proxy == ' ' || *proxy == '\t') proxy++;

	if (*proxy) 
	{
		lstrcpyn(connect_host,proxy,sizeof(connect_host)/sizeof(*connect_host));
	}
	else
	{
		lstrcpyn(connect_host,"www.winamp.com",sizeof(connect_host)/sizeof(*connect_host));
	}
	connect_port=80;
	
	
  if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==INVALID_SOCKET) 
	{
		WSACleanup();
		FreeLibrary(hws);
		return 0;
	}

	{
		int t;
		struct sockaddr_in blah;
		struct hostent *he;
		memset((char *)&blah,0,sizeof(blah));
		blah.sin_family=AF_INET;
		blah.sin_addr.s_addr=inet_addr(connect_host);
		blah.sin_port=htons(connect_port);

		if (blah.sin_addr.s_addr == INADDR_NONE)
		{
			if ((he = gethostbyname(connect_host)) != NULL)
				memcpy((char *)&blah.sin_addr, he->h_addr, he->h_length);
			else if ((blah.sin_addr.s_addr = inet_addr(connect_host))==INADDR_NONE) 
			{
				closesocket(sock);
				WSACleanup();
				FreeLibrary(hws);
				return 0;
			}
		}
		t=connect(sock,(struct sockaddr *)&blah,16);
		if (t==-1) goto cleanup;
	}
	{
		char send_string[2048];
		char request_file[1024];
		StringCchCopy(send_string,2048,rf_url);
		if (*proxy) 
		{
      StringCchPrintf(request_file,1024, "http://www.winamp.com%s",rf_url);
		}
    else StringCchCopy(request_file,1024,rf_url);
		StringCchPrintf(send_string,2048,"GET %s HTTP/1.0\r\n"
							 "User-Agent: Winamp/%s\r\n"
                             "Host: www.winamp.com\r\n"
                             "Accept: */*\r\n\r\n",request_file,app_version);
//		MessageBox(NULL,send_string,"SENDING:",0);
		send(sock,send_string,lstrlen(send_string),0);
	}

	{ // get the standard HTTP 1.0 200 OK
		char buf[1024];
		int x = recv_string(sock,buf,sizeof(buf));
//    MessageBox(NULL,buf,buf,0);
		if (x < 0 || !strstr(buf,"OK")) goto cleanup;
	}

  if (g_nvck&2) stats_clear();
	while (1)
	{
		char buf[1024],*p;
		int x = recv_string(sock,buf,sizeof(buf));
		if (x < 0) goto cleanup;
		if (buf[0] == '\r' || !buf[0]) break;
		
		{
			p=buf;
			while (*p && *p != ':') p++;
			if (*p == ':')
			{
				*p++=0;
				while (*p == ' ' || *p == '\t') p++;
			}
			else p=NULL;
		}
	}

	{
		int is_upd=0;
		char obuf[32768]="";
		char buf[1024];
		int x = recv_string(sock,buf,sizeof(buf));
		if (x < 0 || buf[1] != '.') goto cleanup;
		if (buf[0] > app_version[0]) is_upd=1;
		else if (buf[0] == app_version[0])
    {
      int r2;
	  char oldver[4];
	  int oldlen;
	  oldlen = lstrlen(app_version+2);
	  if ( oldlen == 3 )
	  {
		  oldver[0]=*(app_version+2);
		  oldver[1]=*(app_version+3);
		  oldver[2]=*(app_version+4);
	  }
      if ( oldlen == 2 )
	  {
		  oldver[0]=*(app_version+2);
		  oldver[1]=*(app_version+3);
		  oldver[2]='0';
	  }
	  if ( oldlen == 1 )
	  {
		  oldver[0]=*(app_version+2);
		  oldver[1]='0';
		  oldver[2]='0';
	  }
	  oldver[3]=0;
      r2=atoi(oldver);
      if (atoi(buf+2)>r2) is_upd=1;
    }

	if (!is_upd) 
    {
			if ((g_nvck&1)&&config_newverchk) config_newverchk=getDay();
           goto cleanup;
    }
		while (lstrlen(obuf)<32768-sizeof(buf)*2)
		{
			x=recv_string(sock,buf,sizeof(buf));
			if (x < 0) break;
			StringCchCat(obuf,32768,buf);
			StringCchCat(obuf,32768,"\r\n");
		}
    if (g_nvck&1)
    {
		  if (MessageBox(NULL,obuf,getString(IDS_WINAMP_UPDATE_MSG,NULL,0),MB_YESNO) == IDYES)
		  {
			  myOpenURL(NULL, L"http://www.winamp.com/getwinamp/");
		  }
		  else
		  {
			  if ((g_nvck&1)&&config_newverchk) config_newverchk=getDay();
		  }
    }
	}
cleanup:
	closesocket(sock);
	WSACleanup();
	FreeLibrary(hws);
	return 0;
}

void newversioncheck(void)
{
	DWORD id;
  static char s[512];
  int stats[NUM_STATS]={0,};
  int x;
  g_nvck=(config_newverchk2?2:0)|(config_newverchk?1:0);
  if (g_nvck&3)
  {
    StringCchPrintf(s,512,"/update/latest-version.jhtml?v=%s",app_version);
    if (g_nvck&2)
    {
      stats_get(stats);
      StringCchCat(s,512,"&ID=");
      stats_getuidstr(s+lstrlen(s));
      for (x = 0; x < NUM_STATS; x ++)
      {
        StringCchPrintf(s+lstrlen(s),512-lstrlen(s),"&st%d=%d",x+1,stats[x]);
      }
      StringCchPrintf(s+lstrlen(s),512-lstrlen(s),"&regv=%d",g_regver);
    }

  //  MessageBox(NULL,s,"metric",0);
	  CloseHandle(CreateThread(NULL,0,_Thread,(LPVOID)s,0,&id));
  }
}

#endif
