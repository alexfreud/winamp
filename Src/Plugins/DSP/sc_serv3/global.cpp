#ifdef _WIN32
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#endif
#include <stdio.h>
#include "global.h"
#include "aolxml/aolxml.h"
#include "threadedRunner.h"
#include "w3cLog.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"
#ifndef _WIN32
#include <sys/time.h>
#endif

using namespace std;
using namespace uniString;
using namespace stringUtil;

config gOptions;

unsigned char appname_tmpbuf[] =
{
	(unsigned char)~'B', (unsigned char)~'a', (unsigned char)~'n', (unsigned char)~'a', (unsigned char)~'n', (unsigned char)~'a', (unsigned char)~'r', (unsigned char)~'a', (unsigned char)~'m', (unsigned char)~'a', 255, 0,
};

void tealike_crappy_code(unsigned long v[2], unsigned long k[4])
{
	unsigned long y = v[0], z = v[1], sum = 0,    /* set up */
	              delta = 0x9e3779b9UL, n = 32 ;  /* key schedule constant*/

	while (n-- > 0)
	{
		/* basic cycle start */
		sum += delta;
		y += ((z << 4) + k[0]) ^(z + sum) ^((z >> 5) + k[1]);
		z += ((y << 4) + k[2]) ^(y + sum) ^((y >> 5) + k[3]);   /* end cycle */
	}
	v[0] = y; v[1] = z;
}

const utf8 bob()
{
	static utf8 _bob;
	if (_bob.empty())
	{
		char* app_name = (char*)appname_tmpbuf;
		tealike_crappy_code((unsigned long *)(app_name += 12), (unsigned long *)appname_tmpbuf);
		for (int x = 0; x < 12; x ++)
		{
			appname_tmpbuf[x] ^= 255;
		}
		app_name -= 12;
		_bob = app_name;
	}
	return _bob;
}

static int gs_kill = false;
static int gs_postSetup = false;

const int iskilled() throw() { return gs_kill; }
void setkill(int v) throw()
{
	gs_kill = v;
	threadedRunner::wakeup();
}

const int isPostSetup() throw() { return gs_postSetup; }
void setPostSetup(int v) throw() { gs_postSetup = v; }

utf8 MSG_ICY_HTTP401;
utf8 MSG_ICY200;
utf8 MSG_ICY_HTTP200;
utf8 MSG_UVOX_HTTP200;

int MSG_ICY_HTTP401_LEN;

const utf8::value_type* MSG_INVALIDPASSWORD = (const utf8::value_type*)"Invalid Password\r\n";
const int MSG_INVALIDPASSWORD_LEN = (int)strlen(MSG_INVALIDPASSWORD);

const utf8::value_type* MSG_VALIDPASSWORD = (const utf8::value_type*)"OK2\r\nicy-caps:11\r\n\r\n";
const int MSG_VALIDPASSWORD_LEN = (int)strlen(MSG_VALIDPASSWORD);

const utf8::value_type* MSG_HTTP_VALIDPASSWORD = (const utf8::value_type*)"HTTP/1.1 200 OK\r\n\r\n";
const int MSG_HTTP_VALIDPASSWORD_LEN = (int)strlen(MSG_VALIDPASSWORD);

const utf8::value_type* MSG_STREAMMOVED = (const utf8::value_type*)"Stream Moved\r\n";
const int MSG_STREAMMOVED_LEN = (int)strlen(MSG_STREAMMOVED);

const utf8::value_type* MSG_BADSTREAMID = (const utf8::value_type*)"Bad Stream ID\r\n";
const int MSG_BADSTREAMID_LEN = (int)strlen(MSG_BADSTREAMID);

const utf8::value_type* MSG_STREAMINUSE = (const utf8::value_type*)"Stream In Use\r\n";
const int MSG_STREAMINUSE_LEN = (int)strlen(MSG_STREAMINUSE);

const utf8::value_type* MSG_200 = (const utf8::value_type*)"HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n";

const utf8::value_type* MSG_NO_CLOSE_200 = (const utf8::value_type*)"HTTP/1.1 200 OK\r\n"
															  "Content-Type:text/html;charset=utf-8\r\n";

const utf8::value_type* MSG_STD200 = (const utf8::value_type*)"HTTP/1.1 200 OK\r\n"
															  "Content-Type:text/html;charset=utf-8\r\n"
															  "Connection:close\r\n\r\n";

const utf8::value_type* MSG_HTTP400 = (const utf8::value_type*)("HTTP/1.1 400 Bad Request\r\nConnection:close\r\n\r\n");

const utf8::value_type* MSG_AUTHFAILURE401 = (const utf8::value_type*)
						("HTTP/1.1 401 Unauthorized\r\n"
						 "Connection:close\r\n"
						 "Server:Shoutcast DNAS\r\n"
						 "WWW-authenticate:basic realm=\"Shoutcast Server\"\r\n"
						 "Content-type:text/html;charset=utf-8\r\n\r\n");

const utf8::value_type* MSG_HTTP403 = (const utf8::value_type*)("HTTP/1.1 403 Service Forbidden\r\nConnection:close\r\n\r\n");

const utf8::value_type* MSG_HTTP404 = (const utf8::value_type*)("HTTP/1.1 404 Not Found\r\nConnection:close\r\n\r\n");
const int MSG_HTTP404_LEN = (int)strlen(MSG_HTTP404);

const utf8::value_type* MSG_HTTP405 = (const utf8::value_type*)("HTTP/1.1 405 Method Not Allowed\r\nAllow:GET\r\nConnection:close\r\n\r\n");

const utf8::value_type* MSG_HTTP503 = (const utf8::value_type*)("HTTP/1.1 503 Server limit reached\r\nConnection:close\r\n\r\n");
const int MSG_HTTP503_LEN = (int)strlen(MSG_HTTP503);

void constructMessageResponses()
{
	MSG_ICY_HTTP200 = utf8("HTTP/1.0 200 OK\r\n"
						   "icy-notice1:<BR>This stream requires "
						   "<a href=\"http://www.winamp.com\">Winamp</a><BR>\r\n"
						   "icy-notice2:Shoutcast DNAS/" SERV_OSNAME " v" +
						   gOptions.getVersionBuildStrings() + "<BR>\r\n"
						   "Accept-Ranges:none\r\n"
						   "Access-Control-Allow-Origin:*\r\n"
						   "Cache-Control:no-cache,no-store,must-revalidate,max-age=0\r\n"
						   "Connection:close\r\n");

	MSG_UVOX_HTTP200 = utf8("HTTP/1.0 200 OK\r\n"
							"Accept-Ranges:none\r\n"
							"Access-Control-Allow-Origin:*\r\n"
							"Cache-Control:no-cache,no-store,must-revalidate,max-age=0\r\n"
							"Connection:close\r\n"
							"Server:Ultravox/2.1 Shoutcast v" + gOptions.getVersionBuildStrings() + "/" SERV_OSNAME"\r\n" +
							"Content-Type:misc/ultravox\r\n");

	// this is only used for WMP which won't play the stream correctly when the in-stream
	// metadata is being provided and we use the now standard HTTP response (added 2.4.3)
	MSG_ICY200 = utf8("ICY 200 OK\r\n"
					  "icy-notice1:<BR>This stream requires "
					  "<a href=\"http://www.winamp.com\">Winamp</a><BR>\r\n"
					  "icy-notice2:Shoutcast DNAS/" SERV_OSNAME " v" +
					  gOptions.getVersionBuildStrings() + "<BR>\r\n"
					  "Connection:close\r\n");

	MSG_ICY_HTTP401 = utf8("HTTP/1.0 401 Unauthorized\r\n"	// Service Unavailable
						   "icy-notice1:<BR>Shoutcast DNAS/" SERV_OSNAME " v" +
						   gOptions.getVersionBuildStrings() + "<BR>\r\n"
						   "icy-notice2:The resource requested is currently unavailable<BR>\r\n"
						   "Connection:close\r\n\r\n");
	MSG_ICY_HTTP401_LEN = (int)MSG_ICY_HTTP401.size();
}

utf8 g_userAgentBase = "Ultravox/2.1 ""Shoutcast Server "/**/; // user agent for sc_serv2
															   // comment out for testing
utf8 g_userAgent;
time_t g_upTime = 0;

const bool isUserAgentRelay(const utf8 &user_agent) throw()
{
	if (!user_agent.empty())
	{
		return ((user_agent.find(utf8("shoutcast")) != utf8::npos) &&
				(user_agent != utf8("shoutcast directory tester")) &&
				(user_agent != utf8("relay")) &&
				(user_agent != utf8("icecast")));
	}
	return false;
}

const bool isUserAgentOfficial(const utf8 &user_agent) throw()
{
	if (!user_agent.empty())
	{
		return ((user_agent == utf8("shoutcast directory tester")) ||
				(user_agent.find(utf8("shoutcast-to-dnas message sender")) == 0));
	}
	return false;
}

const utf8 redirect(const utf8 &url, const bool compress) throw()
{
	utf8 header = "HTTP/1.1 302 Found\r\n"
				  "Content-Type:text/html;charset=utf-8\r\n"
				  "Location:" + url + "\r\n";
	utf8 body = "<html><head><title>Redirect</title>"
				"</head><body>Click <a href=\"/" + url + "\">here</a> for redirect.</body>"
				"</html>";
	if (compress && compressData(body))
	{
		header += "Content-Encoding:gzip\r\n";
	}
	header += "Content-Length:" + tos(body.size()) + "\r\n\r\n";
	return header + body;
}

const utf8 urlLink(const utf8 &url, const utf8 &text, const bool embed) throw()
{
	// so the pages have a better chance of wrapping
	// we'll attempt to insert <wbr> to give a hint.
	utf8 wbrUrl = stripHTTPprefix((!text.empty() ? text : url)), fixedUrl;
	vector<utf8> parts = tokenizer(wbrUrl, '/');
	for (vector<utf8>::const_iterator i = parts.begin(); i != parts.end(); ++i)
	{
		fixedUrl += (i == parts.begin() ? "" : "/<wbr>") + aolxml::escapeXML((*i));
	}

	wbrUrl.clear();
	parts = tokenizer(fixedUrl, '.');
	for (vector<utf8>::const_iterator i = parts.begin(); i != parts.end(); ++i)
	{
		wbrUrl += (i == parts.begin() ? "" : ".<wbr>") + (*i);
	}

	return "<a target=" + (embed ? "\\" : (utf8)"") + "\"_blank" +
		   (embed ? "\\" : "") + "\" href=" + (embed ? "\\" : "") + "\"" +
		   utf8((url.find(utf8("://")) == utf8::npos) ? "http://" : "") +
		   url + (embed ? "\\" : "") + "\">" + wbrUrl + "</a>";
}

const utf8 http302(const utf8 &url) throw()
{
	return "HTTP/1.1 302 Found\r\n"
		   "Content-Type:text/html;charset=utf-8\r\n"
		   "Location:" + url + "\r\n\r\n"
		   "<html><head><title>Redirect</title></head><body>Click <a href=\"" + url + "\">HERE</a> for redirect.</body></html>";
}

const utf8 addWBR(const utf8 &str) throw()
{
	if (!str.empty())
	{
		// this allows browsers to word break the string on small displays
		// and is aimed for user agent strings, hence / and ( to break on
		utf8 wbrIdent, fixedIdent;
		vector<utf8> parts = tokenizer(str, '/');
		for (vector<utf8>::const_iterator i = parts.begin(); i != parts.end(); ++i)
		{
			wbrIdent += (i == parts.begin() ? "" : "/<wbr>") + aolxml::escapeXML((*i));
		}

		parts = tokenizer(wbrIdent, '(');
		for (vector<utf8>::const_iterator i = parts.begin(); i != parts.end(); ++i)
		{
			fixedIdent += (i == parts.begin() ? "" : "(<wbr>") + (*i);
		}

		return fixedIdent;
	}
	return str;
}

utf8 getfooterStr()
{
	return "</body></html>";
}

const utf8 randomId(utf8 &temp)
{
	temp.clear();

	// construct a secondary temp id for this
	// action for use after the check happened
	for (int i = 0; i < 8; i++)
	{
		temp.append(utf8(tos(rand() % 10)));
	}
	return temp;
}

utf8 getStreamListeners(const size_t sid, const bool nowrap, const int fh)
{
	return "<script type=\"text/javascript\">" EL
		   "function $(id){return document.getElementById(id);}" EL
		   "var registerOnWindowLoad=function(callback){" EL
		   "if(window.addEventListener){" EL
		   "window.addEventListener('load',callback,false);" EL
		   "}else{" EL
		   "window.attachEvent('onload',callback);" EL
		   "}" EL
		   "};" EL
		   "function getHTTP(){" EL
		   "if(window.XDomainRequest){" EL
		   "return new XDomainRequest();" EL
		   "}else if(window.XMLHttpRequest){" EL
		   "return new XMLHttpRequest();" EL
		   "}else{" EL
		   "return new ActiveXObject(\"Microsoft.XMLHTTP\");" EL
		   "}" EL
		   "}" EL
		   "function getListeners(){" EL
		   "var xmlhttp = getHTTP();" EL
		   "try{" EL
		   "xmlhttp.open(\"GET\",\"admin.cgi?sid="+tos(sid)+"&mode=listeners&server="+randomId(listenerId)+"&nw="+tos(nowrap)+"&fh="+tos(fh)+"\",true);" EL
		   "if(window.XDomainRequest){" EL
		   "xmlhttp.onload=function(){$('listeners').innerHTML=xmlhttp.responseText;};" EL
		   "xmlhttp.onerror=function(){$('listeners').innerHTML=xmlhttp.responseText;};" EL
		   "}else{" EL
		   "xmlhttp.onreadystatechange=function(){$('listeners').innerHTML=xmlhttp.responseText;};" EL
		   "}" EL
		   "xmlhttp.send(null);" EL
		   "}" EL
		   "catch(e){" EL
		   "$('listeners').innerHTML=e;" EL
		   "}" EL
		   "}" EL
		   "registerOnWindowLoad(getListeners());" EL
		   "</script>";
}

utf8 getHTML5Remover()
{
	return "<script type=\"text/javascript\">" EL
		   "var registerOnWindowLoad=function(callback){" EL
		   "if(window.addEventListener){" EL
		   "window.addEventListener('load',callback,false);" EL
		   "}else{" EL
		   "window.attachEvent('onload',callback);" EL
		   "}" EL
		   "};" EL
		   "registerOnWindowLoad(function(){" EL
		   "var audio=document.getElementsByTagName(\"audio\");" EL
		   "if(audio.length){" EL
		   "var canPlay = !!audio[0].canPlayType && audio[0].canPlayType('audio/mpeg; codecs=\"mp3\"') != \"\";" EL
		   "if(!canPlay){" EL
		   "for(var i=audio.length-1;i>=0;i--){" EL
		   "var parent = audio[i].parentNode;" EL
		   "parent.removeChild(audio[i]);" EL
		   "var parent2 = parent.parentNode;" EL
		   "parent2.removeChild(parent);" EL
		   "}" EL
		   "}" EL
		   "}" EL
		   "});" EL
		   "</script>";
}

utf8 getIEFlexFix()
{
	return "<script type=\"text/javascript\">" EL
		   "function $(id){return document.getElementById(id);}" EL
		   "var registerOnWindowLoad=function(callback){" EL
		   "if(window.addEventListener){" EL
		   "window.addEventListener('load',callback,false);" EL
		   "}else{" EL
		   "window.attachEvent('onload',callback);" EL
		   "}" EL
		   "};" EL
		   "registerOnWindowLoad(function(){" EL
		   "if(navigator.userAgent.match(/msie|trident/i)){" EL
  		   "$('hdrbox').style.display = 'block';" EL
		   "}" EL
		   "});" EL
		   "</script>";
}

utf8 g_IPAddressForClients;		// address clients will connect to
u_short g_portForClients = 0;	// port clients will connect to, generally portBase
int g_legacyPort = -1;			// port legacy v1 sources will connect to, or not

// attempt to compress everything in one go though
// only use if its smaller than the original data.
const bool compressData(utf8 &body)
{
	if (!body.empty())
	{
		z_stream stream = {0};

		const uInt size = (uInt)body.size();
		stream.next_in = (Bytef*)body.data();
		stream.avail_in = size;

		char *m_outMsg = new char[size * 2];
		stream.next_out = (Bytef*)m_outMsg;
		stream.avail_out = size * 2;

		// set windowBits to 31 to allow gzip encoded output
		if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY) == Z_OK)
		{
			int ret = deflate(&stream, Z_FINISH);
			deflateEnd(&stream);
			if (ret == Z_STREAM_END)
			{
				// if it's bigger than we started with then there's no
				// sensible reason to return the compressed data block
				if (stream.total_out < size)
				{
					body = utf8(m_outMsg, stream.total_out);
					delete [] m_outMsg;
					return true;
				}
			}
		}
		delete [] m_outMsg;
	}
	return false;
}

const bool compressDataStart(utf8 &body, z_stream *stream, Bytef* name, const bool local)
{
	if (!stream || body.empty())
	{
		return false;
	}

    stream->zalloc = Z_NULL;
    stream->zfree = Z_NULL;
    stream->opaque = Z_NULL;

	// set windowBits to 31 to allow gzip encoded output
	if (deflateInit2(stream, (local ? Z_BEST_COMPRESSION : Z_DEFAULT_COMPRESSION), Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY) != Z_OK)
	{
        return false;
	}

	if (local)
	{
		gz_header head = {0};
		head.text = 1;
		head.name = head.comment = name;
		head.time = (uLong)::time(NULL);
		deflateSetHeader(stream, &head);
	}

	return (compressDataCont(body, stream) != 0);
}

const int compressDataCont(utf8 &body, z_stream *stream)
{
	#define CHUNK 1024
	static utf8 body2;
	body2.clear();
	static unsigned char out[CHUNK] = {0};
	int ret = Z_OK;
	uInt start = (uInt)body.size(), have = 0, pos = 0;

    do
	{
		if (start - pos > 0)
		{
			if (have == 0)
			{
				stream->next_in = (Bytef*)body.data();
				pos += (stream->avail_in = min(CHUNK, (int)start));
				have = start - stream->avail_in;
			}
			else
			{
				stream->next_in = (Bytef*)&body[pos];
				pos += (stream->avail_in = min(CHUNK, (int)have));
				have = have - stream->avail_in;
			}
		}

        do
		{
            stream->avail_out = CHUNK;
            stream->next_out = out;
			ret = deflate(stream, Z_PARTIAL_FLUSH);
			body2 += utf8(out, (CHUNK - stream->avail_out));
			if (!stream->avail_in)
			{
				break;
			}
        } while (stream->avail_out == 0);
	} while (ret != Z_BUF_ERROR);

	if (!body2.empty())
	{
		body = body2;
	}
	return stream->total_out;
}

void compressDataFinish(utf8 &body, z_stream *stream)
{
	if (stream)
	{
		#define CHUNK 1024
		static utf8 body2;
		body2.clear();
		static unsigned char out[CHUNK] = {0};
		stream->avail_out = CHUNK;
		stream->next_out = out;
		deflate(stream, Z_FINISH);
		body2 = utf8(out, (CHUNK - stream->avail_out));
		if (!body2.empty())
		{
			body = body2;
		}
	}
}

const bool compressDataEnd(z_stream *stream)
{
	return (stream && (deflateEnd(stream) == Z_OK));
}

const utf8 loadLocalFile(uniFile::filenameType fn, const utf8& logPrefix, const size_t sizeLimit)
{
	utf8 body;
	if (!fn.empty())
	{
		size_t fileSize = uniFile::fileSize(fn);
		if (fileSize && (!sizeLimit || fileSize <= sizeLimit))
		{
			FILE *f = uniFile::fopen(fn, "rb");
			if (f)
			{
				size_t alloc = fileSize + 1;
				utf8::value_type *buf = new utf8::value_type[alloc];
				memset(buf, 0, sizeof(utf8::value_type) * alloc);
				::fread(buf, 1, fileSize, f);
				::fclose(f);

				body = utf8(buf,fileSize);
				delete [] buf;
			}
		}
		else if (fileSize)
		{
			WLOG(logPrefix + "Unable to load `" + fn + "' as it is over the allowed size limit (" + tos(sizeLimit) + " bytes)");
		}
	}
	return body;
}

#ifdef _WIN32
static const char *abb_weekdays[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    NULL
};

static const char *abb_month[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
    NULL
};

/*
 * tm_year is relative this year 
 */
const int tm_year_base = 1900;

inline int match_string(const char **buf, const char **strs)
{
    for (int i = 0; (strs && strs[i] != NULL); ++i) {
		size_t len = (strs && strs[i] ? strlen(strs[i]) : 0);
		if (len && strncmp(*buf, strs[i], len) == 0) {
			*buf += len;
			return i;
		}
    }
    return -1;
}

// stripped down version which only processes against '%a, %d %b %Y %H:%M:%S'
// which is only used on the Windows builds as it's native for other targets
char *strptime(const char *buf, const char *format, struct tm *timeptr)
{
    char c;

    for (; (c = *format) != '\0'; ++format)
	{
		char *s = 0;

		if (isspace (c))
		{
			while (isspace (*buf)) ++buf;
		}
		else if (c == '%' && format[1] != '\0')
		{
			int ret;
			c = *++format;
			if (c == 'E' || c == 'O') c = *++format;
			switch (c)
			{
				case 'a':
				{
					ret = match_string (&buf, abb_weekdays);
					if (ret < 0) return NULL;
					timeptr->tm_wday = ret;
					break;
				}
				case 'b':
				{
					ret = match_string (&buf, abb_month);
					if (ret < 0) return NULL;
					timeptr->tm_mon = ret;
					break;
				}
				case 'd':
				{
					ret = strtol (buf, &s, 10);
					if (s == buf) return NULL;
					timeptr->tm_mday = ret;
					buf = s;
					break;
				}
				case 'H':
				{
					ret = strtol (buf, &s, 10);
					if (s == buf) return NULL;
					timeptr->tm_hour = ret;
					buf = s;
					break;
				}
				case 'M':
				{
					ret = strtol (buf, &s, 10);
					if (s == buf) return NULL;
					timeptr->tm_min = ret;
					buf = s;
					break;
				}
				case 'S':
				{
					ret = strtol (buf, &s, 10);
					if (s == buf) return NULL;
					timeptr->tm_sec = ret;
					buf = s;
					break;
				}
				case 'Y':
				{
					ret = strtol (buf, &s, 10);
					if (s == buf) return NULL;
					timeptr->tm_year = ret - tm_year_base;
					buf = s;
					break;
				}
				case '\0':
				{
					--format;
					/* FALLTHROUGH */
				}
				case '%':
				{
					if (*buf == '%') ++buf;
					else return NULL;
					break;
				}
				default:
				{
					if (*buf == '%' || *++buf == c) ++buf;
					else return NULL;
					break;
				}
			}
		}
		else
		{
			if (*buf == c) ++buf;
			else return NULL;
		}
    }
    return (char *)buf;
}
#endif

const time_t readRFCDate(const utf8& str)
{
	struct tm tmdate = {0};
	if (strptime(str.toANSI().c_str(), "%a, %d %b %Y %H:%M:%S", &tmdate))
	{
		#ifdef _WIN32
		return _mkgmtime(&tmdate);
		#else
		return timegm(&tmdate);
		#endif
	}
	return 0;
}

const utf8 getRFCDate(const time_t use)
{
	char buf[1024] = {0};
	const time_t now = ::time(NULL);
	struct tm tm = *gmtime((use ? &use : &now));
	const size_t size = strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm);
	return (size > 0 ? utf8(buf, size) : (utf8)"");
}

const utf8 timeString(const time_t t, const bool slim) throw()
{
	__int64 sec = t;
	__int64 min = sec / 60;
	sec -= min * 60;
	__int64 hours = min / 60;
	min -= hours * 60;

	if (slim)
	{
		char buf[24] = {0};
		snprintf(buf, sizeof(buf), "%02lld:%02u:%02u", hours, (unsigned int)min, (unsigned int)sec);
		return buf;
	}
	else
	{
		utf8 result;
		__int64 days = hours / 24;
		hours -= days * 24;
		const __int64 years = days / 365;
		days -= years * 365;

		if (years) result += tos(years) + " year" + (years != 1 ? "s" : "") + " ";
		if (days) result += tos(days) + " day" + (days != 1 ? "s" : "") + " ";
		if (hours) result += tos(hours) + " hour" + (hours != 1 ? "s" : "") + " ";
		if (min) result += tos(min) + " minute" + (min != 1 ? "s" : "") + " ";
		if (sec) result += tos(sec) + " second" + (sec != 1 ? "s" : "");
		return stripWhitespace(result);
	}
}

class jsonEscapes: public map<char,std::string>
{
public:
	jsonEscapes()
	{  		
		(*this)['\\'] = "\\\\";
		(*this)['/'] = "\\/";
		(*this)['\"'] = "\\\"";
		(*this)['\b'] = "\\b";
		(*this)['\f'] = "\\f";
		(*this)['\n'] = "\\n";
		(*this)['\r'] = "\\r";
		(*this)['\t'] = "\\t";
	}
};

static const jsonEscapes gsJSONEscapes;

const utf8 escapeJSON(const utf8 &s) throw()
{
	string result;
	const string::size_type siz = s.size();
	for (string::size_type x = 0; x < siz; ++x)
	{
		jsonEscapes::const_iterator i = gsJSONEscapes.find(s[x]);
		if (i != gsJSONEscapes.end())
		{
			result += (*i).second;
		}
		else
		{
			result += s[x];
		}
	}
	return result;
}

void rotatew3cFiles(utf8 files)
{
	// w3c logging (global)
	if (gOptions.w3cEnable())
	{
		if (files == "w3c" || files == "")
		{
			w3cLog::rotate_log(gOptions.w3cLog());
		}
	}

	// w3c logging (per stream)
	if (files == "w3c" || files == "")
	{
		config::streams_t streams;
		gOptions.getStreamConfigs(streams);
		for (config::streams_t::const_iterator i = streams.begin(); i != streams.end(); ++i)
		{
			if (gOptions.read_stream_w3cLog((*i).first))
			{
				w3cLog::rotate_log(gOptions.stream_w3cLog((*i).first),(*i).first);
			}
		}
	}
}

utf8 &fixSongMetadata(utf8& metadata, int& trigger)
{
    utf8::size_type pos = metadata.find((unsigned char *)"Advert:", 0, 7);

    if (pos == utf8::npos)
        pos = metadata.find((unsigned char *)"Advert!", 0, 7);

    if (!metadata.empty() && (pos == 0))
    {
        trigger = 1;

        // got a first matching block
        metadata.erase (0, 7);

        // look for an end block
        metadata = stripWhitespace(metadata);

        // check for it being empty after stripping out non-alpha
        // characters so we can show what is sent to the listener
        if (stripAlphaDigit(metadata).empty())
        {
            metadata.clear();
        }
    }
    return metadata;
}

const utf8 getCurrentSong(utf8 currentSong)
{
	int trigger = 0;
	utf8 &song = fixSongMetadata(currentSong, trigger);
	if (trigger)
	{
		if (song.empty())
		{
			song = "<i>" + utf8(trigger == 2 ? "Test " : "") + "Advert Trigger</i>";
		}
		else
		{
			song = "<i>" + utf8(trigger == 2 ? "Test " : "") + "Advert Trigger:&nbsp;&nbsp;</i>" + aolxml::escapeXML(song) + "</i>";
		}
	}
	else
	{
		if (song.empty())
		{
			song = "<i>Empty Title</i>";
		}
		else
		{
			song = aolxml::escapeXML(song);
		}
	}

	return song;
}

const utf8 stripHTTPprefix(const utf8& addr)
{
	if (!addr.empty())
	{
		utf8::size_type pos = addr.find(utf8("://"));
		if (pos != utf8::npos)
		{
			return addr.substr(pos + 3);
		}
	}
	return addr;
}

const bool isAddress(const utf8& addr)
{
	if (!addr.empty())
	{
		if (addr.find(utf8(".")) != utf8::npos)
		{
			return true;
		}
	}
	return false;
}

const bool extractPassword(utf8 &dj_password, utf8 &dj_name, int &streamID)
{
	if (!dj_password.empty())
	{
		const vector<utf8> tokens = tokenizer(dj_password, ':');
		// if 2 or 3 then we've got user:password or password:#sid or
		// user:password:#sid so we need to check for which version
		if (tokens.size() >= 2)
		{
			if (tokens[1].size() > 0)
			{
				// this is user:password:#sid or user:password
				if (tokens[1].find(utf8("#")) != 0)
				{
					dj_password = tokens[1];
					dj_name = tokens[0];

					if (tokens.size() == 3)
					{
						if (tokens[2].size() > 1)
						{
							if (tokens[2].find(utf8("#")) == 0)
							{
								streamID = atoi((const char *)tokens[2].c_str()+1);
							}
						}
					}
				}
				// this could be password:#sid
				else
				{
					streamID = atoi((const char *)tokens[1].c_str()+1);
					dj_password = tokens[0];
				}
				return true;
			}
		}
	}
	return false;
}

#ifdef _WIN32

const utf8 errMessage() throw()
{
	LPVOID lpMsgBuf = NULL;
	::FormatMessageW( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
	);
	if (lpMsgBuf)
	{
		utf32 u32((const wchar_t*)lpMsgBuf);
		const utf8 result = stripWhitespace(u32.toUtf8());
		::LocalFree(lpMsgBuf);
		return result;
	}
	return "Unknown error";
}
	
#else
#include <string>
#include <ctype.h>
#include "stl/stringUtils.h"
#include <errno.h>

const utf8 errMessage() throw()
{
	const int e = errno;
	const utf8 b = strerror(e);
	return (!b.empty() ? utf8(b) : tos(e));
}

#endif

#ifdef _WIN32
#define _WS2DEF_
#define _WINSOCK2API_
#endif
#include "threadedRunner.h"
const int getStreamBitrate(const httpHeaderMap_t &headers)
{
	const int bitrate = mapGet(headers, "icy-br", 0);
	if (!bitrate)
	{
		// try to find a bitrate if not provided by looking at the possible Icecast info header
		const utf8 info = mapGet(headers, "ice-audio-info", utf8());
		if (!info.empty())
		{
			vector<utf8> blocks = tokenizer(info, ';');
			for (vector<utf8>::const_iterator i = blocks.begin(); i != blocks.end(); ++i)
			{
				vector<utf8> pairs = tokenizer((*i), '=');
				if (pairs.size() == 2)
				{
					utf8 key = toLower(stripWhitespace(pairs[0]));
					if (!key.empty())
					{
						// should just be "ice-bitrate" but seen some servers use "bitrate"
						if ((key == "bitrate") || (key == "ice-bitrate"))
						{
							return atoi((const char *)stripWhitespace(pairs[1]).c_str());
						}
					}
				}
			}
		}
	}
	return bitrate;
}

const int getStreamSamplerate(const httpHeaderMap_t &headers)
{
	const int samplerate = mapGet(headers, "icy-sr", 0);
	if (!samplerate)
	{
		// try to find a bitrate if not provided by looking at the possible Icecast info header
		const utf8 info = mapGet(headers, "ice-audio-info", utf8());
		if (!info.empty())
		{
			vector<utf8> blocks = tokenizer(info, ';');
			for (vector<utf8>::const_iterator i = blocks.begin(); i != blocks.end(); ++i)
			{
				vector<utf8> pairs = tokenizer((*i), '=');
				if (pairs.size() == 2)
				{
					utf8 key = toLower(stripWhitespace(pairs[0]));
					if (!key.empty())
					{
						if (key == "samplerate")
						{
							return atoi((const char *)stripWhitespace(pairs[1]).c_str());
						}
					}
				}
			}
		}
	}
	return samplerate;
}

const int getHTTPRequestDetails(const string &firstLine, string &request, string &url, string &protocolAndVersion)
{
	const vector<string> parts = tokenizer(firstLine, ' ');
	if (!parts.empty())
	{
		int state = 0, partsCount = (int)parts.size();
		switch (partsCount)
		{
			case 3:
			{
				request = parts[0];
				url = parts[1];
				protocolAndVersion = parts[2];
				state = partsCount;
			}
			break;
			case 2:
			{
				request = parts[0];
				url = parts[1];
				// this allows things like old v1 sc_trans to send
				// title updates without failing (like v1 allowed)
				protocolAndVersion = "HTTP/1.0";
				state = 3;
			}
			break;
			case 1:
			{
				request = parts[0];
				state = partsCount;
			}
			break;
			default:
			{
				// if we're here then it's likely that it's an
				// incorrectly encoded request and we'll need
				// to re-combine to make the 'url' not cludge
				// the expected data for 'protocolAndVersion'.
				request = parts[0];
				for (int i = 1; i < partsCount - 1; i++)
				{
					url += (i != 1 ? " " : "") + parts[i];
				}
				protocolAndVersion = parts[partsCount - 1];
				state = 3;
			}
			break;
		}
		return state;
	}
	return 0;
}

const utf8 fixMimeType(const utf8& mimeType)
{
	if (mimeType.empty() || (mimeType == "mp3") || (mimeType == "audio/mp3"))
	{
		return "audio/mpeg";
	}
	if (mimeType == "audio/aac")
	{
		return "audio/aacp";
	}
	return mimeType;
}

const int detectAutoDumpTimeout(time_t &cur_time, const time_t lastActivityTime,
								const utf8& msg, const bool debug,
								const size_t streamID) throw(runtime_error)
{
	const int autoDumpTime = gOptions.getAutoDumpTime(streamID);
	cur_time = ::time(NULL);
	if ((autoDumpTime > 0) && ((cur_time - lastActivityTime) >= autoDumpTime))
	{
		throwEx<runtime_error>((debug ? (msg + " (" + tos(cur_time) + " " +
							   tos(lastActivityTime) + " [" +
							   tos(cur_time - lastActivityTime) + "])") : (utf8)""));
	}
	return autoDumpTime;
}

const utf8 sampleRateStr(const int sr)
{
	switch (sr)
	{
		case 88200:
		{
			return "88.2 kHz";
		}
		case 44100:
		{
			return "44.1 kHz";
		}
		case 22050:
		{
			return "22.05 kHz";
		}
		case 11025:
		{
			return "11.025 kHz";
		}
		case 7350:
		{
			return "7.35 kHz";
		}
		default:
		{
			// 96, 64, 48, 32, 24, 16, 12, 8
			return (sr > 0 ? tos(sr / 1000) : "unknown") + " kHz";
		}
	}
}

const __uint64 time_now_ms()
{
#ifdef _WIN32
	FILETIME ft = {0};
    ::GetSystemTimeAsFileTime(&ft);

	__uint64 t = ((__uint64)(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;

	t -= 116444736000000000LL;	// convert epoch as there's this
								// many 100ns between 1601 & 1970
	t /= 10000;	// convert to milliseconds as this is
				// based on the number of 100-nanosecond
				// intervals since January 1, 1601 (UTC)
	return t;
#else
	struct timeval now;
	gettimeofday(&now, NULL);
	return (((__uint64)now.tv_sec) * 1000) + (now.tv_usec / 1000);
#endif
}

const bool isRemoteAddress(const utf8 &addr)
{
	return (!addr.empty() &&
			(addr.find(utf8("127.")) != 0) &&
			(addr.find(utf8("10.")) != 0) &&
			(addr.find(utf8("192.168.")) != 0) &&
			(addr.find(utf8("192.0.0.")) != 0) &&
			(addr.find(utf8("198.18.")) != 0) &&
			(addr.find(utf8("198.19.")) != 0) &&
			(toLower(addr).find(utf8("localhost")) != 0));
}
