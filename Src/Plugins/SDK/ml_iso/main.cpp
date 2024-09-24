#include "main.h"
#include "../Winamp/wa_ipc.h"
#include "api__ml_iso.h"
#include <api/service/waservicefactory.h>


/* wasabi services we'll be using */
api_service *WASABI_API_SVC = 0;
api_application *WASABI_API_APP = 0;
api_playlistmanager *AGAVE_API_PLAYLISTMANAGER = 0;

/* gen_ml calls this function when it loads your plugin.  return non-zero to abort loading your plugin */
int Init()
{
	// this plugin requires an interface only present on 5.54 and up, so we'll just refuse to load on older versions
	if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETVERSION) < 0x5054) 
		return 1;

	// go ahead and grab the wasabi service manager.  we'll need it later when we get an ISO Creator object
	WASABI_API_SVC = (api_service *)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);

	// get the application API
	// we need this to get/set Winamp's current working directory
	waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (factory)
		WASABI_API_APP = (api_application *)factory->getInterface();

	// get the playlist manager API
	// we'll need this for loading playlists
	factory = WASABI_API_SVC->service_getServiceByGuid(api_playlistmanagerGUID);
	if (factory)
		AGAVE_API_PLAYLISTMANAGER = (api_playlistmanager *)factory->getInterface();

	// this media library plugin doesn't add a node to the treeview, so we don't really do anything in here besides
	// grabbing the service manager
	// all of the action will come via Send-To which is handled in MessageProc

	return 0;
}

void Quit()
{

}

INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	switch(message_type)
	{
		// this gets sent when Winamp wants to build the send-to menu.  If we want to be in the send-to
		// we make some API calls during this function
	case ML_MSG_ONSENDTOBUILD:
		{
			INT_PTR source_type = param1; // param 1 is the source type
			INT_PTR context = param2; // param 2 is just some context value that we have to use when we call back into the API

			// we only accept certain types of sources, so we'll explicitly check
			// if we were to handle ALL types, checking against the known types
			// is good practice in case new Winamp versions add additional source types
			switch(source_type)
			{
			case ML_TYPE_ITEMRECORDLIST: // Item Record List.  Used by the local media library
			case ML_TYPE_FILENAMES: // raw list of filenames
			case ML_TYPE_PLAYLIST: // a playlist.  we'll use the playlist loading API to crack it open
			case ML_TYPE_PLAYLISTS: // a list of playlists.  we'll use the playlist loading API to crack each one open
			case ML_TYPE_ITEMRECORDLISTW: // unicode version of an Item Record List
			case ML_TYPE_FILENAMESW: // raw list of unicode filenames
				{
					// add ourselves to the send-to menu!
					mlAddToSendToStructW s;
					s.context = context;  // pass in the context value passed to this function.
					s.desc = L"Create new ISO image";
					s.user32 = (INT_PTR)MessageProc; // this value has to be some unique value that you can identify later
																					 // by convention, use the pointer to this function, since it's obviously unique

					SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&s, ML_IPC_ADDTOSENDTOW); 
				}
				// returning 0 tells the media library to continue building the send-to menu
				// it doesn't mean we added or didn't add items to the send-to menu
				return 0;
			case ML_TYPE_STREAMNAMES: // doesn't make sense to burn a stream to an ISO file so we won't even popup on the send-to menu when it's streams
			case ML_TYPE_CDTRACKS: // we'll avoid CD tracks.  in theory we could use the ripping API but let's not complicate this example
			case ML_TYPE_QUERYSTRING: // media library query. not sure that this is even used anywhere. either way we're not going to handle it
			case ML_TYPE_STREAMNAMESW: // don't cross the streams
			case ML_TYPE_TREEITEM: // not even sure this is used
				// break out of here because it's not supported. returning 0 just tells the send-to menu to continue building, 
				// it doesn't mean we added or didn't add items to the send-to menu
				return 0; 
			}
			// shouldn't get here 
			return 0;
		}

// this gets sent when a send-to menu item got selected
		// it might be ours. it might not.
	case ML_MSG_ONSENDTOSELECT:
		{
			// let's see if it's ours.  We check 'user32' against the function pointer for this function
			INT_PTR unique = param3;
			if (unique != (INT_PTR)MessageProc) // not ours? let's bail
				return 0; // remember to always return 0 or else other media library plugins won't get the notification

			INT_PTR type = param1; // what type of data got sent
			INT_PTR data = param2; // pointer to the data.  depends on the type

			switch(type)
			{
			case ML_TYPE_ITEMRECORDLIST: // Item Record List.  Used by the local media library
				ConvertItemRecordListToISO((const itemRecordList *)data);
				return 1; // return 1 to say we handled it
			case ML_TYPE_FILENAMES: // raw list of filenames
				ConvertFilenamesToISO((const char *)data);
				return 1; // return 1 to say we handled it
			case ML_TYPE_PLAYLIST: // a playlist.  we'll use the playlist loading API to crack it open
				ConvertPlaylistToISO((const mlPlaylist *)data);
				return 1; // return 1 to say we handled it
			case ML_TYPE_PLAYLISTS: // a list of playlists.  we'll use the playlist loading API to crack each one open
				ConvertPlaylistsToISO((const mlPlaylist **)data);
				return 1; // return 1 to say we handled it
			case ML_TYPE_ITEMRECORDLISTW: // unicode version of an Item Record List
				ConvertUnicodeItemRecordListToISO((const itemRecordListW *)data);
				return 1; // return 1 to say we handled it
			case ML_TYPE_FILENAMESW: // raw list of unicode filenames
				ConvertUnicodeFilenamesToISO((const wchar_t *)data);
				return 1; // return 1 to say we handled it

			default: // something we didn't support
				return 0; 
			}
		}
	}
	return 0;
}

winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
		"ISO Creator",
		Init,
		Quit,
		MessageProc,
		0,
		0,
		0,
};

extern "C" 	__declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
	{
		return &plugin;
	}
