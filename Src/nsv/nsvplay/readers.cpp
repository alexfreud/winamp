#include <windows.h>
#include "main.h"
#include <sys/stat.h>

#ifdef NO_WASABI
#include "../../jnetlib/httpget.h"
api_httpreceiver *CreateGet() 
{
	return new JNL_HTTPGet;
}

void ReleaseGet(api_httpreceiver *&get)
{
	delete (JNL_HTTPGet *)get;
	get=0;
}
#else
#include "../..\Components\wac_network\wac_network_http_receiver_api.h"
#include <api.h>
#include <api/service/waservicefactory.h>
#include "../../Winamp/in2.h"
extern In_Module mod;

waServiceFactory *httpFactory = 0;
api_httpreceiver *CreateGet()
{
	api_httpreceiver *get = 0;
	if (!httpFactory && mod.service)
		httpFactory = mod.service->service_getServiceByGuid(httpreceiverGUID);

	if (httpFactory)
		get = (api_httpreceiver *)httpFactory->getInterface();

	return get;
}

void ReleaseGet(api_httpreceiver *&get)
{
	if (!get)
		return ;

	if (!httpFactory && mod.service)
		waServiceFactory *sf = mod.service->service_getServiceByGuid(httpreceiverGUID);

	if (httpFactory)
		httpFactory->releaseInterface(get);

	get = 0;
}


#endif
#define MAX_MULTICONNECTS 8

class HTTPReader : public IDataReader
{
public:
	HTTPReader(const char *url);
	~HTTPReader();
	size_t read(char *buf, size_t len);
	bool iseof() { return !!m_eof; }
	char *gettitle() { return m_title; }
	char *geterror() { return m_err; }
	bool canseek() { return m_content_length != 0xFFFFFFFF && m_accept_ranges && !m_meta_interval; }
	int seek(unsigned __int64 newpos)
	{
		if (!canseek()) return 1;
		doConnect((int)newpos);
		return 0;
	}
unsigned __int64 getsize() { return m_content_length; }
	char *getheader(char *header_name)
	{
		return m_get ? (char *)m_get->getheader(header_name) : NULL;
	}
private:

	int serialconnect( int seekto , int timeout);
	void doConnect(int seekto);
	int getProxyInfo(const char *url, char *out);
	char *m_url;
	char *m_err;
	char *m_title;
	int m_eof;
	int m_meta_init, m_meta_interval, m_meta_pos, m_meta_size, m_meta_buf_pos;
	char m_meta_buf[4096];
	int m_read_headers;
	unsigned int m_content_length;
	int m_accept_ranges;
	int m_is_uvox;
	int m_uvox_readpos;
	int m_uvox_enough_bytes;
	char proxybuf[8400], *proxy;

	api_httpreceiver *m_get;

	// this structure is allocated once, and freed once
	struct
	{
		char *url; //pointers into m_url
		// these two are only active temporarily.
		api_httpreceiver *get;
		char *error;
	}
	m_cons[MAX_MULTICONNECTS];
	int m_numcons;
	int m_newcons;
	int m_serialized;
	int	m_mstimeout;
	int	m_contryptr;
	int m_serialfailed;
	int m_useaproxy;
};

HTTPReader::HTTPReader(const char *url)
{
	m_title = 0;
	m_is_uvox = m_uvox_readpos = 0;
	m_meta_init = m_meta_interval = m_meta_pos = m_meta_size = m_meta_buf_pos = 0;
	m_meta_buf[0] = 0;
	m_err = NULL;
	m_eof = 0;
	m_read_headers = 0;
	m_content_length = 0xFFFFFFFF;
	m_accept_ranges = 0;
	m_get = NULL;
	m_serialized = 0;
	m_mstimeout = 0;
	m_contryptr = 0;
	m_newcons = 0;
	m_serialfailed = 0;
	m_useaproxy = 0;

	// TCP multiconnect
	// JF> using ; as a delimiter is vomit inducing and breaks a lot of other
	//     code. I petition we use <> to delimit, and I'm making it do that.

	m_numcons = 0;
	m_url = _strdup(url);

	int allowproxy = 1;
	char *tmpurl = m_url;
	while (m_numcons < MAX_MULTICONNECTS)
	{
		char *next = strstr( tmpurl, "<>" );
		if ( next ) *next = '\0';

		if (tmpurl[0])
		{
			m_cons[m_numcons].error = NULL;
			m_cons[m_numcons].get = NULL;
			m_cons[m_numcons++].url = tmpurl;
			if (!_strnicmp(tmpurl, "uvox:", 5)) allowproxy = 0;
			if (!_strnicmp(tmpurl, "order://", 8))
			{
				char *p = tmpurl + 8;
				// serialized mctp
				m_serialized = 1;
				m_numcons--;
				m_mstimeout = atoi(p);
				if ( m_mstimeout < 1 )
				{
					m_serialized = 0;
					m_mstimeout = 0;
				}
			}
		}

		if (!next) break;

		tmpurl = next + 2;
	}

	memset(proxybuf, 0, sizeof(proxybuf));
	proxy = NULL;
	if (allowproxy && getProxyInfo(url, proxybuf))
	{
		proxy = strstr(proxybuf, "http=");
		if (!proxy) proxy = proxybuf;
		else
		{
			proxy += 5;
			char *tp = strstr(proxy, ";");
			if (tp) *tp = 0;
		}
	}
	m_is_uvox = 0;

	if ( m_serialized && m_numcons > 1 )  // sanity check
	{
		int rval = 0, i;
		m_newcons = 1;
		// walk the list, set the url such that m_cons[0].url points to each item. try to connect
		// serialconnect returns error codes -1 on error, 0 on timeout, 1 on successfull connect
		for ( i = 0; i < m_numcons; i++ )
		{
			if ( i )
			{
				m_cons[0].url = m_cons[i].url;
			}
			rval = serialconnect(0, m_mstimeout);
			if ( rval == 1 ) break;
		}
		if ( rval < 1 )
		{
			// we didnt get a connection so...
			m_serialfailed = 1;
		}

	}
	else
		doConnect(0);
}


void HTTPReader::doConnect(int seekto)
{
	ReleaseGet(m_get);
	m_uvox_readpos = 0;

	m_eof = 0;


	int i;
	for (i = 0; i < m_numcons; i++ )
	{
		free(m_cons[i].error);
		m_cons[i].error = NULL;
		ReleaseGet(m_cons[i].get);
		m_cons[i].get = CreateGet();
		if (!m_cons[i].get)
			break;
		m_cons[i].get->open(API_DNS_AUTODNS, 65536, (proxy && proxy[0]) ? proxy : NULL);
#ifdef WINAMP_PLUGIN
		m_cons[i].get->addheader("User-Agent:Winamp NSV Player/5.12 (ultravox/2.0)");
#else
 # ifdef WINAMPX
		m_cons[i].get->addheader("User-Agent:" UNAGI_USER_AGENT " (ultravox/2.0)");
#  else
		m_cons[i].get->addheader("User-Agent:NSV Player/0.0 (ultravox/2.0)");
#  endif
#endif
		m_cons[i].get->addheader("Accept:*/*");
		m_cons[i].get->addheader("Connection:close");
		m_cons[i].get->addheader("Ultravox-transport-type: TCP");

		if (seekto)
		{
			char buf[64] = {0};
			wsprintfA(buf, "Range:bytes=%d-", seekto);
			m_cons[i].get->addheader(buf);
		}
		else
			m_cons[i].get->addheader("icy-metadata:1");

		m_cons[i].get->connect(m_cons[i].url, !!seekto);
	}
	m_uvox_enough_bytes = 1;
}

HTTPReader::~HTTPReader()
{
	ReleaseGet(m_get);
	free(m_title);
	free(m_err);
	free(m_url);

	int i;
	for (i = 0; i < m_numcons; i++)
	{
		ReleaseGet(m_cons[i].get);
		free(m_cons[i].error);
	}
}

int HTTPReader::serialconnect(int seekto , int timeout)
{

	ReleaseGet(m_get);
	m_uvox_readpos = 0;

	m_eof = 0;

	int64_t mythen, mynow , myref;
	LARGE_INTEGER then, now, ref;


	QueryPerformanceFrequency( &ref);
	myref = ref.QuadPart;


	QueryPerformanceCounter( &then );
	mythen = then.QuadPart;


	int timer = 0;

	int i = 0;

	{
		ReleaseGet(m_cons[i].get);
		m_cons[i].get = CreateGet();
		if (m_cons[i].get == NULL)
			return 0;
		m_cons[i].get->open(API_DNS_AUTODNS, 65536, (proxy && proxy[0]) ? proxy : NULL);
#ifdef WINAMP_PLUGIN
		m_cons[i].get->addheader("User-Agent:Winamp NSV Player/5.12 (ultravox/2.0)");
#else
# ifdef WINAMPX
		m_cons[i].get->addheader("User-Agent:" UNAGI_USER_AGENT " (ultravox/2.0)");
#  else
		m_cons[i].get->addheader("User-Agent:NSV Player/0.0 (ultravox/2.0)");
#  endif
#endif
		m_cons[i].get->addheader("Accept:*/*");
		m_cons[i].get->addheader("Connection:close");
		m_cons[i].get->addheader("Ultravox-transport-type: TCP");

		if (seekto)
		{
			char buf[64] = {0};
			wsprintfA(buf, "Range:bytes=%d-", seekto);
			m_cons[i].get->addheader(buf);
		}
		else m_cons[i].get->addheader("icy-metadata:1");

		m_cons[i].get->connect(m_cons[i].url, !!seekto);
	}
	m_uvox_enough_bytes = 1;

	int ret, status;

	if (!m_get)
	{
		if (m_err) return 0;
		int i;
		int found = 0;
		i = 0;
		while ( timer < timeout )
		{
			if (!m_cons[i].get) return 0;
			found = 1;

			QueryPerformanceCounter( &now );
			mynow = now.QuadPart;

			float profiletime = (float)(mynow - mythen);
			profiletime /= myref;
			profiletime *= 1000.0;
			timer = (int) profiletime;

			ret = m_cons[i].get->run();
			status = m_cons[i].get->get_status();

			if (ret < 0 || status < 0)
			{
				const char *t = m_cons[i].get->geterrorstr();
				if (t)
				{}
				ReleaseGet(m_cons[i].get);
				break;
			}

			if ( status > 0 )
			{
				int code = m_cons[i].get->getreplycode();
				if ( code < 200 || code > 299 )
				{
					ReleaseGet(m_cons[i].get);
					//wsprintf( m_cons[i].error, "Error: Server returned %d", code );
					break;
				}
				else
				{
					// we're in good shape, make our getter current, and delete all the gay ones
					ReleaseGet(m_get); // just in case, probably zero anyway
					m_get = m_cons[i].get;
					m_cons[i].get = NULL;

					// trash i here, but we are breaking anyway :)
					/* for (i = 0; i < m_numcons; i++)
					    {
						  delete m_cons[i].get;
					  m_cons[i].get = NULL;
					 free( m_cons[i].error );
					 m_cons[i].error = NULL;
					 }*/
					break;
				}
			}
#ifdef _WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		} // while
		if ( timer > timeout )
		{
			ReleaseGet(m_cons[i].get);
			ReleaseGet(m_get);
			return 0;
		}


		if (!m_get)
		{
			return 0;
		}
	}
	if ( m_get ) return 1;
	else return 0;
}


size_t HTTPReader::read(char *buffer, size_t len)
{
	int ret, status;

	if (!m_get)
	{
		if (m_err) return 0;
		int i;
		int found = 0;
		for (i = 0; !m_get && i < m_numcons; i++)
		{
			if (!m_cons[i].get) continue;
			found = 1;

			ret = m_cons[i].get->run();
			status = m_cons[i].get->get_status();

			if (ret < 0 || status < 0)
			{
				const char *t = m_cons[i].get->geterrorstr();
				if (t)
				{
					free(m_cons[i].error);
					m_cons[i].error = _strdup( t );
				}
				ReleaseGet(m_cons[i].get);

			}

			if ( status > 0 )
			{
				int code = m_cons[i].get->getreplycode();
				if ( code < 200 || code > 299 )
				{
					ReleaseGet(m_cons[i].get);
					m_cons[i].get = NULL;

					free(m_cons[i].error);
					m_cons[i].error = (char *)malloc( 100 );
					wsprintfA( m_cons[i].error, "Error: Server returned %d", code );
				}
				else
				{
					// we're in good shape, make our getter current, and delete all the gay ones
					ReleaseGet(m_get); // just in case, probably zero anyway
					m_get = m_cons[i].get;
					m_cons[i].get = NULL;

					// trash i here, but we are breaking anyway :)
					for (i = 0; i < m_numcons; i++)
					{
						ReleaseGet(m_cons[i].get);
						free( m_cons[i].error );
						m_cons[i].error = NULL;
					}
					break; // exit loop of connections
				}
			}
		} // loop of connections

		if (!found) // out of attempted connections heh
		{
			free( m_err );
			if (m_numcons > 1)
			{
				size_t size = 0;
				for (i = 0; i < m_numcons; i++)
					if ( m_cons[i].error ) size += strlen( m_cons[i].error ) + 1;

				m_err = (char *)malloc(size + 100);
				wsprintfA( m_err, "No Valid Multiconnect URLs (%d);", m_numcons );
				for (i = 0; i < m_numcons; i++)
				{
					strcat( m_err, m_cons[i].error );
					strcat( m_err, ";" );
					free(m_cons[i].error);
					m_cons[i].error = NULL;

				}
			}
			else
			{
				m_err = m_cons[0].error;
				m_cons[0].error = NULL;
				if (!m_err) m_err = _strdup("Connection error (Invalid URL?)");
			}
		}
		if (!m_get) return 0;
	}

	ret = m_get->run();
	status = m_get->get_status();

	if (ret > 0 && (!m_get->bytes_available() || !m_uvox_enough_bytes) && status > 1)
	{
		m_eof = 1;
	}

	if (ret < 0 || status < 0)
	{
		const char *t = m_get->geterrorstr();
		if (t)
		{
			free( m_err );
			m_err = (char *)malloc( strlen( t) + 16 );
			wsprintfA( m_err, "Error: %s", t );
			return 0;
		}
	}

	if (status > 0)
	{
		if (!m_read_headers)
		{
			if (status > 1)
			{
				const char *v = m_get->getheader("Content-Length");
				if (v) m_content_length = atoi(v);
				v = m_get->getheader("Accept-Ranges");
				if (v) while (v && *v)
					{
						if (!_strnicmp(v, "bytes", 5))
						{
							m_accept_ranges = 1;
							break;
						}
						v++;
					}
				v = m_get->getheader("icy-metaint");
				if (v)
				{
					m_meta_interval = atoi(v);
				}
				if (!m_title)
				{
					v = m_get->getheader("icy-name");
					if (v)
						m_title = _strdup(v);
				}
#ifdef WINAMP_PLUGIN
				extern void process_url(char *url);
				v = m_get->getheader("icy-url");
				if (v && !strstr(v, "shoutcast.com"))
				{
					char *p = (char *)v; while (p && *p && *p == ' ') p++;
					process_url(p);
				}
#endif

				v = m_get->getheader("content-type");
				if (v && !_stricmp(v, "misc/ultravox"))
				{
					v = m_get->getheader("ultravox-max-msg");
					if (v) m_is_uvox = atoi(v);
					if (!m_is_uvox) m_is_uvox = 16000;
				}
				if (!m_title)
				{
					v = m_get->getheader("content-disposition");
					if (v) v = strstr(v, "filename=");
					if (v)
					{
						m_title = _strdup(v + 9);
					}
				}
				m_read_headers = 1;
			}
		}

		size_t l = m_get->bytes_available();
		if (m_is_uvox)
		{
		again:
			if (l >= 6)
			{
				unsigned char buf[32768*2] = {0};
				m_get->peek_bytes((char *)buf, 6);
				if (buf[0] != 0x5A)
				{
					l--;
					m_get->get_bytes((char *)buf, 1);
					goto again;
				}
				int resqos = buf[1];
				int classtype = (buf[2] << 8) | buf[3];
				int msglen = (buf[4] << 8) | buf[5];
				if (msglen > m_is_uvox) // length is too long
				{
					m_get->get_bytes((char *)buf, 1);
					l--;
					goto again;
				}
				if (msglen + 7 <= (int)l)
				{
					m_uvox_enough_bytes = 1;

					m_get->peek_bytes((char *)buf, msglen + 7);
					if (buf[msglen + 6])
					{
						m_get->get_bytes((char *)buf, 1);
						l--;
						goto again;
					}
					if (classtype == 0x7777) // take any data for now, ignore all other frames
					{
						l = msglen - m_uvox_readpos;
						if (l > len) l = len;
						memcpy(buffer, buf + 6 + m_uvox_readpos, l);
						m_uvox_readpos += (int)l;

						if (m_uvox_readpos >= msglen)
						{
							m_uvox_readpos = 0;
							m_get->get_bytes((char *)buf, msglen + 7);

						}
						return l;

#ifdef WINAMP_PLUGIN

					}
					else if ( classtype == 0x3001 )
					{
						extern void process_metadata(char *buf, int size);
						m_get->get_bytes((char *)buf, msglen + 7);
						process_metadata((char*)buf + 6, msglen + 1);
#endif

					}
					else
					{
						m_get->get_bytes((char *)buf, msglen + 7);
					}
				}
				else
				{
					m_uvox_enough_bytes = 0;
				}
			}
			return 0;
		}
		else
		{
			if (l > len) l = len;
			m_get->get_bytes(buffer, (int)l);

			if (m_meta_interval)
			{
				int x = (int)l;
				unsigned char *buf = (unsigned char *)buffer;
				if (m_meta_size) // already in meta block
				{
					int len = min(x, m_meta_size - m_meta_buf_pos);

					memcpy(m_meta_buf + m_meta_buf_pos, buf, len);
					m_meta_buf_pos += len;

					if (m_meta_buf_pos == m_meta_size)
					{
						//	if(metacb) metacb->metaDataReader_onData(m_meta_buf,m_meta_size);
						m_meta_buf_pos = 0;
						m_meta_size = 0;
						m_meta_pos = 0;
					}

					x -= len;
					if (x) memcpy(buf, buf + len, x);
				}
				else if (m_meta_pos + x > m_meta_interval) // block contains meta data somewhere in it, and we're not alreayd reading a block
				{
					int start_offs = m_meta_interval - m_meta_pos;
					int len;
					m_meta_size = ((unsigned char *)buf)[start_offs] * 16;

					len = min(x - start_offs - 1, m_meta_size);

					if (len) memcpy(m_meta_buf, buf + start_offs + 1, len);
					m_meta_buf_pos = len;

					if (m_meta_buf_pos == m_meta_size) // full read of metadata successful
					{
						x -= m_meta_size + 1;
						if (x > start_offs) memcpy(buf + start_offs, buf + start_offs + 1 + m_meta_size, x - start_offs);
#ifdef WINAMP_PLUGIN
						extern void process_metadata(char *buf, int size);
						process_metadata(m_meta_buf, m_meta_size);
#endif
						//if(metacb) metacb->metaDataReader_onData(m_meta_buf,m_meta_size);
						m_meta_buf_pos = 0;
						m_meta_pos = -start_offs;
						m_meta_size = 0;
					}
					else
					{
						x = start_offs; // otherwise, there's only the first block of data
					}
				}
				if (x > 0)
				{
					m_meta_pos += x;
				}
				l = x;
			} // end of poopie metadata
		} // !uvox

#if 0
		{
			FILE *fh = fopen("c:\\dump.nsv", "ab");
			fwrite(buffer, 1, l, fh);
			fclose(fh);
		}
#endif

		return l;
	}
	return 0;
}

static void parseURL(char *url, char *lp, char *host, int *port, char *req)
{
	char *p, *np;
	/*  if (_strnicmp(url,"http://",4) &&
		    _strnicmp(url,"icy://",6) &&
		    _strnicmp(url,"sc://",6) &&
		    _strnicmp(url,"shoutcast://",12)) return;
	      */
	np = p = strstr(url, "://");
	if (!np) np = (char*)url;
	else np += 3;
	if (!p) p = (char*)url;
	else p += 3;

	while (np && *np != '/' && *np) *np++;
	if (np && *np)
	{
		lstrcpynA(req, np, 2048);
		*np++ = 0;
	}
	else strcpy(req, "/");
	np = p;
	while (np && *np != '@' && *np) np++;
	if (np && *np)
	{
		*np++ = 0;
		lstrcpynA(lp, p, 256);
		p = np;
	}
	else lp[0] = 0;
	np = p;
	while (np && *np != ':' && *np) np++;
	if (*np)
	{
		*np++ = 0;
		*port = atoi(np);
	}
	else *port = 80;
	lstrcpynA(host, p, 256);
}

int HTTPReader::getProxyInfo(const char *url, char *out)
{
#ifndef WINAMPX
	char INI_FILE[MAX_PATH] = {0};
	char *p;
	GetModuleFileNameA(NULL, INI_FILE, sizeof(INI_FILE));
	p = INI_FILE + strlen(INI_FILE);
	while (p >= INI_FILE && *p != '.') p--;
	strcpy(++p, "ini");
	GetPrivateProfileStringA("Winamp", "proxy", "", out, 8192, INI_FILE);
	return !!out[0];
#else
	DWORD v = 0;
	HKEY hKey;


	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\AOL\\Unagi", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		DWORD l = 4;
		DWORD t;
		if (RegQueryValueEx(hKey, "ProxyEnable", NULL, &t, (unsigned char *)&v, &l) == ERROR_SUCCESS && t == REG_DWORD)
		{
			if ( v != 2 )
			{
				l = 8192;
				if (RegQueryValueEx(hKey, "ProxyServer", NULL, &t, (unsigned char *)out, &l ) != ERROR_SUCCESS || t != REG_SZ)
				{
					v = 0;
					*out = 0;
				}
			}
			else return 0;
		}
		else v = 0;
		out[512 - 1] = 0;
		RegCloseKey(hKey);
	}
	if ( !v && m_useaproxy )
	{
		char blah[8192] = "";
		lstrcpyn(blah, url, 8192);
		char plp[512] = {0};
		char phost[512] = {0};
		int pport = 80;
		char pthereq[1024] = {0};
		parseURL(blah, plp, phost, &pport, pthereq);
		v = ResolvProxyFromURL(url, phost, out);
		if ( v < 0 ) v = 0;  // error getting proxy
	}
	if ( v > 0)
	{
		char prox[1024] = {0};
		wsprintf(prox, "PROXY: %s", out);
		SendMetadata(prox, 1);
	}
	return v;
#endif
}



class Win32FileReader : public IDataReader
{
public:
	Win32FileReader(HANDLE file) { m_hFile = file; m_eof = 0; m_err = NULL; }
	~Win32FileReader() { CloseHandle(m_hFile); }
	size_t read(char *buf, size_t len)
	{
		DWORD ob = 0;
		if (!len) return 0;
		if (!ReadFile(m_hFile, buf, (DWORD)len, &ob, NULL))
		{
			m_err = "Error calling ReadFile()!";
			return 0;
		}
		else if (!ob) m_eof = true;
		return ob;
	}
bool iseof() { return m_eof; }
	bool canseek() { return 1; }
	int seek(uint64_t newpos)
	{
		LARGE_INTEGER li;
		li.QuadPart = newpos;
		li.LowPart = SetFilePointer (m_hFile, li.LowPart, &li.HighPart, SEEK_SET);

		if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
		{
			li.QuadPart = -1;
		}

		return li.QuadPart== ~0;
	}

	uint64_t getsize()
	{
		LARGE_INTEGER position;
		position.QuadPart=0;
		position.LowPart = GetFileSize(m_hFile, (LPDWORD)&position.HighPart); 	

		if (position.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
			return INVALID_FILE_SIZE;
		else
			return position.QuadPart;
	}

	char *geterror() { return m_err; }
private:
	HANDLE m_hFile;
	bool m_eof;
	char *m_err;
};


#define VAR_TO_FPOS(fpos, var) (fpos) = (var)
#define FPOS_TO_VAR(fpos, typed, var) (var) = (typed)(fpos)
class FileReader : public IDataReader
{
public:
	FileReader(FILE *file) { fp = file;  m_err = NULL; }
	~FileReader() { fclose(fp); }
	size_t read(char *buf, size_t len)
	{
		size_t ob;
		if (!len) return 0;
		ob = fread(buf, 1, len, fp);
		if (!ob && ferror(fp))
		{
			m_err = "Error calling fread()!";
			return 0;
		}
		return ob;
	}
	bool iseof() { return !!feof(fp); }
	bool canseek() { return 1; }
	int seek(uint64_t newpos)
	{
		fpos_t pos= newpos;
		VAR_TO_FPOS(pos, newpos);
		return fsetpos(fp, &pos);
	}

	unsigned __int64 getsize()
	{
		struct stat s;
		if (fstat(fileno(fp), &s) < 0) 
		{ 
			m_err = "Error calling fread()!";
			return 0;
		}
		return s.st_size;
	}

	char *geterror() { return m_err; }
private:
	FILE *fp;
	char *m_err;
};


IDataReader *CreateReader(const char *url)
{
	if (strstr(url, "://")) return new HTTPReader(url);
#ifdef _WIN32
	HANDLE hFile = CreateFileA(url, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
	                          OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
		return new Win32FileReader(hFile);
#else
	FILE *fp = fopen(url, "r");
	if (fp)
		return new FileReader(fp);
#endif
	return NULL;
}

