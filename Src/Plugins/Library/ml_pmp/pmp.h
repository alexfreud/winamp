#ifndef __PMP_H_
#define __PMP_H_
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // needed for HDC and stuff
#include <stddef.h>
#include "..\..\General\gen_ml/ml.h" // for itemRecordW
// make sure you include ml.h before you include this, wherever you include it.

#ifdef __cplusplus
class api_service;
#endif

typedef intptr_t songid_t;
typedef intptr_t pmpart_t;

// What metadata the device supports
#define SUPPORTS_ARTIST			0x00000001
#define SUPPORTS_ALBUM			0x00000002
#define SUPPORTS_TITLE			0x00000004
#define SUPPORTS_TRACKNUM		0x00000008
#define SUPPORTS_DISCNUM		0x00000010
#define SUPPORTS_GENRE			0x00000020
#define SUPPORTS_YEAR			0x00000040
#define SUPPORTS_SIZE			0x00000080
#define SUPPORTS_LENGTH			0x00000100
#define SUPPORTS_BITRATE		0x00000200
#define SUPPORTS_PLAYCOUNT		0x00000400
#define SUPPORTS_RATING			0x00000800
#define SUPPORTS_LASTPLAYED		0x00001000
#define SUPPORTS_LASTUPDATED	0x00002000
#define SUPPORTS_ALBUMARTIST	0x00004000
#define SUPPORTS_COMPOSER		0x00008000
#define SUPPORTS_PUBLISHER		0x00010000
#define SUPPORTS_ALBUMART		0x00020000
#define SUPPORTS_MIMETYPE		0x00040000
#define SUPPORTS_DATEADDED		0x00080000

// constants for sorting playlists
#define SORTBY_ARTIST		0
#define SORTBY_ALBUM		1
#define SORTBY_TITLE		2
#define SORTBY_TRACKNUM		3
#define SORTBY_DISCNUM		4
#define SORTBY_GENRE		5
#define SORTBY_RATING		6
#define SORTBY_PLAYCOUNT	7
#define SORTBY_LASTPLAYED	8
#define SORTBY_DATEADDED	9

// for get/setTrackExtraInfo, FIELD_* will be passed in as the "field" parameter. check using "if(!wcscmp(field,FIELD_*))" or similar
#define FIELD_EXTENSION L"ext" // return the file extention, eg L"mp3". Only needed for indirect playback, will never be set.

// firstly a little clarification between what a playlistnumber is and a songid is.
// playlist numbers are always 0,1,2,...,getPlaylistCount() (thus, if playlist 1 is deleted,
// the number of all playlists except 0 change)
// songids are unique identifiers which persist even when other songs are removed. 
// Feel free to use a pointer OR an integer as a songid.

// NOTE: wherever stated, len means number of characters NOT bytes available in the buffer

/* benski> All calls will be made on the 'main thread', with the following exceptions
transferTrackToDevice() on the transfer thread
trackRemovedFromTransferQueue() on the transfer thread
copyToHardDrive() on the transfer thread
*/
class Device {
	protected:
		Device() {}
		~Device() {}
	public:
		virtual __int64 getDeviceCapacityAvailable()=0; // in bytes
		virtual __int64 getDeviceCapacityTotal()=0; // in bytes

		virtual void Eject()=0; // if you ejected successfully, you MUST call PMP_IPC_DEVICEDISCONNECTED and delete this;
		virtual void Close()=0; // save any changes, and call PMP_IPC_DEVICEDISCONNECTED AND delete this;

		// return 0 for success, -1 for failed or cancelled
		virtual int transferTrackToDevice(const itemRecordW * track, // the track to transfer
		void * callbackContext, //pass this to the callback
		void (*callback)(void *callbackContext, wchar_t *status),  // call this every so often so the GUI can be updated. Including when finished!
						 songid_t * songid, // fill in the songid when you are finished
						 int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
						)=0;
		virtual int trackAddedToTransferQueue(const itemRecordW *track)=0; // return 0 to accept, -1 for "not enough space", -2 for "incorrect format"
		virtual void trackRemovedFromTransferQueue(const itemRecordW *track)=0; 

		// return the amount of space that will be taken up on the device by the track (once it has been tranferred)
		// or 0 for incompatable. This is usually the filesize, unless you are transcoding. An estimate is acceptable.
		virtual __int64 getTrackSizeOnDevice(const itemRecordW *track)=0; 

		virtual void deleteTrack(songid_t songid)=0; // physically remove from device. Be sure to remove it from all the playlists!

		virtual void commitChanges(){} // optional. Will be called at a good time to save changes

		virtual int getPlaylistCount()=0; // always at least 1. playlistnumber 0 is the Master Playlist containing all tracks.
		// PlaylistName(0) should return the name of the device.
		virtual void getPlaylistName(int playlistnumber, wchar_t *buf, int len)=0;
		virtual int getPlaylistLength(int playlistnumber)=0;
		virtual songid_t getPlaylistTrack(int playlistnumber,int songnum)=0; // returns a songid

		virtual void setPlaylistName(int playlistnumber, const wchar_t *buf)=0; // with playlistnumber==0, set the name of the device.
		virtual void playlistSwapItems(int playlistnumber, int posA, int posB)=0; // swap the songs at position posA and posB
		virtual void sortPlaylist(int playlistnumber, int sortBy)=0;
		virtual void addTrackToPlaylist(int playlistnumber, songid_t songid)=0; // adds songid to the end of the playlist
		virtual void removeTrackFromPlaylist(int playlistnumber, int songnum)=0; //where songnum is the position of the track in the playlist

		virtual void deletePlaylist(int playlistnumber)=0;
		virtual int newPlaylist(const wchar_t *name)=0; // create empty playlist, returns playlistnumber. -1 for failed.

		virtual void getTrackArtist(songid_t songid, wchar_t *buf, int len)=0;
		virtual void getTrackAlbum(songid_t songid, wchar_t *buf, int len)=0;
		virtual void getTrackTitle(songid_t songid, wchar_t *buf, int len)=0;
		virtual int getTrackTrackNum(songid_t songid)=0;
		virtual int getTrackDiscNum(songid_t songid)=0;
		virtual void getTrackGenre(songid_t songid, wchar_t * buf, int len)=0;
		virtual int getTrackYear(songid_t songid)=0;
		virtual __int64 getTrackSize(songid_t songid)=0; // in bytes
		virtual int getTrackLength(songid_t songid)=0; // in millisecs
		virtual int getTrackBitrate(songid_t songid)=0; // in kbps
		virtual int getTrackPlayCount(songid_t songid)=0;
		virtual int getTrackRating(songid_t songid)=0; //0-5
		virtual __time64_t getTrackLastPlayed(songid_t songid)=0; // in unix time format
		virtual __time64_t getTrackLastUpdated(songid_t songid)=0; // in unix time format
		virtual void getTrackAlbumArtist(songid_t songid, wchar_t *buf, int len){};
		virtual void getTrackPublisher(songid_t songid, wchar_t *buf, int len){};
		virtual void getTrackComposer(songid_t songid, wchar_t *buf, int len){};
		virtual void getTrackMimeType(songid_t songid, wchar_t *buf, int len){};
		virtual __time64_t getTrackDateAdded(songid_t songid){ return -1; }; // in unix time format
		virtual int getTrackType(songid_t songid) { return 0; }
		virtual void getTrackExtraInfo(songid_t songid, const wchar_t *field, wchar_t *buf, int len) {}; //optional

		// feel free to ignore any you don't support
		virtual void setTrackArtist(songid_t songid, const wchar_t *value)=0;
		virtual void setTrackAlbum(songid_t songid, const wchar_t *value)=0;
		virtual void setTrackTitle(songid_t songid, const wchar_t *value)=0;
		virtual void setTrackTrackNum(songid_t songid, int value)=0;
		virtual void setTrackDiscNum(songid_t songid, int value)=0;
		virtual void setTrackGenre(songid_t songid, const wchar_t *value)=0;
		virtual void setTrackYear(songid_t songid, int year)=0;
		virtual void setTrackPlayCount(songid_t songid, int value)=0;
		virtual void setTrackRating(songid_t songid, int value)=0;
		virtual void setTrackLastPlayed(songid_t songid, __time64_t value)=0; // in unix time format
		virtual void setTrackLastUpdated(songid_t songid, __time64_t value)=0; // in unix time format
		virtual void setTrackAlbumArtist(songid_t songid, const wchar_t *value){};
		virtual void setTrackPublisher(songid_t songid, const wchar_t *value){};
		virtual void setTrackComposer(songid_t songid, const wchar_t *value){};
		virtual void setTrackExtraInfo(songid_t songid, const wchar_t *field, const wchar_t *value) {}; //optional

		virtual bool playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue)=0; // return false if unsupported

		virtual intptr_t extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4){return 0;}

		virtual bool copyToHardDriveSupported() {return false;}

		virtual __int64 songSizeOnHardDrive(songid_t song) {return -1;} // how big a song will be when copied back. Return -1 for not supported.

		virtual int copyToHardDrive(songid_t song, // the song to copy
		wchar_t * path, // path to copy to, in the form "c:\directory\song". The directory will already be created, you must append ".mp3" or whatever to this string! (there is space for at least 10 new characters).
		void * callbackContext, //pass this to the callback
		void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
						 int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
						) {return -1;} // -1 for failed/not supported. 0 for success.

		// art functions
		virtual void setArt(songid_t songid, void *buf, int w, int h){} //buf is in format ARGB32*
		virtual pmpart_t getArt(songid_t songid){return NULL;}
		virtual void releaseArt(pmpart_t art){}
		virtual int drawArt(pmpart_t art, HDC dc, int x, int y, int w, int h) {return 0;}
		virtual void getArtNaturalSize(pmpart_t art, int *w, int *h){*w=*h=0;}
		virtual void setArtNaturalSize(pmpart_t art, int w, int h){}
		virtual void getArtData(pmpart_t art, void* data){} // data ARGB32* is at natural size
		virtual bool artIsEqual(pmpart_t a, pmpart_t b){return false;}
};

#define PMPHDR_VER 0x10
/* 
0x10 is for Winamp 5.66+
- it adds passing a api_service *service

0x9 is for Winamp 5.63+
- it now requires that plugins handle addTrackToPlaylist(0, ...)
  so plugins that don't want to directly add to their database via transferTrackToDevice() [which happens off-thread]
  can do it during addTrackToPlaylist(0, ...) [which happens on the main thread]
- changes description from char* to wchar_t* (as there's no 3rd party pmp_* we can make such a breaking change)
*/

// The MessageProc could recieve any of the following..
#define PMP_DEVICECHANGE	0x100 // param1=WPARAM, param2=LPARAM. See http://msdn.microsoft.com/library/en-us/devio/base/wm_devicechange.asp
#define PMP_CONFIG			0x101 // param1 will be the parent HWND. return 1 if you implement this
#define PMP_NO_CONFIG		0x102 // return TRUE to allow the plug-in config button to be disabled as applicable

// use SendMessage(hwndPortablesParent,WM_PMP_IPC,param,PMP_IPC_*); on any of the following
#define WM_PMP_IPC WM_USER+10
#define PMP_IPC_DEVICECONNECTED		0x100	// pass a Device *
#define PMP_IPC_DEVICEDISCONNECTED	0x101	// pass a Device *

#define PMP_IPC_DEVICELOADING		0x102	// pass a pmpDeviceLoading *.
											// This is optional, call PMP_IPC_DEVICECONNECTED when loading is finished
											// or PMP_IPC_DEVICEDISCONNECTED to cancel.
											// while a device is being loaded, DEVICE_SET_ICON will be called, nothing else.

#define PMP_IPC_DEVICENAMECHANGED	0x103	// pass a Device *
											// added 5.64+

#define PMP_IPC_DEVICECLOUDTRANSFER	0x104	// pass a cloudDeviceTransfer *
											// added 5.64+

#define PMP_IPC_GETCLOUDTRANSFERS	0x105	// pass a Cloudfiles * (defined as typedef nu::PtrList<wchar_t>)
											// added 5.64+

typedef struct {
	wchar_t filenames[MAX_PATH + 1];	// list of filename(s) to transfer to the cloud device
	void * device_token;				// token identifier of the cloud device to transfer to
} cloudDeviceTransfer;	// added 5.64+

typedef struct {
	Device * dev;
	// filled in by ml_pmp
	void (*UpdateCaption)(wchar_t * caption, void * context); // call this with the context to update the caption
	void * context;
} pmpDeviceLoading;

typedef struct {
	HWND parent;
	const char * dev_name;
} pmpDevicePrefsView;

#define PMP_IPC_GET_TRANSCODER 0x200		// returns a Transcoder*, pass your Device* (you must have previously called PMP_IPC_DEVICECONNECTED)
#define PMP_IPC_RELEASE_TRANSCODER 0x201	// pass your Transcoder*, no return value
#define PMP_IPC_ENUM_ACTIVE_DRIVES 0x300	// pass your function in wParam, or if you pass wParam = 0, it will return a function pointer (type ENUMDRIVES) you can call directly 
typedef void (*ENUM_DRIVES_CALLBACK)(wchar_t, UINT);
typedef void (*ENUMDRIVES)(ENUM_DRIVES_CALLBACK callback);
#define PMP_IPC_GET_INI_FILE 0x301			// pass a Device*, returns a wchar_t*
#define PMP_IPC_GET_PREFS_VIEW 0x302		// pass a pmpDevicePrefsView*, returns a HWND

// The following may be recieved by Device::extraActions (as param1)
#define DEVICE_SET_ICON 0x0				// param2 is of type MLTREEIMAGE *, modify it to use your own icon for the device in the ML tree
#define DEVICE_SUPPORTED_METADATA 0x1	// return a load of SUPPORTS_* ORed together. If you return 0, all is assumed.
#define DEVICE_DOES_NOT_SUPPORT_EDITING_METADATA 0x3	// return 1 if metadata cannot be edited, 0 otherwise.
#define DEVICE_CAN_RENAME_DEVICE 0x4	// return 1 if the device can be renamed. setPlaylistName(0,name) will be used to rename the device
#define DEVICE_GET_INI_FILE 0x5			// param2 is wchar_t * of length MAX_PATH. Fill it with the location of the inifile that should be used.
#define DEVICE_GET_PREFS_DIALOG 0x6		// param2 is a pref_tab*, fill it in if you want to put your own config in. On WM_INITDIALOG lParam will be a prefsParam *
#define DEVICE_REFRESH 0x7				// F5 was pressed. return 1 to do an in-place update
#define DEVICE_ADDPODCASTGROUP 0x8		// for ipod. param2=int playlistid, param3=int position, param4=wchar_t* channelname
#define DEVICE_ADDPODCASTGROUP_FINISH 0x9	// for ipod. param2=int playlistid
#define DEVICE_SUPPORTS_VIDEO 0xA		// return 1 if you support video
#define DEVICE_DONE_SETTING 0xB			// param2=songid_t, tells your plugin that a series of setTrack*() functions are done being called for a particular track
#define DEVICE_VETO_ENCODER 0xC			// param2==fourcc.  return 1 to remove the encoder from the transcoding preferences
#define DEVICE_GET_ICON 0xD				// param2=width, param3=height, param4 = wchar_t[260] to put your path into (possibly res:// protocol)
#define DEVICE_SUPPORTS_PODCASTS 0xE	// return 0 if you support podcasts.  return 1 if you don't want them transferred
#define DEVICE_GET_CONNECTION_TYPE 0xF	// return 1 if you support.  param2 is a const char ** that you should set to a static connect type string.
#define DEVICE_GET_UNIQUE_ID 0x10		// return 1 if you support.  copy a unique name into param2 (char *), param3 will be the allocated size (# of characters)
#define DEVICE_GET_MODEL 0x11			// return 1 if you support, param2 = (wchar_t*)buffer - model name; param3 = (unsigned int)bufferSize - buffer size; param4 - not used.
#define DEVICE_SENDTO_UNSUPPORTED 0x12	// return 1 if you don't support send-to
#define DEVICE_GET_DISPLAY_TYPE 0x13	// return 1 if you support, param2 = (wchar_t*)buffer - model name; param3 = (unsigned int)bufferSize - buffer size; param4 - not used.
#define DEVICE_VETO_TRANSCODING 0x14	// return 1 if you don't support transcoding and don't want to show the 'Transcoding' preference tab (also see DEVICE_VETO_ENCODER)
#define DEVICE_GET_PREFS_PARENT 0x15	// return prefsDlgRecW * of the parent preference page you want the device preference page to be a child off. return 0 for default placing
#define DEVICE_GET_CLOUD_SOURCES_MENU 0x16	// return HMENU if you support cloud source menus and have one to provide (only called as needed). param2=(int*)num_cloud_devices. param4=(int)songid i.e. clicked item or -1 for selection
#define DEVICE_DO_CLOUD_SOURCES_MENU 0x17	// a cloud sources menu item was clicked. param2=(int)menu_id from the menu provided via DEVICE_GET_CLOUD_SOURCES_MENU.
											// param3=(int)mode where 0 is via submenu and 1 is the menu only i.e. single-selection.
											// param4=(CItemList*) of songid_t so it is possible to do multiple selection handling
											// return 1 if needing the item to be removed from the view
#define DEVICE_IS_CLOUD_TX_DEVICE 0x18	// param1=(nx_string_t) return 1 if device token matches ours
#define DEVICE_SYNC_UNSUPPORTED 0x19	// return 1 if you don't support sync
#define DEVICE_GET_CLOUD_DEVICE_ID 0x20	// return the device id
#define DEVICE_NOT_READY_TO_VIEW 0x21	// return 1 if view is not ready to be shown
#define DEVICE_GET_NODE_ICON_ID 0x22	// return the resource id of the node icon
#define DEVICE_PLAYLISTS_UNSUPPORTED 0x23	// return 1 if you don't support playlists
#define DEVICE_DOES_NOT_SUPPORT_REMOVE 0x24	// return 1 if you don't support remove / eject

#define CLOUD_SOURCE_MENUS 60000
#define CLOUD_SOURCE_MENUS_UPPER 60000 + 20
#define CLOUD_SOURCE_MENUS_PL_UPPER 60000 + 200

typedef struct {
	wchar_t title[98];
	int res_id;
	DLGPROC dlg_proc;
	HINSTANCE hinst;
} pref_tab;

typedef struct {
	HWND parent;
	Device * dev;
	void (*config_tab_init)(HWND tab,HWND m_hwndDlg); // call this on WM_INITDIALOG
} prefsParam;

typedef struct {
	int version; // should be PMPHDR_VER
	wchar_t *description; // a textual desciption (including version info)
	int ( __cdecl *init)(); // called when winamp is loaded, for any one-time init
	void ( __cdecl *quit)(); // called when winamp is unloaded, for any one-time deinit

	INT_PTR ( __cdecl *MessageProc)(int msg, INT_PTR param1, INT_PTR param2, INT_PTR param3);

	// All the following data is filled in by ml_pmp
	HWND hwndWinampParent;  // send this any of the WM_WA_IPC messages
	HWND hwndLibraryParent; // send this any of the WM_ML_IPC messages
	HWND hwndPortablesParent; // send this any of the WM_PMP_IPC messages
	HINSTANCE hDllInstance; // this plugins instance

	// filled in by Winamp (added 5.66+ to replace need to call IPC_GET_API_SERVICE on loading)
	#ifdef __cplusplus
	api_service *service;
	#else
	void * service;
	#endif
} PMPDevicePlugin;

// return values from the init(..) which determines if Winamp will continue loading
// and handling the plugin or if it will disregard the load attempt. If PMP_INIT_FAILURE
// is returned then the plugin will be listed as [NOT LOADED] on the plug-in prefs page.
#define PMP_INIT_SUCCESS 0
#define PMP_INIT_FAILURE 1

// return values from the winampUninstallPlugin(HINSTANCE hdll, HWND parent, int param)
// which determine if we can uninstall the plugin immediately or on winamp restart
#define PMP_PLUGIN_UNINSTALL_NOW	0x0
#define PMP_PLUGIN_UNINSTALL_REBOOT 0x1

#endif