// -----------------------------------------------------------------------------------------------------
//		Playlist Script Object
//
//		functions for <pledit.mi>
// -----------------------------------------------------------------------------------------------------

#include <precomp.h>
#include "main.h"

#include "PlaylistScriptObject.h"

#include <bfc/wasabi_std.h>
#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/script/scriptobj.h>
#include <api/script/scriptguid.h>

#ifdef WASABI_COMPILE_MEDIACORE
#include <api/service/svcs/svc_player.h>
#include <api/core/buttons.h>
#include <api/core/api_core.h>
#include <api/core/corehandle.h>	// safe to include even if core isn't there
#endif

static PlaylistScriptController _playlistController;
ScriptObjectController *playlistController = &_playlistController;


#ifndef _WASABIRUNTIME

BEGIN_SERVICES(PlaylistObject_Svc);
DECLARE_SERVICE(ScriptObjectCreator<PlaylistScriptObjectSvc>);
END_SERVICES(PlaylistObject_Svc, _PlaylistObject_Svc);

#ifdef _X86_
extern "C" { int _link_PlaylistObjectSvc; }
#else
extern "C" { int __link_PlaylistObjectSvc; }
#endif

#endif


// -----------------------------------------------------------------------------------------------------
// Functions Table
function_descriptor_struct PlaylistScriptController::exportedFunction[] =
{
	{L"showCurrentlyPlayingTrack",			0, (void*)SPlaylist::script_vcpu_showCurrentlyPlayingEntry},
	{L"showTrack",							1, (void*)SPlaylist::script_vcpu_showEntry},
	{L"getNumTracks",						0, (void*)SPlaylist::script_vcpu_getNumItems},
	{L"getCurrentIndex",					0, (void*)SPlaylist::script_vcpu_getCurrentIndex},
	{L"getRating",							1, (void*)SPlaylist::script_vcpu_getTrackRating},
	{L"setRating",							2, (void*)SPlaylist::script_vcpu_setTrackRating},
	{L"enqueueFile",						1, (void*)SPlaylist::script_vcpu_enqueueFile },
	{L"clear",								0, (void*)SPlaylist::script_vcpu_clear },
	{L"removeTrack",						1, (void*)SPlaylist::script_vcpu_removeTrack },
	{L"swapTracks",							2, (void*)SPlaylist::script_vcpu_swapTrack },
	{L"moveUp",								1, (void*)SPlaylist::script_vcpu_moveUp },
	{L"moveDown",							1, (void*)SPlaylist::script_vcpu_moveDown },
	{L"moveTo",								2, (void*)SPlaylist::script_vcpu_moveTo },
	{L"getTitle",							1, (void*)SPlaylist::script_vcpu_getTitle },
	{L"getLength",							1, (void*)SPlaylist::script_vcpu_getLength },
	{L"getMetaData",						2, (void*)SPlaylist::script_vcpu_getExtendedInfo },
	{L"getNumSelectedTracks",				0, (void*)SPlaylist::script_vcpu_getNumSelectedItems },  
	{L"getNextSelectedTrack",				1, (void*)SPlaylist::script_vcpu_getNextSelectedItem },  
	{L"getFileName",						1, (void*)SPlaylist::script_vcpu_getFileName },  
	{L"playTrack",							1, (void*)SPlaylist::script_vcpu_playTrack }, 
	{L"onPleditModified",					0, (void*)SPlaylist::script_vcpu_onPleditModified }, 
};

ScriptObjectController *PlaylistScriptObjectSvc::getController(int n) 
{
  switch (n) {
    case 0:
      return playlistController;
  }
  return NULL;
}

ScriptObject *PlaylistScriptController::instantiate()
{
	SPlaylist *c = new SPlaylist;
	if (!c) return NULL;
	return c->getScriptObject();
}

void PlaylistScriptController::destroy(ScriptObject *o)
{
	SPlaylist *obj = static_cast<SPlaylist *>(o->vcpu_getInterface(playlistScriptObjectGUID));
	ASSERT(obj != NULL);
	delete obj;
}

void *PlaylistScriptController::encapsulate(ScriptObject *o)
{
	return NULL;
}

void PlaylistScriptController::deencapsulate(void *o)
{
}

int PlaylistScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

SPlaylist::SPlaylist()
{
	getScriptObject()->vcpu_setInterface(playlistScriptObjectGUID, (void *)static_cast<SPlaylist *>(this));
	getScriptObject()->vcpu_setClassName(L"PlEdit");
	getScriptObject()->vcpu_setController(playlistController);

	SOList.addItem(this);
}

SPlaylist::~SPlaylist()
{
	SOList.removeItem(this);
}

// Script helper functions

void SPlaylist::onPleditModified ()
{
 //This one is not working
	for (int i=0; i < SOList.getNumItems(); i++)
	{
		//script_vcpu_onPleditModified(SCRIPT_CALL, getScriptObject()); 
		script_vcpu_onPleditModified(SCRIPT_CALL, SOList.enumItem(i)->getScriptObject());
		//Std::messageBox(SOList.enumItem(i)->getClassName(), t, 0);
	}
}

void SPlaylist::showEntry( int i)
{
	HWND hPeWindow = wa2.getWnd(IPC_GETWND_PE);	
	SendMessageW(hPeWindow, WM_USER, 666, i);
}

void SPlaylist::swap(int track1, int track2)
{
	HWND hPeWindow = wa2.getWnd(IPC_GETWND_PE);	

	int i = wa2.PE_getCurrentIndex();

	int param = ((track1) << 16) | (track2);
	SendMessageW(hPeWindow, WM_USER, IPC_PE_SWAPINDEX, param);

	//Swap our currently playing item as well
	if (i == track1)
	{
		wa2.PE_setCurrentIndex(track2);
	}
	else if (i == track2)
	{
		wa2.PE_setCurrentIndex(track1);
	}
}

fileinfoW *SPlaylist::getFileInfoStructW1 (int item)
{
	static fileinfoW fi;
	fi.index = item;
	*(fi.file) = 0;
	HWND hPeWindow = wa2.getWnd(IPC_GETWND_PE);	
	SendMessageW(hPeWindow, WM_USER, IPC_PE_GETINDEXINFOW_INPROC, (LPARAM)&fi);
	return &fi;
}

// Script Calls

scriptVar SPlaylist::script_vcpu_showCurrentlyPlayingEntry(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT

	showEntry (wa2.PE_getCurrentIndex());

	RETURN_SCRIPT_VOID;
}


scriptVar SPlaylist::script_vcpu_showEntry(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i)
{
	SCRIPT_FUNCTION_INIT

	showEntry (GET_SCRIPT_INT(i));

	RETURN_SCRIPT_VOID;
}

scriptVar SPlaylist::script_vcpu_getNumItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT

	int i = wa2.PE_getNumItems();

	return MAKE_SCRIPT_INT(i);
}

scriptVar SPlaylist::script_vcpu_getCurrentIndex(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT

	int i = wa2.PE_getCurrentIndex();

	return MAKE_SCRIPT_INT(i);
}

scriptVar SPlaylist::script_vcpu_setTrackRating(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i, scriptVar rating)
{
	SCRIPT_FUNCTION_INIT

	HWND hwnd_winamp = wa2.getMainWindow();

	int cur_pos = wa2.PE_getCurrentIndex();
	wa2.PE_setCurrentIndex(GET_SCRIPT_INT(i));
	SendMessageW(hwnd_winamp, WM_WA_IPC, GET_SCRIPT_INT(rating), IPC_SETRATING);
	wa2.PE_setCurrentIndex(cur_pos);

	RETURN_SCRIPT_VOID;
}

scriptVar SPlaylist::script_vcpu_getTrackRating(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i)
{
	SCRIPT_FUNCTION_INIT

	HWND hwnd_winamp = wa2.getMainWindow();

	int cur_pos = wa2.PE_getCurrentIndex();
	wa2.PE_setCurrentIndex(GET_SCRIPT_INT(i));
	int r = 0;
	r = SendMessageW(hwnd_winamp, WM_WA_IPC, GET_SCRIPT_INT(i), IPC_GETRATING);
	wa2.PE_setCurrentIndex(cur_pos);

	return MAKE_SCRIPT_INT(r);
}

scriptVar SPlaylist::script_vcpu_enqueueFile(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar file)
{
	SCRIPT_FUNCTION_INIT; 
	
	wa2.enqueueFile (GET_SCRIPT_STRING(file));

	RETURN_SCRIPT_VOID;  
}

scriptVar SPlaylist::script_vcpu_clear(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT; 
	
	wa2.clearPlaylist();

	RETURN_SCRIPT_VOID;  
}

scriptVar SPlaylist::script_vcpu_removeTrack(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i)
{
	SCRIPT_FUNCTION_INIT; 

	HWND hPeWindow = wa2.getWnd(IPC_GETWND_PE);	
	SendMessageW(hPeWindow, WM_USER, IPC_PE_DELETEINDEX, GET_SCRIPT_INT(i));

	RETURN_SCRIPT_VOID;  
}

scriptVar SPlaylist::script_vcpu_swapTrack(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track1, scriptVar _track2)
{
	SCRIPT_FUNCTION_INIT; 

	int track1 = GET_SCRIPT_INT(_track1);
	int track2 = GET_SCRIPT_INT(_track2);

	if (track1 >= wa2.PE_getNumItems() || track1 < 0 || track2 >= wa2.PE_getNumItems() || track2 < 0) RETURN_SCRIPT_VOID;
	swap ((track1), (track2));
	
	RETURN_SCRIPT_VOID;  
}

scriptVar SPlaylist::script_vcpu_moveUp(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track)
{
	SCRIPT_FUNCTION_INIT; 

	int track = GET_SCRIPT_INT(_track);
	if (track < wa2.PE_getNumItems()-1) swap (track, track+1);
	
	RETURN_SCRIPT_VOID;  
}

scriptVar SPlaylist::script_vcpu_moveDown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track)
{
	SCRIPT_FUNCTION_INIT; 

	int track = GET_SCRIPT_INT(_track);
	if (track > 0) swap (track, track-1);
	
	RETURN_SCRIPT_VOID;  
}

scriptVar SPlaylist::script_vcpu_moveTo(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track, scriptVar _pos)
{
	SCRIPT_FUNCTION_INIT; 

	int track = GET_SCRIPT_INT(_track);
	int pos = GET_SCRIPT_INT(_pos);

	if (track == pos || pos >= wa2.PE_getNumItems() || pos < 0 || track >= wa2.PE_getNumItems() || track < 0)
	{
		RETURN_SCRIPT_VOID;
	}

	// Martin> This can be done much faster :P
	/*if (track > pos)
	{
		for (int i = track; i > pos; i--)
		{
			swap (i, i-1);
		}
	}
	else
	{
		for (int i = track; i < pos; i++)
		{
			swap (i, i+1);
		}
	}*/
	HWND hPeWindow = wa2.getWnd(IPC_GETWND_PE);	
	fileinfoW *fi = getFileInfoStructW1(track);
	SendMessageW(hPeWindow, WM_USER, IPC_PE_DELETEINDEX, fi->index);
	fi->index = pos;
	static COPYDATASTRUCT cds;
	cds.dwData = IPC_PE_INSERTFILENAMEW;
	cds.lpData = fi;
	SendMessageW(hPeWindow, WM_COPYDATA, NULL, (LPARAM)&cds);
	
	RETURN_SCRIPT_VOID;  
}

scriptVar SPlaylist::script_vcpu_getTitle(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track)
{
	SCRIPT_FUNCTION_INIT; 

	int track = GET_SCRIPT_INT(_track);

	// Martin> There might be a better way to convert this;)
	// benski> yes, there is :) see below.  There's a unicode equivalent API for this, just wasn't enabled in wa2frontend object
	//         so I added it
	fileinfo2W *fi = wa2.PE_getFileTitleW(track);
	if (fi)
		return MAKE_SCRIPT_STRING(fi->filetitle);
	else
		return MAKE_SCRIPT_STRING(L"");
}

scriptVar SPlaylist::script_vcpu_getLength(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track)
{
	SCRIPT_FUNCTION_INIT; 

	int track = GET_SCRIPT_INT(_track);

	fileinfo2W *fi = wa2.PE_getFileTitleW(track);
	if (fi)
		return MAKE_SCRIPT_STRING((fi->filelength));
	else
		return MAKE_SCRIPT_STRING(L"");
}

scriptVar SPlaylist::script_vcpu_getFileName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track)
{
	SCRIPT_FUNCTION_INIT; 

	int track = GET_SCRIPT_INT(_track);

	fileinfoW *fi = getFileInfoStructW1(track);

	return MAKE_SCRIPT_STRING(fi->file);
}


scriptVar SPlaylist::script_vcpu_getExtendedInfo(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track, scriptVar _name)
{
	SCRIPT_FUNCTION_INIT; 

	int track = GET_SCRIPT_INT(_track);
	const wchar_t *name = GET_SCRIPT_STRING(_name);

	fileinfoW *fi = getFileInfoStructW1(track);

	wa2.getMetaData((fi->file), name, staticStr, 4096);

	return MAKE_SCRIPT_STRING(staticStr);
}

scriptVar SPlaylist::script_vcpu_getNumSelectedItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT; 
	
	HWND hPeWindow = wa2.getWnd(IPC_GETWND_PE);	
	int ret = SendMessageW(hPeWindow, WM_USER, IPC_PE_GETSELECTEDCOUNT, 0);

	return MAKE_SCRIPT_INT(ret);
}

scriptVar SPlaylist::script_vcpu_getNextSelectedItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track)
{
	SCRIPT_FUNCTION_INIT; 

	int track = GET_SCRIPT_INT(_track);
	
	track = SendMessageW(wa2.getMainWindow(), WM_USER, track, IPC_PLAYLIST_GET_NEXT_SELECTED);

	return MAKE_SCRIPT_INT(track);
}

scriptVar SPlaylist::script_vcpu_playTrack(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track)
{
	SCRIPT_FUNCTION_INIT; 
#ifdef WASABI_COMPILE_MEDIACORE
	int track = GET_SCRIPT_INT(_track);
	WASABI_API_MEDIACORE->core_userButton(0, UserButton::STOP);
	wa2.PE_setCurrentIndex(track);
	WASABI_API_MEDIACORE->core_userButton(0, UserButton::PLAY);
	return MAKE_SCRIPT_VOID();
#else
	RETURN_SCRIPT_VOID;
#endif
}

scriptVar SPlaylist::script_vcpu_onPleditModified(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, playlistController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

wchar_t SPlaylist::staticStr[4096] = {0};
PtrList < SPlaylist > SPlaylist::SOList;