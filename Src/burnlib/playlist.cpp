#include "./playlist.h"
#include "../Agave/DecodeFile/ifc_audiostream.h"
#include <strsafe.h>
#include "../winamp/wa_ipc.h"

BurnerPlaylist::BurnerPlaylist()
{
	evntCancel = NULL;
}

BurnerPlaylist::~BurnerPlaylist()
{	
	if (manager.GetDecodeAPI())
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(decodeFileGUID);
		factory->releaseInterface(manager.GetDecodeAPI());
		manager.SetDecodeAPI(NULL);
	}
	clear();
}

HRESULT BurnerPlaylist::Load(const wchar_t *filename)
{
	if (!WASABI_API_SVC) return PLAYLISTMANAGER_FAILED;
	waServiceFactory *plmFactory = WASABI_API_SVC->service_getServiceByGuid(api_playlistmanagerGUID);
	if (!plmFactory) return 0;
	api_playlistmanager *plManager = (api_playlistmanager*)plmFactory->getInterface();
	if (!plManager) return 0;
	length = 0;
	int retCode = plManager->Load(filename, this);
	return (PLAYLISTMANAGER_SUCCESS == retCode);
}

DWORD BurnerPlaylist::GetTotalSectors(void)
{
	DWORD ts = 0;
	for(size_t i = 0; i < GetCount(); i++)
	{
		ts += BurnerVector::at(i)->GetSizeInSectors();
	}
	return ts;
}
DWORD BurnerPlaylist::GetStatus(DWORD *retCode)
{ 
	if(retCode) *retCode = errorCode;
	return statusCode; 
}

size_t BurnerPlaylist::GetStateCount(DWORD state, DWORD code)
{
	size_t count = 0;
	for (size_t i = 0; i < GetCount(); i++)
	{
		if (BurnerVector::at(i)->itemStatus == state && BurnerVector::at(i)->errorCode == code) count++;
	}
	return count;
}

DWORD BurnerPlaylist::GetStateLengthMS(DWORD state, DWORD code)
{
	DWORD len = 0;
	for (size_t i = 0; i < GetCount(); i++)
	{
		if (BurnerVector::at(i)->itemStatus == state && BurnerVector::at(i)->errorCode == code) len += BurnerVector::at(i)->GetLength();
	}
	return len;
}

DWORD BurnerPlaylist::GetStateSectors(DWORD state, DWORD code)
{
	DWORD ts = 0;
	for(size_t i = 0; i < GetCount(); i++)
	{
		if (BurnerVector::at(i)->itemStatus == state && BurnerVector::at(i)->errorCode == code) ts += BurnerVector::at(i)->GetSizeInSectors();
	}
	return ts;
}

HRESULT BurnerPlaylist::CheckLicense(BURNERPLAYLISTCALLBACK notifyCB, void *userparam)
{
	this->notifyCB = notifyCB;
	this->userparam = userparam;

	statusCode = BURNERPLAYLIST_LICENSINGSTARTING;
	errorCode = BURNERPLAYLIST_SUCCESS;
	OnNotify(statusCode, errorCode, 0); 

	size_t count = BurnerVector::size();
	size_t realCount = 0;
	manager.CancelBurn(); // ha-ha
	wchar_t **filenames = (count) ? (wchar_t**)malloc(count*sizeof(wchar_t*)) : NULL;
	DWORD nc, ec;
		
	for (size_t i = 0; i < count; i++)
	{
		nc = BurnerVector::at(i)->GetStatus(&ec);
		if (nc == BURNERITEM_READY || 
			((nc == BURNERITEM_LICENSED || nc == BURNERITEM_DECODED)  && ec == BURNERITEM_SUCCESS))
		{
			filenames[realCount] = (wchar_t*)BurnerVector::at(i)->GetFullName();
			BurnerVector::at(i)->itemStatus = BURNERITEM_LICENSING;
			errorCode = BURNERPLAYLIST_ITEMADDED;
			realCount++;
		}
		else
		{
			BurnerVector::at(i)->itemStatus = BURNERITEM_SKIPPED;
			errorCode = BURNERPLAYLIST_ADDITEMSKIPPED;
		}
		OnNotify(statusCode, errorCode, i); 	
	}
	if (realCount == 0)
	{
		statusCode = BURNERPLAYLIST_LICENSINGFINISHED;
		errorCode = BURNERPLAYLIST_NOFILES;
		OnNotify(statusCode, errorCode, 0);
		if (filenames) free(filenames);
		return statusCode;
	}
	if (!manager.GetDecodeAPI())
	{
		api_decodefile *decoder = NULL;	
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(decodeFileGUID);
		if (factory) decoder = (api_decodefile *)factory->getInterface();
		if (!factory || !decoder)
		{		
			statusCode = BURNERPLAYLIST_LICENSINGFINISHED;
			errorCode = BURNERPLAYLIST_DECODESERVICEFAILED;
			OnNotify(statusCode, errorCode, 0);
			free(filenames);
			return statusCode;
		}
		manager.SetDecodeAPI(decoder);
	}

	manager.SetFiles(realCount, (const wchar_t**)filenames, this);
	free(filenames);
	return statusCode;
}

HRESULT BurnerPlaylist::Decode(void* hFile, BURNERPLAYLISTCALLBACK notifyCB, void *userparam, BOOL block)
{
	this->notifyCB = notifyCB;
	this->userparam = userparam;
	this->hFile = hFile;
	hThread = NULL;
	
	statusCode = BURNERPLAYLIST_DECODESTARTING;
	errorCode = BURNERPLAYLIST_SUCCESS;
	OnNotify(statusCode, errorCode, 0);
	
	if (!manager.GetDecodeAPI())
	{
		api_decodefile *decoder = NULL;	
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(decodeFileGUID);
		if (factory) decoder = (api_decodefile *)factory->getInterface();
		if (!factory || !decoder)
		{		
			statusCode = BURNERPLAYLIST_DECODEFINISHED;
			errorCode = BURNERPLAYLIST_DECODESERVICEFAILED;
			OnNotify(statusCode, errorCode, 0);
			return statusCode;
		}
		manager.SetDecodeAPI(decoder);
	}

	if (block) 
	{
		statusCode = DecodeWorker(this);
		OnNotify(statusCode, errorCode, 100);
		if (manager.GetDecodeAPI())
		{
			waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(decodeFileGUID);
			factory->releaseInterface(manager.GetDecodeAPI());
			manager.SetDecodeAPI(NULL);
		}
		
	}
	else
	{
		DWORD id;
		hThread = CreateThread(NULL, 0, DecodeWorker, this, 0, &id);
		if (NULL == hThread)
		{
			statusCode = BURNERPLAYLIST_DECODEFINISHED;
			errorCode = BURNERPLAYLIST_THREADCREATEFAILED;
			OnNotify(statusCode, errorCode, 0);
		}
	}
	return statusCode;
}

HRESULT BurnerPlaylist::Burn(obj_primo *primoSDK, DWORD drive, DWORD maxspeed, DWORD burnFlags, void* hFile, 
														BURNERPLAYLISTCALLBACK notifyCB, void *userparam, BOOL block)
{
	this->primoSDK = primoSDK;
	this->drive = drive;
	this->hFile = hFile;
	this->maxspeed = maxspeed;
	this->burnFlags = burnFlags;
	this->notifyCB = notifyCB;
	this->userparam = userparam;
	statusCode = BURNERPLAYLIST_BURNSTARTING;
	errorCode = BURNERPLAYLIST_SUCCESS;
	evntCancel = CreateEvent(NULL, FALSE, FALSE, NULL);
	OnNotify(statusCode, errorCode, 0); // here we go
	
	DWORD retCode;
	DWORD dwUnits[2];
	dwUnits[0] = drive;
	dwUnits[1] = 0xFFFFFFFF;
	retCode = primoSDK->NewAudio(dwUnits);
	if (PRIMOSDK_OK != retCode)
	{
		statusCode = BURNERPLAYLIST_BURNFINISHED;
		errorCode = BURNERPLAYLIST_NEWAUDIOFAILED;
		OnNotify(statusCode, errorCode, retCode);
		return statusCode;
	}
	
	if (BurnerVector::size() == 0)
	{
		statusCode = BURNERPLAYLIST_BURNFINISHED;
		errorCode = BURNERPLAYLIST_NOFILES;
		OnNotify(statusCode, errorCode, retCode);
		return statusCode;
	}
	size_t i;
	for(i = 0; i < BurnerVector::size(); i++)
	{
		DWORD ec;
		if (BURNERITEM_DECODED == BurnerVector::at(i)->GetStatus(&ec) && BURNERITEM_SUCCESS == ec)
		{
			BurnerVector::at(i)->itemStatus = BURNERITEM_READY;
			retCode = BurnerVector::at(i)->AddStream(primoSDK, hFile);
			if (PRIMOSDK_OK != retCode) 
			{
				BurnerVector::at(i)->itemStatus = BURNERITEM_BURNED;
				BurnerVector::at(i)->errorCode = BURNERITEM_FAILED;
				errorCode = BURNERPLAYLIST_ADDITEMFAILED;
				break;
			}
			else errorCode = BURNERPLAYLIST_ITEMADDED;
		}
		else 	
		{
			errorCode = BURNERPLAYLIST_ADDITEMSKIPPED;
			BurnerVector::at(i)->itemStatus = BURNERITEM_SKIPPED;
		}
		OnNotify(statusCode, errorCode, i);

	}
	
	if (PRIMOSDK_OK != retCode || (WAIT_OBJECT_0 == WaitForSingleObject(evntCancel, 0)))
	{
		
		statusCode = (PRIMOSDK_OK == retCode) ? BURNERPLAYLIST_BURNCANCELING : BURNERPLAYLIST_BURNFINISHING;
		errorCode = (PRIMOSDK_OK == retCode) ? BURNERPLAYLIST_ABORTED : BURNERPLAYLIST_ADDITEMFAILED; 
		BPLRUNSTATUS burnStatus;
		
		for(i = 0; i < BurnerVector::size(); i++)
		{
			burnStatus.iIndex = (int)i;
			if (BURNERITEM_SKIPPED == BurnerVector::at(burnStatus.iIndex)->itemStatus) continue;
			
			BurnerVector::at(burnStatus.iIndex)->itemStatus = (PRIMOSDK_OK == retCode) ? BURNERITEM_ABORTED : BURNERITEM_FAILED;
			OnNotify(statusCode, errorCode, (ULONG_PTR)&burnStatus);
		}
		
		CloseHandle(evntCancel);
		evntCancel = NULL;
		primoSDK->CloseAudio();
		statusCode = BURNERPLAYLIST_BURNFINISHED;
		OnNotify(statusCode, errorCode, i);
		return statusCode;
	}

	if (block) 
	{
		DWORD notifyCode = BurnerWorker(this);
		OnNotify(BURNERPLAYLIST_BURNFINISHED, notifyCode, errorCode);
	}
	else
	{
		DWORD id;
		hThread = CreateThread(NULL, 0, BurnerWorker, this, 0, &id);
		if (NULL == hThread)
		{
			statusCode = BURNERPLAYLIST_BURNFINISHED;
			errorCode = BURNERPLAYLIST_THREADCREATEFAILED;
			OnNotify(statusCode, errorCode, i);
		}
	}
	
	return statusCode;
}

DWORD BurnerPlaylist::AddCompilationToCDDB(void)
{
	wchar_t buf[64] = {0};
	wchar_t albumbuf[256]= L"Mix CD ";

	wchar_t dateString[128] = {0};
	GetTimeFormatW(LOCALE_USER_DEFAULT, TIME_NOTIMEMARKER, NULL, NULL, dateString, 128);
	StringCchCatW(albumbuf, 256, dateString);							

	StringCchPrintfW(buf, 64, L"cda://%c.cda", (char)drive);
	extendedFileInfoStructW efis = { buf, L"album", albumbuf, 256, };
	
	if (SendMessageW(winampWnd, WM_WA_IPC, (WPARAM)&efis, IPC_SET_EXTENDED_FILE_INFOW))
	{
		efis.metadata = L"albumartist";
		efis.ret = L"Various Artists";
		SendMessageW(winampWnd, WM_WA_IPC, (WPARAM)&efis, IPC_SET_EXTENDED_FILE_INFOW);
		efis.metadata = L"genre";
		efis.ret = L"Mix";
		SendMessageW(winampWnd, WM_WA_IPC, (WPARAM)&efis, IPC_SET_EXTENDED_FILE_INFOW);
		SYSTEMTIME syst;
		GetLocalTime(&syst);
		if (syst.wYear)
		{
			wchar_t yearbuf[64] = {0};
			StringCchPrintfW(yearbuf, 64, L"%04d", syst.wYear);
			efis.metadata = L"year";
			efis.ret = yearbuf;
			SendMessageW(winampWnd, WM_WA_IPC, (WPARAM)&efis, IPC_SET_EXTENDED_FILE_INFOW);
		}
		wchar_t buf2[32] = {0};
		int index = 1;
		for (size_t i = 0;i < GetCount();i++)
		{
			DWORD is, ec;
			is = BurnerVector::at(i)->GetStatus(&ec);
			if (BURNERITEM_BURNED == is && BURNERITEM_SUCCESS == ec) 
			{
				StringCchPrintfW(buf, 64, L"cda://%c,%d.cda", (char)drive, i);
				lstrcpynW(buf2, L"title", 32);
				efis.metadata = buf2;
				efis.ret = const_cast<wchar_t*>(BurnerVector::at(i)->GetTitle());
				SendMessageW(winampWnd, WM_WA_IPC, (WPARAM)&efis, IPC_SET_EXTENDED_FILE_INFOW);

				lstrcpynW(buf2, L"artist", 32);
				efis.ret=L"Various Artists"; // TODO: use actual track artist
				SendMessageW(winampWnd, WM_WA_IPC, (WPARAM)&efis, IPC_SET_EXTENDED_FILE_INFOW);

				index++;
			}
		}
		SendMessageW(winampWnd, WM_WA_IPC, (WPARAM)0, IPC_WRITE_EXTENDED_FILE_INFO);
	}
	return 1;
}
DWORD WINAPI BurnerPlaylist::DecodeWorker(void* param)
{
	DWORD retCode;
	BurnerPlaylist *playlist = (BurnerPlaylist*)param;
	
	playlist->percentStep = (float)(1.f / ((double)playlist->GetCount()));
	playlist->activeDecode = NULL;
	DWORD itemError = BURNERITEM_SUCCESS;
	
	// for nice  ui reason lets do it twice
	playlist->statusCode = BURNERPLAYLIST_DECODESTARTING;
	for(size_t i = 0; i < playlist->GetCount(); i++)
	{
		DWORD ec;
		if (BURNERITEM_LICENSED == playlist->at(i)->GetStatus(&ec) && BURN_OK == ec)
		{
			playlist->errorCode = BURNERPLAYLIST_ITEMADDED;
			playlist->at(i)->itemStatus = BURNERITEM_READY;
		}
		else
		{
			
			playlist->errorCode = BURNERPLAYLIST_ADDITEMSKIPPED;
			playlist->at(i)->itemStatus = BURNERITEM_SKIPPED;
			
		}
		playlist->OnNotify(playlist->statusCode, playlist->errorCode, i);
	}

	// actual work
	for(size_t i = 0; i < playlist->GetCount(); i++)
	{
		if (BURNERITEM_READY == playlist->at(i)->GetStatus(NULL))
		{
			BPLDECODEINFO info;
			info.iInstance = playlist->at(i);
			info.iIndex = (int)i;
			info.iNotifyCode = 0;
			info.iErrorCode = 0;
			info.percentCompleted = (playlist->activeDecode) ? playlist->percentStep*i*100.0f : 0;
			playlist->activeDecode = &info;
			playlist->statusCode = BURNERPLAYLIST_DECODEPROGRESS;
			playlist->errorCode = BURNERPLAYLIST_DECODENEXTITEM;
			playlist->OnNotify(playlist->statusCode, playlist->errorCode, (ULONG_PTR)playlist->activeDecode);
			retCode = playlist->at(i)->Decode(&playlist->manager, playlist->hFile, OnItemDecode, playlist);
			if (BURNERITEM_SUCCESS != retCode)
			{
				itemError = retCode;
			}
			if (BURNERITEM_ABORTED == retCode)
			{
				playlist->statusCode = BURNERPLAYLIST_DECODECANCELING;
				playlist->errorCode = BURNERPLAYLIST_ABORTED;
				DWORD notifyCode = BURNERITEM_DECODEFINISHED;
				for(size_t k = i; k < playlist->GetCount(); k++)
				{
					info.iIndex = (int)k;
					info.iInstance = playlist->at(k);
					info.iInstance->errorCode = BURNERITEM_ABORTED;
					info.iInstance->itemStatus = BURNERITEM_ABORTED;
					OnItemDecode(info.iInstance, playlist, notifyCode, info.iInstance->errorCode);
				}
				break;
			}
		}
	}

	// release decoderAPI
	waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(decodeFileGUID);
	factory->releaseInterface(playlist->manager.GetDecodeAPI());
	playlist->manager.SetDecodeAPI(NULL);

	if (playlist->hThread) 
	{
		CloseHandle(playlist->hThread);
		playlist->hThread = NULL;
	}

	playlist->statusCode = BURNERPLAYLIST_DECODEFINISHED; 
	if (playlist->GetCount() == 0)  playlist->errorCode = BURNERPLAYLIST_NOFILES;
	else if (BURNERITEM_ABORTED == itemError) playlist->errorCode = BURNERPLAYLIST_ABORTED;
	else if (BURNERITEM_SUCCESS == itemError) playlist->errorCode = BURNERPLAYLIST_SUCCESS;
	else playlist->errorCode = BURNERPLAYLIST_FAILED;
	
	playlist->OnNotify(playlist->statusCode , playlist->errorCode, (ULONG_PTR)playlist->activeDecode);
	playlist->activeDecode = NULL;
	return playlist->statusCode;
}
DWORD WINAPI BurnerPlaylist::BurnerWorker(void* param)
{
	DWORD primoCode;
	BurnerPlaylist *playlist = (BurnerPlaylist*)param;

	WABURNSTRUCT bs;
	playlist->statusCode = BURNERPLAYLIST_BURNSTARTING;
	playlist->errorCode = BURNERPLAYLIST_BEGINBURN;
	playlist->OnNotify(playlist->statusCode, playlist->errorCode, 0);
	primoCode = BeginBurn(playlist->primoSDK, playlist->drive, &bs);
	bs.eject = TRUE;
	DWORD leadin = 0;
	BPLRUNSTATUS burnStatus;
	ZeroMemory(&burnStatus, sizeof(BPLRUNSTATUS));
	burnStatus.iIndex  = -1;
		
	BOOL canceled = FALSE;
	DWORD itemSector = 0;
	BOOL cp1 = FALSE;
	if (PRIMOSDK_OK != primoCode)
	{
		playlist->errorCode = BURNERPLAYLIST_BEGINBURNFAILED;
	}
	else
	{
		playlist->statusCode = BURNERPLAYLIST_BURNPROGRESS;
		primoCode = playlist->primoSDK->WriteAudioEx(playlist->burnFlags, playlist->maxspeed, 0x00);
		if (PRIMOSDK_OK == primoCode)
		{
			DWORD waitResult;
			while(BURNERPLAYLIST_BURNPROGRESS == playlist->statusCode && WAIT_TIMEOUT == (waitResult = WaitForSingleObject(playlist->evntCancel, 500)))
			{
				primoCode = playlist->primoSDK->RunningStatus(PRIMOSDK_GETSTATUS, &burnStatus.sCurrent, &burnStatus.sTotal);
				if (PRIMOSDK_RUNNING == primoCode)
				{
					playlist->statusCode = BURNERPLAYLIST_BURNPROGRESS;
					if (burnStatus.sCurrent == 0) 
					{
						if (cp1) continue; // do not send anymore
						playlist->errorCode = BURNERPLAYLIST_WRITELEADIN; //BURNERPLAYLIST_DISCOPEN
						cp1 = TRUE; 
					}
					else if (burnStatus.sCurrent > 0 && (!leadin || leadin == burnStatus.sCurrent)) 
					{
						if (!leadin) 
						{ 
							leadin = burnStatus.sCurrent;
							playlist->errorCode = BURNERPLAYLIST_WRITELEADIN; // unreachable :)
						}
						continue;
					}
					else if (burnStatus.sCurrent == burnStatus.sTotal)
					{
						if (burnStatus.iIndex >= 0 && BURNERITEM_BURNING == playlist->at(burnStatus.iIndex)->itemStatus)
						{
							playlist->at(burnStatus.iIndex)->itemStatus = BURNERITEM_BURNED;
							playlist->at(burnStatus.iIndex)->errorCode = BURNERITEM_SUCCESS;
							playlist->at(burnStatus.iIndex)->percentCompleted = 100;
							playlist->OnNotify(playlist->statusCode, BURNERPLAYLIST_WRITEITEMEND, (ULONG_PTR)&burnStatus);
						}
						if (burnStatus.iIndex  == -1) continue;
						playlist->errorCode = BURNERPLAYLIST_WRITELEADOUT;
						burnStatus.iIndex = -1;
						burnStatus.iInstance = NULL;
						
					}
					else
					{						
						playlist->errorCode = BURNERPLAYLIST_WRITEDATA;
						while (itemSector <  burnStatus.sCurrent && burnStatus.iIndex < (int)playlist->size()) 
						{	if (burnStatus.iIndex >= 0 && BURNERITEM_BURNING == playlist->at(burnStatus.iIndex)->itemStatus)
							{
								playlist->at(burnStatus.iIndex)->itemStatus = BURNERITEM_BURNED;
								playlist->at(burnStatus.iIndex)->errorCode = BURNERITEM_SUCCESS;
								playlist->at(burnStatus.iIndex)->percentCompleted = 100;
								playlist->OnNotify(playlist->statusCode, BURNERPLAYLIST_WRITEITEMEND, (ULONG_PTR)&burnStatus);
							}
							while (++burnStatus.iIndex < (int)playlist->size() && BURNERITEM_READY != playlist->at(burnStatus.iIndex)->itemStatus);
							if (burnStatus.iIndex < (int)playlist->size()) itemSector += playlist->at(burnStatus.iIndex)->GetSizeInSectors();
						}
						if ( burnStatus.iIndex >= 0 && burnStatus.iIndex < (int)playlist->size())
						{
							BurnerItem *bi =  playlist->at(burnStatus.iIndex);
							burnStatus.iInstance = bi;
							if (BURNERITEM_READY == bi->itemStatus)
							{
								bi->itemStatus = BURNERITEM_BURNING;
								bi->percentCompleted = 0;
								playlist->OnNotify(playlist->statusCode, BURNERPLAYLIST_WRITEITEMBEGIN, (ULONG_PTR)&burnStatus);
							}
							if (BURNERITEM_SUCCESS == bi->errorCode)
							{
								DWORD is = bi->GetSizeInSectors();
								bi->itemStatus = BURNERITEM_BURNING;
								bi->percentCompleted = (is - (itemSector - burnStatus.sCurrent))*100 / is;
							}
							else
							{
								bi->itemStatus = BURNERITEM_BURNED;
								bi->errorCode = BURNERITEM_SUCCESS;
								bi->percentCompleted = 100;
							}
						}
						
					}
					playlist->OnNotify(playlist->statusCode, playlist->errorCode, (ULONG_PTR)&burnStatus);
				}
				else 
				{
					playlist->errorCode = BURNERPLAYLIST_WRITEAUDIOFAILED;
					break;
				}
			}

			if (WAIT_OBJECT_0 == waitResult)
			{  // aborting
				canceled = TRUE;
				
				playlist->statusCode = BURNERPLAYLIST_BURNCANCELING;
				playlist->errorCode = BURNERPLAYLIST_ABORTED;
				playlist->at(burnStatus.iIndex)->itemStatus = BURNERITEM_CANCELING;
				playlist->OnNotify(playlist->statusCode, playlist->errorCode, (ULONG_PTR)&burnStatus);
				DWORD test = playlist->primoSDK->RunningStatus(PRIMOSDK_ABORT, &burnStatus.sCurrent, &burnStatus.sTotal);
				do
				{
					Sleep(1000);
					primoCode = playlist->primoSDK->RunningStatus(PRIMOSDK_GETSTATUS, &burnStatus.sCurrent, &burnStatus.sTotal);
				}while(PRIMOSDK_RUNNING == primoCode);
				for (size_t i = 0; i < playlist->GetCount(); i++)
				{
					burnStatus.iIndex = (int)i;
					DWORD cs = playlist->at(burnStatus.iIndex)->itemStatus;
					if (BURNERITEM_BURNING != cs && BURNERITEM_READY != cs && BURNERITEM_CANCELING != cs)  continue;
					playlist->at(burnStatus.iIndex)->itemStatus = BURNERITEM_ABORTED;
					playlist->OnNotify(playlist->statusCode, playlist->errorCode, (ULONG_PTR)&burnStatus);
				}
			}
		
		}
		else
		{
			playlist->errorCode = BURNERPLAYLIST_WRITEAUDIOFAILED;
		}
	}
	if (PRIMOSDK_USERABORT == primoCode) playlist->errorCode = BURNERPLAYLIST_ABORTED;
	else if (PRIMOSDK_OK == primoCode) playlist->errorCode = BURNERPLAYLIST_SUCCESS;
	playlist->statusCode = BURNERPLAYLIST_BURNFINISHING;
	playlist->OnNotify(playlist->statusCode, playlist->errorCode, 0);

	// check unit status and notify later
	DWORD statCode, cmd(0), sense(0), asc(0), ascq(0);
	statCode = playlist->primoSDK->UnitStatus(&bs.drive, &cmd, &sense, &asc, &ascq);
		
	bs.eject = playlist->ejectDone;
	
	primoCode = EndBurn(&bs);

	if (PRIMOSDK_OK != primoCode) playlist->errorCode = BURNERPLAYLIST_ENDBURNFAILED;
	
	playlist->primoSDK->CloseAudio();
			
	if (playlist->hThread)
	{
		CloseHandle(playlist->hThread);
		playlist->hThread = NULL;
	}		
	primoCode = (0xFF000000 & (statCode << 24)) | (0x00FF0000 & (sense << 16)) | (0x0000FF00 & (asc << 8)) | (0x000000FF & ascq);
	if (BURNERPLAYLIST_SUCCESS == playlist->errorCode && statCode != PRIMOSDK_OK) playlist->errorCode = BURNERPLAYLIST_FAILED;
		
	if (BURNERPLAYLIST_SUCCESS == playlist->errorCode && (PRIMOSDK_WRITE == (playlist->burnFlags&PRIMOSDK_WRITE)))
	{
		playlist->manager.BurnFinished();
	}
	else
	{
		playlist->manager.CancelBurn();
	}

	playlist->statusCode = BURNERPLAYLIST_BURNFINISHED;
	playlist->OnNotify(playlist->statusCode, playlist->errorCode, primoCode);
	return playlist->statusCode;
}

DWORD BurnerPlaylist::OnNotify(DWORD notifyCode, DWORD errorCode, ULONG_PTR param)
{
	DWORD retCode = (notifyCB) ? notifyCB(this, userparam, notifyCode, errorCode, param) : BURNERPLAYLIST_CONTINUE;
	if ( BURNERPLAYLIST_STOP == retCode && evntCancel) SetEvent(evntCancel);
	return retCode;
}


void BurnerPlaylist::OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
{
	if (NULL == filename) return;
	BurnerItem *item = new BurnerItem();
	item->Create(filename,title, lengthInMS);
	length += lengthInMS;
	push_back(item);
}

DWORD WINAPI BurnerPlaylist::OnItemDecode(void* sender, void *param, DWORD notifyCode, DWORD errorCode)
{
	BurnerPlaylist *pl = (BurnerPlaylist*)param;
	pl->activeDecode->iNotifyCode = notifyCode;
	pl->activeDecode->iErrorCode = errorCode;
	pl->activeDecode->percentCompleted += pl->percentStep; 
	pl->statusCode = BURNERPLAYLIST_DECODEPROGRESS;
	pl->errorCode = BURNERPLAYLIST_DECODEITEM;
	DWORD retCode = pl->OnNotify(pl->statusCode , pl->errorCode ,  (ULONG_PTR)pl->activeDecode);
	return (BURNERPLAYLIST_STOP == retCode) ?  BURNERITEM_STOP : BURNERITEM_CONTINUE;
}

void BurnerPlaylist::OnLicenseCallback(size_t numFiles, WRESULT *results)
{
	statusCode = BURNERPLAYLIST_LICENSINGPROGRESS;
	DWORD errorCode = BURNERPLAYLIST_SUCCESS;
	size_t realCount = 0;
	size_t allowed = 0;
	for (size_t i = 0; i < BurnerVector::size(); i++)
	{
		if (BURNERITEM_LICENSING == BurnerVector::at(i)->itemStatus)
		{
			if (realCount == numFiles) 
			{
				statusCode = BURNERPLAYLIST_LICENSINGFINISHED;
				errorCode = BURNERPLAYLIST_WRONGFILECOUNT;
				break;
			}
			BurnerVector::at(i)->itemStatus = BURNERITEM_LICENSED;
			BurnerVector::at(i)->errorCode = results[realCount];
			OnNotify(statusCode, results[realCount], (LPARAM)i);
			if (BURN_OK != results[realCount]) errorCode = BURNERPLAYLIST_FILENOTLICENSED;
			else allowed++;
			realCount++;
		}
		
	}
	statusCode = BURNERPLAYLIST_LICENSINGFINISHED;
	OnNotify(statusCode, errorCode, allowed);
}

#define CBCLASS BurnerPlaylist
START_DISPATCH;
VCB(IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile)
END_DISPATCH;
#undef CBCLASS