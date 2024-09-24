#ifndef WINAMP_IFC_SETUP_PAGE_HEADER
#define WINAMP_IFC_SETUP_PAGE_HEADER

#include <bfc/dispatch.h>
#include <windows.h>

class NOVTABLE ifc_setuppage : public Dispatchable
{
protected:
	ifc_setuppage(void) {}
	virtual ~ifc_setuppage(void) {}

public:
	HRESULT GetName(bool bShort, const wchar_t **pszName);
	HRESULT Save(HWND hwndText); // setup will allways call this no matter of IsDirty result. (you can check dirty state inside save());
	HRESULT CreateView(HWND hwndParent, HWND *phwnd);
	HRESULT Revert(void);
	HRESULT IsDirty(void);  // S_OK - valid, S_FALSE - page is not dirty ( no save required), E_NOTIMPL - not impl. all errors counts as dirty 
	HRESULT Validate(void); // S_OK - valid, S_FALSE - invalid, E_NOTIMPL - not impl. all errors counts as page is valid 

public:
	DISPATCH_CODES
	{
		API_SETUPPAGE_GET_NAME = 10,
		API_SETUPPAGE_CREATEVIEW = 20,
		API_SETUPPAGE_SAVE = 30,
		API_SETUPPAGE_ISDIRTY = 40,
		API_SETUPPAGE_REVERT = 50,
		API_SETUPPAGE_VALIDATE = 60,
	};
};

inline HRESULT ifc_setuppage::GetName(bool bShort, const wchar_t **pszName)
{
	return _call(API_SETUPPAGE_GET_NAME, E_NOTIMPL, bShort, pszName);
}

inline HRESULT ifc_setuppage::CreateView(HWND hwndParent, HWND *phwnd)
{
	return _call(API_SETUPPAGE_CREATEVIEW, E_NOTIMPL, hwndParent, phwnd);
}

inline HRESULT ifc_setuppage::Save(HWND hwndText)
{
	return _call(API_SETUPPAGE_SAVE, E_NOTIMPL, hwndText);
}

inline HRESULT ifc_setuppage::IsDirty(void)
{
	return _call(API_SETUPPAGE_ISDIRTY, E_NOTIMPL);
}

inline HRESULT ifc_setuppage::Revert(void)
{
	return _call(API_SETUPPAGE_REVERT, E_NOTIMPL);
}

inline HRESULT ifc_setuppage::Validate(void)
{
	return _call(API_SETUPPAGE_VALIDATE, E_NOTIMPL);
}
#endif //WINAMP_IFC_SETUP_PAGE_HEADER