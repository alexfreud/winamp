#include "DevEnum.h"
#include "ds2.h"
#include "strsafe.h"

static int device_count=0;
static const GUID NULL_GUID;


BOOL WINAPI DsDevEnum::DSEnumCallback(LPGUID guid,LPCTSTR desc,LPCTSTR,DS_DEV *** var)
{
	DS_DEV * dev=new DS_DEV;

	size_t length=lstrlen(desc);
	length+=4; // "##: "
	length+=1; // null terminator

	dev->name = (TCHAR *)calloc(sizeof(TCHAR),length);

	StringCchPrintf(dev->name,length, TEXT("%02d: %s"), device_count++, desc);

	dev->guid=guid ? *guid : NULL_GUID;

	**var = dev;
	*var=&dev->next;

	return 1;
}

DsDevEnum::DsDevEnum()
{
	DS2::InitDLL();
	ds_devs=0;
  
	DS_DEV ** dev_ptr=&ds_devs;
  device_count = 1;
	if (DS2::pDirectSoundEnumerate)
		DS2::pDirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumCallback,&dev_ptr);

	*dev_ptr=0;
	pDev=ds_devs;
}

DsDevEnum::~DsDevEnum()
{
	DS_DEV* dev=ds_devs;
	while(dev)
	{
		DS_DEV * next=dev->next;
		free(dev->name);
		delete dev;
		dev=next;
	}
//	ds_devs=0;
}

bool DsDevEnum::FindGuid(const GUID & guid)
{
	pDev=ds_devs;
	while(pDev)
	{
		if (pDev->guid== guid) return true;
		pDev=pDev->next;
	}
	return false;
}

bool DsDevEnum::FindDefault()
{
	return FindGuid(NULL_GUID);
}

bool DsDevEnum::FindName(const TCHAR *deviceToFind)
{
	pDev=ds_devs;
	if (!deviceToFind) return true;
	while(pDev)
	{
		if (!lstrcmpi(pDev->name,deviceToFind)) return true;
		pDev=pDev->next;
	}
	return false;
}
/*
const GUID  DsDevEnum::GuidFromName(const char* name)
{
	return FindName(name) ? GetGuid() : NULL_GUID; 
}

const char * DsDevEnum::NameFromGuid(const GUID & g, char * def)
{
	return FindGuid(g) ? GetName() : def;
}
*/
const GUID DsDevEnum::GetGuid()
{
	return pDev ? pDev->guid : NULL_GUID;
}


static bool _getcaps(IDirectSound * pDS,LPDSCAPS pCaps,DWORD * speakercfg)
{
	bool rv = true;
	if (pCaps)
	{
		memset(pCaps,0,sizeof(*pCaps));
		pCaps->dwSize=sizeof(*pCaps);
		if (FAILED(pDS->GetCaps(pCaps))) rv = false;
	}
	if (speakercfg)
	{
		if (FAILED(pDS->GetSpeakerConfig(speakercfg))) rv = false;
	}
	return rv;
}

bool DsDevEnum::GetCapsFromGuid(const GUID *dev,LPDSCAPS pCaps,DWORD * speakercfg)
{
	bool rv=false;
	if (!dev) dev=&NULL_GUID;

	if (DS2::TryGetDevCaps(dev,pCaps,speakercfg)) rv=true;
	else
	{
		IDirectSound * l_pDS=0;
		if (SUCCEEDED(DS2::myDirectSoundCreate(dev,&l_pDS)) && l_pDS)
		{
			rv=_getcaps(l_pDS,pCaps,speakercfg);
			l_pDS->Release();
		}
	}
	return rv;
}
