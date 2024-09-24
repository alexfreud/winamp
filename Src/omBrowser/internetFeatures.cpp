#include <main.h>
#include <internetFeatures.h>
#include <shlwapi.h>
#include <strsafe.h>

#define REGISTRY_FEATURE_CONTROL		L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl"

InternetFeatures::InternetFeatures()
	: module(NULL), loadResult(S_FALSE), 
	  CoInternetSetFeatureEnabled(NULL),
	  CoInternetIsFeatureEnabled(NULL),
	  processName_(NULL)
{
}

InternetFeatures::~InternetFeatures()
{
	if (NULL != module)
	{
		FreeLibrary(module);
	}

	if (NULL != processName_)
		free(processName_);
}

HRESULT InternetFeatures::LoadModule()
{
	if (S_FALSE != loadResult) 
		return loadResult;

	module = LoadLibrary(L"UrlMon.dll");
	if (NULL == module)
	{
		DWORD errorCode = GetLastError();
		loadResult = HRESULT_FROM_WIN32(errorCode);
		return loadResult;
	}
	else 
	{
		loadResult = S_OK;
	}

	CoInternetSetFeatureEnabled = (COINTERNETSETFEATUREENABLED)GetProcAddress(module, "CoInternetSetFeatureEnabled");
	CoInternetIsFeatureEnabled = (COINTERNETISFEATUREENABLED)GetProcAddress(module, "CoInternetIsFeatureEnabled");

	return loadResult;
}

const wchar_t *InternetFeatures::GetProcessName()
{
	if (NULL == processName_)
	{
		wchar_t buffer[2*MAX_PATH] = {0};
		unsigned long length = GetModuleFileNameW(NULL, buffer, ARRAYSIZE(buffer));
		if (0 == length)
			return NULL;

		const wchar_t *fileName = PathFindFileName(buffer);
		length = lstrlenW(fileName);
		processName_ = (wchar_t *)calloc((length + 1), sizeof(wchar_t));
		if (NULL == processName_)
			return NULL;

		memcpy(processName_, fileName, sizeof(wchar_t)*length);
		processName_[length]=L'\0';
	}

	return processName_;
}

HRESULT InternetFeatures::SetEnabled(INTERNETFEATURELIST FeatureEntry, DWORD dwFlags, BOOL fEnable)
{
	HRESULT hr = LoadModule();
	if (FAILED(hr)) return hr;
	if (NULL == CoInternetSetFeatureEnabled) return E_NOINTERFACE;
    return CoInternetSetFeatureEnabled(FeatureEntry, dwFlags, fEnable);
}

HRESULT InternetFeatures::IsEnabled(INTERNETFEATURELIST FeatureEntry, DWORD dwFlags)
{
	HRESULT hr = LoadModule();
	if (FAILED(hr)) return hr;
	if (NULL == CoInternetIsFeatureEnabled) return E_NOINTERFACE;
	return CoInternetIsFeatureEnabled(FeatureEntry, dwFlags);
}

HRESULT InternetFeatures::SetDWORDFeature(const wchar_t *featureName, BOOL perUser, unsigned long value)
{
	if (NULL == featureName 
		|| L'\0' == *featureName)
	{
		return E_INVALIDARG;
	}

	const wchar_t *processName = GetProcessName();
	if (NULL == processName)
		return E_UNEXPECTED;

	HKEY rootKey = (FALSE != perUser) ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
	HKEY key = NULL;
	unsigned long disposition = 0;
	long errorCode = 0;
	HRESULT result = S_OK;

	wchar_t buffer[MAX_PATH] = {0};
	if (FAILED(StringCchPrintf(buffer, ARRAYSIZE(buffer), 
							  REGISTRY_FEATURE_CONTROL L"\\" L"%s", 
							  featureName)))
	{
		return E_OUTOFMEMORY;
	}

	errorCode = RegCreateKeyEx(rootKey, buffer, 
						  NULL, NULL, REG_OPTION_VOLATILE, KEY_WRITE, 
						  NULL, &key, &disposition);

	if (ERROR_SUCCESS != errorCode)
		return HRESULT_FROM_WIN32(errorCode);

	errorCode = RegSetValueEx(key, processName, 0, REG_DWORD, (const BYTE*)&value, 
							  sizeof(unsigned long));

	if (ERROR_SUCCESS != errorCode)
	{
		result = HRESULT_FROM_WIN32(errorCode);
	}
	else
	{
		result = S_OK;
	}

	RegCloseKey(key);

	return result;
}

HRESULT InternetFeatures::GetDWORDFeature(const wchar_t *featureName, BOOL perUser, unsigned long *value)
{
	if (NULL == featureName 
		|| L'\0' == *featureName)
	{
		return E_INVALIDARG;
	}

	if (NULL == value)
		return E_POINTER;

	const wchar_t *processName = GetProcessName();
	if (NULL == processName)
		return E_UNEXPECTED;

	HKEY rootKey = (FALSE != perUser) ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
	HKEY key = NULL;
	long errorCode = 0;
	HRESULT result = S_OK;
	unsigned long valueType = 0;
	unsigned long valueSize = 0;

	wchar_t buffer[MAX_PATH] = {0};
	if (FAILED(StringCchPrintf(buffer, ARRAYSIZE(buffer), 
							  REGISTRY_FEATURE_CONTROL L"\\" L"%s", 
							  featureName)))
	{
		return E_OUTOFMEMORY;
	}

	errorCode = RegOpenKeyEx(rootKey, buffer, 0, KEY_READ, &key);
	if (ERROR_SUCCESS != errorCode)
		return HRESULT_FROM_WIN32(errorCode);

	valueSize = sizeof(unsigned long);

	errorCode = RegQueryValueEx(key, processName, 0, &valueType, 
								(BYTE*)value, &valueSize);

	if (ERROR_SUCCESS != errorCode)
	{
		result = HRESULT_FROM_WIN32(errorCode);
	}
	else
	{
		if (REG_DWORD != valueType)
			result = E_UNEXPECTED;
		else
			result = S_OK;
	}

	RegCloseKey(key);

	return result;
}

void InternetFeatures::DeleteFeature(const wchar_t *featureName, BOOL perUser)
{
	if (NULL == featureName 
		|| L'\0' == *featureName)
	{
		return;
	}

	const wchar_t *processName = GetProcessName();
	if (NULL == processName)
		return;

	HKEY rootKey = (FALSE != perUser) ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
	HKEY key = NULL;
	long errorCode = 0;

	wchar_t buffer[MAX_PATH] = {0};
	if (FAILED(StringCchPrintf(buffer, ARRAYSIZE(buffer), 
							  REGISTRY_FEATURE_CONTROL L"\\" L"%s", 
							  featureName)))
	{
		return;
	}

	errorCode = RegOpenKeyEx(rootKey, buffer, 0, KEY_WRITE, &key);
	if (ERROR_SUCCESS != errorCode)
		return;

	RegDeleteValue(key, processName);
	RegCloseKey(key);
}