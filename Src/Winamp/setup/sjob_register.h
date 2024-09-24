#ifndef WINAMP_REGISTER_SETUP_JOB_HEADER
#define WINAMP_REGISTER_SETUP_JOB_HEADER

#include "./ifc_setupjob.h"

class setup_job_register: public ifc_setupjob
{

public:
	setup_job_register();
	virtual ~setup_job_register();

public:
	size_t AddRef();
	size_t Release();
	HRESULT Execute(HWND hwndText);
	HRESULT Cancel(HWND hwndText);
	HRESULT IsCancelSupported();
private:
	size_t ref;
	HWND hwndHttp;

protected:
	RECVS_DISPATCH;
};


#endif //WINAMP_REGISTER_SETUP_JOB_HEADER