/***************************************************************************\
*
*               (C) copyright Fraunhofer - IIS (1998)
*                        All Rights Reserved
*
*   filename: giofile.cpp
*   project : MPEG Decoder
*   author  : Martin Sieler
*   date    : 1998-02-11
*   contents/description: file I/O class for MPEG Decoder
*
*
\***************************************************************************/

/* ------------------------ includes --------------------------------------*/

#include "main.h"
#include "api__in_mp3.h"
#include <time.h>
#include <locale.h>
#include "../Winamp/wa_ipc.h"

#include "LAMEinfo.h"
#include "OFL.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"

#include <api/service/waservicefactory.h>
#include <shlwapi.h>
#include "MP3Info.h"
#include "config.h"
#include <math.h>
#include "id3.h"
#include "../apev2/header.h"
#include "uvox_3902.h"
#include <foundation/error.h>
#include <strsafe.h>

#define MAX_REDIRECTS 10

#define SAFE_MALLOC_MATH(orig, x) (((orig)<x)?x:orig)
// seems like a reasonable limit, but we should check with tag first
#define UVOX_MAXMSG_CAP 1048576

static jnl_connection_t CreateConnection(const char *url, jnl_dns_t dns, size_t sendbufsize, size_t recvbufsize)
{
		if (!_strnicmp(url, "https:", strlen("https:")))
			return jnl_sslconnection_create(dns, sendbufsize, recvbufsize);
		else
			return jnl_connection_create(dns, sendbufsize, recvbufsize);
}

static int64_t Seek64(HANDLE hf, int64_t distance, DWORD MoveMethod)
{
	LARGE_INTEGER li;
	li.QuadPart = distance;
	li.LowPart = SetFilePointer (hf, li.LowPart, &li.HighPart, MoveMethod);
	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		li.QuadPart = -1;
	}
	return li.QuadPart;
}

HWND GetDialogBoxParent()
{
	HWND parent = (HWND)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETDIALOGBOXPARENT);
	if (!parent || parent == (HWND)1)
		return mod.hMainWindow;
	return parent;
}

/*-------------------------- defines --------------------------------------*/

/*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//
//                   CGioFile
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   constructor
//-------------------------------------------------------------------------*


static char ultravoxUserAgent[128] = "";
char *GetUltravoxUserAgent()
{
	if (!ultravoxUserAgent[0])
	{
		StringCchPrintfA(ultravoxUserAgent, 128, "User-Agent: WinampMPEG/%01x.%02x, Ultravox/2.1\r\n"
			"Ultravox-transport-type: TCP\r\n",
			WINAMP_VERSION_MAJOR(winampVersion),
			WINAMP_VERSION_MINOR(winampVersion));
	}
	return ultravoxUserAgent;
}

static char userAgent[128] = "";
char *GetUserAgent()
{
	if (!userAgent[0])
	{

		StringCchPrintfA(userAgent, 128, "User-Agent: WinampMPEG/%01x.%02x\r\n",
			WINAMP_VERSION_MAJOR(winampVersion),
			WINAMP_VERSION_MINOR(winampVersion));
	}
	return userAgent;
}

extern int lastfn_status_err;

CGioFile::CGioFile()
{
	req=0;
	proxy_host=0;
	host=0;
	request=0;
	proxy_lp=0;
	lpinfo=0;
	mpeg_length=0;
	file_position=0;
	mpeg_position=0;
	no_peek_hack=false;
	encodingMethod=0;
	uvox_3901=0;
	uvox_3902=0;
	stream_url[0]=0;
	stream_genre[0]=0;
	stream_current_title[0]=0;
	stream_name[0]=0;
	ZeroMemory(&last_write_time, sizeof(last_write_time));
	ZeroMemory(id3v1_data, sizeof(id3v1_data));
	lengthVerified=false;
	m_vbr_bytes = 0;
	uvox_last_message = 0;
	prepad = 0;
	postpad = 0;
	m_vbr_ms = 0;
	m_vbr_hdr = 0;
	stream_id3v2_buf = 0;
	lyrics3_size=0;
	lyrics3_data=0;
	apev2_data=0;
	m_content_type = 0;
	ZeroMemory(uvox_meta, sizeof(uvox_meta));
	is_uvox = 0;
	uvox_sid = uvox_maxbr = uvox_avgbr = 0;
	uvox_message_cnt = uvox_desyncs = 0;
	uvox_stream_data	 = 0;
	uvox_stream_data_len = 0;
	ZeroMemory(&uvox_artwork, sizeof(uvox_artwork));
	uvox_maxmsg = 65535;
	force_lpinfo[0] = 0;
	is_stream_seek = 0;
	last_full_url[0] = 0;
	m_is_stream = 0;
	m_redircnt = 0;
	m_auth_tries = 0;
	m_full_buffer = NULL;
	fEof = false;
	m_connection = NULL;
	m_dns = NULL;
	hFile = INVALID_HANDLE_VALUE;
	m_http_response = 0;
	m_seek_reset = false;
	m_is_stream_seek = false;
	m_is_stream_seekable = false;
	initTitleList();
	m_vbr_frames = 0;
	ZeroMemory(&m_vbr_toc, sizeof(m_vbr_toc));
	m_vbr_frame_len = 0;
	m_vbr_flag = 0;
	m_id3v2_len = 0;
	m_id3v1_len = 0;
	port = 80;
	constate = 0;
	ZeroMemory(&save_filename, sizeof(save_filename));
	ZeroMemory(&server_name, sizeof(server_name));
	recvbuffersize = 32768;

	timeout_start = GetTickCount();
	meta_interval = 0;
	meta_pos = 0;
	ZeroMemory(&stream_title_save, sizeof(stream_title_save));
	ZeroMemory(&last_title_sent, sizeof(last_title_sent));
	ZeroMemory(&dlg_realm, sizeof(dlg_realm));
	m_useaproxy = 0;
}

//-------------------------------------------------------------------------*
//   destructor
//-------------------------------------------------------------------------*

CGioFile::~CGioFile()
{
	int x, y;
	for (x = 0; x < 2; x ++)
		for (y = 0; y < 32; y ++)
			free(uvox_meta[x][y]);
	free(m_content_type);
	free(uvox_stream_data);
	free(stream_id3v2_buf);
	free(lyrics3_data);
	free(apev2_data);
	free(uvox_3901);
	free(uvox_3902);

	Close();
	delete m_vbr_hdr;
	m_vbr_hdr = 0;
	clearTitleList();
	free(lpinfo);
	free(proxy_lp);
	free(request);
	free(host);
	free(req);
	free(proxy_host);
}

static bool GetRG(const char *key, ID3v2 &info, APEv2::Tag &apev2, wchar_t *val, int len)
{
	val[0]=0;
	if (info.GetString(key, val, len) == 1 && val[0])
		return true;
	if (apev2.GetString(key, val, len) == APEv2::APEV2_SUCCESS && val[0])
		return true;
	return false;
}

float CGioFile::GetGain()
{
	if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain", false))
	{
		//		if (info.HasData())
		{
			float dB = 0, peak = 1.0f;
			wchar_t gain[128] = L"", peakVal[128] = L"";
			_locale_t C_locale = WASABI_API_LNG->Get_C_NumericLocale();

			switch (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_source", 0))
			{
			case 0:  // track
				if ((GetRG("replaygain_track_gain", info, apev2, gain, 128) == false || !gain[0])
					&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
					GetRG("replaygain_album_gain", info, apev2, gain, 128);

				if ((GetRG("replaygain_track_peak", info, apev2, peakVal, 128) == false || !peakVal[0])
					&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
					GetRG("replaygain_album_peak", info, apev2, peakVal, 128);
				break;
			case 1:
				if ((GetRG("replaygain_album_gain", info, apev2, gain, 128) == false || !gain[0])
					&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
					GetRG("replaygain_track_gain", info, apev2, gain, 128);

				if ((GetRG("replaygain_album_peak", info, apev2, peakVal, 128) == false || !peakVal[0])
					&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
					GetRG("replaygain_track_peak", info, apev2, peakVal, 128);
				break;
			}

			if (gain[0])
			{
				if (gain[0] == L'+')
					dB = (float)_wtof_l(&gain[1], C_locale);
				else
					dB = (float)_wtof_l(gain, C_locale);
			}
			else
			{
				dB = AGAVE_API_CONFIG->GetFloat(playbackConfigGroupGUID, L"non_replaygain", -6.0);
				return (float)pow(10.0f, dB / 20.0f);
			}

			if (peakVal[0])
			{
				peak = (float)_wtof_l(peakVal, C_locale);
			}

			switch (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_mode", 1))
			{
			case 0:  // apply gain
				return (float)pow(10.0f, dB / 20.0f);
			case 1:  // apply gain, but don't clip
				return min((float)pow(10.0f, dB / 20.0f), 1.0f / peak);
			case 2:  // normalize
				return 1.0f / peak;
			case 3:  // prevent clipping
				if (peak > 1.0f)
					return 1.0f / peak;
				else
					return 1.0f;
			}
		}
		/*	else
		{
		float dB = AGAVE_API_CONFIG->GetFloat(playbackConfigGroupGUID, L"non_replaygain", -6.0);
		return pow(10.0f, dB / 20.0f);
		}*/
	}

	return 1.0f; // no gain
}
//-------------------------------------------------------------------------*
//   Open
//-------------------------------------------------------------------------*

static char *jnl_strndup(const char *str, size_t n)
{
	char *o = (char *)calloc(n+1, sizeof(char));
	if (!o)
		return 0;
	
	strncpy(o, str, n);
	o[n]=0;
	return o;
}

static int parse_url(const char *url, char **prot, char **host, unsigned short *port, char **req, char **lp)
{
	free(*prot); *prot=0;
	free(*host); *host = 0;
	free(*req); *req = 0;
	free(*lp); *lp = 0;
	*port = 0;

	const char *p;
	const char *protocol = strstr(url, "://");
	if (protocol)
	{
		*prot = jnl_strndup(url, protocol-url);
		p = protocol + 3;
	
	}
	else
	{
		p = url;
	}

	while (p && *p == '/') p++; // skip extra /

	size_t end = strcspn(p, "@/");

	// check for username
	if (p[end] == '@')
	{
		*lp = jnl_strndup(p, end);
		p = p+end+1;
		end = strcspn(p, "[:/");
	}

	if (p[0] == '[') // IPv6 style address
	{
		p++;
		const char *ipv6_end = strchr(p, ']');
		if (!ipv6_end)
			return 1;

		*host = jnl_strndup(p, ipv6_end-p);
		p = ipv6_end+1;
	}
	else
	{
		end = strcspn(p, ":/");
		*host = jnl_strndup(p, end);
		p += end;
	}

	// is there a port number?
	if (p[0] == ':')
	{
		char *new_end;
		*port = (unsigned short)strtoul(p+1, &new_end, 10);
		p = new_end;
	}

	if (p[0])	
	{
		*req = _strdup(p);
	}
	
	return 0;
}

static void parseURL(const char *url, char **host, unsigned short *port, char **req, char **lp)
{
	char *prot=0;
	
	parse_url(url, &prot, host, port, req, lp);
	if (!*port)
	{
		if (prot)
		{
			if (!stricmp(prot, "https"))
				*port = 443;
			else
				*port = 80;
		}
		else
			*port=80;
	}
	
	free(prot);

	if (!*req)
		*req = _strdup("/");
}

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
		while (shift >= 6)
		{
			shift -= 6;
			*out++ = alphabet[(accum >> shift) & 0x3F];
		}
	}
	if (shift == 4)
	{
		*out++ = alphabet[(accum & 0xF) << 2];
		*out++ = '=';
	}
	else if (shift == 2)
	{
		*out++ = alphabet[(accum & 0x3) << 4];
		*out++ = '=';
		*out++ = '=';
	}

	*out++ = 0;
}

int CGioFile::doConnect(const char *str, int start_offset)
{
	char *http_ver_str = " HTTP/1.0\r\n";

	unsigned short proxy_port = 80;
	char str2[1024]={0};

	if (!str)
		str = last_full_url;
	else
		lstrcpynA( last_full_url, str, sizeof( last_full_url ) );

	if (start_offset > 0)
	{
		http_ver_str = " HTTP/1.1\r\n";
	}

	lstrcpynA(str2, str, 1024);
	
	is_stream_seek = start_offset || !str;
	
	lstrcpynA(g_stream_title, str, 256);
	
	meta_interval = meta_pos = 0;
	server_name[0] = 0;
	last_title_sent[0] = 0;
	stream_bytes_read = start_offset;
	stream_metabytes_read = 0;
	m_is_stream = 1;
	constate = 0;
	parseURL(str2, &host, &port, &request, &lpinfo);
	if (port == 80 || !GetPrivateProfileIntA("Winamp", "proxyonly80", 0, INI_FILE))
	{
		const char *p;
		const char *winamp_proxy = (const char *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GET_PROXY_STRING);
		if (!winamp_proxy || winamp_proxy == (char *)1)
		{
			char temp[256] = {0};
			GetPrivateProfileStringA("Winamp", "proxy", "", temp, sizeof(temp), INI_FILE);
			p = temp;
		}
		else
			p = winamp_proxy;

		while (p && (*p == ' ' || *p == '\t')) p++;
		char config_proxy[512] = "http://";
		StringCchCatA(config_proxy, 512, p);
		parseURL(config_proxy, &proxy_host, &proxy_port, &req, &proxy_lp);
	}

	if (!host || !host[0]) 
		return 1;
	char *p = request + strlen(request);
	while (p >= request && *p != '/') p--;
	if (p[1])
		lstrcpynA(g_stream_title, ++p, 256);
	lstrcpynA(stream_title_save, g_stream_title, 580);
	lstrcpynA(stream_name, g_stream_title, 256);
	g_stream_title[255] = 0;
	fEof = false;
	timeout_start = GetTickCount();
	EnterCriticalSection(&g_lfnscs);
	WASABI_API_LNGSTRING_BUF(IDS_CONNECTING,lastfn_status,256);
	LeaveCriticalSection(&g_lfnscs);
	PostMessage(mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
	if (force_lpinfo[0])
	{
		free(lpinfo);
		lpinfo = _strdup(force_lpinfo);
	}	

	if (!m_dns) jnl_dns_create(&m_dns);
	m_connection = CreateConnection(str, m_dns, 16384, recvbuffersize = max(32768, config_http_buffersize * 1024));
	if (!m_connection)
		return 1;

	if (!proxy_host || !proxy_host[0])
	{
		jnl_connection_connect(m_connection, host, port);
		jnl_connection_send_string(m_connection, "GET ");
		jnl_connection_send_string(m_connection, request);
		jnl_connection_send_string(m_connection, http_ver_str);
	}
	else
	{
		char s[32]={0};
		jnl_connection_connect(m_connection, proxy_host, proxy_port);
		jnl_connection_send_string(m_connection, "GET http://");
		if (lpinfo && lpinfo[0])
		{
			jnl_connection_send_string(m_connection, lpinfo);
			jnl_connection_send_string(m_connection, "@");
		}
		jnl_connection_send_string(m_connection, host);
		StringCchPrintfA(s, 32, ":%d", port);
		jnl_connection_send_string(m_connection, s);
		jnl_connection_send_string(m_connection, request);
		jnl_connection_send_string(m_connection, http_ver_str);
		if (proxy_lp && proxy_lp[0])
		{
			char temp[1024]={0};
			jnl_connection_send_string(m_connection, "Proxy-Authorization: Basic ");
			encodeMimeStr(proxy_lp, temp);
			jnl_connection_send_string(m_connection, temp);
			jnl_connection_send_string(m_connection, "\r\n");
		}
	}

	jnl_connection_send_string(m_connection, "Host: ");
	jnl_connection_send_string(m_connection, host);
	jnl_connection_send_string(m_connection, "\r\n");

	if (!start_offset)
	{
		jnl_connection_send_string(m_connection, GetUltravoxUserAgent());
	}
	else
		jnl_connection_send_string(m_connection, GetUserAgent());

	jnl_connection_send_string(m_connection, "Accept: */*\r\n");

	if (allow_sctitles && !start_offset) jnl_connection_send_string(m_connection, "Icy-MetaData:1\r\n");

	if (lpinfo && lpinfo[0])
	{
		char str[512] = {0};
		encodeMimeStr(lpinfo, str);
		jnl_connection_send_string(m_connection, "Authorization: Basic ");
		jnl_connection_send_string(m_connection, str);
		jnl_connection_send_string(m_connection, "\r\n");
	}
	if (start_offset > 0)
	{
		char str[512] = {0};
		StringCchPrintfA(str, 512, "Range: bytes=%d-\r\n", start_offset);
		jnl_connection_send_string(m_connection, str);
	}
	jnl_connection_send_string(m_connection, "Connection: close\r\n");
	jnl_connection_send_string(m_connection, "\r\n");

	return 0;
}

int CGioFile::Open(const wchar_t *pszName, int maxbufsizek)
{
	peekBuffer.reserve(16384);

	m_vbr_flag      = 0;
	m_vbr_frames    = 0;
	m_vbr_samples   = 0;
	m_vbr_ms        = 0;
	m_vbr_frame_len = 0;
	
	m_id3v2_len     = 0;
	m_id3v1_len     = 0;
	m_apev2_len     = 0;
	
	lyrics3_size    = 0;
	
	m_is_stream     = 0;
	
	mpeg_length     = 0;
	mpeg_position   = 0;
	file_position   = 0;
	
	if ( !_wcsnicmp( pszName, L"file://", 7 ) )
		pszName += 7;

	if ( PathIsURL( pszName ) )
	{
		wchar_t str[ 8192 ] = { 0 };
		wchar_t *p;
		hFile = INVALID_HANDLE_VALUE;
		lstrcpyn( str, pszName, 8192 );
		save_filename[ 0 ] = 0;
		if ( p = wcsstr( str, L">save.file:" ) )
		{
			*p = 0;
			p += 11;
			if ( !wcsstr( p, L".." ) && !wcsstr( p, L"\\" ) && !wcsstr( p, L"/" ) )
			{
				lstrcpynA( save_filename, AutoChar( p ), 256 );
			}
		}

		if ( doConnect( AutoChar( str ), 0 ) )
			return NErr_ConnectionFailed;
	}
	else
	{
		hFile = CreateFile(pszName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			GetFileTime( hFile, 0, 0, &last_write_time );
			mpeg_length = Seek64( hFile, 0, FILE_END );

			uint64_t startOffset = 0;
			unsigned char buf[ 1448 ] = { 0 };

			DWORD len = 0;
			while ( 1 ) // read all tags (sometimes programs get stupid and make multiple tags)
			{
				len = 0;
				Seek64( hFile, startOffset, FILE_BEGIN );
				ReadFile( hFile, buf, 10, &len, NULL );

				if ( len >= 10 &&
					 buf[ 0 ] == 'I' &&
					 buf[ 1 ] == 'D' &&
					 buf[ 2 ] == '3' &&
					 buf[ 3 ] != 255 &&
					 buf[ 4 ] != 255 &&
					 buf[ 6 ] < 0x80 &&
					 buf[ 7 ] < 0x80 &&
					 buf[ 8 ] < 0x80 &&
					 buf[ 9 ] < 0x80 )
				{
					DWORD thisLen = 10;
					if ( buf[ 5 ] & 0x10 ) // check for footer flag
						thisLen += 10;

					thisLen += ( (int)buf[ 6 ] ) << 21;
					thisLen += ( (int)buf[ 7 ] ) << 14;
					thisLen += ( (int)buf[ 8 ] ) << 7;
					thisLen += ( (int)buf[ 9 ] );

					SetFilePointer( hFile, -10, NULL, FILE_CURRENT );

					if ( stream_id3v2_buf ) // already read a tag?
					{
						startOffset   += thisLen;
						m_id3v2_len   += thisLen;
						mpeg_length   -= thisLen;
						mpeg_position += thisLen;
						
						SetFilePointer( hFile, thisLen, NULL, FILE_CURRENT );

						continue;
					}

					stream_id3v2_buf = (char *)malloc( thisLen );
					if ( stream_id3v2_buf )
					{
						memcpy( stream_id3v2_buf, buf, 10 );

						DWORD dummy = 0;
						if ( !ReadFile( hFile, stream_id3v2_buf, thisLen, &dummy, 0 ) || dummy < thisLen )
						{
							free( stream_id3v2_buf );
							stream_id3v2_buf = NULL;
							
							thisLen = 0;
							SetFilePointer( hFile, 0, NULL, FILE_END );

							break;
						}
						else
						{
							mpeg_position     += thisLen;
							stream_id3v2_read  = thisLen;
							mpeg_length       -= thisLen;
							startOffset       += thisLen;

							info.Decode( stream_id3v2_buf, thisLen );
						}
					}
					else
					{
						/* memory allocation failed, let's assume the ID3v2 tag was valid ... */
						mpeg_position     += thisLen;
						stream_id3v2_read  = thisLen;
						mpeg_length       -= thisLen;
						startOffset       += thisLen;
					}

					m_id3v2_len += thisLen;
				}
				else
				{
					// benski> unnecessary because we call SetFilePointer immediately after the loop:
					// CUT: SetFilePointer(hFile, -10, NULL, FILE_CURRENT);
					break;
				}
			}

			/* Read ID3v1 Tag */
			if ( mpeg_length >= 128 )
			{
				SetFilePointer( hFile, -128, NULL, FILE_END );

				len = 0;
				if ( ReadFile( hFile, id3v1_data, 128, &len, NULL ) && len == 128 )
				{
					if ( id3v1_data[ 0 ] == 'T' && id3v1_data[ 1 ] == 'A' && id3v1_data[ 2 ] == 'G' )
					{
						m_id3v1_len  = 128;
						mpeg_length -= m_id3v1_len;
					}
				}
			}

			/* read appended ID3v2.4 tag */
			if ( mpeg_length >= 10 && Seek64( hFile, mpeg_position + mpeg_length - 10, FILE_BEGIN ) != -1 )
			{
				len = 0;
				ReadFile( hFile, buf, 10, &len, NULL );
				if ( len >= 10 &&
					 buf[ 0 ] == '3' &&
					 buf[ 1 ] == 'D' &&
					 buf[ 2 ] == 'I' &&
					 buf[ 3 ] != 255 &&
					 buf[ 4 ] != 255 &&
					 buf[ 6 ] < 0x80 &&
					 buf[ 7 ] < 0x80 &&
					 buf[ 8 ] < 0x80 &&
					 buf[ 9 ] < 0x80 )
				{
					DWORD thisLen = 10;
					if ( buf[ 5 ] & 0x10 ) // check for header flag
						thisLen += 10;

					thisLen += ( (int)buf[ 6 ] ) << 21;
					thisLen += ( (int)buf[ 7 ] ) << 14;
					thisLen += ( (int)buf[ 8 ] ) << 7;
					thisLen += ( (int)buf[ 9 ] );

					mpeg_length -= thisLen;
				}
			}

			/* Read Lyrics3 Tag */
			if ( mpeg_length >= 15 )
			{
				free( lyrics3_data );
				lyrics3_data = NULL;

				lyrics3_size = 0;
				char lyrics3_end_signature[ 15 ] = { 0 };

				Seek64( hFile, ( mpeg_position + mpeg_length - 15 ), FILE_BEGIN );

				len = 0;
				if ( ReadFile( hFile, lyrics3_end_signature, 15, &len, NULL ) && len == 15 )
				{
					if ( !memcmp( lyrics3_end_signature + 6, "LYRICS200", 9 ) )
					{
						lyrics3_size = strtoul( lyrics3_end_signature, 0, 10 );
						if ( lyrics3_size )
						{
							lyrics3_data = (char *)malloc( lyrics3_size );
							if ( lyrics3_data )
							{
								SetFilePointer( hFile, -(LONG)( 15 + lyrics3_size ), NULL, FILE_CURRENT );

								len = 0;
								ReadFile( hFile, lyrics3_data, lyrics3_size, &len, 0 );
								if ( len != lyrics3_size || memcmp( lyrics3_data, "LYRICSBEGIN", 11 ) )
								{
									free( lyrics3_data );
									lyrics3_data = NULL;

									lyrics3_size = 0;
								}
								else
								{
									mpeg_length -= lyrics3_size + 15;
								}
							}
						}
					}
				}
			}

			if ( mpeg_length >= 32 )
			{
				/* Read APEv2 Tag */
				free( apev2_data );
				apev2_data = NULL;
				
				char ape[ 32 ] = { 0 };
				Seek64( hFile, ( mpeg_position + mpeg_length - 32 ), FILE_BEGIN );

				len = 0;
				if ( ReadFile( hFile, ape, 32, &len, NULL ) && len == 32 )
				{
					APEv2::Header footer( ape );
					if ( footer.Valid() )
					{
						m_apev2_len = footer.TagSize();
						if ( mpeg_length >= m_apev2_len )
						{
							Seek64( hFile, -(int64_t)( m_apev2_len ), FILE_CURRENT );

							apev2_data = (char *)malloc( m_apev2_len );
							if ( apev2_data )
							{
								len = 0;
								ReadFile( hFile, apev2_data, m_apev2_len, &len, NULL );
								if ( len != m_apev2_len || apev2.Parse( apev2_data, m_apev2_len ) != APEv2::APEV2_SUCCESS )
								{
									free( apev2_data );
									apev2_data = NULL;

									m_apev2_len = 0;
								}
							}

							mpeg_length -= m_apev2_len;
						}
					}
				}
			}

			{
				Seek64( hFile, mpeg_position, FILE_BEGIN );

				len = 0;
				ReadFile( hFile, buf, sizeof( buf ), &len, NULL );
				
				delete m_vbr_hdr;
				m_vbr_hdr = NULL;
				
				LAMEinfo lame;
				lame.toc = m_vbr_toc;

				m_vbr_frame_len = ReadLAMEinfo( buf, &lame );
				if ( m_vbr_frame_len )
				{
					lengthVerified = false;
					prepad         = lame.encoderDelay;
					postpad        = lame.padding;

					encodingMethod = lame.encodingMethod;
					if ( !encodingMethod && lame.cbr )
						encodingMethod = ENCODING_METHOD_CBR;

					if ( lame.flags & TOC_FLAG )
					{
						int x;
						for ( x = 0; x < 100; x++ )
							if ( m_vbr_toc[ x ] ) break;

						if ( x != 100 )
							m_vbr_flag = 1;
					}

					if ( lame.flags & BYTES_FLAG )
					{
						// some programs are stupid and count the id3v2 length in the lame header
						if ( mpeg_length + m_id3v2_len == lame.bytes || mpeg_length + m_id3v2_len + m_id3v1_len == lame.bytes )
						{
							m_vbr_bytes    = mpeg_length;
							lengthVerified = true;
						}
						else if ( abs( (int)mpeg_length - lame.bytes ) < MAX_ACCEPTABLE_DEVIANCE )
						{
							m_vbr_bytes    = lame.bytes;
							lengthVerified = true;
						}
					}

					if ( lame.flags & FRAMES_FLAG
						 && m_vbr_bytes && lengthVerified ) // only use the length if we're sure it's unmodified
					{
						m_vbr_frames   = lame.frames;
						m_vbr_samples  = Int32x32To64( lame.frames, lame.h_id ? 1152 : 576 );
						m_vbr_samples -= ( prepad + postpad );
						m_vbr_ms       = MulDiv( (int)m_vbr_samples, 1000, lame.samprate );
					}

					if ( !m_vbr_frames || encodingMethod == ENCODING_METHOD_CBR )
						m_vbr_flag = 0;

					mpeg_length   -= m_vbr_frame_len;
					mpeg_position += m_vbr_frame_len;
				}
				else
				{
					m_vbr_hdr    = new CVbriHeader;

					m_vbr_frame_len = m_vbr_hdr->readVbriHeader(buf);
					if (m_vbr_frame_len)
					{
						m_vbr_bytes     = m_vbr_hdr->getBytes();
						m_vbr_frames    = m_vbr_hdr->getNumFrames();
						m_vbr_ms        = m_vbr_hdr->getNumMS();
						
						lengthVerified  = true;

						mpeg_length    -= m_vbr_frame_len;
						mpeg_position  += m_vbr_frame_len;
					}
					else
					{
						delete m_vbr_hdr;
						m_vbr_hdr = NULL;
					}
				}
			}

			// read OFL
			{
				Seek64( hFile, mpeg_position, FILE_BEGIN );
				len = 0;
				ReadFile( hFile, buf, sizeof( buf ), &len, NULL );
				MPEGFrame frame;
				frame.ReadBuffer( buf );
				OFL ofl;
				if ( ofl.Read( frame, &buf[ 4 ], len - 4 ) == NErr_Success )
				{
					m_vbr_ms      = (int)ofl.GetLengthSeconds() * 1000;
					m_vbr_samples = ofl.GetSamples();

					m_vbr_frames = ofl.GetFrames();
					size_t pre, post;
					if ( ofl.GetGaps( &pre, &post ) == NErr_Success )
					{
						prepad  = (int)pre - 529;
						postpad = (int)post + 529;
					}
				}
			}

			ReadiTunesGaps();

			Seek64( hFile, mpeg_position, FILE_BEGIN );

			if ( maxbufsizek * 1024 >= mpeg_length )
			{
				DWORD m_full_buffer_len = 0;
				m_full_buffer = new unsigned char[ (unsigned int)mpeg_length ];
				if ( !ReadFile( hFile, m_full_buffer, (DWORD)mpeg_length, &m_full_buffer_len, NULL ) )
				{
					CloseHandle( hFile );
					hFile = INVALID_HANDLE_VALUE;
					delete[] m_full_buffer;
					m_full_buffer = NULL;
					return NErr_Error;
				}
				CloseHandle( hFile );
				hFile = INVALID_HANDLE_VALUE;
				m_full_buffer_pos = 0;
			}
			else
			{
				m_full_buffer = NULL;
			}

			fEof = false;
			return NErr_Success;
		}
		else
		{
			//DWORD dwError = GetLastError();
			return NErr_Error;
		}
	}

	return NErr_Success;
}

//-------------------------------------------------------------------------*
//   Close
//-------------------------------------------------------------------------*

int CGioFile::Close()
{
	int dwReturn = NErr_Success;

	if (m_is_stream)
	{
		jnl_connection_release(m_connection);
		if (m_dns) jnl_dns_release(m_dns);
		m_dns = 0;
		if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
		m_is_stream = 0;
	}
	else
	{
		delete [] m_full_buffer;
		m_full_buffer = NULL;

		if (hFile != INVALID_HANDLE_VALUE)
		{
			dwReturn = CloseHandle(hFile) ? NErr_Success : NErr_Error;
			hFile = INVALID_HANDLE_VALUE;
		}
	}

	return dwReturn;
}

void CGioFile::processMetaData(char *data, int len, int msgId)
{
	if (len && *data)
	{
		char *ld = NULL;
		int x;
		if (len > 4096) return ;
		for (x = 0; x < len; x ++)
			if (!data[x]) break;
		if (x == len) return ;
		while ((ld = strstr(data, "='")))
		{
			char * n = data;
			ld[0] = 0;
			ld += 2;
			data = strstr(ld, "';");
			if (data)
			{
				data[0] = 0;
				data += 2;
				if (!_stricmp(n, "StreamTitle"))
				{
					lstrcpynA(g_stream_title, ld, sizeof(g_stream_title));
					lstrcpynA(last_title_sent, g_stream_title, sizeof(last_title_sent));
					lstrcpynA(stream_current_title, g_stream_title, sizeof(stream_current_title));
					if (sctitle_format)
					{
						StringCchCatA(g_stream_title, 256, *stream_title_save ? *g_stream_title ? " (" : "(" : "");
						StringCchCatA(g_stream_title, 256, stream_title_save);
						StringCchCatA(g_stream_title, 256, *stream_title_save ? ")" : "");
					}
					PostMessage(mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
				}
				else if (!_stricmp(n, "StreamUrl"))
				{
					lstrcpynA(stream_url, ld, sizeof(stream_url));
					DWORD_PTR dw;
					if (stream_url[0]) SendMessageTimeout(mod.hMainWindow, WM_USER, (WPARAM)stream_url, IPC_MBOPEN, SMTO_NORMAL, 500, &dw);
				}
			}
			else break;
		}
	}
}

INT_PTR CALLBACK CGioFile::httpDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CGioFile *_this;
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			_this = (CGioFile *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (_this->force_lpinfo[0])
				SetDlgItemTextA(hwndDlg, IDC_EDIT1, _this->force_lpinfo);
			else SetDlgItemTextA(hwndDlg, IDC_EDIT1, _this->lpinfo?_this->lpinfo:"");
			SetDlgItemTextA(hwndDlg, IDC_REALM, _this->dlg_realm);
			return 1;
		case WM_COMMAND:
			_this = (CGioFile *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (LOWORD(wParam) == IDOK)
			{
				GetDlgItemTextA(hwndDlg, IDC_EDIT1, _this->force_lpinfo, sizeof(_this->force_lpinfo));
				EndDialog(hwndDlg, 1);
			}
			else if (LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hwndDlg, 0);
			}
			break;
	}
	return 0;
}

class GioFileFiller : public Filler
{
public:
	GioFileFiller(CGioFile *_file)
	{
		ret=NErr_Success;
		file=_file;
	}

	size_t Read(void *dest, size_t len)
	{
		int bytesRead=0;
		ret = file->Read(dest, (int)len, &bytesRead);
		return bytesRead;
	}

	CGioFile *file;
	int ret;
};

int CGioFile::Peek(void *pBuffer, int cbToRead, int *pcbRead)
{
	GioFileFiller filler(this);

	// do we need to fill up?
	if (cbToRead > (int)peekBuffer.size())
	{
		no_peek_hack = true;// benski> HACK ALERT!  we have to set this or else Read() will try to use the peek buffer
		peekBuffer.fill(&filler, cbToRead - peekBuffer.size());
		no_peek_hack=false;
	}

	*pcbRead = (int)peekBuffer.peek(pBuffer, cbToRead);
	return filler.ret;
}

//-------------------------------------------------------------------------*
//   Read
//-------------------------------------------------------------------------*

int CGioFile::Read(void *pBuffer, int cbToRead, int *pcbRead)
{
	TITLELISTTYPE *mylist = TitleLinkedList;
	// these are used for SHOUTCAST2 metadata and artwork since it can provide
	// multi-packet metadata chunks which are out of order or are received in
	// a way which would otherwise cause these to be cleared incorrectly
	static int title_parts = 0, stream_art_parts = 0, stream_art_parts_total_len = 0,
			   playing_art_parts = 0, playing_art_parts_total_len = 0;
	static char **title_blocks = 0, **stream_art_blocks = 0, **playing_art_blocks = 0;

	if (mylist != &TitleListTerminator && mod.outMod)
	{
		while (mylist->Next && mylist != &TitleListTerminator)
		{
			TITLELISTTYPE *next = (TITLELISTTYPE *)mylist->Next;
			long now = mod.outMod->GetOutputTime();

			if (mylist->title[0] || mylist->part_len)
			{
				if (!mylist->timer || now >= mylist->timer)
				{
					switch(mylist->style)
					{
						case  UVOX_METADATA_STYLE_AOLRADIO:
						{
							EnterCriticalSection(&streamInfoLock);
							free(uvox_3901);
							uvox_3901 = _strdup(mylist->title);
							LeaveCriticalSection(&streamInfoLock);
							if (g_playing_file)
							{
								PostMessage(mod.hMainWindow, WM_USER, (WPARAM) "0x3901", IPC_METADATA_CHANGED);
								PostMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_UPDTITLE);
							}
						}
						break;

						case UVOX_METADATA_STYLE_SHOUTCAST:
						{
							processMetaData(mylist->title, (int)strlen(mylist->title) + 1);
						}
						break;

						case UVOX_METADATA_STYLE_SHOUTCAST2:
						{
							EnterCriticalSection(&streamInfoLock);
							// so we can recombine multi-packets we'll store things and
							// when all of the packets have been read then we form them
							if(!title_blocks)
								title_blocks = (char **)malloc(sizeof(char*)*(mylist->total_parts));

							title_blocks[mylist->part-1] = _strdup(mylist->title);
							title_parts++;

							// sanity check so we only try to get the metadata if all parts were processed
							if (title_parts == mylist->total_parts)
							{
								free(uvox_3902);
								uvox_3902 = (char *)malloc(16377 * mylist->total_parts);
								uvox_3902[0] = 0;
								title_parts = 0;

								for(int i = 0; i < mylist->total_parts; i++)
								{
									StringCchCatA(uvox_3902, mylist->total_parts * 16384, title_blocks[i]);
									free(title_blocks[i]);
								}
								free(title_blocks);
								title_blocks = 0;

								// attempt to form a title as 'artist - album - title' as sc_serv2 feeds
								// changed for 5.61 to be just 'artist - title' to match v1 and v2 tools
								Ultravox3902 uvox_metadata;
								if (uvox_metadata.Parse(uvox_3902) != API_XML_FAILURE)
								{
									wchar_t stream_title[256] = {0};
									char* fields[] = {"artist", "title"};
									for(int i = 0; i < sizeof(fields)/sizeof(fields[0]); i++)
									{
										wchar_t temp[256] = {0};
										int ret = uvox_metadata.GetExtendedData(fields[i], temp, 256);
										if(ret && temp[0])
										{
											if(stream_title[0]) StringCchCatW(stream_title, 256, L" - ");
											StringCchCatW(stream_title, 256, temp);
										}
									}

									lstrcpynA(g_stream_title, AutoChar(stream_title, CP_UTF8), sizeof(g_stream_title));
									lstrcpynA(last_title_sent, g_stream_title, sizeof(last_title_sent));
									lstrcpynA(stream_current_title, g_stream_title, sizeof(stream_current_title));
								}
							}
							else
							{
								g_stream_title[0] = 0;
								last_title_sent[0] = 0;
							}

							if (sctitle_format)
							{
								StringCchCatA(g_stream_title, 256, *stream_title_save ? *g_stream_title ? " (" : "(" : "");
								StringCchCatA(g_stream_title, 256, stream_title_save);
								StringCchCatA(g_stream_title, 256, *stream_title_save ? ")" : "");
							}
							LeaveCriticalSection(&streamInfoLock);
							
							if (g_playing_file)
							{
								PostMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_UPDTITLE);
							}
						}
						break;

						case UVOX_METADATA_STYLE_SHOUTCAST2_ARTWORK:
						{
							EnterCriticalSection(&streamInfoLock);
							// so we can recombine multi-packets we'll store things and
							// when all of the packets have been read then we form them
							if(!stream_art_blocks)
							{
								stream_art_parts_total_len = 0;
								stream_art_blocks = (char **)malloc(sizeof(char*)*(mylist->total_parts));
							}

							stream_art_blocks[mylist->part-1] = (char *)malloc(mylist->part_len+1);
							memcpy(stream_art_blocks[mylist->part-1], mylist->title, mylist->part_len);
							stream_art_parts++;
							stream_art_parts_total_len += mylist->part_len;

							// sanity check so we only try to get the metadata if all parts were processed
							if (stream_art_parts == mylist->total_parts)
							{
								free(uvox_artwork.uvox_stream_artwork);
								if (stream_art_parts_total_len <= 0) break;
								uvox_artwork.uvox_stream_artwork = (char *)malloc(stream_art_parts_total_len);
								uvox_artwork.uvox_stream_artwork[0] = 0;
								uvox_artwork.uvox_stream_artwork_len = stream_art_parts_total_len;
								uvox_artwork.uvox_stream_artwork_type = mylist->type;
								stream_art_parts = 0;

								char *art = uvox_artwork.uvox_stream_artwork;
								for(int i = 0; i < mylist->total_parts; i++)
								{
									int size = min(stream_art_parts_total_len, 16371);
									if (size > 0 && size <= 16371)
									{
										memcpy(art, stream_art_blocks[i], size);
										stream_art_parts_total_len -= size;
										art += size;
									}
									free(stream_art_blocks[i]);
								}
								free(stream_art_blocks);
								stream_art_blocks = 0;
							}
							LeaveCriticalSection(&streamInfoLock);
							
							if (g_playing_file)
							{
								PostMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_UPDTITLE);
							}
						}
						break;

						case UVOX_METADATA_STYLE_SHOUTCAST2_ARTWORK_PLAYING:
						{
							EnterCriticalSection(&streamInfoLock);
							// so we can recombine multi-packets we'll store things and
							// when all of the packets have been read then we form them
							if(!playing_art_blocks)
							{
								playing_art_parts_total_len = 0;
								playing_art_blocks = (char **)malloc(sizeof(char*)*(mylist->total_parts));
							}

							playing_art_blocks[mylist->part-1] = (char *)malloc(mylist->part_len+1);
							memcpy(playing_art_blocks[mylist->part-1], mylist->title, mylist->part_len);
							playing_art_parts++;
							playing_art_parts_total_len += mylist->part_len;

							// sanity check so we only try to get the metadata if all parts were processed
							if (playing_art_parts == mylist->total_parts)
							{
								free(uvox_artwork.uvox_playing_artwork);
								if (playing_art_parts_total_len <= 0) break;
								uvox_artwork.uvox_playing_artwork      = (char *)malloc(playing_art_parts_total_len);
								uvox_artwork.uvox_playing_artwork[0]   = 0;
								uvox_artwork.uvox_playing_artwork_len  = playing_art_parts_total_len;
								uvox_artwork.uvox_playing_artwork_type = mylist->type;

								playing_art_parts = 0;

								char *art = uvox_artwork.uvox_playing_artwork;
								for(int i = 0; i < mylist->total_parts; i++)
								{
									int size = min(playing_art_parts_total_len, 16371);
									if (size > 0 && size <= 16371)
									{
										memcpy(art, playing_art_blocks[i], size);
										playing_art_parts_total_len -= size;
										art += size;
									}
									free(playing_art_blocks[i]);
								}
								free(playing_art_blocks);
								playing_art_blocks = 0;
							}
							LeaveCriticalSection(&streamInfoLock);
							
							if (g_playing_file)
							{
								PostMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_UPDTITLE);
							}
						}
						break;
					}
					removeTitleListEntry(mylist);
				}
			}
			mylist = next;
		}
	}

	if (!no_peek_hack && peekBuffer.size())
	{
		*pcbRead = (int)peekBuffer.read(pBuffer, cbToRead);
		return NErr_Success;
	}

	BOOL bSuccess;
	if (m_is_stream)
	{
		if (m_connection)
		{
			char str[4096]={0};
			if (pcbRead) *pcbRead = 0;
			jnl_connection_run(m_connection, -1, -1, 0, 0);
			if (constate == 0)
			{
				if ( jnl_connection_receive_lines_available( m_connection ) > 0 )
				{
					char *p = str;
					jnl_connection_receive_line( m_connection, str, 4096 );

					// check http version and type of data, partial or full
					if ( strlen( str ) >= 13 && !_strnicmp( str, "HTTP/1.1", 8 ) )
					{
						char *p = str + 8; while ( p && *p == ' ' ) p++;
						m_http_response = ( p ? atoi( p ) : 0 );
						if ( m_http_response == 200 )
							m_seek_reset = true;
					}

					EnterCriticalSection( &g_lfnscs );
					lstrcpynA( lastfn_status, str, 256 );
					LeaveCriticalSection( &g_lfnscs );
					PostMessage( mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE );
					while ( p && *p && *p != ' ' ) p++;
					if ( p && *p ) p++;

					if ( p[ 0 ] == '2' ) constate = 1;
					else if ( strstr( p, "301" ) == p || strstr( p, "302" ) == p ||
							  strstr( p, "303" ) == p || strstr( p, "307" ) == p )
					{
						constate = 69;
					}
					else if ( strstr( p, "401" ) == p && m_auth_tries++ < 3 )
					{
						constate = 75;
					}
					else if ( strstr( p, "403" ) == p )
					{
						EnterCriticalSection( &g_lfnscs );
						lstrcpynA( lastfn_status, "access denied", 256 );
						lastfn_status_err = 1;
						LeaveCriticalSection( &g_lfnscs );
						jnl_connection_close( m_connection, 1 );
					}
					else if ( strstr( p, "503" ) == p )
					{
						EnterCriticalSection( &g_lfnscs );
						lstrcpynA( lastfn_status, "server full", 256 );
						lastfn_status_err = 1;
						LeaveCriticalSection( &g_lfnscs );
						jnl_connection_close( m_connection, 1 );
					}
					else
					{
						lastfn_status_err = 1;
						jnl_connection_close( m_connection, 1 );
					}
				}
				else if ( jnl_connection_get_state( m_connection ) == JNL_CONNECTION_STATE_CLOSED || jnl_connection_get_state( m_connection ) == JNL_CONNECTION_STATE_ERROR || jnl_connection_get_state( m_connection ) == JNL_CONNECTION_STATE_NOCONNECTION )
				{
					const char *t = jnl_connection_get_error( m_connection );
					if ( t && *t )
					{
						EnterCriticalSection( &g_lfnscs );
						lstrcpynA( lastfn_status, t, 256 );
						lastfn_status_err = 1;
						LeaveCriticalSection( &g_lfnscs );
					}
					PostMessage( mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE );
				}
			}
			if (constate == 75) // authorization required
			{
				while (jnl_connection_receive_lines_available(m_connection) > 0)
				{
					char *wwwa = "WWW-Authenticate:";
					jnl_connection_receive_line(m_connection, str, 4096);
					if (!str[0])
					{
						lastfn_status_err = 1; jnl_connection_close(m_connection, 1); break;
					}
					if (!_strnicmp(str, wwwa, strlen(wwwa)))
					{
						int has = 0;
						char *s2 = "Basic realm=\"";
						char *p = str + strlen(wwwa); while (p && *p == ' ') p++;
						if (!_strnicmp(p, s2, strlen(s2)))
						{
							p += strlen(s2);
							if (strstr(p, "\""))
							{
								if (p && *p)
								{
									strstr(p, "\"")[0] = 0;
									extern char *get_inifile();
									if (!force_lpinfo[0]) GetPrivateProfileStringA("HTTP-AUTH", p, "", force_lpinfo, sizeof(force_lpinfo), get_inifile());
									if (!force_lpinfo[0] || (lpinfo && lpinfo[0]))
									{
										lstrcpynA(dlg_realm, p, sizeof(dlg_realm));
										if (!WASABI_API_DIALOGBOXPARAM(IDD_HTTPAUTH, GetDialogBoxParent(), httpDlgProc, (LPARAM)this))
										{
											force_lpinfo[0] = 0;
										}
										else
										{
											WritePrivateProfileStringA("HTTP-AUTH", p, force_lpinfo, get_inifile());
										}
									}
									if (force_lpinfo[0])
									{
										jnl_connection_release(m_connection);
										m_connection = NULL;
										doConnect(NULL, 0);
										has = 1;
									}
								}
							}
						}
						if (!has)
						{
							lastfn_status_err = 1;
							jnl_connection_close(m_connection, 1);
						}
						break;
					}
				}
			}
			if (constate == 69) // redirect city
			{
				while (jnl_connection_receive_lines_available(m_connection) > 0)
				{
					jnl_connection_receive_line(m_connection, str, 4096);
					if (!str[0])
					{
						jnl_connection_close(m_connection, 1); break;
					}
					if (!_strnicmp(str, "Location:", 9))
					{
						char *p = str + 9; while (p && *p == ' ') p++;
						if (p && *p)
						{
							if (m_redircnt++ < MAX_REDIRECTS)
							{
								jnl_connection_release(m_connection);
								m_connection = NULL;
								doConnect(p, 0);
							}
							else
							{
								EnterCriticalSection(&g_lfnscs);
								WASABI_API_LNGSTRING_BUF(IDS_REDIRECT_LIMIT_EXCEEDED,lastfn_status,256);
								lastfn_status_err = 1;
								LeaveCriticalSection(&g_lfnscs);
								jnl_connection_close(m_connection, 1);
							}
							break;
						}
					}
				}
			}
			if (constate == 1)
			{
				while (jnl_connection_receive_lines_available(m_connection) > 0)
				{
					jnl_connection_receive_line(m_connection, str, 4096);

					if (!str[0])
					{
						if (config_http_save_dir[0] && (config_miscopts&16))
						{
							if (!save_filename[0] && m_is_stream == 1 &&
								strlen(stream_title_save) > 4 &&
								!strstr(stream_title_save, "..") &&
								!strstr(stream_title_save, "\\") &&
								!strstr(stream_title_save, "/"))
							{
								lstrcpynA(save_filename, stream_title_save, 251);
								char *p = strstr(save_filename, ".mp");
								if (!p) p = strstr(save_filename, ".MP");
								if (!p) p = strstr(save_filename, ".mP");
								if (!p) p = strstr(save_filename, ".Mp");
								if (!p)
								{
									StringCchCatA(save_filename, 256, ".mp3");
								}
								else
								{
									while (p && *p && *p != ' ') p++;
									if (p) *p = 0;
								}
							}

							if (save_filename[0])
							{
								char buf[4096] = {0};
								StringCchPrintfA(buf, 4096, "%s%s%s", config_http_save_dir, config_http_save_dir[strlen(config_http_save_dir) - 1] == '\\' ? "" : "\\", save_filename);
								hFile = CreateFileA(buf, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
							}
						}
						else save_filename[0] = 0;
						constate = meta_interval ? 4 : 2;
						break;
					}

					// check if stream is seekable
					if (strlen(str) >= 14 && !_strnicmp(str, "Accept-Ranges:", 14))
					{
						m_is_stream_seekable = true;
					}

					if (!_strnicmp(str, "Content-Length:", 15))
					{
						char *p = str + 15; while (p && *p == ' ') p++;
						if (!is_stream_seek) 
							mpeg_length = atoi(p);
						m_is_stream_seekable = true;
					}

					if (!_strnicmp(str, "content-type:", 13))
					{
						char *p = str + 13; while (p && *p == ' ') p++;
						free(m_content_type);
						m_content_type = _strdup(p);
						if (!_strnicmp(m_content_type, "misc/ultravox",13))
						{
							stream_title_save[0] = 0;
							stream_name[0]=0;
							g_stream_title[0] = 0;
							is_uvox = 1;
							// TODO get this to id as SC2 stream if possible...
							m_is_stream = 3;
						}
					}

					if (!_strnicmp(str, "Server:", 7))
					{
						char *p = str + 7; while (p && *p == ' ') p++;
						lstrcpynA(server_name, p, sizeof(server_name));
					}

					if (!_strnicmp(str, "ultravox-max-msg:", 17))
					{
						char *p = str + 17; while (p && *p == ' ') p++;
						uvox_maxmsg = (p ? atoi(p) : 0);
						if (uvox_maxmsg > UVOX_MAXMSG_CAP) // benski> security vulnerability fix, too high of value was causing an integer overflow (because we malloc uvox_maxmsg*2
							uvox_maxmsg = UVOX_MAXMSG_CAP;
						m_is_stream = 3;

					}
					else if (!_strnicmp(str, "Ultravox-SID:", 13))
					{
						char *p = str + 13; while (p && *p == ' ') p++;
						uvox_sid = (p ? atoi(p) : 0);
						m_is_stream = 3;
					}
					if (!_strnicmp(str, "Ultravox-Avg-Bitrate:", 21))
					{
						char *p = str + 21; while (p && *p == ' ') p++;
						uvox_avgbr = (p ? atoi(p) : 0);
						m_is_stream = 3;
					}
					if (!_strnicmp(str, "Ultravox-Max-Bitrate:", 21))
					{
						char *p = str + 21; while (p && *p == ' ') p++;
						uvox_maxbr = (p ? atoi(p) : 0);
						m_is_stream = 3;
					}
					if (!_strnicmp(str, "Ultravox-Bitrate:", 17))
					{
						char *p = str + 17; while (p && *p == ' ') p++;
						uvox_avgbr = uvox_maxbr = (p ? atoi(p) : 0);
						m_is_stream = 3;
					}
					if (!_strnicmp(str, "Ultravox-Title:", 15))
					{
						char *p = str + 15; while (p && *p == ' ') p++;
						lstrcpynA(stream_title_save, p, 580);
						m_is_stream = 3;
					}
					if (!_strnicmp(str, "Ultravox-Genre:", 15))
					{
						char *p = str + 15; while (p && *p == ' ') p++;
						lstrcpynA(stream_genre, p, sizeof(stream_genre));
						m_is_stream = 3;
					}
					if (!_strnicmp(str, "Ultravox-URL:", 13)/* && !strstr(str, "shoutcast.com")*/)
					{
						char *p = str + 13; while (p && *p == ' ') p++;
						lstrcpynA(stream_url, p, sizeof(stream_url));
						DWORD_PTR dw = 0;
						if (stream_url[0]) SendMessageTimeout(mod.hMainWindow, WM_USER, (WPARAM)stream_url, IPC_MBOPEN, SMTO_NORMAL, 500, &dw);
						m_is_stream = 3;
					}
					if (!_strnicmp(str, "Ultravox-Class-Type:", 19))
					{
						// id our stream for 'streamtype'
						// added for 5.63 as is a better way to check for
						// a SC2/UVOX2.1 stream when no metadata is sent
						m_is_stream = 5;
					}

					if (!_strnicmp(str, "icy-notice2:", 12))
					{
						char *p = str + 12; while (p && *p == ' ') p++;
						char *p2 = (p ? strstr(p, "<BR>") : 0);
						if (p2)
						{
							*p2 = 0;
							lstrcpynA(server_name, p, sizeof(server_name));
						}
						m_is_stream=2;
					}

					if (!_strnicmp(str, "content-disposition:", 20))
					{
						if (strstr(str, "filename="))
							lstrcpynA(g_stream_title, strstr(str, "filename=") + 9, sizeof(g_stream_title));
						lstrcpynA(stream_title_save, g_stream_title, 580);
					}

					if (!_strnicmp(str, "icy-name:", 9))
					{
						char *p = str + 9; while (p && *p == ' ') p++;
						lstrcpynA(g_stream_title, p, sizeof(g_stream_title));
						lstrcpynA(stream_title_save, g_stream_title, 580);
						lstrcpynA(stream_name, g_stream_title, 256);
						// m_is_stream = 2; // benski> cut: some things use icy-name as a hack to show a title - not a reliable indicator of a SHOUTcast stream
					}

					if (!_strnicmp(str, "icy-metaint:", 12))
					{
						char *p = str + 12; while (p && *p == ' ') p++;
						meta_interval = (p ? atoi(p) : 0);
						m_is_stream = 2;
					}
					if (!_strnicmp(str, "icy-genre:", 10))
					{
						char *p = str + 10; while (p && *p == ' ') p++;
						lstrcpynA(stream_genre, p, sizeof(stream_genre));
						m_is_stream = 2;
					}
					if (!_strnicmp(str, "icy-url:", 8) && !strstr(str, "shoutcast.com"))
					{
						char *p = str + 8; while (p && *p == ' ') p++;
						lstrcpynA(stream_url, p, sizeof(stream_url));
						//if (!strncmp(stream_url,"hTtP",4))
						//{
						//  DWORD dw;
						//  SendMessageTimeout(mod.hMainWindow,WM_USER,(WPARAM)0,241,SMTO_NORMAL,500,&dw);
						//} // Removed by Tag, Annoying.
						DWORD_PTR dw = 0;
						if (stream_url[0]) SendMessageTimeout(mod.hMainWindow, WM_USER, (WPARAM)stream_url, IPC_MBOPEN, SMTO_NORMAL, 500, &dw);
						m_is_stream = 2;
					}
				}
			}

			if (constate == 2) // check for id3v2
			{
				if (jnl_connection_receive_bytes_available(m_connection) >= 10)
				{
					char buf[10]={0};
					constate = 4;
					jnl_connection_peek(m_connection, buf, 10);
					if (buf[0] == 'I'
						&& buf[1] == 'D'
						&& buf[2] == '3'
						&& buf[3] != 255
						&& buf[4] != 255
						&& buf[6] < 0x80
						&& buf[7] < 0x80
						&& buf[8] < 0x80
						&& buf[9] < 0x80)
					{
						jnl_connection_receive(m_connection, buf, 10);
						m_id3v2_len = 10;
						if (buf[5] & 0x10) // check for footer flag
							m_id3v2_len += 10;
						m_id3v2_len += ((int)buf[6]) << 21;
						m_id3v2_len += ((int)buf[7]) << 14;
						m_id3v2_len += ((int)buf[8]) << 7;
						m_id3v2_len += ((int)buf[9]);
						if (m_id3v2_len < 0) m_id3v2_len = 0;
						if (mpeg_length && m_id3v2_len > mpeg_length)
						{
							m_id3v2_len = 0;
						}
						if (m_id3v2_len)
						{
							constate = 3;
							stream_id3v2_read = 0;
							if (m_id3v2_len < 16*1024*1024)
							{
								stream_id3v2_buf = (char*)malloc(10 + m_id3v2_len);
								stream_id3v2_read = 10;
								memcpy(stream_id3v2_buf, buf, 10);
							}
							EnterCriticalSection(&g_lfnscs);
							WASABI_API_LNGSTRING_BUF(IDS_READING_ID3,lastfn_status,256);
							LeaveCriticalSection(&g_lfnscs);
							PostMessage(mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
						}
					}
				}
			}

			if (constate == 3) // id3v2 found
			{
				while ((int)stream_id3v2_read < m_id3v2_len)
				{
					char buf[1024]={0};
					int btoread = m_id3v2_len - stream_id3v2_read;
					if (btoread > sizeof(buf)) btoread = sizeof(buf);

					int avail = (int)jnl_connection_receive_bytes_available(m_connection);
					if (btoread > avail) btoread = avail;
					if (!btoread) break;

					if (stream_id3v2_buf)
						stream_id3v2_read += (uint32_t)jnl_connection_receive(m_connection, stream_id3v2_buf + stream_id3v2_read, btoread);
					else
						stream_id3v2_read += (uint32_t)jnl_connection_receive(m_connection, buf, btoread);
					//stream_id3v2_read+=m_connection->ReceiveBytes(0,btoread);
				}

				if ((int)stream_id3v2_read >= m_id3v2_len)
				{
					if (stream_id3v2_buf)
					{
						if (m_is_stream_seekable /*m_is_stream != 2*/) /* don't want to do id3v2 on an internet stream */
						{
							EnterCriticalSection(&streamInfoLock);
							info.Decode(stream_id3v2_buf, stream_id3v2_read);
							// TODO: streamInfo = info;
							LeaveCriticalSection(&streamInfoLock);
							PostMessage(mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
						}
					}
					constate = 4;
				}
			}

			if (constate == 4) // check for xing header
			{
				if (GetContentLength() < 1 || is_stream_seek) constate = 5;
				else
				{
					int avail = (int)jnl_connection_receive_bytes_available(m_connection);
					if (avail > 4096 || jnl_connection_get_state(m_connection) == JNL_CONNECTION_STATE_CLOSED)
					{
						char buf[4096]={0};
						jnl_connection_peek(m_connection, buf, sizeof(buf));
						constate++;
						{
							delete m_vbr_hdr;
							m_vbr_hdr = 0;
							LAMEinfo lame;
							lame.toc = m_vbr_toc;
							m_vbr_frame_len = ReadLAMEinfo((unsigned char *)buf, &lame);
							if (m_vbr_frame_len)
							{
								prepad = lame.encoderDelay;
								postpad = lame.padding;
								if (lame.flags&TOC_FLAG)
								{
									int x;
									for (x = 0; x < 100; x++)
										if (m_vbr_toc[x]) break;
									if (x != 100)
										m_vbr_flag = 1;
								}
								if (lame.flags&FRAMES_FLAG)
								{
									m_vbr_frames = lame.frames;
									m_vbr_samples = Int32x32To64(lame.frames, lame.h_id ? 1152 : 576);
									m_vbr_samples -= (prepad + postpad);
									m_vbr_ms = MulDiv((int)m_vbr_samples, 1000, lame.samprate);
								}
								if (!m_vbr_frames) m_vbr_flag = 0;
								jnl_connection_receive(m_connection, buf, m_vbr_frame_len);
							}
							else
							{
								m_vbr_hdr = new CVbriHeader;
								m_vbr_frame_len = m_vbr_hdr->readVbriHeader((unsigned char *)buf);
								if (m_vbr_frame_len)
								{
									m_vbr_bytes = m_vbr_hdr->getBytes();
									m_vbr_frames = m_vbr_hdr->getNumFrames();
									m_vbr_ms = m_vbr_hdr->getNumMS();
								}
								else
								{
									delete m_vbr_hdr;
									m_vbr_hdr = 0;
								}
							}
						}
						// TODO OFL
					}
				}
			}

			if (constate == 5) // time to stream
			{
				while (1)
				{
					// Process any timed titles

					int len = (int)jnl_connection_receive_bytes_available(m_connection);
					if (meta_interval && meta_pos >= meta_interval)
					{
						unsigned char b;
						if (len > 0 && jnl_connection_peek(m_connection, (char*)&b, 1) && len > (b << 4))
						{
							char metabuf[4096]={0};
							jnl_connection_receive(m_connection, metabuf, 1);
							jnl_connection_receive(m_connection, metabuf, b << 4);
							processMetaData(metabuf, b << 4);
							stream_metabytes_read += (b << 4) + 1;
							meta_pos = 0;
						}
						else break;
					}
					else if (is_uvox)
					{
						if (!uvox_stream_data)
						{
							/* benski> this was a security vulnerability.
							don't blindly multiply by 2 and pass to malloc
							if uvox_maxmsg happens to be 2^31, multiplying by 2 turns it into 0!
							CUT!!! uvox_stream_data = (unsigned char*)malloc(uvox_maxmsg * 2 + 4096);
							*/
							uvox_stream_data = (unsigned char*)malloc(SAFE_MALLOC_MATH(uvox_maxmsg * 2 + 4096, uvox_maxmsg));
						}
						if (uvox_stream_data_len < 1)
						{
again:
							if (len < 6) break;
							jnl_connection_peek(m_connection, (char*)uvox_stream_data, 6);

							int uvox_qos = uvox_stream_data[1];
							int classtype = (uvox_stream_data[2] << 8) | uvox_stream_data[3];
							int uvox_len = (uvox_stream_data[4] << 8) | uvox_stream_data[5];

							int uvox_class = (classtype >> 12) & 0xF;
							int uvox_type = classtype & 0xFFF;

							if (uvox_stream_data[0] != 0x5A || uvox_len > uvox_maxmsg)
							{
								jnl_connection_receive(m_connection, NULL, 1);
								len--;
								uvox_desyncs++;

								goto again;
							}

							if (uvox_len + 7 > len) break;
							// process uvox chunk

							jnl_connection_peek(m_connection, (char*)uvox_stream_data, 6 + uvox_len + 1);
							if (uvox_stream_data[6 + uvox_len])
							{
								jnl_connection_receive(m_connection, NULL, 1);
								uvox_desyncs++;
								len--;
								goto again;
							}
							else if (uvox_class == 0x2 && uvox_type == 0x003)
							{
								// failover gayness
							}
							else if (uvox_class == 0x2 && uvox_type == 0x002)
							{
								EnterCriticalSection(&g_lfnscs);
								WASABI_API_LNGSTRING_BUF(IDS_STREAM_TERMINATED,lastfn_status,256);
								LeaveCriticalSection(&g_lfnscs);
								PostMessage(mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE);

								jnl_connection_close(m_connection, 0);
							}
							else if (uvox_class == 0x2 && uvox_type == 0x001)
							{
								EnterCriticalSection(&g_lfnscs);
								WASABI_API_LNGSTRING_BUF(IDS_STREAM_TEMPORARILY_INTERRUPTED,lastfn_status,256);
								LeaveCriticalSection(&g_lfnscs);
								PostMessage(mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
							}

							else if (uvox_class == 0x3 && (uvox_type == 0x902 || uvox_type == 0x901)) // SHOUTcast 2 metadata
							{
								// id our stream for 'streamtype'
								m_is_stream = 5;

								// this will allow us to cope with multi-packet metadata messages
								// (used to be needed when the old SC2 spec used an APIC tag to
								// to send the artwork in the metadata message - now it's sent in
								// in the uvox_class == 0x4 messages for stream and playing cases)
								if (!uvox_stream_data[8])
								{
									char *mbuf = (char*)uvox_stream_data + 12;
									if (mbuf)
									{
										unsigned long delay = 0;

										if (uvox_avgbr)
										{
											long byterate = (uvox_avgbr / 8);
											long bytes = 1024 * 11;

											float localdelay = (float)bytes / (float)byterate;
											delay = (long)localdelay * 1000;
										}

										// make sure that we've got a packet which is within the specs
										// as id can only be 1-32, total parts can only be 1-32,
										// id cannot be more than total parts and if not then skip it.
										if(uvox_stream_data[11] < 1 || uvox_stream_data[11] > 32 ||
										   uvox_stream_data[11] > uvox_stream_data[9] ||
										   uvox_stream_data[9] < 1 || uvox_stream_data[9] > 32)
										{
											break;
										}

										TITLELISTTYPE *newtitle = newTitleListEntry();
										newtitle->style = (uvox_type == 0x902 ? UVOX_METADATA_STYLE_SHOUTCAST2 : UVOX_METADATA_STYLE_AOLRADIO);
										// we make sure to only copy up to the maximum metadata payload size
										newtitle->part_len = min((uvox_len - 6), 16371);
										memcpy(newtitle->title, mbuf, newtitle->part_len);
										newtitle->part = uvox_stream_data[11];
										newtitle->total_parts = uvox_stream_data[9];
										newtitle->timer = (stream_bytes_read && mod.outMod ? delay + mod.outMod->GetOutputTime() : 0);
									}
								}
							}

							else if (uvox_class == 0x4) // SHOUTcast 2 albumart
							{
								if (allow_scartwork && !uvox_stream_data[8])
								{
									char *mbuf = (char*)uvox_stream_data + 12;
									if (mbuf)
									{
										// [0x4] [0|1] [00|01|02|03]
										unsigned long delay = 0;

										if (uvox_avgbr)
										{
											long byterate = (uvox_avgbr / 8);
											long bytes = 1024 * 11;

											float localdelay = (float)bytes / (float)byterate;
											delay = (long)localdelay * 1000;
										}

										// make sure that we've got a packet which is within the specs
										// as id can only be 1-32, total parts can only be 1-32,
										// id cannot be more than total parts and if not then skip it.
										if(uvox_stream_data[11] < 1 || uvox_stream_data[11] > 32 ||
										   uvox_stream_data[11] > uvox_stream_data[9] ||
										   uvox_stream_data[9] < 1 || uvox_stream_data[9] > 32)
										{
											break;
										}

										TITLELISTTYPE *newtitle = newTitleListEntry();
										newtitle->style = (uvox_type & 0x100 ? UVOX_METADATA_STYLE_SHOUTCAST2_ARTWORK_PLAYING : UVOX_METADATA_STYLE_SHOUTCAST2_ARTWORK);
										// we make sure to only copy up to the maximum metadata payload size
										newtitle->part_len = min((uvox_len - 6), 16371);
										memcpy(newtitle->title, mbuf, newtitle->part_len);
										newtitle->part = uvox_stream_data[11];
										newtitle->total_parts = uvox_stream_data[9];
										newtitle->type = (uvox_type & 0x00FF);
										newtitle->timer = (stream_bytes_read && mod.outMod ? delay + mod.outMod->GetOutputTime() : 0);
									}
								}
							}

							else if (uvox_class == 0x3 && (uvox_type == 0x001 || uvox_type == 0x002))
							{
								int w = uvox_type - 1; // should be ID?
								int n = (uvox_stream_data[8] << 8) | uvox_stream_data[9];
								if (n && n < 33)
								{
									int t = (uvox_stream_data[10] << 8) | uvox_stream_data[11];
									if (t && t <= n)
									{
										int l = 0;
										int a;
										char *p = (char*)uvox_stream_data + 12; // this almost works
										free(uvox_meta[w][t - 1]);
										uvox_meta[w][t - 1] = _strdup(p);
										for (a = 0;a < n;a++)
										{
											if (!uvox_meta[w][a]) break;
											l += (int)strlen(uvox_meta[w][a]);
										}
										if (a == n)
										{
											char *outtext = (char*)malloc(l + 1);
											p = outtext;
											for (a = 0;a < n;a++)
											{
												lstrcpynA(p, uvox_meta[w][a], l + 1);
												free(uvox_meta[w][a]);
												uvox_meta[w][a] = 0;
												p += strlen(p);
											}
											processMetaData(outtext, l + 1);
											free(outtext);
										}
									}
								}
							}

							else if ((uvox_class == 0x7 && (uvox_type == 0x0 || uvox_type == 0x1))
								|| (uvox_class == 0x8 && (uvox_type == 0x0 || uvox_type == 0x1 || uvox_type == 0x3)))
							{
								memcpy(uvox_stream_data, uvox_stream_data + 6, uvox_len);
								uvox_stream_data_len = uvox_len;
								uvox_last_message = uvox_class << 12 | uvox_type;
							}
							jnl_connection_receive(m_connection, NULL, 6 + uvox_len + 1);

							uvox_message_cnt++;
						}
						if (uvox_stream_data_len > 0)
						{
							len = min(cbToRead, uvox_stream_data_len);

							memcpy(pBuffer, uvox_stream_data, len);
							if (pcbRead) *pcbRead = len;
							stream_bytes_read += len;
							if (len < uvox_stream_data_len)
							{
								memcpy(uvox_stream_data, uvox_stream_data + len, uvox_stream_data_len - len);
							}
							uvox_stream_data_len -= len;
							break;
						}
					}
					else
					{
						len = min(cbToRead, len);
						if (meta_interval) len = min(meta_interval - meta_pos, len);
						if (len > 0)
						{
							DWORD dw = 0;
							len = (int)jnl_connection_receive(m_connection, (char*)pBuffer, len);
							if (hFile != INVALID_HANDLE_VALUE) WriteFile(hFile, pBuffer, len, &dw, NULL);
							if (pcbRead) *pcbRead = len;
							meta_pos += len;
							stream_bytes_read += len;
						}
						else if (pcbRead) *pcbRead = 0;
						break;
					}
				}
			}

			int state = m_connection ? jnl_connection_get_state(m_connection) : JNL_CONNECTION_STATE_CLOSED;
			if (state == JNL_CONNECTION_STATE_ERROR || state == JNL_CONNECTION_STATE_CLOSED)
			{
				if ((!pcbRead || !*pcbRead) && (!m_connection || jnl_connection_receive_bytes_available(m_connection) < 1))
				{
					fEof = 1;
					return NErr_Error;
				}
			}
			else if (constate != 5 && constate != 4 && constate != 3 && timeout_start + 15000 < GetTickCount())  // 15 second net connect timeout
			{
				EnterCriticalSection(&g_lfnscs);
				WASABI_API_LNGSTRING_BUF(IDS_TIMED_OUT,lastfn_status,256);
				lastfn_status_err = 1;
				LeaveCriticalSection(&g_lfnscs);
				PostMessage(mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
				fEof = 1;

				return NErr_Error;
			}
			return NErr_Success;
		}
		return NErr_Error;
	}
	else if (m_full_buffer)
	{
		int len = cbToRead;
		if ((uint64_t)len > (mpeg_length - m_full_buffer_pos))
		{
			len = (int)(mpeg_length - m_full_buffer_pos);
		}
		if (pcbRead) *pcbRead = len;
		if (len)
		{
			memcpy(pBuffer, m_full_buffer + m_full_buffer_pos, len);
			m_full_buffer_pos += len;
		}
		else
		{
			fEof = true;
			return NErr_EndOfFile;
		}
		return NErr_Success;
	}
	else
	{
		if (hFile != INVALID_HANDLE_VALUE)
		{
			if ((uint64_t)cbToRead >= (mpeg_length - file_position))
			{
				cbToRead = (int)(mpeg_length - file_position);
				fEof = true;
			}

			if (cbToRead == 0)
			{
				if (pcbRead)
					*pcbRead = 0;
				return NErr_Success;
			}

			DWORD dwRead = 0;
			bSuccess = ReadFile(hFile, pBuffer, cbToRead, &dwRead, NULL);

			if (bSuccess)
			{
				file_position += dwRead;
				// update pcbRead
				if (pcbRead)
					*pcbRead = dwRead;

				// check for EOF
				if (dwRead == 0)
					fEof = true;

				return NErr_Success;
			}
			else
			{
				// error reading from file
				return NErr_Error;
			}
		}
		else
		{
			// no valid file handle
			return NErr_Error;
		}
	}
}

//-------------------------------------------------------------------------*
//   IsEof
//-------------------------------------------------------------------------*

bool CGioFile::IsEof() const
{
	return fEof;
}

//-------------------------------------------------------------------------*
//   GetContentLength
//-------------------------------------------------------------------------*

DWORD CGioFile::GetContentLength(void) const
{
	DWORD dwSize = 0 ;

	dwSize = (DWORD)mpeg_length;

	return dwSize ;
}

//-------------------------------------------------------------------------*
//   GetCurrentPosition
//-------------------------------------------------------------------------*

DWORD CGioFile::GetCurrentPosition(void) const
{
	DWORD dwPos = 0;

	if (m_is_stream)
	{
		dwPos = (DWORD)stream_bytes_read;
	}
	else if (m_full_buffer)
	{
		dwPos = (DWORD)m_full_buffer_pos;
	}
	else if (hFile != INVALID_HANDLE_VALUE)
	{
		dwPos = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
		dwPos -= (DWORD)(mpeg_position + peekBuffer.size());
	}

	return dwPos ;
}

//-------------------------------------------------------------------------*
//   SetCurrentPosition
//-------------------------------------------------------------------------*

void CGioFile::SetCurrentPosition(long dwPos, int How) // GIO_FILE_BEGIN only
{
	fEof = false;
	if (m_full_buffer)
	{
		m_full_buffer_pos = dwPos;
		if (m_full_buffer_pos < 0) m_full_buffer_pos = 0;
		if (m_full_buffer_pos > mpeg_length) m_full_buffer_pos = mpeg_length;
	}
	else if (hFile != INVALID_HANDLE_VALUE)
	{
		Seek64(hFile, mpeg_position + dwPos, FILE_BEGIN);
		file_position = dwPos;
		peekBuffer.clear();
	}
}

int CGioFile::GetHeaderOffset()
{
	return (int)mpeg_position;
}
/*-------------------------------------------------------------------------*/

void CGioFile::Seek(int posms, int br)
{
	int offs = 0;
	if (m_vbr_hdr)
	{
		offs = m_vbr_hdr->seekPointByTime((float)posms);
	}
	else if (!m_vbr_flag)
		offs = MulDiv(posms, br, 8);
	else
	{
		int ms = 0;
		int fl = GetContentLength();
		if (!m_vbr_frames)
		{
			ms = MulDiv(fl, 8 * 1000, br);
		}
		else ms = m_vbr_ms;
		offs = SeekPoint(m_vbr_toc, fl, (float)posms / ((float)ms / 100.0f));
	}
	if (m_is_stream)
	{
		if (GetContentLength() > 0)
		{
			if (m_connection && m_is_stream_seekable)
			{
				jnl_connection_release(m_connection);
				m_connection = NULL;
				m_is_stream_seek = true;
				m_seek_reset = false;
				doConnect(NULL, offs);
			}
		}
	}
	else
	{
		SetCurrentPosition(offs, GIO_FILE_BEGIN);
	}
}

bool CGioFile::IsSeekable()
{
	return !m_is_stream || GetContentLength() > 0;
}

void CGioFile::GetStreamInfo(wchar_t *obuf, size_t len)
{
	if (m_is_stream)
	{
		wchar_t langbuf[2048]={0};
		StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_NETWORK_RECEIVED_X_BYTES, langbuf, 2048), stream_bytes_read + stream_metabytes_read);
		if (server_name[0])
			StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_SERVER, langbuf, 2048), AutoWide(server_name,CP_UTF8));
		if (m_content_type && m_content_type[0])
		{
			if(is_uvox)
			{
				// report the actual content type instead of just misc/ultravox to make it easier to see what the stream type is (helps debugging)
				static const int MP3_DATA = 0x7000;
				static const int VLB_DATA = 0x8000;
				static const int AAC_LC_DATA = 0x8001;
				static const int AACP_DATA = 0x8003;
				static const int OGG_DATA = 0x8004;
				switch(uvox_last_message)
				{
					case MP3_DATA:
						StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_CONTENT_TYPE, langbuf, 2048), L"audio/mpeg");
					break;

					case VLB_DATA:
						StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_CONTENT_TYPE, langbuf, 2048), L"audio/vlb");
					break;

					case AAC_LC_DATA:
					case AACP_DATA:
						StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_CONTENT_TYPE, langbuf, 2048), L"audio/aacp");
					break;

					case OGG_DATA:
						StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_CONTENT_TYPE, langbuf, 2048), L"audio/ogg");
					break;

					default:
						StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_CONTENT_TYPE, langbuf, 2048), AutoWide(m_content_type,CP_UTF8));
					break;
				}
			}
			else
			{
				StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_CONTENT_TYPE, langbuf, 2048), AutoWide(m_content_type,CP_UTF8));
			}
		}

		if (is_uvox)
		{
			StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_ULTRAVOX_SYNC, langbuf, 2048), uvox_message_cnt, uvox_desyncs);
			StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_ULTRAVOX_DATA_MESSAGE, langbuf, 2048), uvox_last_message);
			StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_ULTRAVOX_SID_AVGBR_MAXBR, langbuf, 2048), uvox_sid, uvox_avgbr, uvox_maxbr);
		}

		if (stream_metabytes_read)
			StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_METADATA_RECEIVED, langbuf, 2048), stream_metabytes_read);
		if (meta_interval)
			StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_METADATA_INTERVAL, langbuf, 2048), meta_interval);
		if (m_id3v2_len)
			StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_ID3v2_TAG, langbuf, 2048), m_id3v2_len);
		if (m_vbr_frame_len)
			StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_VBR_LEADING_FRAME, langbuf, 2048), m_vbr_frame_len);
		if (stream_title_save[0])
		{
			wchar_t name[580]={0};
			ConvertTryUTF8(stream_title_save, name, 580);
			StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_STREAM_NAME, langbuf, 2048), name/*AutoWide(stream_title_save,CP_UTF8)*/);
		}
		if (last_title_sent[0])
		{
			wchar_t title[256]={0};
			ConvertTryUTF8(last_title_sent, title, 256);
			StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_CURRENT_TITLE, langbuf, 2048), title);
		}

		if (mpeg_length)
			StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_CONTENT_LENGTH, langbuf, 2048), mpeg_length);
		if (save_filename[0] && hFile != INVALID_HANDLE_VALUE)
			StringCchPrintfEx(obuf, len, &obuf, &len, 0, WASABI_API_LNGSTRINGW_BUF(IDS_SAVING_TO, langbuf, 2048), AutoWide(save_filename,CP_UTF8));
	}
}

static inline const wchar_t *IncSafe(const wchar_t *val, int x)
{
	while (x--)
	{
		if (val && *val)
			val++;
	}
	return val;
}

void CGioFile::ReadiTunesGaps()
{
	if (info.HasData() && !prepad && !postpad)
	{
		wchar_t str[128] = {0};
		if (info.GetString("pregap", str, 128) == 1)
			prepad = _wtoi(str);

		str[0]=0;
		if (info.GetString("postgap", str, 128) == 1)
			postpad = _wtoi(str);
	}
}

#define CBCLASS CGioFile
START_DISPATCH;
CB( MPEGSTREAM_PEEK, Peek )
CB( MPEGSTREAM_READ, Read )
CB( MPEGSTREAM_EOF,  EndOf )
CB( MPEGSTREAM_GAIN, GetGain )
END_DISPATCH;
#undef CBCLASS