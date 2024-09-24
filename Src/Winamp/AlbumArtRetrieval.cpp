// disabled 30 May 2012 as per email from Tejas w.r.t. to Rovi deal ending
#if 0
#include "main.h"
#include "../nu/AutoUrl.h"
#include "../nu/AutoWide.h"
#include "../nu/GrowBuf.h"
#include "api.h"
#include "../xml/obj_xml.h"
#include "../xml/ifc_xmlreadercallback.h"
#include "..\Components\wac_network\wac_network_http_receiver_api.h"
#include <api/service/waservicefactory.h>
#include <api/service/svcs/svc_imgload.h>
#include "XMLString.h"
#include <tataki/export.h>
#include <tataki/bitmap/bitmap.h>
#include <tataki/canvas/bltcanvas.h>
#include <strsafe.h>

static INT_PTR CALLBACK artDownloader(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK scrollChildHostProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK scrollChildProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK imageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

#define HTTP_BUFFER_SIZE 32768

#define WM_ADDCHILD WM_USER+0
#define WM_SELECTED WM_USER+1
#define WM_UPDATESTATUS WM_USER+2
#define AddImageToList(hChild, param) SendMessageW(hChild,WM_ADDCHILD,0,(LPARAM)param)
#define GetParam(hwndDlg) (ArtParser*)GetWindowLongPtrW(hwndDlg,GWLP_USERDATA)
#define GetParamC(hwndDlg) (ImgDownloader*)GetWindowLongPtrW(hwndDlg,GWLP_USERDATA)
#define UpdateStatus(hwndDlg) PostMessageW(hwndDlg,WM_UPDATESTATUS,0,0)

class ImgDownloader
{
public:
	ImgDownloader(const wchar_t *_desc, const char *_url) : done(false), http(0), started(false), error(false), imgbuf(0), imgbufsize(0), imgbufused(0)
	{
		desc = _wcsdup(_desc);
		url = _strdup(_url);
	}
	~ImgDownloader()
	{
		free(desc);
		free(url);
		free(imgbuf);
		waServiceFactory *httpFactory = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
		if(httpFactory && http)
			httpFactory->releaseInterface(http);
	}
	bool run();
	
	wchar_t *desc;
	char *url;
	bool done, error;
	api_httpreceiver *http;

	BYTE *imgbuf;
	int imgbufsize;
	int imgbufused;
private:
	bool started;
};

class ArtParser : public ifc_xmlreadercallback
{
public:
	ArtParser(artFetchData * data);
	~ArtParser();
	
	wchar_t curAlbumStr[512] = {0};
	void ArtParser::StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void ArtParser::TextHandler(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str);
	
	int run();
	
	artFetchData * data;

	bool doneXML;
	int curImage, numImages, failedImages;

	bool error;
	HWND hwndDlg;
	obj_xml *parser;
	waServiceFactory *parserFactory, *httpFactory;
	api_httpreceiver *http;
	std::vector<ImgDownloader*> imgDownload;

protected:
	#define CBCLASS ArtParser
	START_DISPATCH_INLINE;
	VCB(ONSTARTELEMENT, StartTag);
	VCB(ONCHARDATA, TextHandler);
	END_DISPATCH;
	#undef CBCLASS
};

wchar_t tmp_album[512] = {0}, tmp_artist[512] = {0}, tmp_year[5] = {0};
int RetrieveAlbumArt(artFetchData * data)
{
	if(!data || data->size < sizeof(artFetchData))
		return 1;

	int ret = 1;
	Tataki::Init(serviceManager);
	{
		ArtParser param(data);
		if(!param.error)
			ret = LPDialogBoxParamW(IDD_ARTDOWNLOADER,data->parent,artDownloader,(LPARAM)&param);

		while (ret == 2) // Keep in loop till user aborts custom search
		{
			// TODO: benski> maybe we should save the old values and restore at the end
			data->album = tmp_album;
			data->artist = tmp_artist;
			data->year = _wtoi(tmp_year);
			// Martin> we should also set the other values back to null
			// benski> i'm not sure if we want to set these id fields to NULL
			// but we'll go with it for now
			data->amgAlbumId = NULL;
			data->amgArtistId = NULL;
			data->gracenoteFileId = NULL;
			WASABI_API_MEMMGR->sysFree(data->imgData);
			data->imgData = NULL;
			data->imgDataLen = NULL;

			ArtParser param(data);
			ret = LPDialogBoxParamW(IDD_ARTDOWNLOADER,data->parent,artDownloader,(LPARAM)&param);
		}
	}
	Tataki::Quit();
	return ret;
}

#define USER_AGENT_SIZE (10 /*User-Agent*/ + 2 /*: */ + 6 /*Winamp*/ + 1 /*/*/ + 1 /*5*/ + 3/*.21*/ + 1 /*Null*/)
static void SetUserAgent(api_httpreceiver *http)
{
	char user_agent[USER_AGENT_SIZE] = {0};
	StringCchCopyA(user_agent, USER_AGENT_SIZE, "User-Agent: Winamp/"APP_VERSION); // as a nice side effect, this will cut off any extra digits after the first two.  e.g. 5.111 becomes 5.11
	http->addheader(user_agent);
}

class UrlBuilder : private GrowBuf
{
public:
	UrlBuilder(const char * base) : first(1) { set(base); }
	~UrlBuilder(){}
	void AddParam(const char * key, const wchar_t * value) { AddParamI(key,(char*)AutoUrl(value)); }
	void AddParam(const char * key, int value) { char buf[16]; StringCchPrintfA(buf,16,"%d",value); AddParamI(key,buf); }
	void Finish() { GrowBuf::add((void*)"",1); }
	const char * Get() { return (const char*)get(); } // don't call this without first calling finish!
private:
	void AddParamI(const char * key, const char * value) { if(first){ first=0; add("?");} else add("&"); add(key); add("="); add(value); }
	void add(const char * x) { GrowBuf::add((char*)x,strlen(x)); }
	void set(const char * x) { GrowBuf::set((char*)x,strlen(x)); }
	int first;
};

static wchar_t* format(const wchar_t *in, wchar_t *buf, int len)
{
	wchar_t *p = buf;
	int inbracket = 0;
	while(in && *in)
	{
		if(p >= buf + len - 1) break;
		else if(*in == L'[' || *in == L'(' || *in == L'<') inbracket++;
		else if(*in == L']' || *in == L')' || *in == L'>') inbracket--;
		else if(!inbracket) *(p++) = *in;
		in++;
	}
	*p=0;
	return buf;
}

ArtParser::ArtParser(artFetchData * data) : data(data), error(false), parserFactory(0), parser(0), http(0), httpFactory(0), doneXML(false), curImage(0), numImages(0), failedImages(0)
{
	curAlbumStr[0]=0;

	parserFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
		parser = (obj_xml *)parserFactory->getInterface();

	if (parser)
	{
		parser->xmlreader_registerCallback(L"document\fartwork", this);
		parser->xmlreader_registerCallback(L"response\fdata\fartwork", this);
		parser->xmlreader_open();
	}
	else
	{
		error=true;
		return;
	}

	// construct xml url
	UrlBuilder url("http://client.winamp.com/metadata/artwork.php");
	
	wchar_t temp[2048] = {0};

	if(data->artist && data->artist[0] && _wcsicmp(data->artist, L"Various Artists") && _wcsicmp(data->artist, L"VA") && _wcsicmp(data->artist, L"OST"))
		url.AddParam("artist",format(data->artist,temp,2048));
	
	if(data->album && data->album[0])
		url.AddParam("album",format(data->album,temp,2048));
	
	if(data->year)
		url.AddParam("recorddate",data->year);
	
	if(data->amgAlbumId)
		url.AddParam("amgalbumid",data->amgAlbumId);
	
	if(data->amgArtistId)
		url.AddParam("amgartistid",data->amgArtistId);
	
	if(data->gracenoteFileId && data->gracenoteFileId[0])
		url.AddParam("tagid",data->gracenoteFileId);

	url.Finish();
	
	httpFactory = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
	if(httpFactory) http = (api_httpreceiver *)httpFactory->getInterface();

	if(http)
	{
		http->AllowCompression();
		http->open(API_DNS_AUTODNS, HTTP_BUFFER_SIZE, config_proxy);
		SetUserAgent(http);
		http->connect(url.Get());
	}
	else error = true;
}

ArtParser::~ArtParser()
{
	imgDownload.deleteAll();
	if(parser)
	{
		parser->xmlreader_unregisterCallback(this);
		parser->xmlreader_close();
		if(parserFactory)
			parserFactory->releaseInterface(parser);
	}
	parser = 0;
	parserFactory = 0;
	if(http && httpFactory)
			httpFactory->releaseInterface(http);
	http = 0;
	httpFactory = 0;
}

void ArtParser::StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	const wchar_t* artist = params->getItemValue(L"amgArtistDispName");
	const wchar_t* album = params->getItemValue(L"amgDispName");
	const wchar_t* year = params->getItemValue(L"recordDate");
	StringCchPrintfW(curAlbumStr,512,L"%s - %s (%s)",artist?artist:L"",album?album:L"",year?year:L"");
}

void ArtParser::TextHandler(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str)
{
	numImages++;
	imgDownload.push_back(new ImgDownloader(curAlbumStr,AutoChar(str)));
	UpdateStatus(hwndDlg);
}

int ArtParser::run()
{
	int ret = http->run();
	if (ret == -1) // connection failed
	{
		error=true;
		UpdateStatus(hwndDlg);
		return 0;
	}

	int replycode = http->getreplycode();
	switch (replycode)
	{
	case 0:
	case 100:
		return 1;
	case 200:
	{
		char downloadedData[HTTP_BUFFER_SIZE] = {0};
		int xmlResult = API_XML_SUCCESS;
		int downloadSize = http->get_bytes(downloadedData, HTTP_BUFFER_SIZE);
		if(downloadSize)
			xmlResult = parser->xmlreader_feed((void *)downloadedData, downloadSize);
		else if(!downloadSize && ret == 1)
		{ // we're finished!
			xmlResult = parser->xmlreader_feed(0,0);
			doneXML=true;
			UpdateStatus(hwndDlg);
			return 0;
		}
		break;
	}
	default:
		error=true;
		UpdateStatus(hwndDlg);
		return 0;
	}
	return 1;
}

bool ImgDownloader::run()
{
	if(!started)
	{
		started = true;
		waServiceFactory *httpFactory = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
		if(httpFactory) http = (api_httpreceiver *)httpFactory->getInterface();
		if(http)
		{
			http->open(API_DNS_AUTODNS, HTTP_BUFFER_SIZE, config_proxy);
			SetUserAgent(http);
			http->connect(url);
		}
		imgbuf = (BYTE*)malloc(HTTP_BUFFER_SIZE);
		imgbufsize = HTTP_BUFFER_SIZE;
	}
	if(!http || !imgbuf)
	{
		error=true;
		return 0;
	}
	int ret = http->run();
	if(ret == -1) //error
	{
		error=true;
		return 0;
	}

	int replycode = http->getreplycode();
	switch (replycode)
	{
	case 0:
	case 100:
		return 1;
	case 200:
	{
		int downloadSize = http->get_bytes(imgbuf+imgbufused, imgbufsize - imgbufused);
		imgbufused += downloadSize;
		if(imgbufused + 4096 >= imgbufsize)
		{
			imgbufsize += HTTP_BUFFER_SIZE;
			imgbuf = (BYTE*)realloc(imgbuf,imgbufsize);
		}

		if(!downloadSize && ret == 1)
			done=true;
		break;
	}
	default:
		return 0;
	}
	return 1;

}

static INT_PTR CALLBACK artDownloader(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static HWND m_child;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			m_child = LPCreateDialogW(IDD_ARTDOWNLOADER_SCROLLHOST,hwndDlg,(WNDPROC)scrollChildHostProc);
			SetWindowLong(hwndDlg,GWLP_USERDATA,lParam);
			ArtParser * parser = (ArtParser *)lParam;
			parser->hwndDlg = hwndDlg;
			SetTimer(hwndDlg,0,50,NULL);
			SetTimer(hwndDlg,1,50,NULL);
			UpdateStatus(hwndDlg);
			if(parser->data->showCancelAll) ShowWindow(GetDlgItem(hwndDlg,IDC_CANCELALL),SW_SHOWNA);
			wchar_t old[100]=L"";
			GetWindowTextW(hwndDlg,old,100);
			wchar_t buf[256]=L"";
			if(parser->data->artist && parser->data->artist[0] && parser->data->album && parser->data->album[0])
				StringCchPrintfW(buf,256,L"%s: %s - %s",old,parser->data->artist,parser->data->album);
			else if(parser->data->album && parser->data->album[0])
				StringCchPrintfW(buf,256,L"%s: %s",old,parser->data->album);
			else if(parser->data->artist && parser->data->artist[0])
				StringCchPrintfW(buf,256,L"%s: %s",old,parser->data->artist);
			
			if(buf[0])
				SetWindowTextW(hwndDlg,buf);

			SetWindowTextW(GetDlgItem(hwndDlg,IDC_SEARCHREFINE_ARTIST), parser->data->artist);
			SetWindowTextW(GetDlgItem(hwndDlg,IDC_SEARCHREFINE_ALBUM), parser->data->album);
			wchar_t yearbuf[5]=L"";
			_itow(parser->data->year,yearbuf,10);
			SetWindowTextW(GetDlgItem(hwndDlg,IDC_SEARCHREFINE_YEAR), (parser->data->year?yearbuf:L""));
		}
		break;
	case WM_SELECTED:
		{
			ArtParser * parser = GetParam(hwndDlg);
			HWND selchild = (HWND)wParam;
			ImgDownloader *d = (ImgDownloader *)lParam;
			if(parser && d && d->imgbuf && d->imgbufused)
			{
				if (!AGAVE_API_AMGSUCKS || AGAVE_API_AMGSUCKS->WriteAlbumArt(d->imgbuf, d->imgbufused, &parser->data->imgData, &parser->data->imgDataLen) != 0)
				{
					void * img = WASABI_API_MEMMGR->sysMalloc(d->imgbufused);
					memcpy(img,d->imgbuf,d->imgbufused);
					parser->data->imgData = img;
					parser->data->imgDataLen = d->imgbufused;
				}
				char * dot = strrchr(d->url,'.');
				if(dot) lstrcpynW(parser->data->type,AutoWide(dot+1),10);
				EndDialog(hwndDlg,0);
			}
		}
		break;
	case WM_UPDATESTATUS:
		{
			ArtParser * parser = GetParam(hwndDlg);
			if(parser)
			{
				wchar_t s[100] = {0};
				if(parser->error) getStringW(IDS_ART_SEARCH_FAILED,s,100);
				else if(parser->doneXML) getStringW(IDS_ART_SEARCH_FINISHED,s,100);
				else getStringW(IDS_ART_SEARCH_PROGRESS,s,100);
				wchar_t buf[512] = {0};
				StringCchPrintfW(buf,512,getStringW(IDS_ART_SEARCH_STATUS,0,0),s,parser->numImages,parser->curImage - parser->failedImages,parser->failedImages,parser->numImages - parser->curImage);
				SetDlgItemTextW(hwndDlg,IDC_STATUS,buf);

			}
		}
		break;
	case WM_TIMER:
		{
			ArtParser * parser = GetParam(hwndDlg);
			switch(wParam)
			{
			case 0:
				if(parser && !parser->run())
					KillTimer(hwndDlg,0);
				break;
			case 1:
				if(parser && parser->imgDownload.size())
				{
					ImgDownloader *d = parser->imgDownload.at(0);
					if(d->error)
					{
						parser->failedImages++;
						parser->curImage++;
						parser->imgDownload.eraseindex(0);
						delete d;
						UpdateStatus(hwndDlg);
					}
					else if(d->done)
					{
						parser->curImage++;
						parser->imgDownload.eraseindex(0);
						AddImageToList(m_child,d);
						UpdateStatus(hwndDlg);
					}
					else d->run();
				}
				if(parser->error || parser->doneXML)
				{
					if(parser->curImage == parser->numImages)
					{
						KillTimer(hwndDlg,1);
						if(!parser->numImages)
						{
							wchar_t title[100] = {0};
							getStringW(IDS_ART_SEARCH_NOT_FOUND_TITLE,title,100);
							MessageBoxW(hwndDlg,getStringW(IDS_ART_SEARCH_NOT_FOUND,0,0),title,0);
							//EndDialog(hwndDlg,-1); //CUT, since we have now a custom search
						}
						EnableWindow(GetDlgItem(hwndDlg, IDC_SEARCHAGAIN), true);
					}
				}
				break;
			}
		}
		break;
	case WM_DESTROY:
		{
			SetWindowLong(hwndDlg,GWLP_USERDATA,0);
		}
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg,-1);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwndDlg,-1);
			break;
		case IDC_CANCELALL:
			EndDialog(hwndDlg,-2);
			break;
		case IDC_SEARCHAGAIN: // copy text field params to parser
			ArtParser * parser = GetParam(hwndDlg);

			// Let the first search process finish
			if (!(parser->doneXML || parser->error))
			{
				//TODO change this string
				MessageBoxW(hwndDlg, L"Please wait till the current retrieval is finished", L"", 0);
				return 0;
			}

			GetDlgItemTextW(hwndDlg,IDC_SEARCHREFINE_ALBUM, tmp_album, 512);
			GetDlgItemTextW(hwndDlg,IDC_SEARCHREFINE_ARTIST, tmp_artist, 512);
			GetDlgItemTextW(hwndDlg,IDC_SEARCHREFINE_YEAR, tmp_year, 5);

			// End Dialog w/ returning 2: indicates that teh users wants  to start a custom search
			EndDialog(hwndDlg,2);
			break;
		}
		break;
	}
	return 0;
}

// from FileInfo.cpp
extern HBITMAP getBitmap(ARGB32 * data, int dw, int dh, int targetW, int targetH, HWND parent);
extern ARGB32 * decompressImage(const void *data, int datalen, int * dataW, int * dataH);

HBITMAP loadImage(const void * data, int datalen, int w, int h, HWND parent, int *dw=0, int *dh=0)
{
	int dataW=0, dataH=0;
	ARGB32* ret = decompressImage(data,datalen,&dataW,&dataH);
	
	if(dw) *dw = dataW;
	if(dh) *dh = dataH;
	if(!ret) return 0;
	HBITMAP r = getBitmap(ret, dataW, dataH, w, h, parent);
	WASABI_API_MEMMGR->sysFree(ret);
	return r;
}

#define GetDlgParent(hwndDlg) GetParent(GetParent(GetParent(hwndDlg)))
static INT_PTR CALLBACK imageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowLong(hwndDlg,GWLP_USERDATA,lParam);
			ImgDownloader *d = (ImgDownloader *)lParam;
			int w=0,h=0;
			HBITMAP bm = loadImage(d->imgbuf,d->imgbufused,140,140,hwndDlg,&w,&h);
			SendDlgItemMessage(hwndDlg,IDC_IMAGE,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)bm);
			wchar_t buf[1024] = {0};
			StringCchPrintfW(buf,1024,L"%s: %dx%d (%d kB)",d->desc,w,h,(d->imgbufused/1024));
			SetDlgItemTextW(hwndDlg,IDC_IMGTEXT,buf);
		}
		break;
	case WM_DESTROY:
		{
			ImgDownloader *d = GetParamC(hwndDlg);
			SetWindowLong(hwndDlg,GWLP_USERDATA,0);
			if(d) delete d;
			HBITMAP	bm = (HBITMAP)SendDlgItemMessage(hwndDlg,IDC_IMAGE,STM_SETIMAGE,IMAGE_BITMAP,0);
			if(bm) DeleteObject(bm);
		}
		break;
	case WM_COMMAND:
		if(LOWORD(wParam) != IDC_SELECT)
			break;
		// else run through
	case WM_LBUTTONDBLCLK:
		SendMessageW(GetDlgParent(hwndDlg),WM_SELECTED,(WPARAM)hwndDlg,(LPARAM)GetParamC(hwndDlg));
		break;
	}
	return 0;
}

// scroll shit, nothing interesting is happening down here...

static INT_PTR CALLBACK scrollChildHostProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	//static HWND m_child;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			RECT r;
			HWND hw = GetParent(hwndDlg);
			GetWindowRect(GetDlgItem(hw,IDC_PLACEHOLDER),&r);
			ScreenToClient(hw,(LPPOINT)&r);
			ScreenToClient(hw,((LPPOINT)&r)+1);
			SetWindowPos(hwndDlg,NULL,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOZORDER|SWP_NOACTIVATE);
			LPCreateDialogW(IDD_ARTDOWNLOADER_SCROLLCHILD,hwndDlg,(WNDPROC)scrollChildProc);

			SCROLLINFO si={sizeof(si),SIF_RANGE|SIF_PAGE,0};
			si.nPage = (r.right - r.left);
			SetScrollInfo(hwndDlg,SB_HORZ,&si,TRUE);
		}
		break;
	case WM_ADDCHILD:
		{
			HWND m_child = GetWindow(hwndDlg,GW_CHILD);
			HWND newChild = (HWND)SendMessageW(m_child,uMsg,wParam,lParam);
			RECT r,r2;
			GetClientRect(m_child,&r);
			GetClientRect(hwndDlg,&r2);
			SCROLLINFO si={sizeof(si),SIF_RANGE|SIF_PAGE,0};
			si.nMin = 0;
			si.nMax = (r.right - r.left);
			si.nPage = (r2.right - r2.left);
			if(si.nMax < 0) si.nMax = 0;
			SetScrollInfo(hwndDlg,SB_HORZ,&si,TRUE);
			return (INT_PTR)newChild;
		}
		break;
	case WM_HSCROLL:
		{
			HWND m_child = GetWindow(hwndDlg,GW_CHILD);
			int v=0;
			RECT r;
			RECT r2;
			GetClientRect(hwndDlg,&r2);
			GetClientRect(m_child,&r);
			int action = LOWORD(wParam);

			if (r2.right < r.right) {
				if (action == SB_THUMBPOSITION || action == SB_THUMBTRACK) {
					SCROLLINFO si={sizeof(si),SIF_TRACKPOS|SIF_POS};
					GetScrollInfo(hwndDlg,SB_HORZ,&si);
					v=si.nTrackPos;
				}
				else if (action == SB_TOP)
					v=0;
				else if (action == SB_BOTTOM)
					v=r.right-r2.right;
				else if (action == SB_PAGEDOWN || action == SB_LINEDOWN) {
					SCROLLINFO si={sizeof(si),SIF_TRACKPOS|SIF_POS};
					GetScrollInfo(hwndDlg,SB_HORZ,&si);
					if(action == SB_LINEDOWN)
						v=si.nPos + (r2.right)/10;
					else
						v=si.nPos + r2.right;
					if (v > r.right-r2.right) v=r.right-r2.right;
				}
				else if (action == SB_PAGEUP || action == SB_LINEUP) {
					SCROLLINFO si={sizeof(si),SIF_TRACKPOS|SIF_POS};
					GetScrollInfo(hwndDlg,SB_HORZ,&si);
					if(action == SB_LINEUP)
						v=si.nPos - (r2.right)/10;
					else
						v=si.nPos - r2.right;
					if (v < 0) v=0;
				}
				else return 0;

				SetScrollPos(hwndDlg,SB_HORZ,v,!(action == SB_THUMBPOSITION || action == SB_THUMBTRACK));
				SetWindowPos(m_child,NULL,0-v,0,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			}
			else {
				SetScrollPos(hwndDlg,SB_HORZ,0,!(action == SB_THUMBPOSITION || action == SB_THUMBTRACK));
				SetWindowPos(m_child,NULL,0,0,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			}
		}
		break;
	}
	return 0;
}

static INT_PTR CALLBACK scrollChildProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			RECT r;
      GetClientRect(hwndDlg,&r);
			SetWindowPos(hwndDlg,0,0,0,0,r.bottom,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
		}
		break;
	case WM_ADDCHILD:
		{
			HWND newChild = LPCreateDialogParamW(IDD_ARTDOWNLOADER_IMAGE,hwndDlg,imageProc,lParam);
			RECT r,r2;
			GetClientRect(hwndDlg,&r);
			GetClientRect(newChild,&r2);
			SetWindowPos(hwndDlg,0,0,0,r.right + r2.right,r.bottom,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
			SetWindowPos(newChild,0,r.right,0,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			ShowWindow(newChild,SW_SHOWNA);
			return (INT_PTR)newChild;
		}
		break;
	}
	return 0;
}
#endif