#include "MyProgress.h"

extern LPARAM ipc_transfer;

MyProgress::MyProgress(TransferItem *_t)
: t(_t), refcount(1), estTicks(0)
{ 
}

MyProgress::~MyProgress() {}


HRESULT MyProgress::Begin(DWORD dwEstimatedTicks) 
{
	estTicks = dwEstimatedTicks / 100;
	return S_OK;
}

HRESULT MyProgress::Progress(DWORD dwTranspiredTicks) 
{
	if (estTicks > 0) {
		int pc = dwTranspiredTicks / estTicks;
		if(pc > 100) pc = 100;
		t->pc = pc;
	}
	else t->pc = 0;

	wchar_t buf[100] = {0};
	wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_TRANSFERRING_PERCENT), t->pc);
	t->callback(t->callbackContext,buf);

	if (*(t->killswitch)) 
		return WMDM_E_USER_CANCELLED;
	
	return S_OK;
}

#define PHASE_START       1
#define PHASE_INPROGRESS  2
#define PHASE_FINISH      3
#define PHASE_DONE        4
#define PHASE_ERROR       5

HRESULT MyProgress::End() 
{
  t->phase = PHASE_FINISH;
	return S_OK;
}

#define IMPLEMENTS(ifc) if (riid == IID_ ## ifc) { ++refcount; *ppvObject = static_cast<ifc *>(this); return S_OK; }

HRESULT MyProgress::QueryInterface(REFIID riid,void __RPC_FAR *__RPC_FAR *ppvObject) 
{
	IMPLEMENTS(IUnknown);
	IMPLEMENTS(IWMDMProgress);
	IMPLEMENTS(IWMDMProgress2);
	IMPLEMENTS(IWMDMProgress3);
	*ppvObject = NULL; 
	return E_NOINTERFACE;
}

ULONG MyProgress::AddRef() 
{ 
	return ++refcount; 
}
ULONG MyProgress::Release() 
{ 
	int x = --refcount;
	if (x == 0)
		delete this;

	return x;
}

HRESULT MyProgress::End2(HRESULT hrCompletionCode) 
{
	return End(); 
}

HRESULT MyProgress::Begin3(GUID EventId,DWORD dwEstimatedTicks,OPAQUECOMMAND*  pContext) 
{ 
	return Begin(dwEstimatedTicks); 
}

HRESULT MyProgress::Progress3(GUID EventId,DWORD dwTranspiredTicks,OPAQUECOMMAND*  pContext) 
{
	return Progress(dwTranspiredTicks); 
}

HRESULT MyProgress::End3(GUID EventId,HRESULT hrCompletionCode,OPAQUECOMMAND*  pContext) 
{
	return End2( hrCompletionCode); 
}


