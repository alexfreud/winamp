#include "main.h"

#include "MusicID.h"
#include <vector>
#include "../nu/AutoLock.h"

#include <atlbase.h>
#include <assert.h>

#include "api__gen_ml.h"

static Nullsoft::Utility::LockGuard musicIDGuard;

// {113D413A-5D1F-4f4c-8AB7-5BDED46033A4}
static const GUID developerConfigGroupGUID= 
{ 0x113d413a, 0x5d1f, 0x4f4c, { 0x8a, 0xb7, 0x5b, 0xde, 0xd4, 0x60, 0x33, 0xa4 } };

class MyEventHandler;

#ifndef IGNORE_API_GRACENOTE

static void TestAddRef(IDispatch *d)
{
	try
	{
	ULONG l = d->AddRef();
#ifdef _DEBUG
	char t[55] = {0};
	sprintf(t, "+ %p %x\n", d, l);
	OutputDebugStringA(t);
#endif
	}
	catch(...)
	{
	}
}

static void TestRelease(IDispatch *d)
{
	try
	{
	ULONG l = d->Release();
	#ifdef _DEBUG
	char t[55] = {0};
	sprintf(t, "- %p %x\n", d, l);
	OutputDebugStringA(t);
#endif
	}
	catch(...)
	{
	}
}


static IConnectionPoint *GetConnectionPoint(IUnknown *punk, REFIID riid)
{
	if (!punk)
		return 0;

	IConnectionPointContainer *pcpc;
	IConnectionPoint *pcp = 0;

	HRESULT hr = punk->QueryInterface(IID_IConnectionPointContainer, (void **) & pcpc);
	if (SUCCEEDED(hr))
	{
		pcpc->FindConnectionPoint(riid, &pcp);
		pcpc->Release();
	}
	punk->Release();
	return pcp;
}

// TODO: implement proper reference count so we don't leak the event handler & musicID objects

static HANDLE DuplicateCurrentThread()
{
	HANDLE fakeHandle = GetCurrentThread();
	HANDLE copiedHandle = 0;
	HANDLE processHandle = GetCurrentProcess();
	DuplicateHandle(processHandle, fakeHandle, processHandle, &copiedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
	return copiedHandle;
}

static HRESULT FillTag(ICddbFileInfo *info, BSTR filename)
{
	ICddbID3TagPtr infotag = NULL;
	infotag.CreateInstance(CLSID_CddbID3Tag);

	ICddbFileTag2_5Ptr tag2_5 = NULL;
	infotag->QueryInterface(&tag2_5);
	itemRecordW *record = 0;

	if (AGAVE_API_MLDB)
		record = AGAVE_API_MLDB->GetFile(filename);
	if (record && infotag && tag2_5)
	{
		wchar_t itemp[64] = {0};
		if (record->artist)
			infotag->put_LeadArtist(record->artist);

		if (record->album)
			infotag->put_Album(record->album);

		if (record->title)
			infotag->put_Title(record->title);

		if (record->genre)
			infotag->put_Genre(record->genre);

		if (record->track > 0)
			infotag->put_TrackPosition(_itow(record->track, itemp, 10));

		// TODO:	if (record->tracks > 0)
		if (record->year > 0)
			infotag->put_Year(_itow(record->year, itemp, 10));

		if (record->publisher)
			infotag->put_Label(record->publisher);

		/*
		if (GetFileInfo(filename, L"ISRC", meta, 512) && meta[0])
		infotag->put_ISRC(meta);
		*/

		if (record->disc > 0)
			infotag->put_PartOfSet(_itow(record->disc, itemp, 10));

		if (record->albumartist)
			tag2_5->put_DiscArtist(record->albumartist);

		if (record->composer)
			tag2_5->put_Composer(record->composer);

		if (record->length > 0)
			tag2_5->put_LengthMS(_itow(record->length*1000, itemp, 10));

		if (record->bpm > 0)
			infotag->put_BeatsPerMinute(_itow(record->bpm, itemp, 10));

		/*
		if (GetFileInfo(filename, L"conductor", meta, 512) && meta[0])
		tag2_5->put_Conductor(meta);
		*/
		AGAVE_API_MLDB->FreeRecord(record);
	}

	if (info) info->put_Tag(infotag);

	return S_OK;
}

struct Blah
{
	IDispatch *callback;
	CComBSTR filename, tagID, artist;
};

static VOID CALLBACK InvokeAPC(ULONG_PTR param)
{
	Blah *blah = (Blah *)param;
	VARIANT arguments[2];
	VariantInit(&arguments[0]);
	arguments[0].vt = VT_BSTR;
	arguments[0].bstrVal = blah->tagID;

	VariantInit(&arguments[1]);
	arguments[1].vt = VT_BSTR;
	arguments[1].bstrVal = blah->filename;

	DISPPARAMS params;
	params.cArgs = 2;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = 0;
	params.rgvarg = arguments;
	unsigned int ret;

	if (!ieDisableSEH)
	{
		ieDisableSEH = AGAVE_API_CONFIG->GetItem(developerConfigGroupGUID, L"no_ieseh");
	}

	OLECHAR *setID = L"SetID", *setArtist = L"SetArtist", *onFinish=L"OnFinish";
	DISPID dispid;
	if (ieDisableSEH->GetBool() == false)
	{
		try
		{
			if (SUCCEEDED(blah->callback->GetIDsOfNames(IID_NULL, &setID, 1, LOCALE_SYSTEM_DEFAULT, &dispid)))
				blah->callback->Invoke(dispid, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);
			else
				blah->callback->Invoke(0, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);

			arguments[0].bstrVal = blah->artist;
			if (SUCCEEDED(blah->callback->GetIDsOfNames(IID_NULL, &setArtist, 1, LOCALE_SYSTEM_DEFAULT, &dispid)))
				blah->callback->Invoke(dispid, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);

			arguments[0].bstrVal = blah->filename;
			params.cArgs=1;
			if (SUCCEEDED(blah->callback->GetIDsOfNames(IID_NULL, &onFinish, 1, LOCALE_SYSTEM_DEFAULT, &dispid)))
				blah->callback->Invoke(dispid, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);

		}
		catch (...)
		{
		}
	}
	else
	{
		if (SUCCEEDED(blah->callback->GetIDsOfNames(IID_NULL, &setID, 1, LOCALE_SYSTEM_DEFAULT, &dispid)))
			blah->callback->Invoke(dispid, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);
		else
			blah->callback->Invoke(0, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);

		arguments[0].bstrVal = blah->artist;
		if (SUCCEEDED(blah->callback->GetIDsOfNames(IID_NULL, &setArtist, 1, LOCALE_SYSTEM_DEFAULT, &dispid)))
			blah->callback->Invoke(dispid, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);

		arguments[0].bstrVal = blah->filename;
		params.cArgs=1;
		if (SUCCEEDED(blah->callback->GetIDsOfNames(IID_NULL, &onFinish, 1, LOCALE_SYSTEM_DEFAULT, &dispid)))
			blah->callback->Invoke(dispid, GUID_NULL, 0, DISPATCH_METHOD, &params, 0, 0, &ret);
	}

	delete blah;
}

class MyEventHandler : public _ICDDBMusicIDManagerEvents
{
public:
	MyEventHandler(BSTR _filename, IDispatch *_callback, HANDLE _handle) : callback(_callback), threadHandle(_handle)
	{
		filename = SysAllocString(_filename);
		refCount = 1;
		TestAddRef(callback);
	}

	BSTR filename;
	ICDDBMusicIDManager3 *musicID;
	HANDLE threadHandle;
	IDispatch *callback;

		ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return InterlockedIncrement(&refCount);
	}

	ULONG STDMETHODCALLTYPE Release(void)
	{
		LONG lRef = InterlockedDecrement(&refCount);
		if (lRef == 0) 
			delete this;
		return lRef;
	}

private:
		~MyEventHandler()
	{
		SysFreeString(filename);
		CloseHandle(threadHandle);
		TestRelease(callback);
	}
	LONG refCount;

	STDMETHODIMP STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObject)
	{
		if (!ppvObject)
			return E_POINTER;

		else if (IsEqualIID(riid, __uuidof(_ICDDBMusicIDManagerEvents)))
			*ppvObject = (_ICDDBMusicIDManagerEvents *)this;
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



	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
	{
		switch (dispid)
		{
		case 1: // OnTrackIDStatusUpdate, params: CddbMusicIDStatus Status, BSTR filename, long* Abort
			break;
		case 2: // OnAlbumIDStatusUpdate, params: CddbMusicIDStatus Status, BSTR filename, long current_file, long total_files, long* Abort
			{
				//long *abort = pdispparams->rgvarg[0].plVal;
				//long total_files = pdispparams->rgvarg[1].lVal;
				//long current_file= pdispparams->rgvarg[2].lVal;
				CddbMusicIDStatus status = (CddbMusicIDStatus)pdispparams->rgvarg[4].lVal;
				BSTR filename = pdispparams->rgvarg[3].bstrVal;
			}
			break;
		case 3: // OnTrackIDComplete, params: LONG match_code, ICddbFileInfo* pInfoIn, ICddbFileInfoList* pListOut
			{
				IDispatch *disp1 =pdispparams->rgvarg[0].pdispVal;
				IDispatch *disp2 =pdispparams->rgvarg[1].pdispVal;
				//long match_code = pdispparams->rgvarg[2].lVal;
				ICddbFileInfoPtr pInfoIn;
				ICddbFileInfoListPtr matchList;
				disp1->QueryInterface(&matchList);
				disp2->QueryInterface(&pInfoIn);
			}
			break;
		case 4: // OnAlbumIDComplete, params: LONG match_code, ICddbFileInfoList* pListIn, ICddbFileInfoLists* pListsOut
			{
				IDispatch *disp1 =pdispparams->rgvarg[0].pdispVal;
				IDispatch *disp2 =pdispparams->rgvarg[1].pdispVal;
				//long match_code = pdispparams->rgvarg[2].lVal;
				ICddbFileInfoListPtr pListIn;
				ICddbFileInfoListsPtr pListsOut;
				disp1->QueryInterface(&pListsOut);
				disp2->QueryInterface(&pListIn);
			}
			break;

		case 10: // OnGetFingerprintInfo
			{
				long *abort = pdispparams->rgvarg[0].plVal;
				IDispatch *disp = pdispparams->rgvarg[1].pdispVal;
				BSTR filename = pdispparams->rgvarg[2].bstrVal;

				ICddbFileInfo *info;
				disp->QueryInterface(&info);
				return AGAVE_API_GRACENOTE->CreateFingerprint(musicID, AGAVE_API_DECODE, info, filename, abort);
			}
			break;
		case 11: // OnGetTagInfo
			{
				//long *Abort = pdispparams->rgvarg[0].plVal;
				IDispatch *disp = pdispparams->rgvarg[1].pdispVal;
				BSTR filename = pdispparams->rgvarg[2].bstrVal;

				ICddbFileInfo *info;
				disp->QueryInterface(&info);
				return FillTag(info, filename);
			}
			break;
		}
		return DISP_E_MEMBERNOTFOUND;
	}

	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
	{
		*rgdispid = DISPID_UNKNOWN;
		return DISP_E_UNKNOWNNAME;
	}

	HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int FAR * pctinfo)
	{
		return E_NOTIMPL;
	}

};

static VOID CALLBACK ReleaseAPC(ULONG_PTR param)
{
	MyEventHandler *handler = (MyEventHandler *)param;
	handler->Release();
}

enum
{
	DISPATCH_MUSICID_GETID = 777,
};


static std::vector<MyEventHandler*> tracks;
static HANDLE musicid_killswitch=0, musicid_trigger=0;
static ThreadID *musicIdThread=0;

class MusicIDContext
{
public:
	MusicIDContext()
	{
		musicID=0;
		com_initted=false;
	}
	bool Init();
	void Tick();
	void Quit();
	ICDDBMusicIDManager3 *musicID;
	bool com_initted;
};
static MusicIDContext context;

bool MusicIDContext::Init()
{
	if (SUCCEEDED(CoInitialize(0)))
		com_initted=true;

	musicID=AGAVE_API_GRACENOTE->GetMusicID();
	return !!musicID;
}

void MusicIDContext::Tick()
{
	musicIDGuard.Lock();
	if (tracks.empty())
	{
		musicIDGuard.Unlock();
		return;
	}
	MyEventHandler *track = tracks.front();
	tracks.pop_front();
	musicIDGuard.Unlock();

	if (!musicID)
	{
		Blah *blah = new Blah;
		blah->filename = SysAllocString(track->filename);
		blah->tagID = L"";
		blah->artist = L"";
		blah->callback=track->callback;

		QueueUserAPC(InvokeAPC, track->threadHandle, (ULONG_PTR)blah);
		QueueUserAPC(ReleaseAPC, track->threadHandle, (ULONG_PTR)track);
		return;
	}
	track->musicID = musicID;
	IConnectionPoint *icp = GetConnectionPoint(musicID, DIID__ICDDBMusicIDManagerEvents);

	DWORD m_dwCookie=0;

	if (icp)
	{
		icp->Advise(static_cast<IDispatch *>(track), &m_dwCookie);
		//icp->Release();
	}

	ICddbFileInfoPtr info;
	info.CreateInstance(CLSID_CddbFileInfo);
	if (info == 0)
	{
		QueueUserAPC(ReleaseAPC, track->threadHandle, (ULONG_PTR)track);
		return ;
	}
	info->put_Filename(track->filename);
	ICddbFileInfoListPtr matchList;
	long match_code=666;

	musicID->TrackID(info, MUSICID_RETURN_SINGLE|MUSICID_GET_TAG_FROM_APP|MUSICID_GET_FP_FROM_APP|MUSICID_PREFER_WF_MATCHES, &match_code, &matchList);

	if (matchList)
	{
		long matchcount;
		matchList->get_Count(&matchcount);
		assert(matchcount==1);
		for (int j = 1;j <= matchcount;j++)
		{
			ICddbFileInfoPtr match;
			matchList->GetFileInfo(j, &match);

			Blah *blah = new Blah;

			match->get_Filename(&blah->filename);
			ICddbFileTagPtr tag;
			match->get_Tag(&tag);
			tag->get_FileId(&blah->tagID);
			tag->get_LeadArtist(&blah->artist);
			blah->callback=track->callback;
			if (blah->tagID && AGAVE_API_MLDB)
				AGAVE_API_MLDB->SetField(blah->filename, "GracenoteFileID", blah->tagID);

			QueueUserAPC(InvokeAPC, track->threadHandle, (ULONG_PTR)blah);

			if (AGAVE_API_MLDB)
				AGAVE_API_MLDB->Sync();
			//TODO: optionally write metadata to file permanently
		}
	}

	if (icp)
	{
		icp->Unadvise(m_dwCookie);
	}
	QueueUserAPC(ReleaseAPC, track->threadHandle, (ULONG_PTR)track);
}

void MusicIDContext::Quit()
{
	if (musicID)
	{
		musicID->Shutdown();
		musicID->Release();
		musicID=0;
	}
	if (com_initted)
	{
		CoUninitialize();
		com_initted=false;
	}
}

static int MusicIDTickThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id)
{
	MusicIDContext *context = (MusicIDContext *)user_data;
	context->Tick();
	return 0;
}

static int MusicIDInitThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id)
{
	MusicIDContext *context = (MusicIDContext *)user_data;
	if (context->Init())
	{
		AGAVE_API_THREADPOOL->AddHandle(musicIdThread, musicid_trigger, MusicIDTickThreadPoolFunc, user_data, id, 0);
	}

	return 0;
}

static int MusicIDQuitThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id)
{
	MusicIDContext *context = (MusicIDContext *)user_data;
	AGAVE_API_THREADPOOL->RemoveHandle(musicIdThread, musicid_trigger);
	context->Quit();	
	SetEvent(musicid_killswitch);
	return 0;
}

HRESULT MusicIDCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		if (wcscmp(rgszNames[i], L"GetID") == 0)
			rgdispid[i] = DISPATCH_MUSICID_GETID;
		else
		{
			rgdispid[i] = DISPID_UNKNOWN;
			unknowns = true;
		}

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT MusicIDCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT MusicIDCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT MusicIDCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	if (dispid == DISPATCH_MUSICID_GETID)
	{
		if (pdispparams->cArgs != 2)
			return DISP_E_BADPARAMCOUNT;

		if (pdispparams->rgvarg[0].vt != VT_DISPATCH)
			return DISP_E_TYPEMISMATCH;

		if (pdispparams->rgvarg[1].vt != VT_BSTR)
			return DISP_E_TYPEMISMATCH;

		if (!AGAVE_API_GRACENOTE)
			return E_FAIL;

		if (!musicIdThread)
		{
			musicIDGuard.Lock();
			if (!musicIdThread)
			{
				musicIdThread = AGAVE_API_THREADPOOL->ReserveThread(api_threadpool::FLAG_REQUIRE_COM_STA);
				musicid_trigger = CreateSemaphore(0, 0, 65536, 0);
				AGAVE_API_THREADPOOL->RunFunction(musicIdThread, MusicIDInitThreadPoolFunc, &context, 0, 0);
			}
			musicIDGuard.Unlock();
		}

		MyEventHandler *event = new MyEventHandler(pdispparams->rgvarg[1].bstrVal, pdispparams->rgvarg[0].pdispVal, DuplicateCurrentThread());

		musicIDGuard.Lock();
		tracks.push_back(event);
		musicIDGuard.Unlock();
		ReleaseSemaphore(musicid_trigger, 1, 0);

		return S_OK;
	}

	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP MusicIDCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG MusicIDCOM::AddRef(void)
{
	//	return ++m_cRefs;
	return 0;
}

ULONG MusicIDCOM::Release(void)
{ /*
	if (--m_cRefs)
	return m_cRefs;

	delete this;
	return 0;*/
	return 0;
}

void MusicIDCOM::Quit()
{
	if (musicIdThread)
	{
		musicid_killswitch = CreateEvent(NULL, FALSE, FALSE, NULL);
		AGAVE_API_THREADPOOL->RunFunction(musicIdThread, MusicIDQuitThreadPoolFunc, &context, 0, 0);
		if (NULL != musicid_killswitch)
		{
			WaitForSingleObject(musicid_killswitch, INFINITE);
			CloseHandle(musicid_killswitch);
		}

		CloseHandle(musicid_trigger);
		AGAVE_API_THREADPOOL->ReleaseThread(musicIdThread);
	}
}
#endif