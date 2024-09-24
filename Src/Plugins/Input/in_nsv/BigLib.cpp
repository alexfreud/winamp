#ifdef WINAMPX
#define WIN32_LEAN_AND_MEAN	1
#include <windows.h>
#include "stdio.h"
#include "proxydt.h"
#include <winsock2.h>
#include ".\ws2tcpip.h"
#include ".\wininet.h"
#include "../jnetlib/jnetlib.h"

#include <urlmon.h>


extern void SendMetadata( char *data, int arg );
HRESULT JNetLibDownloadToFile(LPVOID lpUnused1, LPSTR lpWPADLocation, LPSTR lpTempFile, LPVOID lpUnused2, LPVOID lpUnused3);
enum {
	BK_UNKNOWN = 0,
	BK_IE4 = 1,
	BK_NETSCAPE4 = 2,
	BK_NETSCAPE6 = 3,
	BK_MOZILLA = 4,
	BK_FIREFOX = 5
};

// browser info struct
typedef struct{
	LPSTR	lpName;
	BOOL	bSupported;
}BK_INFO;

// browser info
BK_INFO		BrowserInfo[] = {
	"Unknown",			FALSE, 
	"IE 4.0+",			TRUE, 
	"Netscape 4 or 5",	FALSE,
	"Netscape 6+",		TRUE,
	"Mozilla",			TRUE,
	"Firefox",			TRUE
};

// Global variables
int		gBrowserKind = BK_UNKNOWN;

int     gTryAuto = 1;

// Exported C functions
extern "C" BOOL	ProxyInit();
extern "C" void	ProxyDeInit();
extern "C" int ResolvProxyFromURL(LPSTR lpURL, LPSTR lpHostname, LPSTR lpDest);

// Global C functions
BOOL	IsIEProxySet();
int		GetIESettings();
int		ResolveURL_IE(LPSTR lpURL, LPSTR lpHostname, LPSTR lpIPAddress, int sizeof_address, int *pnPort);
BOOL	IsFirefoxProxySet();
int		GetFirefoxSettings();
int		ResolveURL_Firefox(LPSTR lpURL, LPSTR lpHostname, LPSTR lpIPAddress, int sizeof_address, int *pnPort);
BOOL	IsMozillaProxySet();
int		GetMozillaSettings();
int		ResolveURL_Mozilla(LPSTR lpURL, LPSTR lpHostname, LPSTR lpIPAddress, int sizeof_address, int *pnPort);
int		GetDefaultBrowser();
int		ReadWPADFile(LPSTR lpWPADLocation, LPSTR pIPAddress, int *pnPort);
int		GetFirefoxOrMozillaSettings(BOOL bFirefox);
BOOL	IsFirefoxOrMozillaProxySet(BOOL bFirefox);
int		ResolveURL_MozillaOrFirefox(BOOL bFirefox, LPSTR lpURL, LPSTR lpHostname, LPSTR lpIPAddress, int sizeof_address, int *pnPort);

// exported functions
extern "C" BOOL ProxyInit()
{
	BOOL	bRet;

	bRet = FALSE;
	gBrowserKind = GetDefaultBrowser();
	
	switch(gBrowserKind) {
		case BK_IE4:
			bRet = IsIEProxySet();
			break;

		case BK_MOZILLA:
			bRet = IsMozillaProxySet();
		
			break;

		case BK_FIREFOX:
			bRet = IsFirefoxProxySet();
			break;
	}

	return bRet;
}

extern "C" void ProxyDeInit()
{
}

extern "C" int ResolvProxyFromURL(LPSTR lpURL, LPSTR lpHostname, LPSTR lpDest)
{
	// lpURL = URL to resolve
	// lpHostname = hostname
	// lpDest = where to store the result, such as "www.proxyserver.com:8080"	
	char	szIPAddress[MAX_PATH] = {0};
	int		ret, nPort=0;

	lpDest[0]=0;

	if(lpURL && lpHostname && lpDest) {
		switch(gBrowserKind) {
			case BK_IE4:
				ret = ResolveURL_IE(lpURL, lpHostname, szIPAddress, sizeof(szIPAddress), &nPort);
				break;

			case BK_MOZILLA:
				ret = ResolveURL_Mozilla(lpURL, lpHostname, szIPAddress, sizeof(szIPAddress), &nPort);
				break;

			case BK_FIREFOX:
				ret = ResolveURL_Firefox(lpURL, lpHostname, szIPAddress, sizeof(szIPAddress), &nPort);
				break;
		}

		if(ret == 0) {
      if ( szIPAddress[0] )
      {
        wsprintf(lpDest, "%s:%d", szIPAddress, nPort);
        return 1;
      }
      else return 0;
		}
    else return 0;
	}
  else return -1;

}

int GetDefaultBrowser()
{
	DWORD	dwSize, dwType;
	TCHAR	valueBuf[MAX_PATH] = {0};
	DWORD	valueSize = sizeof(valueBuf);
	HKEY	hKey;
	long	lRet;


	memset(valueBuf, 0, sizeof(valueBuf));
	lRet = RegOpenKeyEx(HKEY_CLASSES_ROOT, "http\\shell\\open\\ddeexec\\Application", 0, KEY_READ, &hKey);
	if (lRet == ERROR_SUCCESS) {
		dwSize = valueSize;
		lRet = RegQueryValueEx(hKey, "", NULL, &dwType, (LPBYTE)valueBuf, &dwSize);
		if(lRet == ERROR_SUCCESS && dwType == REG_SZ) {
			if (_tcsicmp(_T("NSShell"), valueBuf) == 0) {			//NS 4.x
				return BK_NETSCAPE4;
			} else if (_tcsicmp(_T("IExplore"), valueBuf) == 0)	{	//IE 4+
				return BK_IE4;
			} else if (_tcsicmp(_T("Mozilla"), valueBuf) == 0) {	//Mozilla
				return BK_MOZILLA;
			} else if (_tcsicmp(_T("Firefox"), valueBuf) == 0) {	//Firefox
				return BK_FIREFOX;
			}
		}
	}
	RegCloseKey(hKey);

	lRet = RegOpenKeyEx(HKEY_CLASSES_ROOT, "http\\shell\\open\\command", 0, KEY_READ, &hKey);
	if(lRet == ERROR_SUCCESS) {
		dwSize = valueSize;
		lRet = RegQueryValueEx(hKey, "", NULL, &dwType, (LPBYTE)valueBuf, &dwSize);
		if(lRet == ERROR_SUCCESS && dwType == REG_SZ) {
			if(strstr(valueBuf, "MOZILLA")) {
				return BK_MOZILLA;
			}
			if(strstr(valueBuf, "NETSCAPE")) {
				return BK_MOZILLA;
			}
			if(strstr(valueBuf, "FIREFOX")) {
				return BK_FIREFOX;
			}
		}
	}
	RegCloseKey(hKey);

	return BK_UNKNOWN;
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper functions for JSProxy.dll
DWORD __stdcall	ResolveHostName(LPSTR lpszHostName, LPSTR   lpszIPAddress, LPDWORD lpdwIPAddressSize);
BOOL __stdcall	IsResolvable(LPSTR lpszHost);
DWORD __stdcall	GetIPAddress(LPSTR lpszIPAddress, LPDWORD lpdwIPAddressSize);
BOOL __stdcall	IsInNet(LPSTR lpszIPAddress, LPSTR lpszDest, LPSTR lpszMask);

// functions to get IE checkbox state
BOOL			GetAutomaticallyDetectSettingsCheckboxState();
BOOL			GetUseAProxyServerForYourLanCheckboxState();
BOOL			GetAutomaticConfigurationScriptCheckboxState();
BOOL			GetBypassProxyServerForLocalAddressesCheckboxState();

// functions to actually get an IP address and port # of the proxy server
int				GetAutomaticDetectSettings(LPSTR lpIPAddress, int *pnPort);
int				GetProxyServerForLanProxySettings(LPSTR lpIPAddress, int *pnPort);
int				GetAutoConfigScriptProxySettings(LPSTR lpIPAddress, int *pnPort);

// various helper functions
BOOL			IsDirect(LPSTR proxy);
BOOL			IsAProxy(LPSTR proxy);
void			reportFuncErr(TCHAR* funcName);
char *			strstri(LPSTR lpOne, LPSTR lpTwo);
int				GetProxyIP(LPSTR proxy, LPSTR szProxyIP);
int				GetProxyPort(LPSTR proxy);

// some global variables
char			gszURL[1025] = {0};
char			gszHost[256] = {0};

// returns TRUE if the user has set a proxy in IE
BOOL IsIEProxySet()
{
	BOOL		bAutomaticallyDetectSettings = GetAutomaticallyDetectSettingsCheckboxState();
	BOOL		bUseAutomaticConfigurationScript = GetAutomaticConfigurationScriptCheckboxState();
	BOOL		bUseAProxyServerForYourLan = GetUseAProxyServerForYourLanCheckboxState();

	if(bAutomaticallyDetectSettings || bUseAutomaticConfigurationScript || bUseAProxyServerForYourLan) {
		return TRUE;
	}

	return FALSE;
}

int ResolveURL_IE(LPSTR lpURL, LPSTR lpHostname, LPSTR lpIPAddress, int sizeof_address, int *pnPort)
{
	// get the state of the four checkboxes in the proxy settings dialog for IE
	BOOL		bAutomaticallyDetectSettings = GetAutomaticallyDetectSettingsCheckboxState();
	BOOL		bUseAutomaticConfigurationScript = GetAutomaticConfigurationScriptCheckboxState();
	BOOL		bUseAProxyServerForYourLan = GetUseAProxyServerForYourLanCheckboxState();
	//BOOL		bBypassProxyServerForLocalAddresses = GetBypassProxyServerForLocalAddressesCheckboxState();
	int			ret;

	lstrcpyn(gszURL, lpURL, 1025);
	lstrcpyn(gszHost, lpHostname, 256);

	// if nothing checked, return
	if(!bAutomaticallyDetectSettings && !bUseAutomaticConfigurationScript && !bUseAProxyServerForYourLan) {
		return 0;
	}

	// if all three checkboxes on...
	if(bAutomaticallyDetectSettings && gTryAuto)
  {
    		// try the automatic configuration next
		ret = GetAutomaticDetectSettings(lpIPAddress, pnPort);
		if(ret == 0 && *pnPort) {
			return 0;
		}
		gTryAuto = 0;

  }

  if ( bUseAutomaticConfigurationScript)
  {
    		// try the automatic config script method first
		ret = GetAutoConfigScriptProxySettings(lpIPAddress, pnPort);
		if(ret == 0 && *pnPort ) {
			return 0;
		}

  }

  if ( bUseAProxyServerForYourLan) 
  {
    		// if still no success, try the "Use a proxy server for your lan" settings
		ret = GetProxyServerForLanProxySettings(lpIPAddress, pnPort);
		if(ret == 0 && *pnPort) {
			return 0;
		}
  }





		// no success...
		return 0;


}

// handles the "Automatically Detect" checkbox
int GetAutomaticDetectSettings(LPSTR lpIPAddress, int *pnPort)
{
	// By not specifying a domain name, Windows uses the local domain name,
	// so form an http request to go to http://wpad/wpad.dat, 
	// store results in szWPADLocation and call URLDownloadToFileA()
	if(lpIPAddress && pnPort) {
		// download wpad.dat from the URL in szURL
		return ReadWPADFile("http://wpad/wpad.dat", lpIPAddress, pnPort);
	}

	return -1;
}

// handles the "Use automatic configuration script" checkbox
int GetAutoConfigScriptProxySettings(LPSTR lpIPAddress, int *pnPort)
{
	DWORD	dwType, dwSize;
	HKEY	hKey;
	char	szWPADLocation[MAX_PATH] = {0};
	long	lRet;
	int		retval = -1;


	if(!lpIPAddress) {
		return retval;
	}
	if(!pnPort) {
		return retval;
	}

	// use the registry read of HKCU\\software\microsoft\windows\current version\internet settings to see if "Use Automatic Configuration Script" is checked
	lstrcpyn(szWPADLocation, "", MAX_PATH);

	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_READ, &hKey);
	if (lRet == ERROR_SUCCESS) {
		dwSize = sizeof(szWPADLocation);
		lRet = RegQueryValueEx(hKey, "AutoConfigURL", NULL, &dwType, (LPBYTE)szWPADLocation, &dwSize);
		if(lRet == ERROR_SUCCESS && dwType == REG_SZ) {
			retval = ReadWPADFile(szWPADLocation, lpIPAddress, pnPort);
		}
	}
	RegCloseKey(hKey);

	return retval;	//0 = success
}

// handles the "Use a proxy server for your LAN" checkbox
int GetProxyServerForLanProxySettings(LPSTR lpIPAddress, int *pnPort)
{
	DWORD	dwType, dwSize;
	HKEY	hKey;
	BOOL	bDirectOrProxy;
	char	szProxy[MAX_PATH] = {0};
	long	lRet;
	int		retval = -1;

	if(lpIPAddress) {
		strcpy(lpIPAddress, "");
	}
	if(pnPort) {
		*pnPort = 0;
	}
	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_READ, &hKey);
	if (lRet == ERROR_SUCCESS) {
		dwSize = sizeof(szProxy);
		lRet = RegQueryValueEx(hKey, "ProxyServer", NULL, &dwType, (LPBYTE)szProxy, &dwSize);
		if(lRet == ERROR_SUCCESS && dwType == REG_SZ) {
			retval = 0;

			bDirectOrProxy = FALSE;
			if(IsDirect(szProxy)) {
				// string is something like "DIRECT"
				// It's a 'direct' kind of proxy.
				bDirectOrProxy = TRUE;

				// set the 'out' parameters
				if(lpIPAddress) {
					strcpy(lpIPAddress, "");
				}
				if(pnPort) {
					*pnPort = 0;
				}
			}

			if(IsAProxy(szProxy)) {
				char	szProxyIP[MAX_PATH] = {0};

				// string is something like "D
				bDirectOrProxy = TRUE;
				GetProxyIP(szProxy, szProxyIP);
				// It's a 'regular' kind of proxy, with an IP of %s and a port of %d\n", szProxyIP, GetProxyPort(szProxy)

				// set the 'out' parameters
				if(lpIPAddress) {
					strcpy(lpIPAddress, szProxyIP);
				}
				if(pnPort) {
					*pnPort = GetProxyPort(szProxy);
				}
			}

			if(!bDirectOrProxy) {
				// string is something like "10.0.0.1:4543"
				LPSTR	lpColon = NULL;
				
				if ( isdigit(szProxy[0]) )
				{
					lpColon = strchr(szProxy, ':');
					if(lpColon) {
					*lpColon = '\0';

					// set the 'out' parameters
					if(lpIPAddress) {
						strcpy(lpIPAddress, szProxy);
					}
					*lpColon = ':';
					if(pnPort) {
						*pnPort = GetProxyPort(szProxy);
						}
					}
				}
				else if ( strstr(szProxy,"http=") )
				{
					char *p = strstr(szProxy,"http=");
					int offset=  strlen("http=");
					char *semi = strchr(p+offset, ';');
					if(semi) {
					*semi= '\0';
					}
					lpColon = strchr(p+offset, ':');
					if(lpColon) {
					*lpColon = '\0';
					}
					// set the 'out' parameters
					if(lpIPAddress) {
						strcpy(lpIPAddress, p+offset);
					}
					if (lpColon)
					if(pnPort) {
						*pnPort = (short)atoi(lpColon+1);
						}
					if ( !*pnPort ) *pnPort = 80;
					
				}
				else
				{
					if(lpIPAddress) {
					strcpy(lpIPAddress, "");
					}
					if(pnPort) {
						*pnPort = 0;
					}
				}
			}
		}
	}
	RegCloseKey(hKey);
		
	return retval;
}

int ReadWPADFile(LPSTR lpWPADLocation, LPSTR lpIPAddress, int *pnPort)
{
	// Declare function pointers for the three autoproxy functions
	pfnInternetInitializeAutoProxyDll		pInternetInitializeAutoProxyDll;
	pfnInternetDeInitializeAutoProxyDll		pInternetDeInitializeAutoProxyDll;
	pfnInternetGetProxyInfo					pInternetGetProxyInfo;

	// Declare and populate an AutoProxyHelperVtbl structure, and then 
	// place a pointer to it in a containing AutoProxyHelperFunctions 
	// structure, which will be passed to InternetInitializeAutoProxyDll:
	AutoProxyHelperVtbl			Vtbl = {IsResolvable, GetIPAddress, ResolveHostName, IsInNet };
	AutoProxyHelperFunctions	HelperFunctions = { &Vtbl };
	HMODULE						hModJS;
	HRESULT						hr;
	char						szTempPath[MAX_PATH] = {0};
	char						szTempFile[MAX_PATH] = {0};
	int							retval = 0;


	if(!(hModJS = LoadLibrary("jsproxy.dll"))) {
		reportFuncErr("LoadLibrary");
		return -1;
	}

	if(!(pInternetInitializeAutoProxyDll = (pfnInternetInitializeAutoProxyDll)
			GetProcAddress(hModJS, "InternetInitializeAutoProxyDll")) ||
		!(pInternetDeInitializeAutoProxyDll = (pfnInternetDeInitializeAutoProxyDll)
			GetProcAddress(hModJS, "InternetDeInitializeAutoProxyDll")) ||
		!(pInternetGetProxyInfo = (pfnInternetGetProxyInfo)
			GetProcAddress(hModJS, "InternetGetProxyInfo"))) {
				FreeLibrary(hModJS);
				reportFuncErr("GetProcAddress");
				return -1;
	}

	if(lpIPAddress) 
	{
		strcpy(lpIPAddress, "");
	}
	if(pnPort) {
		*pnPort = 0;
	}

	GetTempPathA(sizeof(szTempPath)/sizeof(szTempPath[0]), szTempPath);
	GetTempFileNameA(szTempPath, "X", 2, szTempFile);
	//printf("  Downloading %s ...\n", lpWPADLocation);
	hr = JNetLibDownloadToFile(NULL, lpWPADLocation, szTempFile, NULL, NULL);
	if(hr == S_OK) {
		if(!pInternetInitializeAutoProxyDll(0, szTempFile, NULL, &HelperFunctions, NULL)) {
			//printf("  Calling 'InternetInitializeAutoProxyDll' in JSPROXY.DLL failed\n  (usually because 'Use Automatic Configuration Script' checkbox is OFF)\n");
			pInternetDeInitializeAutoProxyDll(NULL, 0);
			FreeLibrary(hModJS);
			retval = -1;
		}else{
			//  printf("\n  InternetInitializeAutoProxyDll returned: %d\n", returnVal);

			// Delete the temporary file
			// (or, to examine the auto-config script, comment out the
			// file delete and substitute the following printf call)
			// printf("\n  The auto-config script temporary file is:\n    %s\n", szTempFile);
			DeleteFileA(szTempFile);

			DWORD	dwSize = 0;
			LPSTR	pProxy = NULL;
			if(!pInternetGetProxyInfo((LPSTR)gszURL,  sizeof(gszURL), (LPSTR)gszHost, sizeof(gszHost), &pProxy, &dwSize)) {
				reportFuncErr("InternetGetProxyInfo");
				retval = -1;
			}else{
				//  printf("\n  Proxy is: %s\n", proxy);
				if(IsDirect(pProxy)) {
					//printf("  It's a 'direct' kind of proxy.\n");

					// set the 'out' parameters
					if(lpIPAddress) {
						strcpy(lpIPAddress, "");
					}
					if(pnPort) {
						*pnPort = 0;
					}
				}

				if(IsAProxy(pProxy)) {
					char	szProxyIP[MAX_PATH] = {0};

					GetProxyIP(pProxy, szProxyIP);
					//printf("  It's a 'regular' kind of proxy, with an IP of %s and a port of %d\n", szProxyIP, GetProxyPort(szProxy));

					// set the 'out' parameters
					if(lpIPAddress) {
						strcpy(lpIPAddress, szProxyIP);
					}
					if(pnPort) {
						*pnPort = GetProxyPort(pProxy);
					}
				}
			}
		}
	}else{
		//printf("  Error downloading %s (hr=0x%X)\n", lpWPADLocation, hr);
		// there is no proxy, go direct
		if(lpIPAddress) {
			strcpy(lpIPAddress, "");
		}
		if(pnPort) {
			*pnPort = 0;
		}
		retval = 0;
	}

	if(!pInternetDeInitializeAutoProxyDll(NULL, 0)) {
		reportFuncErr("InternetDeInitializeAutoProxyDll");
	}

	return retval;	// 0 = success
}


// Puts "10.0.0.1" into lpDest from a string like "PROXY 10.0.0.1:8088" 
// Returns 0 if success, -1 if an error
int GetProxyIP(LPSTR lpProxy, LPSTR lpDest)
{
	LPSTR	lpData;
	LPSTR	lpLastColon;
	BOOL	bDone;
	char	szProxy[MAX_PATH] = {0};
	int		ret = 0;

	if(lpProxy && lpDest) {
		lstrcpyn(szProxy, lpProxy, MAX_PATH);

		// find the last ":" in the string
		lpLastColon = NULL;
		lpData = szProxy;
		while(*lpData) {
			if(*lpData == ':') {
				lpLastColon = lpData;
			}
			lpData++;
		}

		if(lpLastColon) {
			// truncate the string at the last colon
			*lpLastColon = '\0';

			bDone = FALSE;
			while(lpData > szProxy && !bDone) {
				if(*lpData == ' ') {
					bDone = TRUE;
					lpData++;
				}else{
					lpData--;
				}
			}
			strcpy(lpDest, lpData);
			ret = 0;
		}else {
			strcpy(lpDest, lpProxy);
			ret =0;
		}
	}else{
		ret = -1;
	}

	return ret;
}

// Returns 8088 from a string like "PROXY 10.0.0.1:8088" 
// Returns a port # if success, -1 if an error
int GetProxyPort(LPSTR lpProxy)
{
	LPSTR	lpData;
	LPSTR	lpLastColon = NULL;
	char	szProxy[MAX_PATH] = {0};
	int		ret = -1;

	if(lpProxy) {
		lstrcpyn(szProxy, lpProxy, MAX_PATH);

		// find the last ":" in the string
		lpData = szProxy;
		while(*lpData) {
			if(*lpData == ':') {
				lpLastColon = lpData;
			}
			lpData++;
		}

		// from the last colon to the end of the string is the port number
    if ( lpLastColon )
    {
		  lpLastColon++;
		  ret = (unsigned short)atoi(lpLastColon);
    }
    else ret = 80;
	}

	return ret;
}

BOOL IsDirect(LPSTR proxy)
{
	BOOL	bRet = FALSE;

	if(proxy) {
		if(strstri("DIRECT", proxy)) {
			bRet = TRUE;
		}
	}

	return bRet;
}

BOOL IsAProxy(LPSTR proxy)
{
	BOOL	bRet = FALSE;

	if(proxy) {
		if(strstri("PROXY", proxy)) {
			bRet = TRUE;
		}
	}

	return bRet;
}

// like strstr() but case-insensitive
char * strstri(LPSTR lpOne, LPSTR lpTwo)
{
	unsigned int	b;
	char			szOne[MAX_PATH] = {0}, szTwo[MAX_PATH] = {0};

	if(lpOne && lpTwo) {
		strcpy(szOne, lpOne);
		strcpy(szTwo, lpTwo);

		for(b=0; b<strlen(szOne); b++) {
			szOne[b] = tolower(szOne[b]);
		}

		for(b=0; b<strlen(szTwo); b++) {
			szTwo[b] = tolower(szTwo[b]);
		}
	}

	return strstr(szTwo, szOne);
}

BOOL GetAutomaticallyDetectSettingsCheckboxState()
{
	DWORD	dwSize, dwType;
	HKEY	hKey;
	BOOL	bAutomaticallyDetectSettings = FALSE;
	long	lRet;

	// see if the "Automatically Detect Settings" checkbox is on (I know, it's ugly)
	// I noticed that the 9th byte in a binary struct at HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Connections\DefaultConnectionSettings"
	// changes a bit to 1 or 0 based on the state of the checkbox.  I'm using Windows XP.  Not sure what byte to check on other Windows versions.
	BYTE	Buffer[200] = {0};

	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Connections", 0, KEY_READ, &hKey);
	if (lRet == ERROR_SUCCESS) {
		dwSize = sizeof(Buffer);
		lRet = RegQueryValueEx(hKey, "DefaultConnectionSettings", NULL, &dwType, (LPBYTE)&Buffer, &dwSize);
		if(lRet == ERROR_SUCCESS && dwType == REG_BINARY) {
			if(Buffer[8] & 8) {
				bAutomaticallyDetectSettings = TRUE;
			}
		}
	}
	RegCloseKey(hKey);

	return bAutomaticallyDetectSettings;
}

BOOL GetUseAProxyServerForYourLanCheckboxState()
{
	DWORD	dwSize, dwValue, dwType;
	HKEY	hKey;
	BOOL	bUseAProxyServerForYourLan = FALSE;
	long	lRet;


	// see if the "Use a proxy server for your LAN" checkbox is on
	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_READ, &hKey);
	if (lRet == ERROR_SUCCESS) {
		dwSize = sizeof(DWORD);
		lRet = RegQueryValueEx(hKey, "ProxyEnable", NULL, &dwType, (LPBYTE)&dwValue, &dwSize);
		if(lRet == ERROR_SUCCESS && dwType == REG_DWORD) {
			bUseAProxyServerForYourLan = dwValue;
		}
	}
	RegCloseKey(hKey);

	return bUseAProxyServerForYourLan;
}

BOOL GetAutomaticConfigurationScriptCheckboxState()
{
	DWORD	dwType, dwSize;
	HKEY	hKey;
	BOOL	bUseAutomaticConfigurationScript = FALSE;
	char	szWPAD[MAX_PATH] = {0};
	long	lRet;


#if 1
	// use the registry read of HKCU\\software\microsoft\windows\current version\internet settings to see if "Use Automatic Configuration Script" is checked
	szWPAD[0] = '\0';

	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_READ, &hKey);
	if (lRet == ERROR_SUCCESS) {
		dwSize = sizeof(szWPAD);
		lRet = RegQueryValueEx(hKey, "AutoConfigURL", NULL, &dwType, (LPBYTE)szWPAD, &dwSize);
		if(lRet == ERROR_SUCCESS && dwType == REG_SZ) {

		}
	}
	RegCloseKey(hKey);
#else
	// use DetectAutoProxyURL
	if(!DetectAutoProxyUrl(szWPADLocation, sizeof(szWPADLocation), PROXY_AUTO_DETECT_TYPE_DHCP | PROXY_AUTO_DETECT_TYPE_DNS_A)) {
		reportFuncErr("DetectAutoProxyUrl");
	}
#endif

	if(strlen(szWPAD)) {
		bUseAutomaticConfigurationScript = TRUE;
	}

	return bUseAutomaticConfigurationScript;
}

BOOL GetBypassProxyServerForLocalAddressesCheckboxState()
{
	DWORD	dwSize, dwType;
	HKEY	hKey;
	BOOL	bBypassProxyServerForLocalAddresses = FALSE;
	char	szBuffer[MAX_PATH] = {0};
	long	lRet;

	dwSize = sizeof(szBuffer);
	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_READ, &hKey);
	if (lRet == ERROR_SUCCESS) {
		lRet = RegQueryValueEx(hKey, "ProxyOverride", NULL, &dwType, (LPBYTE)&szBuffer, &dwSize);
		if(lRet == ERROR_SUCCESS && dwType == REG_SZ) {
	
		}
	}
	RegCloseKey(hKey);

	if(strcmp(szBuffer, "<local>") == 0) {
		bBypassProxyServerForLocalAddresses = TRUE;
	}

	return bBypassProxyServerForLocalAddresses;
}


/* ==================================================================
                            HELPER FUNCTIONS
   ================================================================== */
//  ResolveHostName (a helper function)
DWORD __stdcall ResolveHostName(LPSTR lpszHostName, LPSTR lpszIPAddress, LPDWORD lpdwIPAddressSize)
{ 
  DWORD dwIPAddressSize;
  addrinfo Hints;
  LPADDRINFO lpAddrInfo;
  LPADDRINFO IPv4Only;
  DWORD error;

  // Figure out first whether to resolve a name or an address literal.
  // If getaddrinfo() with the AI_NUMERICHOST flag succeeds, then
  // lpszHostName points to a string representation of an IPv4 or IPv6 
  // address. Otherwise, getaddrinfo() should return EAI_NONAME.
  ZeroMemory(&Hints, sizeof(addrinfo));
  Hints.ai_flags    = AI_NUMERICHOST;  // Only check for address literals.
  Hints.ai_family   = PF_UNSPEC;       // Accept any protocol family.
  Hints.ai_socktype = SOCK_STREAM;     // Constrain results to stream socket.
  Hints.ai_protocol = IPPROTO_TCP;     // Constrain results to TCP.

  error = getaddrinfo(lpszHostName, NULL, &Hints, &lpAddrInfo);
  if(error != EAI_NONAME) {
    if(error != 0) {
      error = (error == EAI_MEMORY) ?
              ERROR_NOT_ENOUGH_MEMORY : ERROR_INTERNET_NAME_NOT_RESOLVED;
      goto quit;
    }
    freeaddrinfo(lpAddrInfo);

    // An IP address (either v4 or v6) was passed in, so if there is 
    // room in the lpszIPAddress buffer, copy it back out and return.
    dwIPAddressSize = lstrlen(lpszHostName);

    if((*lpdwIPAddressSize < dwIPAddressSize) || (lpszIPAddress == NULL)) {
      *lpdwIPAddressSize = dwIPAddressSize + 1;
      error = ERROR_INSUFFICIENT_BUFFER;
      goto quit;
    }
    lstrcpy(lpszIPAddress, lpszHostName);
    goto quit;
  }

  // Call getaddrinfo() again, this time with no flag set.
  Hints.ai_flags = 0;
  error = getaddrinfo(lpszHostName, NULL, &Hints, &lpAddrInfo);
  if(error != 0) {
    error = (error == EAI_MEMORY) ?
            ERROR_NOT_ENOUGH_MEMORY : ERROR_INTERNET_NAME_NOT_RESOLVED;
    goto quit;
  }

  // Convert the IP address in addrinfo into a string.
  // (the following code only handles IPv4 addresses)
  IPv4Only = lpAddrInfo;
  while(IPv4Only->ai_family != AF_INET) {
    IPv4Only = IPv4Only->ai_next;
    if(IPv4Only == NULL)
    {
      error = ERROR_INTERNET_NAME_NOT_RESOLVED;
      goto quit;
    }
  }
  error = getnameinfo(IPv4Only->ai_addr, (socklen_t)IPv4Only->ai_addrlen, lpszIPAddress, *lpdwIPAddressSize, NULL, 0, NI_NUMERICHOST);
  if(error != 0)
    error = ERROR_INTERNET_NAME_NOT_RESOLVED;

quit:
  return(error);
}


//  IsResolvable (a helper function)
BOOL __stdcall IsResolvable(LPSTR lpszHost)
{
  char szDummy[255] = {0};
  DWORD dwDummySize = sizeof(szDummy) - 1;

  if(ResolveHostName(lpszHost, szDummy, &dwDummySize))
    return(FALSE);

  return TRUE;
}


//  GetIPAddress (a helper function)
DWORD __stdcall GetIPAddress(LPSTR   lpszIPAddress, LPDWORD lpdwIPAddressSize)
{
  char szHostBuffer[255] = {0};

  if(gethostname(szHostBuffer, sizeof(szHostBuffer) - 1) != ERROR_SUCCESS)
    return(ERROR_INTERNET_INTERNAL_ERROR);

  return(ResolveHostName(szHostBuffer, lpszIPAddress, lpdwIPAddressSize));
}


//  IsInNet (a helper function)
BOOL __stdcall IsInNet(LPSTR lpszIPAddress, LPSTR lpszDest, LPSTR lpszMask)
{
  DWORD dwDest;
  DWORD dwIpAddr;
  DWORD dwMask;

  dwIpAddr = inet_addr(lpszIPAddress);
  dwDest   = inet_addr(lpszDest);
  dwMask   = inet_addr(lpszMask);

  if((dwDest == INADDR_NONE) || (dwIpAddr == INADDR_NONE) || ((dwIpAddr & dwMask) != dwDest))
    return(FALSE);

  return(TRUE);
}


//  reportFuncErr (simple error reporting)
void reportFuncErr(TCHAR* funcName)
{
  //printf("  ERROR: %s failed with error number %d.\n", funcName, GetLastError());
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////
class CProfileFolder
{
public:
	int					GetProfileFolder(LPSTR lpProfileFolder, BOOL bFirefox);

private:				
	int					GetProfileFolder_9598ME(LPSTR lpProfileFolder, BOOL bFirefox);
	int					GetProfileFolder_2000XP(LPSTR lpProfileFolder, BOOL bFirefox);
};



/////////////////////////////////////////////////////////////////////////////////////////////////////////
class CMozSettings
{
public:
						CMozSettings(BOOL bFirefox);
	virtual				~CMozSettings();
	int					GetPreference(LPSTR lpPreferenceWanted, int *pnDest);
	int					GetPreference(LPSTR lpPreferenceWanted, LPSTR lpDest, int sizeof_dest);

private:
	CProfileFolder		m_pf;
	HGLOBAL				m_hData;
	LPSTR				m_lpData;
	int					m_sizeof_data;
};

int CProfileFolder::GetProfileFolder(LPSTR lpProfileFolder, BOOL bFirefox)
{
	// See http://www.mozilla.org/support/firefox/edit for where I got this info:
	// On Windows XP/2000, the path is usually %AppData%\Mozilla\Firefox\Profiles\xxxxxxxx.default\, where xxxxxxxx is a random string of characters. Just browse to C:\Documents and Settings\[User Name]\Application Data\Mozilla\Firefox\Profiles\ and the rest should be obvious.
	// On Windows 95/98/Me, the path is usually C:\WINDOWS\Application Data\Mozilla\Firefox\Profiles\xxxxxxxx.default\
	// On Linux, the path is usually ~/.mozilla/firefox/xxxxxxxx.default/
	// On Mac OS X, the path is usually ~/Library/Application Support/Firefox/Profiles/xxxxxxxx.default/
	OSVERSIONINFO		version;

	ZeroMemory(&version, sizeof(version));
	version.dwOSVersionInfoSize = sizeof(version);
	GetVersionEx(&version);
	if(version.dwMajorVersion == 3) {
		return -1;	// NT 3.51 not supported
	}
	if(version.dwMajorVersion == 4) {
		return GetProfileFolder_9598ME(lpProfileFolder, bFirefox);
	}
	if(version.dwMajorVersion >= 5) {
		return GetProfileFolder_2000XP(lpProfileFolder, bFirefox);
	}

	return -1;
}

// private function for GetProfileFolder()
// on my test system, the folder to get is c:\windows\application data\mozilla\profiles\default\y3h9azmi.slt
int CProfileFolder::GetProfileFolder_9598ME(LPSTR lpProfileFolder, BOOL bFirefox)
{
	WIN32_FIND_DATA		fd;
	HANDLE				hFind;
	BOOL				bDone, bFound;
	char				szHomePath[MAX_PATH] = {0};
	char				szTemp[MAX_PATH] = {0};


	if(lpProfileFolder) {
		GetEnvironmentVariable("WINDIR", szHomePath, sizeof(szHomePath));
		strcpy(lpProfileFolder, szHomePath);
		if(bFirefox) {
			strcat(lpProfileFolder, "\\Application Data\\Mozilla\\Firefox\\Profiles\\");
		}else{
			strcat(lpProfileFolder, "\\Application Data\\Mozilla\\Profiles\\default\\");
		}

		// find the first folder in the the path specified in szProfileFolder
		lstrcpyn(szTemp, lpProfileFolder, MAX_PATH-4);
		strcat(szTemp, "*.*");

		bDone = FALSE;
		bFound = FALSE;
		hFind = FindFirstFile(szTemp, &fd);
		while(hFind != INVALID_HANDLE_VALUE && !bDone) {
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// we're at a directory.
				// make sure it's not "." or ".."
				if(fd.cFileName[0] != '.') {
					bFound = TRUE;
				}
			}

			bDone = !FindNextFile(hFind, &fd);
		}
		FindClose(hFind);

		if(bFound) {
			strcat(lpProfileFolder, fd.cFileName);
			return 0;
		}
	}

	return -1;
}

// private function for GetProfileFolder()
int CProfileFolder::GetProfileFolder_2000XP(LPSTR lpProfileFolder, BOOL bFirefox)
{
	WIN32_FIND_DATA		fd;
	HANDLE				hFind;
	BOOL				bDone, bFound;
	char				szHomePath[MAX_PATH] = {0};
	char				szTemp[MAX_PATH] = {0};


	if(lpProfileFolder) {
		GetEnvironmentVariable("APPDATA", szHomePath, sizeof(szHomePath));
		strcpy(lpProfileFolder, szHomePath);
		if(bFirefox) {
			strcat(lpProfileFolder, "\\Mozilla\\Firefox\\Profiles\\");
		}else{
			strcat(lpProfileFolder, "\\Mozilla\\Profiles\\default\\");
		}

		// find the first folder in the the path specified in szProfileFolder
		strcpy(szTemp, lpProfileFolder);
		strcat(szTemp, "*.*");

		bDone = FALSE;
		bFound = FALSE;
		hFind = FindFirstFile(szTemp, &fd);
		while(hFind != INVALID_HANDLE_VALUE && !bDone) {
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// we're at a directory.
				// make sure it's not "." or ".."
				if(fd.cFileName[0] != '.') {
					bFound = TRUE;
				}
			}

			bDone = !FindNextFile(hFind, &fd);
		}
		FindClose(hFind);

		if(bFound) {
			strcat(lpProfileFolder, fd.cFileName);
			return 0;
		}
	}

	return -1;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////
CMozSettings::CMozSettings(BOOL bFirefox)
{
	WIN32_FIND_DATA		fd;
	OFSTRUCT			of;
	HANDLE				hFind;
	HFILE				hPrefsFile;
	char				szProfileFolder[MAX_PATH] = {0};
	char				szPrefsFile[MAX_PATH] = {0};
	int					ret;


	m_hData = NULL;
	m_lpData = NULL;

	ret = m_pf.GetProfileFolder(szProfileFolder, bFirefox);


	if(ret == 0) {
		// We found the folder where prefs.js lives. Read it in.
		strcpy(szPrefsFile, szProfileFolder);
		strcat(szPrefsFile, "\\prefs.js");

		// get the size of the file and alloc memory for it
		hFind = FindFirstFile(szPrefsFile, &fd);
		if(hFind != INVALID_HANDLE_VALUE) {
			m_hData = GlobalAlloc(GHND, fd.nFileSizeLow + 256);
			if(m_hData) {
				m_lpData = (LPSTR)GlobalLock(m_hData);
				if(m_lpData) {
					hPrefsFile = OpenFile(szPrefsFile, &of, OF_READ);
					if(hPrefsFile) {
						m_sizeof_data = fd.nFileSizeLow;
						_lread(hPrefsFile, m_lpData, m_sizeof_data);
						_lclose(hPrefsFile);
						hPrefsFile = NULL;
					}
				}
			}

			FindClose(hFind);
		}
	}
}

CMozSettings::~CMozSettings()
{
	if(m_lpData) {
		GlobalUnlock(m_hData);
		m_lpData = NULL;
	}

	if(m_hData) {
		GlobalFree(m_hData);
		m_hData = NULL;
	}
}

int CMozSettings::GetPreference(LPSTR lpPreferenceWanted, LPSTR lpDest, int sizeof_dest)
{
	LPSTR	lpPointer, lpPointerEnd, lpData;
	LPSTR	lpLineStart, lpSearchStart, lpFoundString, lpResult;
	BOOL	bDone;
	int		nDoubleQuoteCount, retval;


	retval = -1;
	if(m_lpData) {
		if(lpPreferenceWanted) {
			if(lpDest) {
				*lpDest = '\0';
				bDone = FALSE;
				lpPointer = m_lpData;
				lpPointerEnd = lpPointer + m_sizeof_data;

				while(lpPointer < lpPointerEnd && !bDone) {
					if(strncmp(lpPointer, "user_pref(", 10) == 0) {
						lpLineStart = lpPointer;
						lpSearchStart = lpLineStart + 11;
						if(strncmp(lpSearchStart, lpPreferenceWanted, strlen(lpPreferenceWanted)) == 0) {
							lpFoundString = lpSearchStart + strlen(lpPreferenceWanted);

							// lpFoundString almost points to what we want.  Skip over the " character it's at now, skip over the " character
							// starting the value we want and null-terminate what we want when we find the 3rd " character
							lpData = lpFoundString;
							nDoubleQuoteCount = 0;
							while(nDoubleQuoteCount <= 3 && !bDone && lpData < lpPointerEnd) {
								if(*lpData == '"') {
									nDoubleQuoteCount++;
									if(nDoubleQuoteCount == 2) {
										// we're at the starting quote
										lpResult = lpData;	
										lpResult++;
									}
									if(nDoubleQuoteCount == 3) {
										// we're at the ending quote
										// null-terminate what we want, and copy it to the dest buffer
										*lpData = '\0';
										lstrcpyn(lpDest, lpResult, sizeof_dest);

										bDone = TRUE;
										retval = 0;
									}
								}
								lpData++;
							}
						}
					}

					lpPointer++;
				}
			}
		}
	}

	return retval;
}

int CMozSettings::GetPreference(LPSTR lpPreferenceWanted, int *pnDest)
{
	LPSTR	lpPointer, lpPointerEnd, lpData;
	LPSTR	lpLineStart, lpSearchStart, lpFoundString;
	BOOL	bDone;
	int		retval;


	retval = -1;
	if(m_lpData) {
		if(lpPreferenceWanted) {
			if(pnDest) {
				bDone = FALSE;
				lpPointer = m_lpData;
				lpPointerEnd = lpPointer + m_sizeof_data;

				while(lpPointer < lpPointerEnd && !bDone) {
					if(strncmp(lpPointer, "user_pref(", 10) == 0) {
						lpLineStart = lpPointer;
						lpSearchStart = lpLineStart + 11;
						if(strncmp(lpSearchStart, lpPreferenceWanted, strlen(lpPreferenceWanted)) == 0) {
							lpFoundString = lpSearchStart + strlen(lpPreferenceWanted);

							// lpFoundString almost points to what we want.  Skip over the " character it's at now, skip over the ","
							// starting the value we want and null-terminate what we want when we find the 3rd " character
							lpData = lpFoundString;
							while(*lpData != ',' && lpData < lpPointerEnd) {
								lpData++;
							}
							if(*lpData == ',') {
								lpData++;

								lpFoundString = lpData;
								while(*lpData != ')' && lpData < lpPointerEnd) {
									lpData++;
								}
								if(*lpData == ')') {
									// null-terminate what we want, and copy it to the dest buffer
									*lpData = '\0';
									*pnDest = atoi(lpFoundString);
									bDone = TRUE;

									retval = 0;
								}
							}
						}
					}

					lpPointer++;
				}
			}
		}
	}

	return retval;
}

////////////////////////////////////////////////////////////////////////
BOOL IsFirefoxProxySet()
{
	return IsFirefoxOrMozillaProxySet(TRUE);
}

BOOL IsMozillaProxySet()
{
	return IsFirefoxOrMozillaProxySet(FALSE);
}

BOOL IsFirefoxOrMozillaProxySet(BOOL bFirefox)
{
	CMozSettings		settings(bFirefox);
	int					ret, nValue;

	ret = settings.GetPreference("network.proxy.type", &nValue);
	if(ret == 0) {
		switch(nValue) {
			case 0:		// shouldn't get here
				break;
			case 1:		// manual configuration
				return TRUE;
			case 2:		// automatic configuration
				return TRUE;
			case 4:		// auto-detect
				return TRUE;
			default:	// don't know
				break;
		}
	}

	return FALSE;
}

int	ResolveURL_Mozilla(LPSTR lpURL, LPSTR lpHostname, LPSTR lpIPAddress, int sizeof_address, int *pnPort)
{
	return ResolveURL_MozillaOrFirefox(FALSE, lpURL, lpHostname, lpIPAddress, sizeof_address, pnPort);
}

int	ResolveURL_Firefox(LPSTR lpURL, LPSTR lpHostname, LPSTR lpIPAddress, int sizeof_address, int *pnPort)
{
	return ResolveURL_MozillaOrFirefox(TRUE, lpURL, lpHostname, lpIPAddress, sizeof_address, pnPort);
}

int	ResolveURL_MozillaOrFirefox(BOOL bFirefox, LPSTR lpURL, LPSTR lpHostname, LPSTR lpIPAddress, int sizeof_address, int *pnPort)
{
	CMozSettings		setting(bFirefox);
	int					ret, nValue;


	// search for the "network.proxy.http" preference
	ret = setting.GetPreference("network.proxy.type", &nValue);
	if(ret == 0) {
		switch(nValue) {
			case 0:
				// shouldn't get here
				break;

			case 1:
				// manual configuration
				setting.GetPreference("network.proxy.http", lpIPAddress, sizeof_address);
				setting.GetPreference("network.proxy.http_port", pnPort); 
				break;

			case 2:
				// automatic configuration
				{
					char	szWPADLocation[MAX_PATH] = {0};

					setting.GetPreference("network.proxy.autoconfig_url", szWPADLocation, sizeof(szWPADLocation));
					ret = ReadWPADFile(szWPADLocation, lpIPAddress, pnPort);
				}
				break;

			case 4:	
				// Auto-detect proxy settings for this network
				ret = ReadWPADFile("http://wpad/wpad.dat", lpIPAddress, pnPort);
				break;

			default:
				break;
		}
	}

	return ret;
}


// My function that downloads from a URL to a file using the Nullsoft JNetLib library instead of using
// URLDownloadToFile().  Only parameters 2 and 3 are used, to mimick the parameters of URLDownloadToFile().
HRESULT JNetLibDownloadToFile(LPVOID lpUnused1, LPSTR lpWPADLocation, LPSTR lpTempFile, LPVOID lpUnused2, LPVOID lpUnused3)
{
	api_httpreceiver *http=0;
	waServiceFactory *sf=0;

	OFSTRUCT		of;
	HGLOBAL			hData;
	HRESULT			hRet = S_FALSE;		// default return value
	LPSTR			lpData;
	DWORD			dwSize;
	HFILE			hFile;
	BOOL			bDone;
	//JNL				jNetLib;
	int				ret;

 
	if(lpWPADLocation && lpTempFile) 
	{
			if (WASABI_API_SVC)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
		if (sf)  http = (api_httpreceiver *)sf->getInterface();
	}
	if (!http)
		return S_FALSE;

		// init the library and open a connection to the URL
    http->Open();
		http->Connect(lpWPADLocation);

		// loop until JNetLib gets the data.
		// run() returns 0 if OK, -1 if error (call geterrorstr()), or 1 if connection closed.
		bDone = FALSE;
		while(!bDone) {
			ret = http->Run();
			if(ret == -1 || ret == 1) {
				bDone = TRUE;
			}
			Sleep(50);
		}
  

		dwSize = (DWORD)http->GetContentLength();
		if(dwSize && ret == 1) {
			// Got something!  
			// Allocate memory for it and write it to lpTempFile
			hData = GlobalAlloc(GHND, dwSize + 100);
			if(hData) {
				lpData = (LPSTR)GlobalLock(hData);
				if(lpData) {
					http->GetBytes(lpData, (int)dwSize);

					// create the output file and write to it
					hFile = OpenFile(lpTempFile, &of, OF_CREATE);
					if(hFile != HFILE_ERROR) {
						_lwrite(hFile, lpData, (UINT)dwSize);
						_lclose(hFile);

						hRet = S_OK;		// success
					}

					GlobalUnlock(hData);
					lpData = NULL;
				}

				GlobalFree(hData);
				hData = NULL;
			}
		}
	}
if (http && sf)
sf->releaseInterface(http);
	return hRet;
}
#endif