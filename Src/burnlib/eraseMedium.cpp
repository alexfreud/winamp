#include "./eraseMedium.h"
#include "api.h"
#include <api/service/waservicefactory.h>

EraseMedium::EraseMedium(void)
{
	primoSDK=0;
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_primo::getServiceGuid());
	if (sf) primoSDK = reinterpret_cast<obj_primo *>(sf->getInterface());	

	hThread = NULL;
	evntStop = NULL;
	evntThreadExit = NULL;
	errorCode = PRIMOSDK_OK;
	notifyCB = NULL;
}

EraseMedium::~EraseMedium(void)
{
	Stop();
		
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_primo::getServiceGuid());
	if (sf) sf->releaseInterface(primoSDK);
}

DWORD EraseMedium::SetEject(DWORD eject)
{
	DWORD tmp = this->eject;
	this->eject = eject;
	return tmp;
}

DWORD EraseMedium::Start(DWORD drive, DWORD eraseMode, ERASEMEDIUMCALLBACK notifyCB, void *userParam, int block)
{
	this->notifyCB = notifyCB;
	this->userparam = userParam;
	OnNotify(ERASEMEDIUM_READY, PRIMOSDK_OK);
	if (hThread) 
	{
		OnNotify(ERASEMEDIUM_ALREADYSTARTED, PRIMOSDK_OK);
		return PRIMOSDK_OK;
	}
	
	OnNotify(ERASEMEDIUM_INITIALIZING, PRIMOSDK_OK);
	
	if (!primoSDK)
	{
		OnNotify(ERASEMEDIUM_UNABLEINITPRIMO, PRIMOSDK_NOTLOADED);
		return errorCode;
	}
	
	// check unit
	errorCode = primoSDK->UnitReady(&drive);
	if (PRIMOSDK_OK != errorCode) 	
	{
		OnNotify(ERASEMEDIUM_DEVICENOTREADY, errorCode);
		return errorCode;
	}

	// check that disc is erasable
	DWORD erasable;
	errorCode = primoSDK->DiscInfoEx(&drive, 0, NULL, NULL, &erasable, NULL, NULL, NULL);
	if (PRIMOSDK_OK != errorCode)
	{	
		OnNotify(ERASEMEDIUM_DISCINFOERROR, errorCode);
		return errorCode;
	}
	if (!erasable)
	{
		OnNotify(ERASEMEDIUM_DISCNOTERASABLE, PRIMOSDK_OK);
		return errorCode;
	}

	// begin burn	
	errorCode = BeginBurn(primoSDK, drive, &bs);
	if (PRIMOSDK_OK != errorCode) 
	{
		OnNotify(ERASEMEDIUM_BEGINBURNFAILED, errorCode);
		return errorCode;
	}
	
	// erasing
	errorCode = primoSDK->EraseMedium(&bs.drive, eraseMode);
	if(PRIMOSDK_OK == errorCode)
	{
		OnNotify(ERASEMEDIUM_ERASING, PRIMOSDK_OK);
		evntStop = CreateEvent(NULL, FALSE, FALSE, NULL); 
		if (block) 	errorCode = StatusThread(this);
		else
		{
			DWORD threadID;
			hThread = CreateThread(NULL, 0, StatusThread, (LPVOID)this, NULL, &threadID);
		}
	}
	else
	{
		OnNotify(ERASEMEDIUM_ERASEMEDIUMFAILED, errorCode);
	}
	return errorCode;

}
void EraseMedium::Stop(void)
{
	
	if (hThread && evntStop)
	{
		DWORD waitResult;
		MSG msg;
		if (!evntThreadExit) evntThreadExit = CreateEvent(NULL, FALSE, FALSE, NULL);
		SetEvent(evntStop);
		
		while(WAIT_TIMEOUT == (waitResult = WaitForSingleObject(evntThreadExit, 10)))
		{
			if (!evntStop) break;
			while(PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
		CloseHandle(evntThreadExit);
		evntThreadExit = NULL;
	}
}


DWORD EraseMedium::OnNotify(DWORD eraseCode, DWORD primoCode)
{
	return (notifyCB) ? notifyCB(this, userparam, eraseCode, primoCode) : ERASEMEDIUM_CONTINUE;
}
	
DWORD WINAPI EraseMedium::StatusThread(void* parameter)
{
	EraseMedium *object = (EraseMedium*)parameter;
	DWORD current, total, waitResult;
	object->errorCode = PRIMOSDK_RUNNING;
	while(PRIMOSDK_RUNNING == object->errorCode && WAIT_TIMEOUT == (waitResult = WaitForSingleObject(object->evntStop, 1000)))
	{
		if (ERASEMEDIUM_STOP == object->OnNotify(ERASEMEDIUM_ERASING, PRIMOSDK_OK)) SetEvent(object->evntStop);
		object->errorCode =  object->primoSDK->RunningStatus(PRIMOSDK_GETSTATUS, &current, &total);
	}
	if (WAIT_OBJECT_0 == waitResult)
	{  // aborting
		object->OnNotify(ERASEMEDIUM_CANCELING, PRIMOSDK_OK);
		DWORD test = object->primoSDK->RunningStatus(PRIMOSDK_ABORT, &current, &total);
		do
		{
			Sleep(1000);
			object->errorCode = object->primoSDK->RunningStatus(PRIMOSDK_GETSTATUS, &current, &total);
		}while(PRIMOSDK_RUNNING == object->errorCode);
	}
	
	if (PRIMOSDK_OK != object->errorCode && PRIMOSDK_USERABORT != object->errorCode)
	{
		object->OnNotify(ERASEMEDIUM_ERASEMEDIUMFAILED, object->errorCode);
	}
	// check unit status
	DWORD cmd, sense, asc, ascq;
	object->errorCode = object->primoSDK->UnitStatus(&object->bs.drive, &cmd, &sense, &asc, &ascq);
	if (object->errorCode != PRIMOSDK_OK)
	{
		object->OnNotify(ERASEMEDIUM_ERASEMEDIUMFAILED, object->errorCode);
	}
	
	// end burn
	object->bs.eject = object->eject;
	object->OnNotify(ERASEMEDIUM_FINISHING, PRIMOSDK_OK);
	DWORD errorCode2 = EndBurn(&object->bs);
    if (PRIMOSDK_OK != errorCode2)
	{
		object->OnNotify(ERASEMEDIUM_ENDBURNFAILED, object->errorCode);
	}
	if (PRIMOSDK_OK == object->errorCode) object->errorCode = errorCode2;
	object->primoSDK->Release();
	CloseHandle(object->hThread);
	object->hThread = NULL;
	CloseHandle(object->evntStop);
	object->evntStop = NULL;
	object->OnNotify( (PRIMOSDK_USERABORT == object->errorCode)  ? ERASEMEDIUM_ABORTED : ERASEMEDIUM_COMPLETED, object->errorCode);
	
	if (object->evntThreadExit) SetEvent(object->evntThreadExit);
	return object->errorCode;
}
