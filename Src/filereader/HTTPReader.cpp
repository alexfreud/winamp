#include "HTTPReader.h"
#include "..\Components\wac_network\wac_network_http_receiver_api.h"
#include "api__filereader.h"
#include "../nu/AutoChar.h"
#include <api/service/waservicefactory.h>
#include <api/filereader/api_readercallback.h>
#include <wchar.h>
#include <bfc/platform/strcmp.h>
#include <bfc/platform/minmax.h>
#ifdef _WIN32
#include <shlwapi.h>
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif

#include <stdexcept>

// so we don't accidently call these CRT functions
#ifdef close
#undef close
#endif
#ifdef open
#undef open
#endif
#ifdef read
#undef read
#endif

#define config_guess_prebuffer true
#define config_buffer_size 64
#define config_prebuffer_size 24
#define config_prebuffer_min 0
#define config_allowseek true
#define config_fullseek true
#define config_seekprebuffer 1
#define config_suppressstatus false

// {C0A565DC-0CFE-405a-A27C-468B0C8A3A5C}
static const GUID internetConfigGroupGUID =
  {
    0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c }
  };

class HttpReader
{
public:
	HttpReader(const char *url, uint64_t start_offset = 0, uint64_t total_len = 0, int is_seek = 0);
	~HttpReader();

	int connect();
	int read(int8_t *buffer, int length);
	
	void abort()                                                      { killswitch = 1; }
	
	int bytesAvailable();

	uint64_t getContentLength()
	{
		if (m_contentlength)
			return m_contentlength;

		return -1;
	}

	int canSeek()
	{
		return (m_contentlength &&
		        /* JF> this is correct but not as compatible: m_accept_ranges && */
		        !m_meta_interval);
	}

	uint64_t getPos()                                                 { return m_contentpos; }

	const char *getHeader( const char *header )                       { return httpGetter->getheader( (char *)header ); }

	void setMetaCB( api_readercallback *cb )                          { metacb = cb; }

	//static BOOL CALLBACK httpDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	api_httpreceiver *httpGetter = NULL;
	api_dns          *dns        = NULL;

	char *m_AllHeaders;

	int buffer_size;
	int prebuffer_size, prebuffer_min;
	int need_prebuffer;
	uint64_t m_contentlength, m_contentpos;
	int m_accept_ranges;

	int proxy_enabled;
	char *proxy;

	int killswitch = -1;

	int m_meta_init, m_meta_interval, m_meta_pos, m_meta_size, m_meta_buf_pos;
	char m_meta_buf[4096];

	api_readercallback *metacb;

	int guessed_prebuffer_size;

	char lpinfo[256];
	char force_lpinfo[256];
	char *dlg_realm;
	char *m_url;
};


HttpReader::HttpReader( const char *url, uint64_t start_offset, uint64_t total_len, int is_seek )
{
	m_accept_ranges        = 0;
	buffer_size            = (config_buffer_size * 1024);
	prebuffer_size         = (config_prebuffer_size * 1024);
	prebuffer_min          = (config_prebuffer_min * 1024);
	guessed_prebuffer_size = !config_guess_prebuffer;

	if (is_seek)
	{
		prebuffer_min = prebuffer_size = config_seekprebuffer;
		guessed_prebuffer_size = 1;
	}

	proxy_enabled   = 0;
	killswitch      = 0;
	need_prebuffer  = 0;
	m_contentlength = total_len;
	m_contentpos    = start_offset;
	m_meta_init     = m_meta_interval = m_meta_pos = m_meta_size = m_meta_buf_pos = 0;
	m_meta_buf[0]   = 0;
	metacb          = NULL;
	force_lpinfo[0] = 0;
	lpinfo[0]       = 0;

	m_url           = _strdup(url);

	int use_proxy = 1;
	bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
	if (proxy80 && strstr(url, ":") && (!strstr(url, ":80/") && strstr(url, ":80") != (url + strlen(url) - 3)))
		use_proxy = 0;

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid( httpreceiverGUID );
	if (sf)
		httpGetter = (api_httpreceiver *)sf->getInterface();

	const wchar_t *proxy = AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0);

	httpGetter->open(API_DNS_AUTODNS, buffer_size, (use_proxy && proxy && proxy[0]) ? (char *)AutoChar(proxy) : NULL);
	httpGetter->addheader("Accept:*/*");
	if (!_strnicmp(url, "uvox://", 7))
	{
		httpGetter->addheader("User-Agent: ultravox/2.0");
	}
	else
	{
		httpGetter->AddHeaderValue("User-Agent", AutoChar(WASABI_API_APP->main_getVersionString()));
	}

	if (start_offset > 0)
	{
		char temp[128];
		sprintf(temp, "Range: bytes=%d-", (int)start_offset);
		httpGetter->addheader(temp);
	}
	else
		httpGetter->addheader("Icy-Metadata:1");

	httpGetter->connect((char *)m_url, start_offset > 0);
	HttpReader::connect();
	HttpReader::read(0, 0);

	//if (!config_suppressstatus) 		api->core_setCustomMsg(0, StringPrintf("[Connecting] %s",url));
}

HttpReader::~HttpReader()
{
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid( httpreceiverGUID );
	if ( sf )
		sf->releaseInterface( httpGetter );
}

// TODO: BOOL CALLBACK HttpReader::httpDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)

int HttpReader::connect()
{
	try
	{
		while ( killswitch >= 0 && httpGetter->run() == 0 && httpGetter->get_status() == 0 )
		{
#ifdef _WIN32
			//Sleep( 50 );
#else
			usleep( 50000 );
#endif
		}
		if ( killswitch )
			return 0;

		if ( httpGetter->get_status() == -1 )
		{
			int code = httpGetter->getreplycode();
			if ( code == 401 )
			{
				/* TODO:
					// authorization required
					JNL_Connection *m_connection=httpGetter->get_con();
					char str[4096];
					while (m_connection->recv_lines_available() > 0) {
						char *wwwa="WWW-Authenticate:";
						m_connection->recv_line(str,4096);
						if (!str[0]) break;
						if (!_strnicmp(str,wwwa,strlen(wwwa))) {
						char *s2="Basic realm=\"";
						char *p=str+strlen(wwwa); while (p && *p== ' ') p++;
						if (!_strnicmp(p,s2,strlen(s2))) {
							p+=strlen(s2);
							if (strstr(p,"\"")) {
							strstr(p,"\"")[0]=0;
							if (*p) {
								if(force_lpinfo[0]) {
								force_lpinfo[0]=0; // invalid l/p
								} else WASABI_API_CONFIG->getStringPrivate(StringPrintf("HTTP-AUTH/%s",p),force_lpinfo,sizeof(force_lpinfo),"");
								if (!force_lpinfo[0] || lpinfo[0]) {
								dlg_realm = p;
								api->pushModalWnd();
								RootWnd *pr=api->main_getRootWnd();
								while(pr->getParent()) pr=pr->getParent();
								if (!DialogBoxParam(the->gethInstance(),MAKEINTRESOURCE(IDD_HTTPAUTH),pr->gethWnd(),httpDlgProc,(long)this)) {
									force_lpinfo[0]=0;
								} else {
									WASABI_API_CONFIG->setStringPrivate(StringPrintf("HTTP-AUTH/%s",p),force_lpinfo);
								}
								api->popModalWnd();
								}
								if (force_lpinfo[0]) {
								const char *p=STRSTR(m_url,"http://");
								if(p) {
									p+=7;
									StringPrintf tmp("http://%s@%s",force_lpinfo,p);
									httpGetter->connect((char *)tmp.getValue());
									return connect(); // recursive city
								}
								}
							}
							}
						}
						break;
						}
					}*/
			}
			// TODO: api->core_setCustomMsg(0, StringPrintf("HTTP: can't connect (%i)",code));
			return 0;
		}

		if ( httpGetter->getreplycode() < 200 || httpGetter->getreplycode() > 299 )
		{
			// TODO: api->core_setCustomMsg(0, StringPrintf("HTTP: returned %i",httpGetter->getreplycode()));
			return 0;
		}

		need_prebuffer = 1;
	}
	catch ( const std::exception &e )
	{
		return 0;
	}


	return 1;
}

int HttpReader::bytesAvailable()
{
	int code = httpGetter->run();
	int ba = httpGetter->bytes_available();

	if ( !ba && code )
		return -1;

	return ba;
}

int HttpReader::read(int8_t *buffer, int length)
{
	if (!httpGetter->GetConnection())
		return 0;

	if ( httpGetter->GetConnection()->get_state() == CONNECTION_STATE_CONNECTED && httpGetter->bytes_available() < prebuffer_min )
		need_prebuffer = 1;

	if (need_prebuffer)
	{
		need_prebuffer = 0;
		// TODO: if (!config_suppressstatus) api->core_setCustomMsg(0, "Prebuffering ...");

		if (!guessed_prebuffer_size)
		{
			// wait for headers
			int s;
			do
			{
				s = httpGetter->run();
			}
			while (s == 0 && httpGetter->get_status() != 2);

			// calculate the needed prebuffer size if it's a shoutcast stream
			const char *icybr;
			if (icybr = httpGetter->getheader("icy-br"))
			{
				prebuffer_size = (atoi(icybr) / 8) * 4096;
				prebuffer_min = (atoi(icybr) / 8) * 1024;

				if (prebuffer_size > buffer_size)
					prebuffer_size = buffer_size;
			}

			guessed_prebuffer_size = 1;
		}

		int last_pre = -1;
		while (httpGetter->bytes_available() < prebuffer_size && !killswitch)
		{
			int s = httpGetter->run();
//      JNL_Connection::state s = getter->get_state();
			//    if (s == JNL_Connection::STATE_ERROR || s == JNL_Connection::STATE_CLOSED) break;
			if (s == -1 || s == 1) break;
#ifdef _WIN32
			Sleep(50);
#else
      usleep(50000);
#endif
			if (last_pre != httpGetter->bytes_available() && !killswitch)
			{
// TODO:      if (!config_suppressstatus) api->core_setCustomMsg(0, StringPrintf(0, "Prebuffering : %i/%i bytes",httpGetter->bytes_available(),prebuffer_size));
			}
		}

//		if (!killswitch)
//		{
//// TODO:      if (!config_suppressstatus) api->core_setCustomMsg(0, "Prebuffering done.");
//		}
	}

	if (killswitch) return 0;

	// metadata filtering
	if ( !m_meta_init )
	{
		const char *v;
		if ( v = httpGetter->getheader( "icy-metaint:" ) )
			m_meta_interval = atoi( v );

		if ( !m_contentlength )
		{
			if ( v = httpGetter->getheader( "content-length:" ) )
				m_contentlength = _strtoui64( v, NULL, 10 );//atoi(v);
		}

		v = httpGetter->getheader( "accept-ranges:" );
		if ( v && strcasestr( v, "bytes" ) )
			m_accept_ranges = 1;

		m_meta_init = 1;
	}

	int error = 0, recvBytes = 0;
	while (length && !error && !killswitch)
	{
		int code = httpGetter->run();

		if (code != 0)
			error = 1;

		// old metadata parsing
		/*if (httpGetter->bytes_available()>0) {
		  int l=httpGetter->get_bytes(buffer,length);

		  // metadata stuff
		if (m_meta_interval) {
		int x=l;
		unsigned char *buf=(unsigned char *)buffer;
		if (m_meta_size)// already in meta block
		{				
			int len=MIN(x,m_meta_size-m_meta_buf_pos);

			MEMCPY(m_meta_buf+m_meta_buf_pos,buf,len);
			m_meta_buf_pos+=len;

			if (m_meta_buf_pos==m_meta_size)
			{
				if(metacb) metacb->metaDataReader_onData(m_meta_buf,m_meta_size);
				m_meta_buf_pos=0;
				m_meta_size=0;
				m_meta_pos=0;
			}

			x-=len;
			if (x) MEMCPY(buf,buf+len,x);
		}
		else if (m_meta_pos+x > m_meta_interval) // block contains meta data somewhere in it, and we're not alreayd reading a block
		{
			int start_offs=m_meta_interval-m_meta_pos;
			int len;
			m_meta_size=((unsigned char *)buf)[start_offs]*16;

			len=MIN(x-start_offs-1,m_meta_size);

			if (len) MEMCPY(m_meta_buf,buf+start_offs+1,len);
			m_meta_buf_pos=len;

			if (m_meta_buf_pos==m_meta_size) // full read of metadata successful
			{
				x-=m_meta_size+1;
				if (x > start_offs) MEMCPY(buf+start_offs,buf+start_offs+1+m_meta_size,x-start_offs);
				if(metacb) metacb->metaDataReader_onData(m_meta_buf,m_meta_size);
				m_meta_buf_pos=0;
				m_meta_pos=-start_offs;
				m_meta_size=0;
			}
			else 
			{
				x=start_offs; // otherwise, there's only the first block of data
			}
		}
		if (x > 0) 
		{
			m_meta_pos+=x;
		}
		l=x;
		}

		  length-=l;
		  buffer+=l;
		  recvBytes+=l;
		} else Sleep(50);*/

		while (1)
		{
			int len = httpGetter->bytes_available();
			if (m_meta_interval && m_meta_pos >= m_meta_interval)
			{
				unsigned char b;
				if (len > 0 && httpGetter->peek_bytes((char*)&b, 1) && len > (b << 4))
				{
					char metabuf[4096];
					httpGetter->get_bytes(metabuf, 1);
					httpGetter->get_bytes(metabuf, b << 4);
					if (metacb) metacb->metaDataReader_onData(metabuf, b << 4);
					//stream_metabytes_read+=(b<<4)+1;
					m_meta_pos = 0;
				}
				else
					break;
			}
			else
			{
				len = MIN(length, len);
				if (m_meta_interval)
					len = MIN(m_meta_interval - m_meta_pos, len);

				if (len > 0)
				{
					len = httpGetter->get_bytes((char*)buffer, len);
					m_meta_pos += len;
					//stream_bytes_read+=len;
					length -= len;
					buffer += len;
					recvBytes += len;
				}
				else
				{
#ifdef _WIN32
          Sleep(50);
#else
          usleep(50000);
#endif
				}
				break;
			}
		}

		/*    int s=httpGetter->get_con()->get_state();
		    if(code==0) {*/
		/*      char tmp[512];
		      wsprintf(tmp,"[Connected] Retrieving list (%i bytes)", recvBytes);
		      api->status_setText(tmp);*/
//    } else error=1;
	}

	m_contentpos += recvBytes;
	
	return recvBytes;
}


/* ---------------------------------------------------------------------- */
int HTTPReader::isMine(const wchar_t *filename, int mode)
{
	if (!_wcsnicmp(filename, L"http://", 7) ||
	    !_wcsnicmp(filename, L"https://", 8) ||
	    !_wcsnicmp(filename, L"icy://", 6) ||
	    !_wcsnicmp(filename, L"sc://", 5) ||
	    !_wcsnicmp(filename, L"uvox://", 7)) return 1;
	return 0;
}

int HTTPReader::open( const wchar_t *filename, int mode )
{
	if ( !isMine( filename, mode ) )
		return 0;

	m_filename = _strdup( AutoChar( filename ) );
	reader     = new HttpReader( m_filename );

	return 1;
}

uint64_t HTTPReader::bytesAvailable( uint64_t requested )
{
	int v = reader ? reader->bytesAvailable() : 0;
	if ( v > requested )
		return requested;

	return v;
}

size_t HTTPReader::read( int8_t *buffer, size_t length )
{
	if ( !reader )
		return 0;

	if ( !hasConnected )
	{
		int res = reader->connect();
		if ( !res )
			return 0;

		hasConnected = 1;
	}

	return reader->read( buffer, (int)length );
}

void HTTPReader::close()
{
	delete reader;
	reader = NULL;
}

void HTTPReader::abort()
{
	if ( reader )
		reader->abort();
}

uint64_t HTTPReader::getLength()
{
	return reader ? reader->getContentLength() : -1;
}

uint64_t HTTPReader::getPos()
{
	return reader ? reader->getPos() : 0;
}

int HTTPReader::canSeek()
{
	return ( config_allowseek && reader && reader->canSeek() ) ? ( config_fullseek ? 1 : -1 ) : 0;
}

int HTTPReader::seek( uint64_t position )
{
	if ( reader && reader->canSeek() && config_allowseek )
	{
		if ( position == getPos() ) return 0;
		hasConnected = 0;
		uint64_t cl = reader->getContentLength();
		delete( (HttpReader *)reader );
		reader = new HttpReader( m_filename, position, cl, 1 );
		return 0;
	}

	return -1;
}

int HTTPReader::hasHeaders()
{
	return 1;
}

const char *HTTPReader::getHeader( const char *header )
{
	return reader ? reader->getHeader( header ) : NULL;
}

void HTTPReader::setMetaDataCallback( api_readercallback *cb )
{
	if ( reader )
		reader->setMetaCB( cb );
}

#define CBCLASS HTTPReader
START_DISPATCH;
CB(ISMINE, isMine);
CB(OPEN, open);
CB(READ, read);
CB(WRITE, write);
VCB(CLOSE, close);
VCB(ABORT, abort);
CB(GETLENGTH, getLength);
CB(GETPOS, getPos);
CB(CANSEEK, canSeek);
CB(SEEK, seek);
CB(HASHEADERS, hasHeaders);
CB(GETHEADER, getHeader);
CB(EXISTS, exists);
//  CB(REMOVE,remove);
//  CB(REMOVEUNDOABLE,removeUndoable);
//  CB(MOVE,move);
CB(BYTESAVAILABLE, bytesAvailable);
VCB(SETMETADATACALLBACK, setMetaDataCallback);
CB(CANPREFETCH, canPrefetch);
//  CB(CANSETEOF, canSetEOF);
//  CB(SETEOF, setEOF);
END_DISPATCH;
#undef CBCLASS

