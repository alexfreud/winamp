#include "main.h"
#include "./loaderIni.h"
#include "./service.h"
#include "./ifc_wasabihelper.h"
#include "./ifc_omservicehost.h"
#include "./ifc_omstorage.h"
#include "./ifc_omstoragehandler.h"
#include "./ifc_omstoragehandlerenum.h"

#include <shlwapi.h>
#include <strsafe.h>

#define OMS_GROUP			"OnlineService"
#define OMS_ID				"id"
#define OMS_NAME			"name"
#define OMS_URL				"url"
#define OMS_ICON			"icon"
#define OMS_FLAGS			"flags"
#define OMS_RATING			"rating"
#define OMS_VERSION			"version"
#define OMS_DESCRIPTION		"description"
#define OMS_AUTHORFIRST		"authorFirst"
#define OMS_AUTHORLAST		"authorLast"
#define OMS_PUBLISHED		"publishedDate"
#define OMS_UPDATED			"updatedDate"
#define OMS_THUMBNAIL		"thumbnail"
#define OMS_SCREENSHOT		"screenshot"
#define OMS_GENERATION		"generation"

typedef HRESULT (ifc_omservice::*SVCSTRGET)(LPWSTR, UINT);
typedef HRESULT (ifc_omservice::*SVCUINTGET)(UINT*);
typedef HRESULT (ifc_omservicedetails::*DETAILGET)(LPWSTR, UINT);

#define WRITE_OBJECT_STR(__mofifiedFlag, __class, __getterType, __object, __getter, __key)\
	{if (0 != (ifc_omserviceeditor::##__mofifiedFlag & modified)) {\
		HRESULT r = WriteObjectStr<__class, __getterType>((__key), (__object), &##__class::##__getter);\
		if (FAILED(r)) {hr = E_FAIL;} else { saved |= ifc_omserviceeditor::##__mofifiedFlag; }}}

#define WRITE_OBJECT_STR_FALLBACK(__mofifiedFlag, __class, __getterType, __object, __getter1, __getter2, __key)\
	{if (0 != (ifc_omserviceeditor::##__mofifiedFlag & modified)) {\
		HRESULT r = WriteObjectStr<__class, __getterType>((__key), (__object), &##__class::##__getter1);\
		if (S_FALSE == r) r = WriteObjectStr<__class, __getterType>((__key), (__object), &##__class::##__getter2);\
		if (FAILED(r)) {hr = E_FAIL;} else { saved |= ifc_omserviceeditor::##__mofifiedFlag; }}}

#define WRITE_OBJECT_UINT(__mofifiedFlag, __class, __getterType, __object, __getter, __key, __writeFlags)\
	{if (0 != (ifc_omserviceeditor::##__mofifiedFlag & modified)) {\
		HRESULT r = WriteObjectUInt<__class, __getterType>((__key), (__object), &##__class::##__getter, __writeFlags);\
		if (FAILED(r)) {hr = E_FAIL;} else { saved |= ifc_omserviceeditor::##__mofifiedFlag; }}}

#define WRITE_SERVICE_STR(__mofifiedFlag, __getter, __key) WRITE_OBJECT_STR(__mofifiedFlag, ifc_omservice, SVCSTRGET, service, __getter, __key)
#define WRITE_SERVICE_STR_FALLBACK(__mofifiedFlag, __getter1, __getter2, __key) WRITE_OBJECT_STR_FALLBACK(__mofifiedFlag, ifc_omservice, SVCSTRGET, service, __getter1, __getter2, __key)
#define WRITE_SERVICE_UINT(__mofifiedFlag, __getter, __key, __writeZero) WRITE_OBJECT_UINT(__mofifiedFlag, ifc_omservice, SVCUINTGET, service, __getter, __key, __writeZero)
#define WRITE_DETAILS_STR(__mofifiedFlag, __getter, __key) WRITE_OBJECT_STR(__mofifiedFlag, ifc_omservicedetails, DETAILGET, details, __getter, __key)

LoaderIni::LoaderIni()
	: bufferAnsi(NULL), bufferAnsiMax(0), buffer(NULL), bufferMax(0), handlerEnum(NULL)
{
	memset(pathAnsi, 0, sizeof(pathAnsi));
}

LoaderIni::~LoaderIni()
{
	//Plugin_FreeAnsiString(pathAnsi);
	Plugin_FreeAnsiString(bufferAnsi);
	Plugin_FreeString(buffer);

	if (NULL != handlerEnum)
		handlerEnum->Release();
}

HRESULT LoaderIni::Load(LPCWSTR pszAddress, ifc_omservicehost *host, ifc_omservice **serviceOut)
{
	if (NULL == serviceOut) 
		return E_POINTER;

	*serviceOut = NULL;

	HRESULT hr = MakeAnsiPath(pszAddress);
	if (FAILED(hr)) return hr;

	UINT id = ReadInt(OMS_ID, 0);
	if (0 == id) return E_UNEXPECTED;

	OmService *service;

	hr = OmService::CreateInstance(id, host, &service);
	if (FAILED(hr)) return hr;

	ifc_omserviceeditor *editor;
	if (SUCCEEDED(service->QueryInterface(IFC_OmServiceEditor, (void**)&editor)))
		editor->BeginUpdate();
	else	
		editor = NULL;

	service->SetAddress(pszAddress);

	#define READ_SERVICE_STR(__key, __putter)\
		{ LPCSTR value; HRESULT r = ReadAnsi((__key), NULL, &value);\
		if (SUCCEEDED(r)) { if ('\0' != *value) r = service->##__putter((LPCWSTR)value, TRUE); }\
		if (FAILED(r)) hr = E_FAIL;}

	READ_SERVICE_STR(OMS_NAME, SetName);
	READ_SERVICE_STR(OMS_URL, SetUrl);
	READ_SERVICE_STR(OMS_ICON, SetIcon);

	UINT val = ReadInt(OMS_FLAGS, 0);
	if (0 != val) service->SetFlags(val, val & ~ifc_omservice::RuntimeFlagsMask);

	val = ReadInt(OMS_RATING, 0);
	if (0 != val) service->SetRating(val);

	val = ReadInt(OMS_VERSION, 0);
	if (0 != val) service->SetVersion(val);

	val = ReadInt(OMS_GENERATION, 0);
	if (0 != val) service->SetGeneration(val);

	READ_SERVICE_STR(OMS_DESCRIPTION, SetDescription);
	READ_SERVICE_STR(OMS_AUTHORFIRST, SetAuthorFirst);
	READ_SERVICE_STR(OMS_AUTHORLAST, SetAuthorLast);
	READ_SERVICE_STR(OMS_PUBLISHED, SetPublished);
	READ_SERVICE_STR(OMS_UPDATED, SetUpdated);
	READ_SERVICE_STR(OMS_THUMBNAIL, SetThumbnail);
	READ_SERVICE_STR(OMS_SCREENSHOT, SetScreenshot);

	if (NULL != handlerEnum)
	{
		handlerEnum->Reset();
		ifc_omstoragehandler *storageHandler;
		while (S_OK == handlerEnum->Next(1, &storageHandler, NULL))
		{
			LPCWSTR pszKey;
			if (SUCCEEDED(storageHandler->GetKey(&pszKey)))
			{
				if (SUCCEEDED(WideCharToAnsiBuffer(CP_UTF8, 0, pszKey, -1, NULL, NULL)))
				{
					LPCSTR keyAnsi = bufferAnsi;
					LPCSTR valueAnsi;
					if (SUCCEEDED(ReadAnsi(keyAnsi, NULL, &valueAnsi)) && 
						NULL != valueAnsi && L'\0' != *valueAnsi)
					{
						if (SUCCEEDED(MultiByteToBuffer(CP_UTF8, 0, valueAnsi, -1)))
							storageHandler->Invoke(service, pszKey, buffer);
					}
				}
			}
			storageHandler->Release();
		}
	}

	if (NULL != editor)
	{
		editor->SetModified(0, ((UINT)-1));
		editor->EndUpdate();
		editor->Release();
	}

	*serviceOut = service;

	return hr;
}

HRESULT LoaderIni::Reload(ifc_omservice *service)
{
	if (NULL == service) 
		return E_INVALIDARG;

	HRESULT hr = RequestBuffer(&buffer, &bufferMax, MAX_PATH * 2);
	if (FAILED(hr)) return hr;

	hr = service->GetAddress(buffer, bufferMax);
	if (FAILED(hr)) return hr;
	if (L'\0' == *buffer) return E_UNEXPECTED;

	hr = MakeAnsiPath(buffer);
	if (FAILED(hr)) return hr;

	UINT id = ReadInt(OMS_ID, 0);
	if (id != service->GetId()) return E_UNEXPECTED;

	ifc_omserviceeditor *editor;
	hr = service->QueryInterface(IFC_OmServiceEditor, (void**)&editor);
	if (SUCCEEDED(hr))
	{
		#define READ_EDITOR_STR(__key, __putter)\
			{ LPCSTR value; HRESULT r = ReadAnsi((__key), NULL, &value);\
			if (SUCCEEDED(r)) { r = editor->##__putter((LPCWSTR)value, TRUE); }\
			if (FAILED(r)) hr = E_FAIL;}

		editor->BeginUpdate();

		READ_EDITOR_STR(OMS_NAME, SetName);
		READ_EDITOR_STR(OMS_URL, SetUrl);
		READ_EDITOR_STR(OMS_ICON, SetIcon);

		UINT val;
		val = ReadInt(OMS_FLAGS, 0);
		editor->SetFlags(val, val & ~ifc_omservice::RuntimeFlagsMask);

		val = ReadInt(OMS_RATING, 0);
		editor->SetRating(val);

		val = ReadInt(OMS_VERSION, 0);
		editor->SetVersion(val);

		val = ReadInt(OMS_GENERATION, 0);
		editor->SetGeneration(val);

		READ_EDITOR_STR(OMS_DESCRIPTION, SetDescription);
		READ_EDITOR_STR(OMS_AUTHORFIRST, SetAuthorFirst);
		READ_EDITOR_STR(OMS_AUTHORLAST, SetAuthorLast);
		READ_EDITOR_STR(OMS_PUBLISHED, SetPublished);
		READ_EDITOR_STR(OMS_UPDATED, SetUpdated);
		READ_EDITOR_STR(OMS_THUMBNAIL, SetThumbnail);
		READ_EDITOR_STR(OMS_SCREENSHOT, SetScreenshot);
	}
	else
	{
		editor = NULL;
	}

	if (NULL != handlerEnum)
	{
		handlerEnum->Reset();
		ifc_omstoragehandler *storageHandler;
		while (S_OK == handlerEnum->Next(1, &storageHandler, NULL))
		{
			LPCWSTR pszKey;
			if (SUCCEEDED(storageHandler->GetKey(&pszKey)))
			{
				if (SUCCEEDED(WideCharToAnsiBuffer(CP_UTF8, 0, pszKey, -1, NULL, NULL)))
				{
					LPCSTR keyAnsi = bufferAnsi;
					LPCSTR valueAnsi;
					if (SUCCEEDED(ReadAnsi(keyAnsi, NULL, &valueAnsi)) && 
						NULL != valueAnsi && L'\0' != *valueAnsi)
					{
						if (SUCCEEDED(MultiByteToBuffer(CP_UTF8, 0, valueAnsi, -1)))
							storageHandler->Invoke(service, pszKey, buffer);
					}
				}
			}
			storageHandler->Release();
		}
	}

	if (NULL != editor)
	{
		editor->EndUpdate();
		editor->Release();
	}

	return hr;
}

HRESULT LoaderIni::Save(ifc_omservice *service, UINT flags)
{
	if(NULL == service) 
		return E_INVALIDARG;

	HRESULT hr;
	BOOL generatedName;

	hr = RequestBuffer(&buffer, &bufferMax, MAX_PATH * 2);
	if (FAILED(hr)) return hr;

	hr = GetServicePath(service, buffer, bufferMax, &generatedName);
	if (FAILED(hr)) return hr;

	if (FALSE != generatedName)
		service->SetAddress(buffer);

	hr = MakeAnsiPath(buffer);
	if (FAILED(hr)) return hr;

	if (0 == (ifc_omstorage::saveModifiedOnly & flags))
		WritePrivateProfileSectionA(OMS_GROUP, NULL, pathAnsi);

	UINT modified = ((UINT)-1);
	UINT saved = 0;
	if (0 != (ifc_omstorage::saveModifiedOnly & flags))
	{
		ifc_omserviceeditor *editor;
		if (SUCCEEDED(service->QueryInterface(IFC_OmServiceEditor, (void**)&editor)))
		{
			editor->GetModified(&modified);
			editor->Release();

			if (0 == modified)
				return S_FALSE;
		}
	}	

	if (FAILED(WriteUint(OMS_ID, service->GetId(), flagWriteZero)))
		return E_FAIL;

	WRITE_SERVICE_STR(modifiedName, GetName, OMS_NAME);
	WRITE_SERVICE_STR_FALLBACK(modifiedUrl, GetUrlDirect, GetUrl, OMS_URL);
	WRITE_SERVICE_STR(modifiedIcon, GetIcon, OMS_ICON);

	if (0 != (ifc_omserviceeditor::modifiedFlags & modified))
	{
		UINT val;
		if (SUCCEEDED(service->GetFlags(&val)) && SUCCEEDED(WriteUint(OMS_FLAGS, val & ~ifc_omservice::RuntimeFlagsMask, flagHexMode)))
			saved |= ifc_omserviceeditor::modifiedFlags;
	}

	WRITE_SERVICE_UINT(modifiedRating, GetRating, OMS_RATING, flagWriteNormal);
	WRITE_SERVICE_UINT(modifiedVersion, GetVersion, OMS_VERSION, flagWriteNormal);
	WRITE_SERVICE_UINT(modifiedGeneration, GetGeneration, OMS_GENERATION, flagWriteNormal);

	ifc_omservicedetails *details;
	if (SUCCEEDED(service->QueryInterface(IFC_OmServiceDetails, (void**)&details)))
	{
		WRITE_DETAILS_STR(modifiedDescription, GetDescription, OMS_DESCRIPTION);
		WRITE_DETAILS_STR(modifiedAuthorFirst, GetAuthorFirst, OMS_AUTHORFIRST);
		WRITE_DETAILS_STR(modifiedAuthorLast, GetAuthorLast, OMS_AUTHORLAST);
		WRITE_DETAILS_STR(modifiedPublished, GetPublished, OMS_PUBLISHED);
		WRITE_DETAILS_STR(modifiedUpdated, GetUpdated, OMS_UPDATED);
		WRITE_DETAILS_STR(modifiedThumbnail, GetThumbnail, OMS_THUMBNAIL);
		WRITE_DETAILS_STR(modifiedScreenshot, GetScreenshot, OMS_SCREENSHOT);
		details->Release();
	}

	if (0 != (ifc_omstorage::saveClearModified & flags) && 0 != saved)
	{
		ifc_omserviceeditor *editor;
		if (SUCCEEDED(service->QueryInterface(IFC_OmServiceEditor, (void**)&editor)))
		{
			editor->SetModified(0, saved);
			editor->Release();
		}
	}

	return hr;
}

HRESULT LoaderIni::RegisterHandlers(ifc_omstoragehandlerenum *enumerator)
{
	if (NULL != handlerEnum)
		handlerEnum->Release();

	handlerEnum = enumerator;
	if (NULL != handlerEnum)
		handlerEnum->AddRef();

	return S_OK;
}

HRESULT LoaderIni::RequestAnsiBuffer(LPSTR *ppBuffer, UINT *pBufferMax, UINT requestSize)
{
	if (*pBufferMax >= requestSize)
		return S_OK;

	UINT size = (0 != *pBufferMax) ? (*pBufferMax * 2) : 128;
	while (size < requestSize) size = size*2;

	Plugin_FreeAnsiString(*ppBuffer);
	*ppBuffer = Plugin_MallocAnsiString(size);
	if (NULL == *ppBuffer) 
	{
		*ppBuffer = 0;
		*pBufferMax = 0;
		return E_OUTOFMEMORY;
	}

	*pBufferMax = size;
	return S_OK;
}

HRESULT LoaderIni::RequestBuffer(LPWSTR *ppBuffer, UINT *pBufferMax, UINT requestSize)
{
	if (*pBufferMax >= requestSize)
		return S_OK;

	UINT size = (0 != *pBufferMax) ? (*pBufferMax * 2) : 128;
	while (size < requestSize) size = size*2;

	Plugin_FreeString(*ppBuffer);
	*ppBuffer = Plugin_MallocString(size);
	if (NULL == *ppBuffer) 
	{
		*ppBuffer = 0;
		*pBufferMax = 0;
		return E_OUTOFMEMORY;
	}

	*pBufferMax = size;
	return S_OK;
}

HRESULT LoaderIni::MakeAnsiPath(LPCWSTR pszAddress)
{
	BOOL fUsedDefaultChar = FALSE;
	WCHAR szShort[MAX_PATH] = {0};

	UINT cch = WideCharToMultiByte(CP_ACP, 0, pszAddress, -1, NULL, 0, NULL, &fUsedDefaultChar);
	if (0 == cch || FALSE != fUsedDefaultChar)
	{
		cch = GetShortPathName(pszAddress, szShort, ARRAYSIZE(szShort));
		if (0 != cch)
		{
			pszAddress = szShort;
			cch = WideCharToMultiByte(CP_ACP, 0, pszAddress, -1, NULL, 0, NULL, &fUsedDefaultChar);
		}

		if (0 == cch || FALSE != fUsedDefaultChar)
		{
			if (NULL != pathAnsi) *pathAnsi = L'\0';
			return E_FAIL;
		}
	}

	cch = WideCharToMultiByte(CP_ACP, 0, pszAddress, cch, pathAnsi, MAX_PATH, NULL, NULL);
	if (0 == cch)
	{
		DWORD errorCode = GetLastError();
		*pathAnsi = L'\0';
		return HRESULT_FROM_WIN32(errorCode);
	}
	pathAnsi[cch] = '\0';
	return S_OK;
}

HRESULT LoaderIni::GetServicePath(ifc_omservice *service, LPWSTR pszBuffer, UINT cchBufferMax, BOOL *fGenerated)
{
	if (NULL == pszBuffer) return E_POINTER;
	pszBuffer[0] = L'\0';

	if (NULL == service) return E_INVALIDARG;

	HRESULT hr;
	WCHAR szPath[MAX_PATH * 2] = {0};
	hr = service->GetAddress(szPath, ARRAYSIZE(szPath));
	if (FAILED(hr)) return hr;

	ifc_omservicehost *host = NULL;
	ifc_omservicehostext *hostExt;
	if (SUCCEEDED(service->QueryInterface(IFC_OmServiceHostExt, (void**)&hostExt)))
	{
		if (FAILED(hostExt->GetHost(&host)))
			host = NULL;
		hostExt->Release();
	}

	BOOL nameGenerated = FALSE;
	if (L'\0' == *szPath)
	{
		hr =  (NULL != host) ?
			host->GetDefaultName(service, szPath, ARRAYSIZE(szPath)) :
				StringCchPrintf(szPath, ARRAYSIZE(szPath), L"omService_{%010u}", service->GetId());

		if (L'\0' == *szPath)
			hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

		nameGenerated = TRUE;
	}

	if (SUCCEEDED(hr))
	{
		hr = Plugin_ResolveRelativePath(szPath, host, pszBuffer, cchBufferMax); 
		if (SUCCEEDED(hr) && FALSE != nameGenerated)
		{
			LPWSTR pExt = PathFindExtension(pszBuffer);
			if (pExt != pszBuffer && L'.' == *pExt && SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), pExt)))
			{
				UINT pExtMax = cchBufferMax - (UINT)(pExt - pszBuffer);
				UINT attempt = 0;
				while (FALSE != PathFileExists(pszBuffer))
				{
					if (FAILED(StringCchPrintf(pExt, pExtMax, L"(%u)%s", ++attempt, szPath)))
					{
						StringCchCopy(pExt, pExtMax, szPath);
						break;
					}
				}
			}
		}

		if (SUCCEEDED(hr) && SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), pszBuffer)))
		{
			PathRemoveFileSpec(szPath);
			Plugin_EnsurePathExist(szPath);
		}
	}

	if (FAILED(hr)) 
		pszBuffer[0] = L'\0';

	if (NULL != host) 
		host->Release();

	if (NULL != fGenerated) 
		*fGenerated = nameGenerated;
	
	return hr;
}

HRESULT LoaderIni::WideCharToAnsiBuffer(UINT codePage, DWORD flags, LPCWSTR pszWideChar, INT cchWideChar, LPCSTR pDefaultChar, BOOL *pUsedDefaultChar)
{
	UINT cch = WideCharToMultiByte(codePage, flags, pszWideChar, cchWideChar, NULL, 0, pDefaultChar, pUsedDefaultChar);
	if (0 == cch)
	{
		DWORD errorCode = GetLastError();
		return HRESULT_FROM_WIN32(errorCode);
	}

	HRESULT hr = RequestAnsiBuffer(&bufferAnsi, &bufferAnsiMax, cch);
	if (FAILED(hr)) return hr;

	if (0 == WideCharToMultiByte(codePage, flags, pszWideChar, cchWideChar, bufferAnsi, bufferAnsiMax, pDefaultChar, pUsedDefaultChar))
	{
		DWORD errorCode = GetLastError();
		return HRESULT_FROM_WIN32(errorCode);
	}

	return S_OK;
}

HRESULT LoaderIni::MultiByteToBuffer(UINT codePage, DWORD flags, LPCSTR pszMultiByte, INT cbMultiByte)
{
	UINT cch = MultiByteToWideChar(codePage, flags, pszMultiByte, cbMultiByte, NULL, 0);
	if (0 == cch)
	{
		DWORD errorCode = GetLastError();
		return HRESULT_FROM_WIN32(errorCode);
	}

	HRESULT hr = RequestBuffer(&buffer, &bufferMax, cch);
	if (FAILED(hr)) return hr;

	if (0 == MultiByteToWideChar(codePage, flags, pszMultiByte, cbMultiByte, buffer, bufferMax))
	{
		DWORD errorCode = GetLastError();
		return HRESULT_FROM_WIN32(errorCode);
	}

	return S_OK;
}

HRESULT LoaderIni::Write(LPCSTR pszKey, LPCWSTR pszValue)
{
	LPCSTR valueAnsi;
	if (NULL == pszValue) valueAnsi = NULL;
	else if (L'\0' == *pszValue) valueAnsi = '\0';
	else
	{
		HRESULT hr = WideCharToAnsiBuffer(CP_UTF8, 0, pszValue, -1, NULL, NULL);
		if (FAILED(hr)) return hr;
		valueAnsi = bufferAnsi;
	}

	return WriteAnsi(pszKey, valueAnsi);
}

HRESULT LoaderIni::WriteAnsi(LPCSTR pszKey, LPCSTR pszValue)
{
	if (FALSE == WritePrivateProfileStringA(OMS_GROUP, pszKey, pszValue, pathAnsi))
	{
		DWORD errorCode = GetLastError();
		return HRESULT_FROM_WIN32(errorCode);
	}
	return S_OK;
}

HRESULT LoaderIni::WriteUint(LPCSTR pszKey, UINT uValue,  UINT flags)
{
	HRESULT hr;
	CHAR szBuffer[16] = {0}, *pVal;
	
	if (0 == uValue && 0 == (flagWriteZero & flags))
	{
		hr = S_OK;
		pVal = NULL;
	}
	else
	{		
		hr = StringCchPrintfA(szBuffer, ARRAYSIZE(szBuffer), ((0 == (flagHexMode & flags)) ? "%u" : "0x%08X"), uValue);
		pVal = szBuffer;
	}
	
	if (SUCCEEDED(hr))
	{
		hr = WriteAnsi(pszKey, pVal);
	}
	return hr;
}

HRESULT LoaderIni::ReadAnsi(LPCSTR pszKey, LPCSTR pszDefault, LPCSTR *ppszValue)
{
	HRESULT hr;
	if (NULL == bufferAnsi)
	{
		hr = RequestAnsiBuffer(&bufferAnsi, &bufferAnsiMax, 2);
		if (FAILED(hr)) 
		{	
			*ppszValue = pszDefault;
			return hr;
		}
	}

	for(;;)
	{
		UINT copied = GetPrivateProfileStringA(OMS_GROUP, pszKey, pszDefault, bufferAnsi, bufferAnsiMax, pathAnsi);
		if (copied != bufferAnsiMax - 1) break;
		
		hr = RequestAnsiBuffer(&bufferAnsi, &bufferAnsiMax, bufferAnsiMax * 2);
		if (FAILED(hr)) 
		{	
			*ppszValue = pszDefault;
			return hr;
		}

	}
	*ppszValue = bufferAnsi;
	return S_OK;
}

UINT LoaderIni::ReadInt(LPCSTR pszKey, UINT nDefault)
{
	return GetPrivateProfileIntA(OMS_GROUP, pszKey, nDefault, pathAnsi);
}

template <class Object, class Getter>
HRESULT LoaderIni::WriteObjectStr(LPCSTR pszKey, Object *object, Getter getter)
{
	HRESULT hr;

	if (NULL == buffer)
	{
		hr = RequestBuffer(&buffer, &bufferMax, 256);
		if (FAILED(hr)) return hr;
	}
	for (;;)
	{		
		hr = (object->*getter)(buffer, bufferMax);
		if (SUCCEEDED(hr)) break;
		if (E_NOTIMPL == hr) return S_FALSE;
		if (OMSVC_E_INSUFFICIENT_BUFFER != hr) return hr;

		hr = RequestBuffer(&buffer, &bufferMax, bufferMax * 2);
		if (FAILED(hr)) return hr;
	}
	return Write(pszKey, buffer);
}

template <class Object, class Getter>
HRESULT LoaderIni::WriteObjectUInt(LPCSTR pszKey, Object *object, Getter getter, UINT flags)
{	
	UINT val;
	HRESULT hr = (object->*getter)(&val);
	if (FAILED(hr))
	{
		return (E_NOTIMPL == hr) ? S_FALSE : hr;
	}
	return WriteUint(pszKey, val, flags);
}