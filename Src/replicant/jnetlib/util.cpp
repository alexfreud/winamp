/*
** JNetLib
** Copyright (C) 2000-2007 Nullsoft, Inc.
** Author: Justin Frankel
** File: util.cpp - JNL implementation of basic network utilities
** License: see jnetlib.h
*/

#include "netinc.h"
#include "util.h"
#include "foundation/error.h"
#ifdef USE_SSL
#include "sslconnection.h"
#ifdef _WIN32
#include <wincrypt.h>
#endif
#include <openssl/rand.h>

#ifdef _WIN32
static HCRYPTPROV GetKeySet()
{
	HCRYPTPROV   hCryptProv;
	LPCWSTR UserName = L"WinampKeyContainer";  // name of the key container

	if (CryptAcquireContext(
	      &hCryptProv,               // handle to the CSP
	      UserName,                  // container name
	      NULL,                      // use the default provider
	      PROV_RSA_FULL,             // provider type
	      0))                        // flag values
	{
		return hCryptProv;
	}
	else if (CryptAcquireContext(
	           &hCryptProv,
	           UserName,
	           NULL,
	           PROV_RSA_FULL,
	           CRYPT_NEWKEYSET))
	{
		return hCryptProv;
	}
	else
		return 0;
}
#endif

static void InitSSL()
{
	SSL_load_error_strings();
	SSL_library_init();
#ifdef _WIN32
	HCRYPTPROV hCryptProv = GetKeySet();
	if (hCryptProv)
	{
		BYTE pbData[8*sizeof(unsigned long)] = {0};
		if (CryptGenRandom(hCryptProv, 8*sizeof(unsigned long), pbData))
		{
			RAND_seed(pbData, 16);
		}
		CryptReleaseContext(hCryptProv,0);
	}
#endif
//	sslContext = SSL_CTX_new(SSLv23_client_method());
//	SSL_CTX_set_verify(sslContext, SSL_VERIFY_NONE, NULL);

//	SSL_CTX_set_session_cache_mode(sslContext, SSL_SESS_CACHE_OFF);
}
static int open_ssl_initted = 0;
#endif

static int was_initted = 0;

int JNL::open_socketlib()
{
#ifdef _WIN32
	if (!was_initted)
	{
		WSADATA wsaData = {0};
		if (WSAStartup(MAKEWORD(1, 1), &wsaData))
		{
			return NErr_Error;
		}
	}
#endif
#ifdef USE_SSL
	if (!open_ssl_initted)
	{
		InitSSL();
		open_ssl_initted=1;
	}
#endif
	return NErr_Success;
}

void JNL::close_socketlib()
{
#ifdef _WIN32
	if (was_initted)
	{
		WSACleanup();
	}
#ifdef USE_SSL
	// TODO need to do some reference counting to free this correctly
	//SSL_CTX_free(sslContext);
#endif
#endif
}

static char *jnl_strndup(const char *str, size_t n)
{
	char *o = (char *)calloc(n+1, sizeof(char));
	if (!o)
	{
		return 0;
	}
	strncpy(o, str, n);
	o[n]=0;
	return o;
}

int JNL::parse_url(const char *url, char **prot, char **host, unsigned short *port, char **req, char **lp)
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

	while (p && *p && *p == '/') p++; // skip extra /

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
			return NErr_Malformed;

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
	
	return NErr_Success;
}
