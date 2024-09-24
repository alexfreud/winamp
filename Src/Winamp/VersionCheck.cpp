/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename: 
** Project:
** Description:
** Author: Ben Allison benski@nullsoft.com
** Created:
**/

#include "Main.h"
#include "resource.h"
#include "api.h"
#include "language.h"

#include "..\Components\wac_network\wac_network_http_receiver_api.h"

#include "api/service/waServiceFactory.h"
#include "Browser.h"
#include "../nu/AutoUrl.h"
#include "../nu/threadname.h"
#include "stats.h"
#include "../nu/threadpool/TimerHandle.hpp"

#include "../WAT/WAT.h"

extern UpdateBrowser *updateBrowser;

class VersionCheckCallback : public ifc_downloadManagerCallback
{
public:
	void OnInit(DownloadToken token)
	{
		api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
		if (http)
		{
			http->AllowCompression();
			http->addheader("Accept: */*");
		}
	}

	int IsCharDigit(char digit)
	{
		WORD type=0;
		GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, &digit, 1, &type);
		return type&C1_DIGIT;
	}

	void OnFinish(DownloadToken token)
	{
		api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
		if (http && http->getreplycode() == 200)
		{
			char *buf;
			size_t size;
			if (WAC_API_DOWNLOADMANAGER->GetBuffer(token, (void **)&buf, &size) == 0)
			{
				//			buf[size] = 0;
				char *p = buf;

				while (size && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
				{
					p++;
					size--;
				}

				char newVer[6] = {0,};
				if (size >= 3 && p[1] == '.')
				{
					size_t i = 0;
					while (size && i != 6 && (i == 1 || IsCharDigit(p[i])))
					{
						newVer[i] = p[i];
						newVer[i + 1] = 0;
						size--;
						i++;
					}
					p += i;
					while (size && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
					{
						size--;
						p++;
					}

					char curVer_tmp[ 6 ] = APP_VERSION;
					char curVer[ 6 ]     = { 0 };

					i = 0;
					size_t j = 0;
					bool l_has_point = false;
					while ( i != 6 && j != 6 )
					{
						if ( IsCharDigit( curVer_tmp[ j ] ) )
						{
							curVer[ i ] = curVer_tmp[ j ];
							i++;
						}

						j++;

						if ( !IsCharDigit( curVer_tmp[ j ] ) && !l_has_point )
						{
							curVer[ i ] = curVer_tmp[ j ];
							l_has_point = true;
							
							i++;
							j++;
						}						
					}

					while (lstrlenA(curVer) < 5)
						StringCchCatA(curVer, 6, "0");
					while (lstrlenA(newVer) < 5)
						StringCchCatA(newVer, 6, "0");

					int verDif = strcmp(newVer, curVer);
					//#if defined(BETA) || defined(NIGHTLY)
					//				if (verDif == 0)
					//					verDif = 1; // if this is a BETA version, then we should upgrade if the versions are equal
					//#endif

					if (verDif == 0) // same version
					{
						char updateNumber[32] = "";
						char *u = updateNumber;
						while (size && u != (updateNumber + 31) && *p && *p != '\r' && *p != '\n')
						{
							size--;
							*u++ = *p++;
							*u = 0;
						}
						int update = atoi(updateNumber);
						if (update > config_newverchk3)
						{
							if (config_newverchk3) // only display update if we've already established a serial #
								verDif = 1;
							config_newverchk3 = update;
						}
					}
					if (verDif > 0) // same version or older
					{
						while (size && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
						{
							size--;
							p++;
						}
						if (size)
						{
							char *disp = (char *)calloc(size + 1, sizeof(char));
							memcpy(disp, p, size);
							disp[size]=0;
							if (!_strnicmp(p, "http://", 7))
							{
								PostMessageW(hMainWindow, WM_WA_IPC, (WPARAM)disp, IPC_UPDATE_URL);
							}
							else
							{
								if (MessageBoxA(NULL, disp, getString(IDS_WINAMP_UPDATE_MSG,NULL,0), MB_YESNO) == IDYES)
								{
									wa::strings::wa_string l_url_new_version( disp );
									myOpenURL( NULL, l_url_new_version.GetW().c_str() );
								}
								free(disp);
							}
						}
					}
				}
			}
		}
		config_newverchk = getDay();
	}

	void OnError(DownloadToken token)
	{
		config_newverchk = getDay();
	}

	RECVS_DISPATCH;
};

#define CBCLASS VersionCheckCallback
START_DISPATCH;
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONINIT, OnInit )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONFINISH, OnFinish )
END_DISPATCH;
#undef CBCLASS

static VersionCheckCallback versionCheckCallback;

static void CheckVersion(int allowedChannel = 0)
{
	if ( WAC_API_DOWNLOADMANAGER )
	{
		char url[ 1024 ] = { 0 };
		char   *urlend      = 0;
		size_t  urlend_size = 0;

		wa::strings::wa_string l_winamp_product_ver( STR_WINAMP_PRODUCTVER );
		l_winamp_product_ver.replaceAll( ",", "." );

		StringCchPrintfExA( url, 1024, &urlend, &urlend_size, 0, "http://client.winamp.com/update/latest-version.php?v=%s", l_winamp_product_ver.GetA().c_str() );

		char uid[ 512 ] = "";
		stats_getuidstr( uid );
		if ( uid[ 0 ] )
			StringCchPrintfExA( urlend, urlend_size, &urlend, &urlend_size, 0, "&ID=%s", uid );

		const wchar_t *langIdentifier = langManager ? ( langManager->GetLanguageIdentifier( LANG_IDENT_STR ) ) : 0;
		if ( langIdentifier )
			StringCchPrintfA( urlend, urlend_size, "&lang=%s", (char *)AutoUrl( langIdentifier ) );


		OSVERSIONINFOEX info;
		ZeroMemory( &info, sizeof( OSVERSIONINFOEX ) );
		info.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );

		GetVersionEx( (LPOSVERSIONINFO)&info );

		char  l_os_version[ 32 ];
		
		sprintf( l_os_version, "&osver=%u.%u.%u", info.dwMajorVersion, info.dwMinorVersion, info.dwBuildNumber );
		strcat( url, l_os_version );

		char szAllowedChannel[32];
		sprintf(szAllowedChannel, "&allowedchannel=%d", allowedChannel);
		strcat(url, szAllowedChannel);

		WAC_API_DOWNLOADMANAGER->DownloadEx( url, &versionCheckCallback, api_downloadManager::DOWNLOADEX_BUFFER );
	}
}

bool DoVerChk(int verchk)
{
	return verchk == 1 || (verchk > 1 && verchk + 1 < (int)getDay());
}

class PingCallback : public ifc_downloadManagerCallback
{
public:
	void OnInit(DownloadToken token)
	{
		api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
		if (http)
			http->addheader("Accept: */*");
	}
	RECVS_DISPATCH;
};

#define CBCLASS PingCallback
START_DISPATCH;
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONINIT, OnInit)
END_DISPATCH;
#undef CBCLASS

static PingCallback pingCallback;

void Ping(const char *url)
{
	if (WAC_API_DOWNLOADMANAGER)
		WAC_API_DOWNLOADMANAGER->DownloadEx(url, &pingCallback, api_downloadManager::DOWNLOADEX_BUFFER);
}

void newversioncheck(void)
{
	if (isInetAvailable())
	{
		if (DoVerChk(config_newverchk))
		{
			// go ahead and call this on the main thread to ensure that the GUID gets created w/o a race condition
			char uid[512]="";
			stats_getuidstr(uid);
			CheckVersion(config_newverchk_rc);
		}
		if (DoVerChk(config_newverchk2))
		{
			char _url[MAX_URL] = {0};
			char *url=_url;
			size_t urlsize=MAX_URL;

			StringCchPrintfExA(url, urlsize, &url, &urlsize, 0,"http://client.winamp.com/update/client_session.php?v=%s",APP_VERSION);

			char uid[512]="";
			stats_getuidstr(uid);
			if (uid[0])
				StringCchPrintfExA(url, urlsize, &url, &urlsize, 0,"&ID=%s", uid);

			int values[Stats::NUM_STATS] = {0, };
			stats.GetStats(values);
			for (int x = 0; x < Stats::NUM_STATS; x ++)
			{
				StringCchPrintfExA(url, urlsize, &url, &urlsize, 0,"&st%d=%d", x + 1, values[x]);
			}

			wchar_t stat_str[256] = {0};
			stats.GetString("skin", stat_str, 256);
			if (stat_str[0])
				StringCchPrintfExA(url, urlsize, &url, &urlsize, 0,"&skin=%s",(char *)AutoUrl(stat_str));

			stats.GetString("colortheme", stat_str, 256);
			if (stat_str[0])
				StringCchPrintfExA(url, urlsize, &url, &urlsize, 0,"&ct=%s",(char *)AutoUrl(stat_str));

			stats.GetString("pmp", stat_str, 256);
			if (stat_str[0])
				StringCchPrintfExA(url, urlsize, &url, &urlsize, 0,"&pmp=%s",(char *)AutoUrl(stat_str));

			const wchar_t *langIdentifier = langManager?(langManager->GetLanguageIdentifier(LANG_IDENT_STR)):0;
			if (langIdentifier)
				StringCchPrintfExA(url, urlsize, &url, &urlsize, 0, "&lang=%s", (char *)AutoUrl(langIdentifier));

			Ping(_url);
			config_newverchk2 = getDay();
		}
	}
}