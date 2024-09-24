#ifndef NULLSOFT_ML_PLG_IDSCANNER_H
#define NULLSOFT_ML_PLG_IDSCANNER_H

#include "../gracenote/gracenote.h"

#include <api/syscb/callbacks/svccb.h>
#include <api/syscb/api_syscb.h>
#include "../ml_local/api_mldb.h"

//#include "../nu/lockfreestack.h"

// Regular declarations

struct ProcessItem
{
	wchar_t *filename;
	ProcessItem *next;
};

class IDScanner : public _ICDDBMusicIDManagerEvents, public SysCallback
{
public:
	IDScanner();
	~IDScanner();
	void ScanDatabase();
	bool GetStatus(long *state, long *track, long *tracks);
	void Kill() { killswitch=1; }
	void Shutdown();

	// Processing data for step 4
	//LockFreeStack<ProcessItem> process_items;
	
	// Thread functions
	static int MLDBFileAddedOnThread(HANDLE handle, void *user_data, intptr_t id);
	static int MLDBFileRemovedOnThread(HANDLE handle, void *user_data, intptr_t id);
	static int MLDBClearedOnThread(HANDLE handle, void *user_data, intptr_t id);
	static int Pass2OnThread(HANDLE handle, void *user_data, intptr_t id);
	int ProcessStackItems(void);

	enum
	{
		STATE_ERROR = -1,
		STATE_IDLE = 0,
		STATE_INITIALIZING=1,
		STATE_SYNC = 2,
		STATE_METADATA = 3,
		STATE_MUSICID = 4,
		STATE_DONE = 5,
	};
private:
	// *** IUnknown Methods ***
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

		// *** IDispatch Methods ***
	STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD (GetTypeInfoCount)(unsigned int FAR * pctinfo);
	STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);

	// *** Sys callback ***
	api_syscb *systemCallbacks;
	FOURCC GetEventType();
	int notify(int msg, intptr_t param1, intptr_t param2);
	static void DebugCallbackMessage(const intptr_t text, const wchar_t *message);
	
	

	// *** ***
	void Pass1();
	void Pass2();


	// *** Helper functions ***
	bool SetupMusicID();
	void CommitFileInfo(ICddbFileInfo *match);
	void SetGracenoteData(BSTR filename, BSTR tagId, BSTR extData); // extData can be NULL


	// *** Data ***
	ICDDBMusicIDManager3 *musicID;
	volatile int killswitch;
	volatile long filesComplete, filesTotal;
	volatile long state;
	DWORD m_dwCookie;
	bool syscb_registered;

protected:
	RECVS_DISPATCH;


};

IConnectionPoint *GetConnectionPoint(IUnknown *punk, REFIID riid);

#endif