/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "BookmarksCOM.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "api.h"

enum
{
    DISP_BOOKMARK_ADD = 777,
	DISP_BOOKMARK_GETXML,
};

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L##str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT BookmarksCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("Add", DISP_BOOKMARK_ADD)
		CHECK_ID("GetXML", DISP_BOOKMARK_GETXML)
		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;
	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT BookmarksCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT BookmarksCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

void Bookmark_WriteAsXML(const wchar_t *filename, int max);

HRESULT BookmarksCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		case DISP_BOOKMARK_ADD:
			switch (pdispparams->cArgs)
			{
				case 1:
				{
					Bookmark_additem(pdispparams->rgvarg[0].bstrVal,
									 pdispparams->rgvarg[0].bstrVal);
					return S_OK;
				}
				case 2:
				{
					Bookmark_additem(pdispparams->rgvarg[1].bstrVal,
									 pdispparams->rgvarg[0].bstrVal);
					return S_OK;
				}
			}
			break;

		case DISP_BOOKMARK_GETXML:
		{
			int max=0;
			if (pdispparams->cArgs == 1)
			{
				max = _wtoi(pdispparams->rgvarg[0].bstrVal);
			}
			wchar_t tempPath[MAX_PATH] = {0};
			GetTempPathW(MAX_PATH, tempPath);
			wchar_t tempFile[MAX_PATH] = {0};
			GetTempFileNameW(tempPath, L"bmx.b4s", 0, tempFile);
			Bookmark_WriteAsXML(tempFile, max);

			HANDLE plFile = CreateFileW(tempFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
			size_t flen = SetFilePointer(plFile, 0, NULL, FILE_END);
			SetFilePointer(plFile, 0, NULL, FILE_BEGIN);

			SAFEARRAY *bufferArray=SafeArrayCreateVector(VT_UI1, 0, (ULONG)flen);
			void *data = 0;
			SafeArrayAccessData(bufferArray, &data);
			DWORD bytesRead = 0;
			ReadFile(plFile, data, (DWORD)flen, &bytesRead, 0);
			SafeArrayUnaccessData(bufferArray);
			CloseHandle(plFile);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_ARRAY|VT_UI1;
			V_ARRAY(pvarResult) = bufferArray;
			DeleteFileW(tempFile);
		}
		return S_OK;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP BookmarksCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG BookmarksCOM::AddRef(void)
{
	return 0;
}

ULONG BookmarksCOM::Release(void)
{
	return 0;
}