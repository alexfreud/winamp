#ifndef NULLSOFT_OUT_DS_DEVENUM_H
#define NULLSOFT_OUT_DS_DEVENUM_H

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include "res_wa2/resource.h"
#include "api.h"
#include "../Winamp/out.h"

class DsDevEnum
{
private:
	struct DS_DEV
	{
		DS_DEV *next;
		TCHAR *name;
		GUID guid;
	}	;

	DS_DEV *pDev;

	DS_DEV *ds_devs;
	static BOOL WINAPI DSEnumCallback(LPGUID guid, LPCTSTR desc, LPCTSTR, DS_DEV *** var);
public:

	const GUID GetGuid();
	inline const TCHAR *GetName(const TCHAR *def = TEXT("device not found")) {
		static wchar_t defStr[64];
		return pDev ? pDev->name : WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_NOT_FOUND,defStr,64);
	}
	inline bool operator++(int) {if (pDev) pDev = pDev->next; return pDev ? true : false;}
	inline operator bool() {return pDev ? true : false;}
	bool FindGuid(const GUID & g);
	bool FindDefault();
	bool FindName(LPCTSTR n);

	DsDevEnum();
	~DsDevEnum();
	inline void Reset() {pDev = ds_devs;}

	static bool GetCapsFromGuid(const GUID *dev, LPDSCAPS pCaps, DWORD * speakercfg = 0);

	inline bool GetCaps(LPDSCAPS pCaps, DWORD * speakercfg = 0) { return GetCapsFromGuid(&pDev->guid, pCaps, speakercfg);}
};

//helpers
class DsDevEnumGuid : public DsDevEnum
{
public:
	DsDevEnumGuid(const GUID & g) {FindGuid(g);}
};

class DsDevEnumName : public DsDevEnum
{
public:
	DsDevEnumName(LPCTSTR n) {FindName(n);}
};

class DsDevEnumDefault : public DsDevEnum
{
public:
	DsDevEnumDefault() {FindDefault();}
};


#endif