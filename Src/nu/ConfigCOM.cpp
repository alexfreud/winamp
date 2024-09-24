#include "ConfigCOM.h"
#include "AutoChar.h"
#include "../Winamp/JSAPI.h"

#include <shlwapi.h>
#include <strsafe.h>

#define CSTR_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)


ConfigCOM::ConfigCOM()
	: ref(1), index(31337), pathA(NULL), nameA(NULL)
{
}

ConfigCOM::~ConfigCOM()
{
	ConfigMap::iterator config_it;
	for(config_it = config_map.begin(); config_it != config_map.end(); config_it++)
	{
		free(config_it->second);
	}
	config_map.clear();
	if (NULL != pathA) free(pathA);
	if (NULL != nameA) free(nameA);
}

HRESULT ConfigCOM::CreateInstanceW(const wchar_t *pszName, const char *pszPath, ConfigCOM **config)
{
	if (NULL == config) return E_POINTER;
	*config = NULL;

	if (NULL == pszName) 
		return E_INVALIDARG;

	
	*config = new ConfigCOM();
	if (NULL == *config) return E_OUTOFMEMORY;
	
	char *buffer = NULL;
	int cbBuffer = WideCharToMultiByte(CP_UTF8, 0, pszName, -1, 0, 0, NULL, NULL);
	if (0 != cbBuffer)
	{
		buffer = (char*)calloc(cbBuffer, sizeof(char));
		if (NULL != buffer && 0 == WideCharToMultiByte(CP_UTF8, 0, pszName, -1, buffer, cbBuffer, NULL, NULL))
		{
			free(buffer);
			buffer = NULL;
		}
	}

	if (NULL == buffer)
	{
		(*config)->Release();
		*config = NULL;
		return E_OUTOFMEMORY;
	}

	(*config)->nameA = buffer;
	if (NULL != pszPath)  
		(*config)->SetPathA(pszPath);
	return S_OK;
	
}

HRESULT ConfigCOM::CreateInstanceA(const char *pszName, const char *pszPath, ConfigCOM **config)
{
	if (NULL == config) return E_POINTER;
	*config = NULL;

	if (NULL == pszName) 
		return E_INVALIDARG;

	
	*config = new ConfigCOM();
	if (NULL == *config) return E_OUTOFMEMORY;
	
	char *nameA = _strdup(pszName);
	if (NULL == nameA)
	{
		(*config)->Release();
		*config = NULL;
		return E_OUTOFMEMORY;
	}

	(*config)->nameA = nameA;
	if (NULL != pszPath)  
		(*config)->SetPathA(pszPath);

	return S_OK;
}

STDMETHODIMP_(ULONG) ConfigCOM::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) ConfigCOM::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}
STDMETHODIMP ConfigCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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

STDMETHODIMP ConfigCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	UNREFERENCED_PARAMETER(riid);

	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		bool found = false;
		AutoChar item(rgszNames[i], CP_UTF8);

		ConfigMap::iterator config_it;
		for(config_it = config_map.begin();config_it != config_map.end(); config_it++)
		{
			if (config_it->second && 
				CSTR_EQUAL == CompareStringA(lcid, NORM_IGNORECASE, config_it->second, -1, item, -1))
			{
				found = true;
				rgdispid[i] = config_it->first;
			}
		}
		if (!found) // if they reference a config item, well by golly they want that config item.
		{
			config_map[++index] = _strdup(item);
			rgdispid[i] = index;
		}

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	
	return S_OK;
}

STDMETHODIMP ConfigCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	UNREFERENCED_PARAMETER(itinfo);
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(pptinfo);

	return E_NOTIMPL;
}

STDMETHODIMP ConfigCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	UNREFERENCED_PARAMETER(pctinfo);

	return E_NOTIMPL;
}


STDMETHODIMP ConfigCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	UNREFERENCED_PARAMETER(riid);
	UNREFERENCED_PARAMETER(lcid);
	UNREFERENCED_PARAMETER(pexecinfo);
	
	ConfigMap::iterator config_it = config_map.find(dispid);
	if (config_it == config_map.end())
		return DISP_E_MEMBERNOTFOUND;
	
	if (0 != (DISPATCH_PROPERTYPUT & wFlags))
	{
		VARIANTARG *varArg;
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);

		varArg = &pdispparams->rgvarg[0];
		switch(V_VT(varArg))
		{
			case VT_BSTR: 
				WriteString(config_it->second, V_BSTR(varArg));
				break;

			case VT_BOOL:
				{
					BOOL boolVal;
					switch(V_BOOL(varArg))
					{
						case VARIANT_TRUE:
							boolVal = TRUE;
							break;
						case VARIANT_FALSE:
							boolVal = FALSE;
							break;
						default:
							*puArgErr = 0;
							return DISP_E_BADVARTYPE;
					}

					WriteBool(config_it->second, boolVal);
				}
				break;

			case VT_I4:
				WriteLong(config_it->second, V_I4(varArg));
				break;
			default:
				*puArgErr = 0;
				return DISP_E_TYPEMISMATCH;
		}
		return S_OK;
	}

	if (0 != (DISPATCH_PROPERTYGET & wFlags))
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		if (NULL == pvarResult)
			return DISP_E_PARAMNOTOPTIONAL;
		
		VariantInit(pvarResult);
		BSTR tag = ReadBSTR(config_it->second, NULL);
		if (NULL != tag)
		{
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = tag; 
		}
		else
		{
			V_VT(pvarResult) = VT_NULL;
		}
		
		return S_OK;
	}
	
	return ResultFromScode(E_INVALIDARG);
}
BOOL ConfigCOM::WriteStringAnsi(const char *key, const char* string)
{
	return WritePrivateProfileStringA(nameA, key, string, pathA);
}
BOOL ConfigCOM::WriteString(const char *key, const wchar_t *string)
{
	AutoChar buffer(string, CP_UTF8);
	return WriteStringAnsi(key, buffer);
}

BOOL ConfigCOM::WriteBool(const char *key, BOOL value)
{	
	return WriteStringAnsi(key, (FALSE != value) ? "true" : "false");
}

BOOL ConfigCOM::WriteLong(const char *key, long value)
{	
	char item[64] = {0};
	if (FAILED(StringCchPrintfA(item, ARRAYSIZE(item), "%ld", value)))
		return FALSE;
	return WriteStringAnsi(key, item);
}

DWORD ConfigCOM::ReadString(const char *key, const char *defaultVal, char *buffer, int bufferMax)
{
	return GetPrivateProfileStringA(nameA, key, defaultVal, buffer, bufferMax, pathA);
}

LONG ConfigCOM::ReadLong(const char *key, long defaultVal)
{
	return GetPrivateProfileIntA(nameA, key, defaultVal, pathA);
}

BOOL ConfigCOM::ReadBool(const char *key, BOOL defaultVal)
{
	char szBuffer[32] = {0};
	INT cchLen = ReadString(key, NULL, szBuffer, ARRAYSIZE(szBuffer));
	if (0 == cchLen) return defaultVal;
	
	if (1 == cchLen)
	{
		switch(*szBuffer)
		{
			case '0':
			case 'n':
			case 'f':
				return FALSE;
			case '1':
			case 'y':
			case 't':
				return TRUE;
		}
	}
	else
	{
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "yes", -1, szBuffer, cchLen) ||
			CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "true", -1, szBuffer, cchLen))
		{
			return TRUE;
		}

		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "no", -1, szBuffer, cchLen) ||
			CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "false", -1, szBuffer, cchLen))
		{
			return FALSE;
		}
	}

	INT v;
	if (FALSE != StrToIntExA(szBuffer, STIF_SUPPORT_HEX,  &v))
		return (0 != v);

	return defaultVal;
}

BSTR ConfigCOM::ReadBSTR(const char *key, const wchar_t *defaultVal)
{
	char szBuffer[16384] = {0};

	ReadString(key, "__emptee__", szBuffer, ARRAYSIZE(szBuffer));
	if (CSTR_EQUAL != CompareStringA(CSTR_INVARIANT, 0, szBuffer, -1, "__emptee__", -1))
	{
		int size = MultiByteToWideChar(CP_UTF8, 0, szBuffer, -1, 0,0);
		if (0 != size)
		{
			BSTR result;
			result = SysAllocStringLen(0, size-1);
			if (NULL == result) return NULL;

			if (0 != MultiByteToWideChar(CP_UTF8, 0, szBuffer, -1, result, size))
				return result;

			SysFreeString(result);
		}
	}
	return (NULL != defaultVal) ? SysAllocString(defaultVal) : NULL;
}

void ConfigCOM::SetPathA(const char *pszPath)
{
	if (NULL != pathA)
	{
		free(pathA);
		pathA = NULL;
	}
	
	if (NULL == pszPath)
	{
		pathA = NULL;
		return;
	}

	pathA = _strdup(pszPath);
}


BOOL ConfigCOM::IsEqual(const char *pszName)
{
	if (NULL == pszName) return FALSE;
	return (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, nameA, -1, pszName, -1));
}