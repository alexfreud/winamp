#include "main.h"
#include "PlaylistsCOM.h"
using namespace Nullsoft::Utility;

enum
{
  DISP_PLALISTS_GETXML = 777,

};

#define CHECK_ID(str, id) 		if (wcscmp(rgszNames[i], L##str) == 0)	{		rgdispid[i] = id; continue; }
HRESULT PlaylistsCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("GetXML", DISP_PLALISTS_GETXML)

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;
	}

	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT PlaylistsCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT PlaylistsCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

static void WriteEscaped(FILE *fp, const wchar_t *str)
{
	// TODO: for speed optimization,
	// we should wait until we hit a special character
	// and write out everything else so before it,
	// like how ASX loader does it
	while (str && *str)
	{
		switch (*str)
		{
			case L'&':
				fputws(L"&amp;", fp);
				break;
			case L'>':
				fputws(L"&gt;", fp);
				break;
			case L'<':
				fputws(L"&lt;", fp);
				break;
			case L'\'':
				fputws(L"&apos;", fp);
				break;
			case L'\"':
				fputws(L"&quot;", fp);
				break;
			default:
				fputwc(*str, fp);
				break;
		}
		// write out the whole UTF-16 character
		wchar_t *next = CharNextW(str);
		while (++str != next)
			fputwc(*str, fp);

	}
}

static void SavePlaylistsXML(const wchar_t *destination, size_t limit)
{
	AutoLockT<api_playlists> lock (AGAVE_API_PLAYLISTS);
	FILE *fp = _wfopen(destination, L"wb");
	fputws(L"\xFEFF", fp);
	fwprintf(fp, L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>");
	size_t numPlaylists = limit ? min(limit, AGAVE_API_PLAYLISTS->GetCount()) : AGAVE_API_PLAYLISTS->GetCount();
	fwprintf(fp, L"<playlists playlists=\"%lu\">", numPlaylists);

	for (size_t i = 0; i != numPlaylists; i++)
	{
		fputws(L"<playlist filename=\"", fp);
		WriteEscaped(fp, AGAVE_API_PLAYLISTS->GetFilename(i));
		fputws(L"\" title=\"", fp);
		WriteEscaped(fp, AGAVE_API_PLAYLISTS->GetName(i));
		unsigned int numItems = 0, length = 0;

		AGAVE_API_PLAYLISTS->GetInfo(i, api_playlists_itemCount, &numItems, sizeof(numItems));
		AGAVE_API_PLAYLISTS->GetInfo(i, api_playlists_totalTime, &length, sizeof(length));

		fwprintf(fp, L"\" songs=\"%u\" seconds=\"%u\"/>",
		         numItems,
		         length);
	}
	fwprintf(fp, L"</playlists>");
	fclose(fp);
}

HRESULT PlaylistsCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		case DISP_PLALISTS_GETXML:
		{
			int max = 0;
			if (pdispparams->cArgs == 1)
			{
				max = _wtoi(pdispparams->rgvarg[0].bstrVal);
			}

			wchar_t tempPath[MAX_PATH] = {0};
			GetTempPathW(MAX_PATH, tempPath);
			wchar_t tempFile[MAX_PATH] = {0};
			GetTempFileNameW(tempPath, L"mpx", 0, tempFile);

			SavePlaylistsXML(tempFile, max);
			HANDLE plFile = CreateFile(tempFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
			size_t flen = SetFilePointer(plFile, 0, NULL, FILE_END);
			SetFilePointer(plFile, 0, NULL, FILE_BEGIN);

			SAFEARRAY *bufferArray = SafeArrayCreateVector(VT_UI1, 0, flen);
			void *data;
			SafeArrayAccessData(bufferArray, &data);
			DWORD bytesRead = 0;
			ReadFile(plFile, data, flen, &bytesRead, 0);
			SafeArrayUnaccessData(bufferArray);
			CloseHandle(plFile);
			DeleteFile(tempFile);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_ARRAY | VT_UI1;
			V_ARRAY(pvarResult) = bufferArray;


		}
		return S_OK;


	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP PlaylistsCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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


ULONG PlaylistsCOM::AddRef(void)
{
	return 0;
}


ULONG PlaylistsCOM::Release(void)
{
	return 0;
}


