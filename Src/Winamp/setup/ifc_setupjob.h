#ifndef WINAMP_IFC_SETUP_JOB_HEADER
#define WINAMP_IFC_SETUP_JOB_HEADER

#include <bfc/dispatch.h>
#include <windows.h>

class NOVTABLE ifc_setupjob : public Dispatchable
{
protected:
	ifc_setupjob(void) {}
	virtual ~ifc_setupjob(void) {}

public:
	HRESULT Execute(HWND hwndText); 
	HRESULT Cancel(HWND hwndText);
	HRESULT IsCancelSupported(void);

public:
	DISPATCH_CODES
	{
		API_SETUPJOB_EXECUTE = 10,
		API_SETUPJOB_CANCEL = 20,
		API_SETUPJOB_ISCANCELSUPPORTED = 30,
	};
};

inline HRESULT ifc_setupjob::Execute(HWND hwndText)
{
	return _call(API_SETUPJOB_EXECUTE, E_NOTIMPL, hwndText);
}

inline HRESULT ifc_setupjob::Cancel(HWND hwndText)
{
	return _call(API_SETUPJOB_CANCEL, E_NOTIMPL, hwndText);
}

inline HRESULT ifc_setupjob::IsCancelSupported(void)
{
	return _call(API_SETUPJOB_ISCANCELSUPPORTED, E_NOTIMPL);
}

#endif //WINAMP_IFC_SETUP_JOB_HEADER