/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename: 
** Project:
** Description:
** Author: Ben Allison benski@nullsoft.com
** Created:
**/
#include "main.h"
#include "../nu/ns_wc.h"
#include "SkinCOM.h"
#include "resource.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "wa_dlg.h"
#include "Browser.h"

void WriteEscaped(FILE *fp, const char *str);

void WriteSkinsAsXML(const wchar_t *filename, int limit)
{
	FILE *fp = _wfopen(filename, L"wb");
	fputs("<?xml version=\"1.0\" encoding=\"utf-8\"?>", fp);
	fputs("<skins>\n", fp);
	fputs("<skin filename=\"", fp);
	WriteEscaped(fp,"Winamp Classic");
	fputs("\"/>\n", fp);

	HANDLE h;
	WIN32_FIND_DATAW d;

	wchar_t dirmask[MAX_PATH] = {0};
	PathCombineW(dirmask, SKINDIR, L"*");

	h = FindFirstFileW(dirmask, &d);
	if (h != INVALID_HANDLE_VALUE  )
	{
		int count=1;
		do
		{
			if ( limit && count >= limit ) break;
			if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (wcscmp(d.cFileName, L".") && wcscmp(d.cFileName, L"..")) 
				{
					fputs("<skin filename=\"", fp);
					WriteEscaped(fp, AutoChar(d.cFileName, CP_UTF8));
					fputs("\"/>\n", fp);
					count++;
				}
			}
			else if (!_wcsicmp(extensionW(d.cFileName), L"zip") || !_wcsicmp(extensionW(d.cFileName), L"wsz") || !_wcsicmp(extensionW(d.cFileName), L"wal"))
			{
				fputs("<skin filename=\"", fp);
				WriteEscaped(fp, AutoChar(d.cFileName, CP_UTF8));
				fputs("\"/>\n", fp);
				count++;
			}

		}
		while (FindNextFileW(h, &d));
		FindClose(h);

	}
	fputs("</skins>", fp);
	fclose(fp);
}


enum
{
	DISP_CURRENTSKIN_GETCOLOR = 777,
	DISP_CURRENTSKIN_GETNAME,
	DISP_CURRENTSKIN_REGISTERSKINCHANGECALLBACK,
	DISP_CURRENTSKIN_UNREGISTERSKINCHANGECALLBACK,
	DISP_CURRENTSKIN_GETFONTNAME,
	DISP_CURRENTSKIN_GETFONTSIZE,
	DISP_CURRENTSKIN_GETPLAYLISTCOLOR,
	DISP_CURRENTSKIN_SETSKIN,
	DISP_CURRENTSKIN_GETXMLSKINLIST,
};


#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L##str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT SkinCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	UNREFERENCED_PARAMETER(riid);

	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("GetColor", DISP_CURRENTSKIN_GETCOLOR)
		CHECK_ID("GetName", DISP_CURRENTSKIN_GETNAME)
		CHECK_ID("RegisterSkinChangeCallback", DISP_CURRENTSKIN_REGISTERSKINCHANGECALLBACK)
		CHECK_ID("UnregisterSkinChangeCallback", DISP_CURRENTSKIN_UNREGISTERSKINCHANGECALLBACK)
		CHECK_ID("GetFontName", DISP_CURRENTSKIN_GETFONTNAME)
		CHECK_ID("GetFontSize", DISP_CURRENTSKIN_GETFONTSIZE);
		CHECK_ID("GetPlaylistColor", DISP_CURRENTSKIN_GETPLAYLISTCOLOR);
		CHECK_ID("SetSkin", DISP_CURRENTSKIN_SETSKIN);
		CHECK_ID("GetXMLSkinList", DISP_CURRENTSKIN_GETXMLSKINLIST);

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT SkinCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	UNREFERENCED_PARAMETER(itinfo);
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(pptinfo);

	return E_NOTIMPL;
}

HRESULT SkinCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	UNREFERENCED_PARAMETER(pctinfo);

	return E_NOTIMPL;
}


HRESULT SkinCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	UNREFERENCED_PARAMETER(riid);
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(pexecinfo);
	UNREFERENCED_PARAMETER(wFlags);

	switch (dispid)
	{
	case DISP_CURRENTSKIN_SETSKIN:
		if (pdispparams->cArgs == 1)
		{
			const wchar_t *newSkin = pdispparams->rgvarg[0].bstrVal;
			if (newSkin && *newSkin)
			{
				if (_wcsicmp(config_skin, newSkin))
				{
					StringCchCopyW(config_skin, MAX_PATH, newSkin);
					PostMessageW(hMainWindow, WM_COMMAND, WINAMP_REFRESHSKIN, 0);
				}
			}
			return S_OK;
		}
		else
			return DISP_E_BADPARAMCOUNT;
	case DISP_CURRENTSKIN_REGISTERSKINCHANGECALLBACK:
		return callbacks.RegisterFromDispParam(pdispparams, 0, puArgErr);
	case DISP_CURRENTSKIN_UNREGISTERSKINCHANGECALLBACK:
		return callbacks.UnregisterFromDispParam(pdispparams, 0, puArgErr);
	case DISP_CURRENTSKIN_GETCOLOR:
		if (pdispparams->cArgs == 1)
		{
			COLORREF color = WADlg_getColor(pdispparams->rgvarg[0].lVal);
			color = ((color >> 16) & 0xff | (color & 0xff00) | ((color << 16) & 0xff0000));
			char colorString[8] = {0};
			StringCchPrintfA(colorString, 8, "#%06X", color);
			AutoWide answer(colorString);
			BSTR tag = SysAllocString(answer);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = tag;

			return S_OK;
		}
		else
			return DISP_E_BADPARAMCOUNT;

	case DISP_CURRENTSKIN_GETNAME:
		{
			BSTR tag = SysAllocString(config_skin);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = tag;
			return S_OK;
		}
		break;

	case DISP_CURRENTSKIN_GETFONTNAME:
		{
			BSTR tag = SysAllocString(GetFontNameW());
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = tag;
			return S_OK;
		}
		break;

	case DISP_CURRENTSKIN_GETFONTSIZE:
		{
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_I4;
			V_I4(pvarResult) = GetFontSize();
			return S_OK;
		}
	case DISP_CURRENTSKIN_GETPLAYLISTCOLOR:
		if (pdispparams->cArgs == 1)
		{	
			COLORREF color = Skin_PLColors[pdispparams->rgvarg[0].lVal];
			color = ((color >> 16) & 0xff | (color & 0xff00) | ((color << 16) & 0xff0000));
			char colorString[8] = {0};
			StringCchPrintfA(colorString, 8, "#%06X", color);
			AutoWide answer(colorString);
			BSTR tag = SysAllocString(answer);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = tag;

			return S_OK;
		}
		else
			return DISP_E_BADPARAMCOUNT;

	case DISP_CURRENTSKIN_GETXMLSKINLIST:
		{
			int max=0;
			if (pdispparams->cArgs == 1)
			{

				max = _wtoi(pdispparams->rgvarg[0].bstrVal);
			}

			wchar_t tempPath[MAX_PATH] = {0};
			GetTempPathW(MAX_PATH, tempPath);
			wchar_t tempFile[MAX_PATH] = {0};
			GetTempFileNameW(tempPath, L"slx", 0, tempFile);
			WriteSkinsAsXML(tempFile, max);

			HANDLE plFile = CreateFileW(tempFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
			size_t flen = SetFilePointer(plFile, 0, NULL, FILE_END);
			SetFilePointer(plFile, 0, NULL, FILE_BEGIN);

			SAFEARRAY *bufferArray=SafeArrayCreateVector(VT_UI1, 0, (ULONG)flen);
			void *data;
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

STDMETHODIMP SkinCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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


ULONG SkinCOM::AddRef(void)
{
	return 0;
}


ULONG SkinCOM::Release(void)
{
	return 0;
}

static void SkinChangedNotifyCb(IDispatch *dispatch, void *param)
{
	UNREFERENCED_PARAMETER(param);

	DISPPARAMS params;

	params.cArgs = 0;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = 0;
	params.rgvarg = 0;
	unsigned int ret;

	if (!(config_no_visseh&8))
	{
		try
		{
			dispatch->Invoke(0, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);
		}
		catch(...)
		{
		}
	}
	else
		dispatch->Invoke(0, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);
}

void SkinCOM::SkinChanged()
{
	callbacks.Notify(SkinChangedNotifyCb, NULL, NULL);
}
