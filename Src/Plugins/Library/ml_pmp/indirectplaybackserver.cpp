#include "main.h"

#include "api/service/waservicefactory.h"
#include "bfc/dispatch.h"

#include "../Components/wac_network/wac_network_web_server_api.h"
#include "../Components/wac_network/wac_network_onconncb_api.h"
#include "../Components/wac_network/wac_network_connection_api.h"
#include "../Components/wac_network/wac_network_http_server_api.h"
#include "../Components/wac_network/wac_network_page_generator_api.h"

#include "api__ml_pmp.h"

#include "DeviceView.h"

#include <wchar.h>

#include "metadata_utils.h"

#include "nu/AutoWide.h"
#include "nu/AutoChar.h"

#include <strsafe.h>

void url_decode(char *in, char *out, int maxlen)
{
	while (in && *in && maxlen>1)
	{
		if (*in == '+') 
		{
			in++;
			*out++=' ';
		}
		else if (*in == '%' && in[1] != '%' && in[1])
		{
			int a=0;
			int b=0;
			for ( b = 0; b < 2; b ++)
			{
				int r=in[1+b];
				if (r>='0'&&r<='9') r-='0';
				else if (r>='a'&&r<='z') r-='a'-10;
				else if (r>='A'&&r<='Z') r-='A'-10;
				else break;
				a*=16;
				a+=r;
			}
			if (b < 2) *out++=*in++;
			else { *out++=a; in += 3;}
		}
		else *out++=*in++;
		maxlen--;
	}
	*out=0;
}

static void callbackThunk(void * callBackContext, wchar_t * status) {
}

typedef struct {
	int status;
	wchar_t * device; wchar_t * artist; wchar_t * album; wchar_t * title;
	DeviceView * d; songid_t s;
} FindTrackStruct;

static VOID CALLBACK APC_FindTrack(ULONG_PTR dwParam) {
	FindTrackStruct * f = (FindTrackStruct *)dwParam;
	for(int i=0; i<devices.GetSize(); i++) {
		f->d = (DeviceView*)devices.Get(i);
		wchar_t buf[2048] = {0}; int len = 2047;
		f->d->dev->getPlaylistName(0,buf,128);
		if(wcscmp(buf,f->device)) continue;
		int l = f->d->dev->getPlaylistLength(0);
		for(int j=0; j<l; j++) {
			f->s = f->d->dev->getPlaylistTrack(0,j);
			f->d->dev->getTrackArtist(f->s,buf,len);
			if(lstrcmpi(buf,f->artist)) continue;
			f->d->dev->getTrackAlbum(f->s,buf,len);
			if(lstrcmpi(buf,f->album)) continue;
			f->d->dev->getTrackTitle(f->s,buf,len);
			if(lstrcmpi(buf,f->title)) continue;
			f->status = 2;
			return;
		}
	}
	f->status = 1;
}

bool findTrack(Device **dev, songid_t *song,wchar_t * device,wchar_t * artist,wchar_t * album,wchar_t * title) {
	FindTrackStruct f = {0,device,artist,album,title,NULL,0};
	SynchronousProcedureCall(APC_FindTrack,(ULONG_PTR)&f);
	*dev = f.d->dev;
	*song = f.s;
	return f.status == 2;
}

bool copyFileFromDevice(Device * dev, songid_t song,wchar_t * fn) {
	int k=0;
	if(dev->copyToHardDriveSupported()) {
	    if(!dev->copyToHardDrive(song,fn,NULL,callbackThunk,&k))
			return true;
	}
	return false;
}

class FilePageGenerator : public api_pagegenerator // public IPageGenerator
{
public:
	virtual ~FilePageGenerator();
    FilePageGenerator(wchar_t *fn, api_httpserv *serv);
    size_t GetData(char *buf, int size); // return 0 when done
    int is_error() { return !m_fp; }

private:
    FILE *m_fp;
    int m_file_pos;
    int m_file_len;
    wchar_t *m_fn;
protected:
	RECVS_DISPATCH;
};

#define CBCLASS FilePageGenerator
START_DISPATCH;
CB(API_PAGEGENERATOR_GETDATA, GetData);
END_DISPATCH;
#undef CBCLASS

FilePageGenerator::FilePageGenerator(wchar_t *fn, api_httpserv *serv)
{
	m_fn = _wcsdup(fn);
	m_file_pos=m_file_len=0;
	m_fp=_wfopen(fn,L"rb");
	if (m_fp)
	{
		int resume_end=0;
		int resume_pos=0;
		char *range = serv->getheader("Range");
		if (range)
		{
			if (!_strnicmp(range,"bytes=",6))
			{
				range+=6;
				char *t=range;
				while (t && *t && (*t < '0' || *t  > '9')) t++;
				while (t && *t && *t >= '0' && *t <= '9') 
				{
					resume_pos*=10;
					resume_pos+=*t-'0';
					t++;
				}
				if (*t != '-') resume_pos=0;
				else if (t[1])
				{
					resume_end=atoi(t+1);
				}
			}
		}

		fseek(m_fp,0,SEEK_END);
		m_file_len=ftell(m_fp);
		char buf[512] = {0};
		StringCchPrintfA(buf, ARRAYSIZE(buf), "Content-Length: %d",m_file_len);
		serv->set_reply_header(buf);

		int m_file_len_orig=m_file_len;

		if (resume_end && resume_end < m_file_len) m_file_len=resume_end;
		if (resume_pos > 0 && resume_pos < m_file_len) m_file_pos = resume_pos;
		fseek(m_fp,m_file_pos,SEEK_SET);

		StringCchPrintfA(buf, ARRAYSIZE(buf), "Content-Range: bytes=%d-%d/%d",resume_pos,resume_end,m_file_len_orig);
		serv->set_reply_header(buf);
		if (resume_pos != 0) serv->set_reply_string("HTTP/1.1 206 Partial Content");
	}
}

FilePageGenerator::~FilePageGenerator()
{
	if (m_fp) fclose(m_fp);
	if (m_fn) { _wunlink(m_fn); free(m_fn); }
}

size_t FilePageGenerator::GetData(char *buf, int size) // return 0 when done
{
	if (!m_fp) return 0;
	if (m_file_pos+size > m_file_len) size=m_file_len - m_file_pos;
	int l=fread(buf,1,size,m_fp);
	m_file_pos+=l;
	return l;
}

class onConnCB : public api_onconncb { 
public: 
	api_pagegenerator* onConnection(api_httpserv *serv, int port) 
	{
		api_connection *c = serv->get_con();
		if(c && c->GetRemoteAddress() != 0x0100007f) return 0; // if it's not from localhost, disregard.
		serv->set_reply_header("Server:ml_pmp/1.0");
		if (!strcmp(serv->get_request_file(),"/"))
		{
			// find temporary filename
			wchar_t fn[MAX_PATH]={0};
			wchar_t dir[MAX_PATH]={0};
			GetTempPath(MAX_PATH,dir);
			GetTempFileName(dir,L"pmp",0,fn);
			_wunlink(fn);
			{wchar_t * ext = wcsrchr(fn,L'.'); if(ext) *ext=0;}
			// decode the parameters needed to find the track from the URL
			char device[128]={0};
			char artist[2048]={0};
			char album[2048]={0};
			char title[2048]={0};
			char * p;
			p = serv->get_request_parm("d");
			if(p) url_decode(p,device,128);
			p = serv->get_request_parm("a");
			if(p) url_decode(p,artist,2048);
			p = serv->get_request_parm("l");
			if(p) url_decode(p,album,2048);
			p = serv->get_request_parm("t");
			if(p) url_decode(p,title,2048);
			char * sc = strrchr(device,';'); 
			char * ext = "mp3";
			if(sc) { *sc=0; ext = sc+2; }
			// find the track based on this info
			Device * dev;
			songid_t song;
			if(findTrack(&dev,&song,AutoWide(device,CP_UTF8),AutoWide(artist,CP_UTF8),AutoWide(album,CP_UTF8),AutoWide(title,CP_UTF8))) {
				if(dev->copyToHardDriveSupported()) {
					// track found, can be copied back. Lets reply to the user
					serv->set_reply_string("HTTP/1.1 200 OK");
					char buf[150]="";
					StringCchPrintfA(buf, ARRAYSIZE(buf), "Content-Type:audio/%s",ext);
					serv->set_reply_header(buf);
					wchar_t title[128]={0};
					getTitle(dev,song,fn,title,128);
					StringCchPrintfA(buf, ARRAYSIZE(buf), "icy-name:%s",(char*)AutoChar(title,CP_UTF8));
					serv->set_reply_header(buf);
					serv->send_reply();
					// do the actual copy, and start streaming
					if(copyFileFromDevice(dev,song,fn)) return new FilePageGenerator(fn,serv);
				}
			}
		}
		serv->set_reply_string("HTTP/1.1 404 NOT FOUND");
		serv->send_reply();
		return 0; // no data
	}
	void destroyConnection(api_pagegenerator *conn)
	{
		FilePageGenerator *realObject = static_cast<FilePageGenerator *>(conn);
		delete realObject;
	}
protected:
	RECVS_DISPATCH;
};

#define CBCLASS onConnCB
START_DISPATCH;
CB(API_ONCONNCB_ONCONNECTION,onConnection);
VCB(API_ONCONNCB_DESTROYCONNECTION,destroyConnection);
END_DISPATCH;
#undef CBCLASS

int serverPort = -1;

static DWORD WINAPI ThreadFunc_Server(LPVOID lpParam) {
	extern bool quitting;
	if(quitting) return 0;
	onConnCB *pOnConnCB;
	int * killswitch = (int*)lpParam;
	api_webserv *server=0;
	waServiceFactory *sf = plugin.service->service_getServiceByGuid(webservGUID);
	if (sf) server = (api_webserv *)sf->getInterface();
	if(!server) return NULL;
	pOnConnCB = new onConnCB;
	server->SetConnectionCallback(pOnConnCB);
	serverPort = 54387;
	while(server->addListenPort(serverPort,0x0100007f) && serverPort < 54397) serverPort++;
	if(serverPort >= 54397) { serverPort=-1; return NULL; }
	while (killswitch && *killswitch == 0)
	{
		server->run();
		server->run();
		Sleep(20);
	}
	server->removeListenPort(serverPort);
	sf->releaseInterface(server);
	serverPort=-1;
	delete pOnConnCB;
	return NULL;
}

static int killswitch = 0;
static HANDLE serverThread = NULL;

void startServer() {
	if(serverPort == -1) {
		killswitch = 0;
		DWORD dwThreadId;
		serverThread = CreateThread(NULL, 0, ThreadFunc_Server, (LPVOID)&killswitch, 0, &dwThreadId);
	}
}

void stopServer() {
	if(serverThread) {
	    killswitch = 1;
	    WaitForSingleObject(serverThread,INFINITE);
	    CloseHandle(serverThread);
	    serverThread = NULL;
	}
}