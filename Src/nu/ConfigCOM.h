#ifndef NULLSOFT_NU_CONFIGCOMH
#define NULLSOFT_NU_CONFIGCOMH

#include <ocidl.h>
#include <map>

class ConfigCOM : public IDispatch
{
protected:
	 ConfigCOM();
	 ~ConfigCOM();

public:
	static HRESULT CreateInstanceW(const wchar_t *pszName, const char *pszPath, ConfigCOM **config);
	static HRESULT CreateInstanceA(const char *pszName, const char *pszPath, ConfigCOM **config);

public:
	/* IUnknown */
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/* IDispatch */
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

	void SetPathA(const char *pszPath);
	BOOL IsEqual(const char *pszName);

	BOOL WriteStringAnsi(const char *key, const char *string);
	BOOL WriteString(const char *key, const wchar_t *string);
	BOOL WriteBool(const char *key, BOOL value);
	BOOL WriteLong(const char *key, long value);

	DWORD ReadString(const char *key, const char *defaultVal, char *buffer, int bufferMax);
	LONG ReadLong(const char *key, long defaultVal);
	BOOL ReadBool(const char *key, BOOL defaultVal);
	BSTR ReadBSTR(const char *key, const wchar_t *defaultVal);

private:
	typedef std::map<long, char*> ConfigMap;
	ConfigMap config_map;
	long index;
	char *pathA;
	char *nameA;
	LONG ref;
};

#endif