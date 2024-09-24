#include <precomp.h>
#include "main.h"
#include "wa2pldirobj.h"
#include "wa2frontend.h"

#include <bfc/util/timefmt.h>
#include "wa2pledit.h"

/*
TODO:
register for playlists callbacks so we can keep up to date
drag&drop re-ordering
in-place renaming
widget for editing playlists

*/
const wchar_t plDirXuiObjectStr[] = L"PlaylistDirectory"; // This is the xml tag
char plDirXuiSvcName[] = "Playlist Directory XUI object"; // this is the name of the xuiservice

static PlDirScriptObjectController _pldirController;
ScriptObjectController *pldirController = &_pldirController;

BEGIN_SERVICES(wa2PlDirObj_Svcs);
DECLARE_SERVICETSINGLE(svc_scriptObject, PlDirScriptObjectSvc);
DECLARE_SERVICE(XuiObjectCreator<PlDirXuiSvc>);
END_SERVICES(wa2PlDirObj_Svcs, _wa2PlDirObj_Svcs);

#define CB_POPULATE 0x6273
#ifdef _X86_
extern "C"
{
	int _link_wa2PlDirObj_Svcs;
}
#else
extern "C"
{
	int __link_wa2PlDirObj_Svcs;
}
#endif

// -----------------------------------------------------------------------------------------------------
// Service
ScriptObjectController *PlDirScriptObjectSvc::getController(int n)
{
	switch (n)
	{
		case 0:
			return pldirController;
	}
	return NULL;
}

// -----------------------------------------------------------------------------------------------------
// PlDirObject
PlDirObject::PlDirObject()
{
	getScriptObject()->vcpu_setInterface(PLDIR_SCRIPTOBJECT_GUID, (void *)static_cast<PlDirObject *>(this));
	getScriptObject()->vcpu_setClassName(L"PlDir");
	getScriptObject()->vcpu_setController(pldirController);

	_pldirController.mylist.addItem(this);
}

PlDirObject::~PlDirObject()
{
	int numItems = getItemCount();

	for (int i=0;i<numItems;i++)
	{
		GUID *playlist_guid = (GUID *)getItemData(i);
		if (playlist_guid)
			delete playlist_guid;
	}

	if (WASABI_API_SYSCB) WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<PlaylistCallbackI *>(this));
	_pldirController.mylist.removeItem(this);
}

int PlDirObject::playlistcb_added(size_t index)
{
	postDeferredCallback(CB_POPULATE);
	return 1;
}

int PlDirObject::playlistcb_saved(size_t index)
{
	postDeferredCallback(CB_POPULATE);
	return 1;
}

int PlDirObject::onDeferredCallback(intptr_t p1, intptr_t p2)
{
	if (p1 == CB_POPULATE) 
	{
		Populate();
		return 1;
	}
	return ListWnd::onDeferredCallback(p1, p2);
}

void PlDirObject::Populate()
{
	int numItems = getItemCount();

	for (int i=0;i<numItems;i++)
	{
		GUID *playlist_guid = (GUID *)getItemData(i);
		if (playlist_guid)
			delete playlist_guid;
	}

	deleteAllItems();
	if (AGAVE_API_PLAYLISTS)
	{
		AGAVE_API_PLAYLISTS->Lock();
		size_t count = AGAVE_API_PLAYLISTS->GetCount();
		for (size_t i=0;i!=count;i++)
		{
			const wchar_t *playlistName = AGAVE_API_PLAYLISTS->GetName(i);
			GUID *playlist_guid = new GUID(AGAVE_API_PLAYLISTS->GetGUID(i));
			addItem(playlistName, (LPARAM)playlist_guid); // TODO: malloc pointer to GUID and store
			wchar_t temp[256] = {0};
			size_t numItems=0;
			AGAVE_API_PLAYLISTS->GetInfo(i, api_playlists_itemCount, &numItems, sizeof(numItems));
			WCSNPRINTF(temp, 256, L"%u", numItems);
			this->setSubItem(i, 1, temp);
			AGAVE_API_PLAYLISTS->GetInfo(i, api_playlists_totalTime, &numItems, sizeof(numItems)); //numitems holds now time
			wchar_t buf[1024] = {0};
			TimeFmt::printHourMinSec(numItems, buf, 1024, 1);
			this->setSubItem(i, 2, buf);
		}
		AGAVE_API_PLAYLISTS->Unlock();
	}
}

int PlDirObject::onInit()
{
	/*colresize = 1;
 	resizing_col = true;
	colresizeo = 1;*/

	ListWnd::onInit();
	setName(L"Playlists");
	//setItemIcon(
	//setShowColumnsHeaders(FALSE);
	ListColumn *nameCol = new ListColumn(L"Playlist Title", 1);
	//nameCol->setWidth(100);
	insertColumn(nameCol);
	/*ListColumn *countCol= new ListColumn(L"Items", 0);
	countCol->setWidth(40);
	countCol->setAlignment(COL_RIGHTALIGN);
	insertColumn(countCol,1,COL_RIGHTALIGN);
	ListColumn *lenCol= new ListColumn(L"Time", 0);
	lenCol->setWidth(50);
	lenCol->setAlignment(COL_CENTERALIGN);
	insertColumn(lenCol, 1, COL_CENTERALIGN);
	insertColumn(lenCol);*/
	addColumn(L"Items", 40, 0, COL_RIGHTALIGN);
	addColumn(L"Time", 55, 0, COL_RIGHTALIGN);
		
	//addColumn(L"Time", 100);
	Populate();

	WASABI_API_SYSCB->syscb_registerCallback(static_cast<PlaylistCallbackI *>(this));

	return 1;
}
/*
int PlDirObject::onResize()
{
	ListWnd::onResize();
	RECT r;
	getClientRect(&r);
	ListColumn *lc = getColumn(0);
	lc->setWidth(r.right - r.left);
	return 1;
}*/

void PlDirObject::onDoubleClick(int itemnum)
{
	ListWnd::onDoubleClick(itemnum);
	// find a playlisteditor object near us
	Wa2PlaylistEditor *editor = static_cast<Wa2PlaylistEditor *>(findWindowByInterface(Wa2PlaylistEditor::getInterfaceGuid()));
	if (editor != NULL)
	{
		Wa2Playlist *playlist = getPlaylist(itemnum);
		editor->setPlaylist(playlist);
	}
	GUID *playlist_guid = (GUID *)this->getItemData(itemnum);
	if (playlist_guid)
	{
		size_t index;
		AGAVE_API_PLAYLISTS->Lock();
		if (AGAVE_API_PLAYLISTS->GetPosition(*playlist_guid, &index) == API_PLAYLISTS_SUCCESS)
		{
			// TODO: benski> try to retrieve setting from ml_playlists for play vs enqueue on double click
			const wchar_t *playlist_filename = AGAVE_API_PLAYLISTS->GetFilename(index);
			wa2.playFile(playlist_filename);
		}
		AGAVE_API_PLAYLISTS->Unlock();
	}
}

Wa2Playlist *PlDirObject::getPlaylist(int itemnum)
{
	return (Wa2Playlist *) - 1;
}

// -----------------------------------------------------------------------------------------------------
// PlDirScriptObjectController

function_descriptor_struct PlDirScriptObjectController::exportedFunction[] = {
      {L"showCurrentlyPlayingEntry",			0, (void*)PlDirScriptObjectController::pldir_showCurrentlyPlayingEntry},
      {L"getNumItems",							0, (void*)PlDirScriptObjectController::pldir_getNumItems},
	  {L"renameItem",							2, (void*)PlDirScriptObjectController::pldir_renameItem},
	  {L"getItemName",							1, (void*)PlDirScriptObjectController::pldir_getItemName},
	  {L"playItem",								1, (void*)PlDirScriptObjectController::pldir_playItem},
	  {L"enqueueItem",							1, (void*)PlDirScriptObjectController::pldir_enqueueItem},
      {L"refresh",								0, (void*)PlDirScriptObjectController::pldir_refresh},
};

int PlDirScriptObjectController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

ScriptObject *PlDirScriptObjectController::instantiate()
{
	PlDirObject *c = new PlDirObject;
	if (!c) return NULL;
	return c->getScriptObject();
}

void PlDirScriptObjectController::destroy(ScriptObject *o)
{
	PlDirObject *obj = static_cast<PlDirObject *>(o->vcpu_getInterface(PLDIR_SCRIPTOBJECT_GUID));
	ASSERT(obj != NULL);
	delete obj;
}

PlDirScriptObjectController::~PlDirScriptObjectController()
{
	// Destroy our list, otherwise we get a big bang on wasabi shutdown
	if (_pldirController.mylist.getNumItems() > 0)
		_pldirController.mylist.deleteAll();
}

void *PlDirScriptObjectController::encapsulate(ScriptObject *o)
{
	return NULL; // nobody can inherits from me yet (see rootobj or guiobj in studio if you want to allow that)
}

void PlDirScriptObjectController::deencapsulate(void *)
{}

// Script calls
scriptVar PlDirScriptObjectController::pldir_showCurrentlyPlayingEntry(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT

	HWND hPeWindow = wa2.getWnd(IPC_GETWND_PE);	
	SendMessageW(hPeWindow, WM_USER, 666, wa2.PE_getCurrentIndex());

	RETURN_SCRIPT_VOID;
}

scriptVar PlDirScriptObjectController::pldir_getNumItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT

	AGAVE_API_PLAYLISTS->Lock();
	size_t n = AGAVE_API_PLAYLISTS->GetCount();
	AGAVE_API_PLAYLISTS->Unlock();

	return MAKE_SCRIPT_INT(n);
}

scriptVar PlDirScriptObjectController::pldir_refresh(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT

	for (int i = 0; i < _pldirController.mylist.getNumItems(); i++)
	{
		PlDirObject *p = _pldirController.mylist.enumItem(i);
		if (p) p->Populate();
	}

	// TODO: add refresh for ml pl view!
	//HWND hMlWindow = wa2.getMediaLibrary();
	//SendMessageW(hMlWindow, ???);

	//AGAVE_API_PLAYLISTS->MoveBefore(2,1);

	RETURN_SCRIPT_VOID;
}

scriptVar PlDirScriptObjectController::pldir_renameItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar item, scriptVar name)
{
	SCRIPT_FUNCTION_INIT

	AGAVE_API_PLAYLISTS->Lock();
	AGAVE_API_PLAYLISTS->RenamePlaylist(GET_SCRIPT_INT(item), GET_SCRIPT_STRING(name));
	AGAVE_API_PLAYLISTS->Unlock();

	for (int i = 0; i < _pldirController.mylist.getNumItems(); i++)
	{
		PlDirObject* p = _pldirController.mylist.enumItem(i);
		if (p) p->Populate();
	}

	//AGAVE_API_PLAYLISTS->MoveBefore(2,1);

	RETURN_SCRIPT_VOID;
}

scriptVar PlDirScriptObjectController::pldir_getItemName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar item)
{
	SCRIPT_FUNCTION_INIT

	AGAVE_API_PLAYLISTS->Lock();
	const wchar_t *k = AGAVE_API_PLAYLISTS->GetName(GET_SCRIPT_INT(item));
	AGAVE_API_PLAYLISTS->Unlock();

	return MAKE_SCRIPT_STRING(k);
}

scriptVar PlDirScriptObjectController::pldir_playItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _itemnum)
{
	SCRIPT_FUNCTION_INIT
	int itemnum = GET_SCRIPT_INT(_itemnum);

	GUID playlist_guid = AGAVE_API_PLAYLISTS->GetGUID(itemnum);

	size_t index;
	AGAVE_API_PLAYLISTS->Lock();
	if (AGAVE_API_PLAYLISTS->GetPosition(playlist_guid, &index) == API_PLAYLISTS_SUCCESS)
	{
		// TODO: benski> try to retrieve setting from ml_playlists for play vs enqueue on double click
		const wchar_t *playlist_filename = AGAVE_API_PLAYLISTS->GetFilename(index);
		wa2.playFile(playlist_filename);
	}
	AGAVE_API_PLAYLISTS->Unlock();

	RETURN_SCRIPT_VOID
}

scriptVar PlDirScriptObjectController::pldir_enqueueItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _itemnum)
{
	SCRIPT_FUNCTION_INIT

	int itemnum = GET_SCRIPT_INT(_itemnum);

	GUID playlist_guid = AGAVE_API_PLAYLISTS->GetGUID(itemnum);

	size_t index;
	AGAVE_API_PLAYLISTS->Lock();
	if (AGAVE_API_PLAYLISTS->GetPosition(playlist_guid, &index) == API_PLAYLISTS_SUCCESS)
	{
		// TODO: benski> try to retrieve setting from ml_playlists for play vs enqueue on double click
		const wchar_t *playlist_filename = AGAVE_API_PLAYLISTS->GetFilename(index);
		wa2.enqueueFile(playlist_filename);
	}
	AGAVE_API_PLAYLISTS->Unlock();

	RETURN_SCRIPT_VOID
}