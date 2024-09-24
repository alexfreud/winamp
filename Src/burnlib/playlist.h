#ifndef NULLSOFT_BurnerPlaylist_HEADER
#define NULLSOFT_BurnerPlaylist_HEADER

#include "./main.h"
#include "./api.h"

#include <api/service/waServiceFactory.h>
#include "../playlist/api_playlistmanager.h"
#include "../playlist/ifc_playlistloadercallback.h"
#include "../winamp/api_decodefile.h"

#include "../nu/vector.h"

#include "./item.h"
#include "./manager.h"

#define BURNERPLAYLIST_SUCCESS		0x0000

#define BURNERPLAYLIST_STATUS		0x0000
#define BURNERPLAYLIST_ERROR		0x1000

// error codes
#define BURNERPLAYLIST_FAILED				((BURNERPLAYLIST_ERROR) + 0x001)
#define BURNERPLAYLIST_BADFILENAME			((BURNERPLAYLIST_ERROR) + 0x002)
#define BURNERPLAYLIST_UNABLEOPENFILE		((BURNERPLAYLIST_ERROR) + 0x003)
#define BURNERPLAYLIST_WRITEERROR			((BURNERPLAYLIST_ERROR) + 0x004)
#define BURNERPLAYLIST_DECODEERROR			((BURNERPLAYLIST_ERROR) + 0x005)
#define BURNERPLAYLIST_DECODESERVICEFAILED	((BURNERPLAYLIST_ERROR) + 0x006)
#define BURNERPLAYLIST_THREADCREATEFAILED   ((BURNERPLAYLIST_ERROR) + 0x007)
#define BURNERPLAYLIST_NOFILES				((BURNERPLAYLIST_ERROR) + 0x008)
#define BURNERPLAYLIST_WRONGFILECOUNT		((BURNERPLAYLIST_ERROR) + 0x009)

//
#define BURNERPLAYLIST_ABORTED				((BURNERPLAYLIST_STATUS) +0x100)
#define BURNERPLAYLIST_DECODENEXTITEM		((BURNERPLAYLIST_ERROR) + 0x111)
#define BURNERPLAYLIST_DECODEITEM			((BURNERPLAYLIST_ERROR) + 0x112)

#define BURNERPLAYLIST_NEWAUDIOFAILED		((BURNERPLAYLIST_ERROR) + 0x131)
#define BURNERPLAYLIST_ADDITEMFAILED			((BURNERPLAYLIST_ERROR) + 0x132)
#define BURNERPLAYLIST_ADDITEMSKIPPED		((BURNERPLAYLIST_ERROR) + 0x133)
#define BURNERPLAYLIST_ITEMADDED				((BURNERPLAYLIST_ERROR) + 0x134)
#define BURNERPLAYLIST_BEGINBURN				((BURNERPLAYLIST_ERROR) + 0x135)
#define BURNERPLAYLIST_ENDBURN				((BURNERPLAYLIST_ERROR) + 0x136)
#define BURNERPLAYLIST_BEGINBURNFAILED		((BURNERPLAYLIST_ERROR) + 0x137)
#define BURNERPLAYLIST_ENDBURNFAILED			((BURNERPLAYLIST_ERROR) + 0x138)
#define BURNERPLAYLIST_WRITEAUDIOFAILED		((BURNERPLAYLIST_ERROR) + 0x139)
#define BURNERPLAYLIST_WRITELEADIN			((BURNERPLAYLIST_ERROR) + 0x13A)
#define BURNERPLAYLIST_WRITEDATA				((BURNERPLAYLIST_ERROR) + 0x13B)
#define BURNERPLAYLIST_WRITELEADOUT			((BURNERPLAYLIST_ERROR) + 0x13C)
#define BURNERPLAYLIST_DISCOPEN				((BURNERPLAYLIST_ERROR) + 0x13D)
#define BURNERPLAYLIST_DISCCLOSE				((BURNERPLAYLIST_ERROR) + 0x13E)
#define BURNERPLAYLIST_WRITEITEMBEGIN		((BURNERPLAYLIST_ERROR) + 0x13F)
#define BURNERPLAYLIST_WRITEITEMEND			((BURNERPLAYLIST_ERROR) + 0x140)
#define BURNERPLAYLIST_BURNFAILED			((BURNERPLAYLIST_ERROR) + 0x141)
//
#define BURNERPLAYLIST_FILENOTLICENSED		((BURNERPLAYLIST_ERROR) + 0x150)

// statuses
#define BURNERPLAYLIST_LICENSINGFINISHED		((BURNERPLAYLIST_STATUS) + 0x001)
#define BURNERPLAYLIST_LICENSINGSTARTING		((BURNERPLAYLIST_STATUS) + 0x002)
#define BURNERPLAYLIST_LICENSINGPROGRESS		((BURNERPLAYLIST_STATUS) + 0x003)


#define BURNERPLAYLIST_DECODEFINISHED		((BURNERPLAYLIST_STATUS) + 0x010)
#define BURNERPLAYLIST_DECODESTARTING		((BURNERPLAYLIST_STATUS) + 0x011)
#define BURNERPLAYLIST_DECODEPROGRESS		((BURNERPLAYLIST_STATUS) + 0x012)
#define BURNERPLAYLIST_DECODECANCELING		((BURNERPLAYLIST_STATUS) + 0x013)

#define BURNERPLAYLIST_BURNFINISHED			((BURNERPLAYLIST_STATUS) + 0x031)
#define BURNERPLAYLIST_BURNSTARTING			((BURNERPLAYLIST_STATUS) + 0x032)
#define BURNERPLAYLIST_BURNPROGRESS			((BURNERPLAYLIST_STATUS) + 0x033)
#define BURNERPLAYLIST_BURNCANCELING			((BURNERPLAYLIST_STATUS) + 0x034)
#define BURNERPLAYLIST_BURNFINISHING			((BURNERPLAYLIST_STATUS) + 0x035)


// callback returns
#define BURNERPLAYLIST_CONTINUE	0
#define BURNERPLAYLIST_STOP		1


typedef  Vector<BurnerItem*>  BurnerVector;

typedef struct _BPLDECODEINFO
{
	BurnerItem	*iInstance;
	int			iIndex;
	DWORD		iNotifyCode;
	DWORD		iErrorCode;
	float		percentCompleted;
}BPLDECODEINFO;

typedef struct _BPLRUNSTATUS
{
	DWORD	sCurrent;
	DWORD	sTotal;
	BurnerItem	*iInstance;
	int		iIndex;
	float   percentCompleted;
	
}BPLRUNSTATUS;

typedef DWORD (WINAPI *BURNERPLAYLISTCALLBACK)(void *sender, void *userparam, DWORD notifyCode, DWORD errorCode, ULONG_PTR param);


class BurnerPlaylist : private ifc_playlistloadercallback, BurnerVector, BurnManagerCallback
{
public:
	BURNLIB_API BurnerPlaylist(void);
	BURNLIB_API ~BurnerPlaylist(void);
	BURNLIB_API HRESULT Load(const wchar_t *filename);
	BURNLIB_API HRESULT CheckLicense(BURNERPLAYLISTCALLBACK notifyCB, void *userparam);
	BURNLIB_API HRESULT Decode(void* hFile, BURNERPLAYLISTCALLBACK notifyCB, void *userparam, BOOL block);
	BURNLIB_API HRESULT Burn(obj_primo *primoSDK, DWORD drive, DWORD maxspeed, DWORD burnFlags, void* hFile,
								BURNERPLAYLISTCALLBACK notifyCB, void *userparam, BOOL block);
	BURNLIB_API DWORD AddCompilationToCDDB(void);
	BURNLIB_API size_t GetCount(void) { return size(); }
	BURNLIB_API DWORD GetTotalLengthMS(void) { return length; }
	BURNLIB_API BurnerItem* &operator[](size_t index) { return BurnerVector::at(index); }
	BURNLIB_API BurnerItem* &at(size_t index) { return BurnerVector::at(index); }
	BURNLIB_API DWORD GetTotalSectors(void);
	BURNLIB_API BOOL SetEjectWhenDone(BOOL eject) { BOOL tmp = ejectDone; ejectDone = eject; return tmp; }
	BURNLIB_API DWORD GetStatus(DWORD *retCode); // if retCode not NULL - can return completed percentage or error code
	// state based calls
	BURNLIB_API size_t GetStateCount(DWORD state, DWORD code); 
	BURNLIB_API DWORD GetStateLengthMS(DWORD state, DWORD code); 
	BURNLIB_API DWORD GetStateSectors(DWORD state, DWORD code);  

protected:	
	static DWORD WINAPI DecodeWorker(void* param);
	static DWORD WINAPI BurnerWorker(void* param);
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info);
	static DWORD WINAPI OnItemDecode(void* sender, void *param, DWORD notifyCode, DWORD errorCode); 
	DWORD OnNotify(DWORD notifyCode, DWORD errorCode, ULONG_PTR param);
	void OnLicenseCallback(size_t numFiles, WRESULT *results);
	RECVS_DISPATCH;

protected:
	unsigned long			length;
	wchar_t					tmpfullname;
	obj_primo					*primoSDK;
	DWORD					drive;
	BURNERPLAYLISTCALLBACK	notifyCB;
	void					*userparam;
	HANDLE					hThread;
	HANDLE					hFile;
	BPLDECODEINFO			*activeDecode;
	float					percentStep;
	DWORD					maxspeed;
	DWORD					burnFlags;
	DWORD					statusCode;
	DWORD					errorCode;
	HANDLE					evntCancel;
	BOOL					ejectDone;
	BurnManager				manager;
};

#endif //NULLSOFT_BurnerPlaylist_HEADER