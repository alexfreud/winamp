#ifndef NULLSOFT_MYPROGRESSH
#define NULLSOFT_MYPROGRESSH

#include "P4SDevice.h"

class MyProgress : public IWMDMProgress3 
{
public:
	MyProgress(TransferItem * t);
	virtual ~MyProgress();

	/* IUnknown methods */
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(); 
	virtual ULONG STDMETHODCALLTYPE Release();

	/* IWMDMProgress methods */
	virtual HRESULT STDMETHODCALLTYPE Begin(DWORD dwEstimatedTicks);
	virtual HRESULT STDMETHODCALLTYPE Progress(DWORD dwTranspiredTicks);
	virtual HRESULT STDMETHODCALLTYPE End();

	/* IWMDMProgress2 methods */
	virtual HRESULT STDMETHODCALLTYPE End2(HRESULT  hrCompletionCode);

	/* IWMDMProgress3 methods */
	virtual HRESULT STDMETHODCALLTYPE Begin3(GUID EventId,DWORD dwEstimatedTicks,OPAQUECOMMAND*  pContext);
	virtual HRESULT STDMETHODCALLTYPE Progress3(GUID EventId,DWORD dwTranspiredTicks,OPAQUECOMMAND*  pContext);
	virtual HRESULT STDMETHODCALLTYPE End3(GUID EventId,HRESULT hrCompletionCode,OPAQUECOMMAND*  pContext);
public:
	TransferItem *t;
	ULONG refcount;
	DWORD estTicks;
};

#endif