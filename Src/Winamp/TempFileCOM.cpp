#include "TempFileCOM.h"
#include <shlwapi.h>

enum
{
	DISP_TEMPFILE_WRITE_STRING,
	DISP_TEMPFILE_CLOSE,
	DISP_TEMPFILE_GET_FILENAME,
	DISP_TEMPFILE_DELETE,
};

TempFileCOM::TempFileCOM(const wchar_t *ext)
{
	ref=1;

	wchar_t tempPath[MAX_PATH-14] = {0};
	GetTempPathW(MAX_PATH-14, tempPath);
	GetTempFileNameW(tempPath, L"tfc", 0, filename);
	if (ext)
	{
		PathRemoveExtensionW(filename);
		PathAddExtensionW(filename, ext);
	}
	hFile = CreateFileW(filename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, 0);
	wchar_t BOM = 0xFEFF;
	DWORD written = 0;
	WriteFile(hFile, &BOM, 2, &written, NULL);
}

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L##str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT TempFileCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("WriteString", DISP_TEMPFILE_WRITE_STRING);
		CHECK_ID("Close", DISP_TEMPFILE_CLOSE);
		CHECK_ID("GetFilename", DISP_TEMPFILE_GET_FILENAME);
		CHECK_ID("Delete", DISP_TEMPFILE_DELETE);

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT TempFileCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT TempFileCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT TempFileCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
	case DISP_TEMPFILE_WRITE_STRING:
	{
		const wchar_t *str = pdispparams->rgvarg[0].bstrVal;
		DWORD written=0;
		WriteFile(hFile, str, (DWORD)wcslen(str)*sizeof(wchar_t), &written, NULL);
		return S_OK;
	}
	case DISP_TEMPFILE_CLOSE:
		CloseHandle(hFile);
		hFile=INVALID_HANDLE_VALUE;
		return S_OK;
	case DISP_TEMPFILE_GET_FILENAME:
	{
		BSTR tag = SysAllocString(filename);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BSTR;
		V_BSTR(pvarResult) = tag;
		return S_OK;
	}
	case DISP_TEMPFILE_DELETE:
		DeleteFileW(filename);
		return S_OK;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP TempFileCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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


ULONG TempFileCOM::AddRef(void)
{
	return (ULONG)++ref;
}


ULONG TempFileCOM::Release(void)
{
	if (--ref == 0)
		delete this;

	return (ULONG)ref;
}
