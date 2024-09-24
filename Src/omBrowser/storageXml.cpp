#include "main.h"
#include "./storageXml.h"
#include "./resource.h"

#include "./ifc_omservice.h"
#include "./ifc_omserviceenum.h"
#include "./enumXmlFile.h"

#include <strsafe.h>

OmStorageXml::OmStorageXml()
	: ref(1)
{
}

OmStorageXml::~OmStorageXml()
{
}

HRESULT OmStorageXml::CreateInstance(OmStorageXml **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = new OmStorageXml();
	if (NULL == *instance) return E_OUTOFMEMORY;
	
	return S_OK;
}

size_t OmStorageXml::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmStorageXml::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int OmStorageXml::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_OmStorage))
		*object = static_cast<ifc_omstorage*>(this);
	else if (IsEqualIID(interface_guid, STID_OmFileStorage))
		*object = static_cast<ifc_omfilestorage*>(this);
	else if (IsEqualIID(interface_guid, SUID_OmStorageXml))
		*object = static_cast<ifc_omstorage*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT OmStorageXml::GetId(GUID *storageUid)
{
	if (NULL == storageUid) return E_POINTER;
	*storageUid = SUID_OmStorageXml;
	return S_OK;
}

HRESULT OmStorageXml::GetType(GUID *storageType)
{
	if (NULL == storageType) return E_POINTER;
	*storageType = STID_OmFileStorage;
	return S_OK;
}

UINT OmStorageXml::GetCapabilities()
{
	return capLoad | capPublic;
}

HRESULT OmStorageXml::GetDescription(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	Plugin_LoadString(IDS_STORAGE_XML, pszBuffer, cchBufferMax);
	return S_OK;
}

HRESULT OmStorageXml::Load(LPCWSTR pszAddress, ifc_omservicehost *host, ifc_omserviceenum **ppEnum)
{
	if (NULL == ppEnum) return E_POINTER;
	*ppEnum = NULL;

	if (NULL == pszAddress || L'\0' == *pszAddress)
		return E_INVALIDARG;

	return EnumXmlFile::CreateInstance(pszAddress, host, (EnumXmlFile**)ppEnum);
}

HRESULT OmStorageXml::Save(ifc_omservice **serviceList, ULONG listCount, UINT saveFlags, ULONG *savedCount)
{
	return E_NOTIMPL;
}

HRESULT OmStorageXml::Delete(ifc_omservice **serviceList, ULONG listCount, ULONG *deletedCount)
{
	return E_NOTIMPL;
}

HRESULT OmStorageXml::GetFilter(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	return StringCchCopy(pszBuffer, cchBufferMax, L"*.xml");
}

#define CBCLASS OmStorageXml
START_MULTIPATCH;
 START_PATCH(MPIID_OMSTORAGE)

  M_CB(MPIID_OMSTORAGE, ifc_omstorage, ADDREF, AddRef);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, RELEASE, Release);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_GETID, GetId);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_GETTYPE, GetType);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_GETCAPABILITIES, GetCapabilities);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_GETDESCRIPTION, GetDescription);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_LOAD, Load);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_SAVE, Save);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_DELETE, Delete);
   
 NEXT_PATCH(MPIID_OMFILESTORAGE)
  M_CB(MPIID_OMFILESTORAGE, ifc_omfilestorage, ADDREF, AddRef);
  M_CB(MPIID_OMFILESTORAGE, ifc_omfilestorage, RELEASE, Release);
  M_CB(MPIID_OMFILESTORAGE, ifc_omfilestorage, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMFILESTORAGE, ifc_omfilestorage, API_GETFILTER, GetFilter);
 
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS