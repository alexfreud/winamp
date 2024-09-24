#include <precomp.h>
#include "sprivate.h"
#include "main.h"
#include <api/application/api_application.h>
#include "wa2frontend.h"
#include <api.h>
#include "../Agave/Language/api_language.h"
#include "../../../../Components/wac_network/wac_network_http_receiver_api.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"
#include "../Winamp/buildtype.h"
#include "../nu/refcount.h"
#include <shlobj.h>
#include <shlwapi.h>

// {78BD6ED9-0DBC-4fa5-B5CD-5977E3A912F8}
static const GUID SPrivate_script_object_guid = 
{ 0x78bd6ed9, 0xdbc, 0x4fa5, { 0xb5, 0xcd, 0x59, 0x77, 0xe3, 0xa9, 0x12, 0xf8 } };


static SPrivateScriptObjectController _SPrivateController;
ScriptObjectController *SPrivateController=&_SPrivateController;

// -- Functions table -------------------------------------
function_descriptor_struct SPrivateScriptObjectController::exportedFunction[] =
{
	{L"updateLinks", 2, (void*)SPrivate::vcpu_updateLinks },
	{L"onLinksUpdated", 1, (void*)SPrivate::vcpu_onLinksUpdated },
};
// --------------------------------------------------------

const wchar_t *SPrivateScriptObjectController::getClassName()
{
	return L"Private"; 
}

const wchar_t *SPrivateScriptObjectController::getAncestorClassName()
{
	return L"Object";
}

ScriptObjectController *SPrivateScriptObjectController::getAncestorController()
{
	return NULL;
}

ScriptObject *SPrivateScriptObjectController::instantiate()
{
	SPrivate *c = new SPrivate;
	if (!c) return NULL;
	return c->getScriptObject();
}

int SPrivateScriptObjectController::getInstantiable()
{
	return 1;
}

void SPrivateScriptObjectController::destroy(ScriptObject *o)
{
	SPrivate *obj = static_cast<SPrivate *>(o->vcpu_getInterface(SPrivate_script_object_guid));
	ASSERT(obj != NULL);
	obj->dlcb = false;
	delete obj;
}

void *SPrivateScriptObjectController::encapsulate(ScriptObject *o)
{
	return NULL;
}

void SPrivateScriptObjectController::deencapsulate(void *o)
{
}

int SPrivateScriptObjectController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *SPrivateScriptObjectController::getExportedFunctions()
{
	return exportedFunction;
}

GUID SPrivateScriptObjectController::getClassGuid()
{
	return SPrivate_script_object_guid;
}

/*-----------------------------------*/

static void DoAPC(PAPCFUNC apc, ULONG_PTR data)
{
	HANDLE hMainThread = WASABI_API_APP->main_getMainThreadHandle();
	if (hMainThread)
	{
		QueueUserAPC(apc, hMainThread, data);
		CloseHandle(hMainThread);
	}
}

static void CALLBACK LinksUpdatedAPC(ULONG_PTR data)
{
	ScriptObject* scriptCallback = (ScriptObject *)data;
	SPrivate::vcpu_onLinksUpdated(SCRIPT_CALL, scriptCallback);
}

class PDownloadCallback : public Countable<ifc_downloadManagerCallback>
{
public:
	PDownloadCallback (const char *url, ScriptObject* scriptCallback) 
	{
		this->scriptCallback = scriptCallback;
		validSO = true;
	}

	~PDownloadCallback ()
	{
	}


	void OnFinish (DownloadToken token)
	{
		if (!validSO)
		{
			DeleteFileW(WAC_API_DOWNLOADMANAGER->GetLocation(token));
			delete this;
			return;
		}

		api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
		if (http)
		{
			int replyCode;
			replyCode = http->getreplycode();
			if (204 != replyCode)
			{
				BOOL succeeded;
				HANDLE hDest;
				const wchar_t *downloadDest; 
				wchar_t finalFileName[MAX_PATH] = {0};
								
				downloadDest = WAC_API_DOWNLOADMANAGER->GetLocation(token);
				succeeded = FALSE;

				hDest = CreateFileW(downloadDest, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 
									NULL, OPEN_EXISTING, 0, NULL);
				if (INVALID_HANDLE_VALUE != hDest)
				{
					LARGE_INTEGER fileSize;
					if (FALSE != GetFileSizeEx(hDest, &fileSize) &&
						0 != fileSize.QuadPart)
					{
						succeeded = TRUE;
					}

					CloseHandle(hDest);

					if (FALSE == succeeded)
					{
						DeleteFileW(downloadDest);
						downloadDest = NULL;
					}
				}
									
				if (FALSE != succeeded &&
					NULL != (PathCombineW(finalFileName, WASABI_API_APP->path_getUserSettingsPath(), L"links.xml")))
				{					
					// then move the file there
					succeeded = MoveFileW(downloadDest, finalFileName);
					if (FALSE == succeeded)
					{
						succeeded = CopyFileW(downloadDest, finalFileName, FALSE);
						DeleteFileW(downloadDest);
					}
					if (FALSE != succeeded)
					{
						// hop back on the main thread for the callback
						DoAPC(LinksUpdatedAPC, (ULONG_PTR)scriptCallback); 
					}
				}
			}
		}
		Release();
	}

	void OnError (DownloadToken token, int error)
	{
		Release();
	}
	void OnCancel (DownloadToken token)
	{
		Release();
	}

	void OnTick (DownloadToken token) {}

	ScriptObject* scriptCallback;
	bool validSO;
	REFERENCE_COUNT_IMPLEMENTATION;

protected:
	RECVS_DISPATCH;
};


SPrivate::SPrivate()
{
	getScriptObject()->vcpu_setInterface(SPrivate_script_object_guid, static_cast<SPrivate *>(this));
	getScriptObject()->vcpu_setClassName(L"Private");
	getScriptObject()->vcpu_setController(SPrivateController);
	dlcb = false;
}
SPrivate::~SPrivate()
{
}

scriptVar SPrivate::vcpu_updateLinks(SCRIPT_FUNCTION_PARAMS, ScriptObject *object, scriptVar version, scriptVar bversion)
{
	SCRIPT_FUNCTION_INIT;

	String url;

	//if defined(BETA) || defined(INTERNAL)
	const wchar_t *langIdentifier = WASABI_API_LNG?(WASABI_API_LNG->GetLanguageIdentifier(LANG_IDENT_STR)):0;
	if (!langIdentifier)
		langIdentifier = L"en-US";

	url = StringPrintf("http://client.winamp.com/data/skins?o=links&sid=bento&version=%s&waversion=%s&build=%i&browserversion=%s&lang=%s", AutoChar(version.data.sdata), WASABI_API_APP->main_getVersionNumString(), WASABI_API_APP->main_getBuildNumber(),AutoChar(bversion.data.sdata), AutoChar(langIdentifier));
	//url = StringPrintf("http://martin.skinconsortium.com/links.php?o=links&sid=bento&version=%s&waversion=%s&build=%i&browserversion=%s", AutoChar(version.data.sdata), WASABI_API_APP->main_getVersionNumString(), WASABI_API_APP->main_getBuildNumber(),AutoChar(bversion.data.sdata));

	SPrivate *sp = static_cast<SPrivate *>(object->vcpu_getInterface(SPrivate_script_object_guid));
	sp->dlcb = new PDownloadCallback(url, object);
	WAC_API_DOWNLOADMANAGER->Download(url, sp->dlcb);

	RETURN_SCRIPT_VOID;
}

scriptVar SPrivate::vcpu_onLinksUpdated(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, SPrivateController);
	SCRIPT_FUNCTION_CHECKABORTEVENT(o);
	SCRIPT_EXEC_EVENT0(o);
}


#define CBCLASS PDownloadCallback
START_DISPATCH;
REFERENCE_COUNTED;
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONFINISH, OnFinish)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONTICK, OnTick)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONERROR, OnError)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL, OnCancel)
END_DISPATCH;
#undef CBCLASS