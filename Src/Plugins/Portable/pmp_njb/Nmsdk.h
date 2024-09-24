/////////////////////////////////////////////////////////////////////////////////////
// 
// File:	nmsdk.h
//
// Purpose:	Combination of the Jukebox SDK2 header file and the NomadII SDK2 header file.
// 
// Notes:	Please make sure to include the COM Stuffs <Objbase.h> and <initguid.h> 
//			at the beginning of the application programs. Otherwise, it will get Link Errors.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Creative Technology Ltd., 2001.  All rights reserved.
// 
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// nmsdk.h : main header file for export
// Version: 1.0.7.0
/////////////////////////////////////////////////////////////////////////////

#ifndef __nmsdk_h__
#define __nmsdk_h__

//
// COM Interface Declaration
// 

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


//////////////////////////////////////////////////////////////////////////
//								Class ID							//
//////////////////////////////////////////////////////////////////////////
//CLSID CLSID_CTJukeBox2:
// {BD1A6357-3E9B-4f1b-8375-AEE989ED6C5E}
DEFINE_GUID(CLSID_CTJukeBox2, 
0xbd1a6357, 0x3e9b, 0x4f1b, 0x83, 0x75, 0xae, 0xe9, 0x89, 0xed, 0x6c, 0x5e);

//CLSID CLSID_CTNOMAD2:
// {0EBE3156-FD3A-4f5c-ABDB-71E3BEEAD091}
DEFINE_GUID(CLSID_CTNOMAD2, 
0xebe3156, 0xfd3a, 0x4f5c, 0xab, 0xdb, 0x71, 0xe3, 0xbe, 0xea, 0xd0, 0x91);


//////////////////////////////////////////////////////////////////////////
//								return codes							//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////
//			JukeBox & NomadII shared stuffs			//
//////////////////////////////////////////////////////
// Additional Error codes
#define DAPSDK_SUCCESS							0x00
// General Error
#define DAPSDK_FAILED							0x01
#define DAPSDK_E_DEVICE_BUSY					0x02
#define DAPSDK_E_STORAGE_FULL					0x03
#define DAPSDK_E_SETTIME_REJECTED				0x05
#define DAPSDK_E_ITEM_SIZE_MISSING				0x14	//NMJB_E_TRACK_SIZE_MISSING
#define DAPSDK_E_ITEM_UPLOAD_DENIED				0x18	//NMJB_E_TRACK_UPLOAD_DENIED
#define DAPSDK_E_PLAYER_NOT_CONNECTED			0x80
#define DAPSDK_E_CANCELLED						0x81
#define DAPSDK_E_PORT_UNAVAILABLE				0x82
#define DAPSDK_E_OUT_OF_MEMORY					0x83
#define DAPSDK_E_FILEOPEN_ERR					0x84
#define DAPSDK_E_ITEM_NOT_FOUND					0x85
#define DAPSDK_E_LOAD_COMPONENTS_FAILED			0x86
#define DAPSDK_E_ID_INVALID						0x87
#define DAPSDK_E_FILETYPE_ILLEGAL				0x88
#define DAPSDK_E_LOADRES_FAIL					0x89
#define DAPSDK_E_FORMAT_NOT_FOUND				0x8a
#define DAPSDK_E_FILE_ALREADY_EXISTS			0x8b
#define DAPSDK_E_LIB_CORRUPTED					0x8c
#define DAPSDK_E_LIB_BUSY						0x8d
#define DAPSDK_E_FILE_READ_WRITE_FAILED			0x8e	//NMJB_E_FILE_WRITE_FAILED
#define DAPSDK_E_INVALID_FILEPATH				0x8f
#define DAPSDK_E_UNSUPPORTED					0x91
#define DAPSDK_E_NORIGHTS						0x95
#define DAPSDK_E_UNDEFINED_ERR					0xff


//////////////////////////////////////////////////////
//					NomadII stuffs					//
//////////////////////////////////////////////////////
#define DAPSDK_E_SMARTMEDIA_WRITE_PROTECTED		0x98
#define DAPSDK_E_NO_STORAGE						0x99	//No internal media or smart media

//////////////////////////////////////////////////////
//					JukeBox stuffs					//
//////////////////////////////////////////////////////
// General Error
#define DAPSDK_E_HD_GENERAL_ERROR				0x04
// Track Management Error
#define DAPSDK_E_TRACK_NOT_FOUND				0x10
#define DAPSDK_E_TRACK_ALREADY_EXIST			0x11
#define DAPSDK_E_TRACK_TITLE_MISSING			0x12
#define DAPSDK_E_TRACK_CODEC_MISSING			0x13
#define DAPSDK_E_TRACK_IO_OPERATION_ABORTED		0x15
#define DAPSDK_E_TRACK_READ_WRITE_ERROR			0x16
#define DAPSDK_E_TRACK_NOT_OPENED				0x17
// Playlist Error
#define DAPSDK_E_PL_NOT_FOUND					0x20
#define DAPSDK_E_PL_ALREADY_EXIST				0x21
#define DAPSDK_E_PL_ITEM_NOT_FOUND				0x22
#define DAPSDK_E_PL_ITEM_ALREADY_EXIST			0x23
//Additional
#define DAPSDK_E_DISKFULL_FOR_DOWNLOAD			0x90


#define DAPSDK_E_STATUS_TIMEOUT					0x06

// Transport Control Error
#define DAPSDK_E_END_OF_TRACK					0x30
#define DAPSDK_E_END_OF_LIST					0x31
#define DAPSDK_E_CODEC_NOT_SUPPORTED			0x32
#define DAPSDK_E_DATA_CORRUPTED					0x33
#define DAPSDK_E_SAMPLING_RATE_NOT_SUPPORTED	0x34
#define DAPSDK_E_DECODING_ERROR					0x35
#define DAPSDK_E_POSITION_OUTOF_RANGE			0x36
#define DAPSDK_E_NOT_STOPPED					0x37

// Audio Control Error
#define DAPSDK_E_UNKNOW_PROPERTY				0x40
#define DAPSDK_E_VALUE_OUTOF_RANGE				0x41

// USB Transaction Error
#define DAPSDK_E_DATA_FILE_NOT_FOUND			0x60
#define DAPSDK_E_DATA_FILE_TOO_BIG				0x61
#define DAPSDK_E_DATA_FILE_ALREADY_EXIST		0x62
#define DAPSDK_E_TOO_MANY_DATA_FILES			0x63

//additional
#define DAPSDK_E_WMDM_INIT_FAILED				0x92
#define DAPSDK_E_INVALID_ARGUMENT				0x93
#define DAPSDK_E_PARENTNODE_NOT_EXIST			0x94
#define DAPSDK_E_NORIGHTS						0x95
#define DAPSDK_E_PATH_EXCESS_LEN				0x96
#define DAPSDK_E_LOAD_PROC_FAILED				0x97

// New Error code for MultiApplication assess of Nomad Jukebox 2/3/Zen
#define	DAPSDK_E_PMSMAN_CREATEDIRECTORY_FAILED	0x0100
#define	DAPSDK_E_DEVICE_WRITE_FAILED			0x0200
#define	DAPSDK_E_DEVICE_READ_FAILED				0x0300
#define	DAPSDK_E_DB_INVALID_REQUEST_ID			0x0400
#define	DAPSDK_E_DB_INVALID_NODE_ID				0x0500
#define	DAPSDK_E_DWNTHRD_CREATEMETADATA_FAILED	0x0600
#define	DAPSDK_E_DEVINFO_INVALID_INDEX			0x0700
#define	DAPSDK_E_INVALID_DEVICESETTINGTYPE		0x0800
#define	DAPSDK_E_FILESIZE_TOO_BIG				0x0900
#define	DAPSDK_E_AUDIOFILE_FORMAT				0x0A00
#define	DAPSDK_E_AUDIOFILE_INVALID				0x0B00
#define	DAPSDK_E_ACCESS_DENIED					0x0C00
#define	DAPSDK_E_FILE_NOT_FOUND					0x0D00
#define	DAPSDK_E_EOF							0x0E00
#define	DAPSDK_E_COOKIE							0x0F00
#define	DAPSDK_E_PLAYBACK_INPROGRESS			0x1000
#define	DAPSDK_E_TRANSFER_INPROGRESS			0x1100
#define	DAPSDK_E_BUFFER_NOT_ENOUGH				0x1200

// New Error code for Data Folder of Nomad Jukebox 2/3/Zen
#define	DAPSDK_E_NOT_A_FOLDER					0x1400	// the target file is not a folder
#define	DAPSDK_E_FOLDER_NOT_EMPTY				0x1600	// the target folder is not empty
#define	DAPSDK_E_FOLDER_EXIST					0x1700	// the target folder exist
#define	DAPSDK_E_FOLDER_NOTEXIST				0x1800	// the target folder does not exist
#define	DAPSDK_E_PARENTFOLDER_NOTEXIST			0x1900	// the target parent folder does not exist
#define	DAPSDK_E_FILEPATH_TOOLONG				0x1A00	// the target file path is too long 
#define	DAPSDK_E_FILENAME_TOOLONG				0x1B00	// the target file name is too long 
#define	DAPSDK_E_INVALID_OPERATION				0x1E00	// the operation cannot be perform


//////////////////////////////////////////////////////////////////////////
//						Definitions for WM Messages						//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////
//			JukeBox & NomadII shared stuffs			//
//////////////////////////////////////////////////////
// Download message
#define WM_DAPSDK_DOWNLOAD_PROGRESS	WM_USER+500
#define WM_DAPSDK_DOWNLOAD_COMPLETE	WM_USER+501

// Upload message
#define WM_DAPSDK_GETITEM_PROGRESS	WM_USER+502
#define WM_DAPSDK_GETITEM_COMPLETE	WM_USER+503

//////////////////////////////////////////////////////
//					JukeBox stuffs					//
//////////////////////////////////////////////////////
// Device change message	
#define WM_DAPSDK_JUKEBOX_REMOVAL	WM_USER+508
#define WM_DAPSDK_JUKEBOX_ARRIVAL	WM_USER+509
// Playback message
#define WM_DAPSDK_PLAYBACK_COMPLETE	WM_USER+504
#define WM_DAPSDK_PLAYLIST_COMPLETE	WM_USER+505
#define WM_DAPSDK_PLAYBACK_ERROR	WM_USER+506
#define WM_DAPSDK_PLAYBACK_PROGRESS	WM_USER+507


// New callback messages for MultiApplication assess of Hotcake
// Notification Messages for database change
#define		WM_DAPSDK_CHANGE_BASE					WM_USER + 200
#define		WM_DAPSDK_MUSIC_ADD_FILE				WM_DAPSDK_CHANGE_BASE + 0	// wParam = NodeId added, lParam = nil
#define		WM_DAPSDK_MUSIC_DEL_FILE				WM_DAPSDK_CHANGE_BASE + 1	// wParam = NodeId deleted, lParam = nil
#define		WM_DAPSDK_MUSIC_SETATTR_FILE			WM_DAPSDK_CHANGE_BASE + 2	// wParam = NodeId edited, lParam = nil
#define		WM_DAPSDK_DATA_ADD_FILE					WM_DAPSDK_CHANGE_BASE + 3	// wParam = NodeId added, lParam = nil
#define		WM_DAPSDK_DATA_DEL_FILE					WM_DAPSDK_CHANGE_BASE + 4	// wParam = NodeId deleted, lParam = nil
#define		WM_DAPSDK_DATA_SETATTR_FILE				WM_DAPSDK_CHANGE_BASE + 5	// wParam = NodeId edited, lParam = nil
#define		WM_DAPSDK_PLAYLIST_ADD_FILE				WM_DAPSDK_CHANGE_BASE + 6	// wParam = NodeId added, lParam = nil
#define		WM_DAPSDK_PLAYLIST_DEL_FILE				WM_DAPSDK_CHANGE_BASE + 7	// wParam = NodeId deleted, lParam = nil
#define		WM_DAPSDK_PLAYLIST_SETATTR_FILE			WM_DAPSDK_CHANGE_BASE + 8	// wParam = NodeId edited, lParam = nil
#define		WM_DAPSDK_PLAYLIST_ITEM_CHANGE			WM_DAPSDK_CHANGE_BASE + 9	// wParam = PlaylistNodeId affected, lParam = nil
#define		WM_DAPSDK_STORAGEINFO_CHANGE			WM_DAPSDK_CHANGE_BASE + 10	// wParam = deviceIndex that change occurred, lParam = nil


#define WM_DAPSDK_ADDITEM_PROGRESS	WM_USER+500
#define WM_DAPSDK_ADDITEM_COMPLETE	WM_USER+501


/////////////////////////////////////////////////////////////////////////
//							struct defines								//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////
//			JukeBox & NomadII shared stuffs			//
//////////////////////////////////////////////////////
typedef struct _DAPSDK_DATE_TIME
{
	short Year, Month, Day, DayOfWeek;
	short Hour, Min, Sec, MilliSec;
} DATE_TIME, *PDATE_TIME;

typedef struct _DAPSDK_FORMAT_INFO
{
	long	lCodecID;
	UCHAR	szCodecName[128];
	long	lSamplingRate;
	long	lNumChannel;
} FORMAT_INFO, *PFORMAT_INFO;

//-- SDK and firmware version info structure --//
//
//	used in GetSDKVersion()
//	used in GetDeviceProperties( kFirmwareVersion, kHardwareVersion)
typedef struct _DAPSDK_VERSION
{
	WORD	major;
	WORD	minor;
	WORD	build;
	WORD	specialBuild;
} DAPSDK_VERSION, *PDAPSDK_VERSION;

//-- Memory Storage Information Structure --//
typedef struct _DAPSDK_STORAGE_INFO
{
	ULONG	totalH;
	ULONG	totalL;
	ULONG	freeH;
	ULONG	freeL;
} DAPSDK_STORAGE_INFO, *PDAPSDK_STORAGE_INFO;

//-- Identification structure --//
// same for RootItem, ParentItem and Item now	//
typedef struct _DAPSDK_ID
{
	long	lID;			// stores the unique ID
	long	lType;			// stores the type (see enum above)
	BSTR	bstrName;		// stores the name
} DAPSDK_ID, *PDAPSDK_ID;

//////////////////////////////////////////////////////
//					NomadII stuffs					//
//////////////////////////////////////////////////////
typedef struct _DAPSDK_RADIOPRESET
{
	DWORD	dwPresetIndex;
	DWORD	dwPresetValue;
} RADIOPRESET, *PRADIOPRESET;


// device info struct
typedef struct _DAPSDK_DEVICE_INFO
{
    BYTE cDeviceId[16];         // player's ID
    long dwFirmwareVersion;    // player's firmware version
    long dwHardwareVersion;    // player's hardware version
	char cDeviceName[32];		// player's name w/o NULL terminator
	BYTE byPowerSource;			// player's power source status in percent
} DAPSDK_DEVICE_INFO, *PDAPSDK_DEVICE_INFO;

enum
{
	kASCII = 0,
	kBINARY,
	kUNICODE,
};

//////////////////////////////////////////////////////
// Item type for Jukebox (used by lRootItemType,	//
// lParentItemType, lItemType parameter)			//
//////////////////////////////////////////////////////
enum
{
	kAudioTrackType = 1,	// this is a audio track
	kPlaylistType,			// this is a track in the playlist
	kDataFileType,			// this is a data file
	kDataFolderType,	// this is a data file folder
};

/////////////////////////////////////////////////////////
// Item type for NomadII (used in lItemType parameter) //
/////////////////////////////////////////////////////////
enum
{
	kInternalMemType = 0, // this item resides in internal memory
	kExternalMemType,	  // this item resides in external (removeable) media
};


// Device Property Type Constant
//-- Device Property type (used by lDevicePropertyType parameter in --//
//	GetDeviceProperties/SetDeviceProperties 			 --//
enum
{
//////////////////////////////////////////////////////
//			JukeBox & NomadII shared stuffs			//
//////////////////////////////////////////////////////
	kDeviceSerialNumberValue = 1,	// GET, <NOMAD II>given the Item type
									// in "lpValue" parameter, the return value
									// is a pointer to the serial number 
									// of the specified media.
	kFirmwareVersion,		// GET, return value is a pointer to
							// 	DAPSDK_VERSION structure indicating the
							//	current firmware version in the device.
	kDeviceNameString,		// GET, return value is a pointer to
							//  BSTR string.
	kPowerSourceValue,
	kStorageInfoStruct,		// GET, return value is a pointer to 
							//	DAPSDK_STORAGE_INFO structure indicating
							//	the current memory usage.
	kDateTimeStruct,		// GET/SET, return/set value is a pointer to
							//	DATE_TIME structure (defined in SDK 1.0)
	kOwnerNameString,		// GET/SET, return/set value is a pointer to BSTR
							//	string.
	kAudioFormatCount,		// GET, return value is a pointer to the number of
							//	audio format supported by device.
	kAudioFormatInfoStruct,	// GET, given the index value in "lpValue"
							//	parameter, the return value is a pointer to
							// 	FORMATINFO structure (defined in SDK 1.0).
	kLangEncodeSupport,		// GET, returns unsigned long value
							// 0x01 == Latin 1(CP1252), 0x80000000 == UNICODE

//////////////////////////////////////////////////////
//					JukeBox stuffs					//
//////////////////////////////////////////////////////
	kHardwareVersion,		// GET, return value is a pointer to
							// 	DAPSDK_VERSION structure indicating the
							//	current hardware version in the device.
	kAudioVolumeValue,		// GET/SET, return/set value is the pointer to the
							//	volume level in percentage.
	kAudioMuteValue,		// GET/SET, return/set value is the pointer to the
							//	Mute status (1=on, 0=off).
							//	Treble level in percentage.
	kEAXCount,				// GET, return value is the pointer to the number
							//	of EAX settings.
	kEAXNameString,			// GET, given the index value in "lpValue"
							//	parameter, the return value is a pointer to
							//	a BSTR string.
	kEAXAmountValue,		// GET/SET, given the index value in "lpValue"
							//	parameter, the return value is the effect
							//	amount in percentage.
	kEAXCurrentIndex,		// GET/SET, the value in "lpValue" is used to set
							//	or retrieve the current EAX selection.
	kAudioEQPresetCount,	// GET, return value is the pointer to the number
							//	of EQ settings.
	kAudioEQNameString,		// GET, given the index value in "lpValue"
							//	parameter, the return value is a pointer to
							//	a BSTR string.
	kAudioEQAmountValue,	// GET/SET, return/set value is the pointer to the
							//	EQ amount in percentage.
	kAudioEQCurrentIndex,	// GET/SET, the value in "lpValue" is used to set
							//	or retrieve the current EQ selection.

//////////////////////////////////////////////////////
//					NomadII stuffs					//
//////////////////////////////////////////////////////
	kFMRadioPresetCount,	// GET, returns pointer to number of FM radio 
							// presets available in the player.
	kFMRadioPresetValue,		// GET/SET, given the preset index value, 
							// returns an existing value or sets a new
							// preset FM preset. Value in kHz.	
	kFormatStorage			// SET, Do format storage.
};
//Note that all index value is zero-based. Client should call the "kxxxxxxCount" property first before trying to get the value for individual settings in list-type of properties, i.e. kEAXNameString, kAudioFormatInfoStruct...
//Not all properties are readable and writable, some are read-only attributes. Those properties that are read-only are marked with the "GET" strings and those that are read and writable are marked as "GET/SET". Client should not call SetDeviceProperties() with the read-only property type, such call would fail.
//New Property types maybe supported in future by simply adding into the enum list, and publish to developers.


//////////////////////////////////////////////////////
//					JukeBox stuffs					//
//////////////////////////////////////////////////////
//-- Playback operation type (used by lPlayOperationType parameter in --//
//-- PlayControl() & QueueControl()) --//
enum
{
	kPlayTrack = 1,			// lpValue stores the pointer to the
							//	DAPSDK_ITEM_ID structure.
	kStopTrack,				// lpValue is not used, stop current track, no op
							//	if no track is currently playing.
	kPauseTrack,			// lpValue is not used, pause current track, no op
							//	if no track is currently playing.
	kSetPlaybackPosition,	// lpValue stores the pointer to the new playback
							//	position.
	kQueueTrack,			// lpValue stores the pointer to the
							//	DAPSDK_ITEM_ID structure.
	kClearQueue,			// lpValue is not used, clear existing queue.
};

#define TITLE				"TITLE"
#define FILESIZE			"FILE SIZE"
#define CODEC				"CODEC"
#define ALBUM				"ALBUM"
#define ARTIST				"ARTIST"
#define GENRE				"GENRE"
#define LENGTH				"LENGTH"
#define TRACKNUM			"TRACK NUM"
#define YEAR				"YEAR"
#define PLAYONLY			"PLAYONLY"
#define TRACKID				"TRACK ID"

// new attribute for datafile and datafolder
#define MOD_FILETIME		"MODIFIED FILETIME"
#define FILE_ATTRIB			"FILE ATTRIB"
#define PARENT_FOLDER		"PARENT FOLDER"
#define FOLDERNAME			"FOLDER NAME"	// data folder name for Nomad Jukebox
#define MULTI_ATTRIB		"MULTI ATTRIB"	// this is for user to change multiple file attribute at one time.

#define FILENAME			"FILE NAME"		// file name for Nomad II, data file name for Nomad Jukebox

#define ALLTRACKSKEY		-1

//////////////////////////////////////////////////////
//					NomadII stuffs					//
//////////////////////////////////////////////////////
#define DOS_FILEATTRIB		"DOS_FILEATTRIB"
#define DOS_DATETIME		"DOS_DATETIME"


//////////////////////////////////////////////////////////////////////////
//					ICTJukebox (Interface 1) Methods					//
//////////////////////////////////////////////////////////////////////////

// {DFC9207F-4B64-11D4-A4ED-00A0C98E46CC}
DEFINE_GUID(IID_ICTJukebox, 
0xdfc9207f, 0x4b64, 0x11d4, 0xa4, 0xed, 0x00, 0xa0, 0xc9, 0x8e, 0x46, 0xcc);

interface ICTJukebox : public IUnknown
{
	virtual HRESULT		STDMETHODCALLTYPE	CancelTransfer(
									/*[in]*/long lDeviceID) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	RenamePlaylist(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lPlaylistID, 
									/*[in]*/BSTR bstrName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	RemoveTracksFromPlaylist(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lTrackCount, 
									/*[in]*/long* lpTrackList, 
									/*[in]*/long lPlaylist) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	AddTracksToPlaylist(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lTrackCount, 
									/*[in]*/long* lpTrackList, 
									/*[in]*/long lPlaylist) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	InsertPlaylist(
									/*[in]*/long lDeviceID, 
									/*[in]*/BSTR bstrPlaylistName, 
									/*[out]*/long* lpPlaylistID) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	DeletePlaylist(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lPlaylistID) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	FindNextTrackInPlaylist(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lPlaylistID, 
									/*[out]*/long* lpTrackID, 
									/*[out]*/BSTR* lpbstrTrackName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	FindFirstTrackInPlaylist(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lPlaylistID, 
									/*[out]*/long* lpTrackID, 
									/*[out]*/BSTR* lpbstrTrackName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	FindNextPlaylist(
									/*[in]*/long lDeviceID, 
									/*[out]*/long* lpPlaylistID, 
									/*[out]*/BSTR* lpbstrPlaylistName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	FindFirstPlaylist(
									/*[in]*/long lDeviceID, 
									/*[out]*/long* lpPlaylistID, 
									/*[out]*/BSTR* lpbstrPlaylistName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	ChangeTrackInfo(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lTarckID, 
									/*[in]*/long lSize, 
									/*[in]*/IUnknown* lpTrackInfo) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	DeleteTrack(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lTrackID) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	InsertTrack(
									/*[in]*/long lDeviceID, 
									/*[in]*/BSTR bstrFilePath, 
									/*[in]*/long lSize, 
									/*[in]*/IUnknown* lpTrackInfo, 
									/*[out]*/long* lpTrackID) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	GetTrackInfo(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lTrackID, 
									/*[in]*/long lInSize, 
									/*[out]*/long* lpOutSize, 
									/*[out]*/IUnknown* lpTrackInfo) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	FindNextTrack(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lKeyID, 
									/*[out]*/long* lpTrackID, 
									/*[out]*/BSTR* lpbstrName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	FindFirstTrack(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lKeyID, 
									/*[out]*/long* lpTrackID, 
									/*[out]*/BSTR* lpbstrName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	FindNextKey(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lRootKeyID, 
									/*[out]*/long* lpKeyID, 
									/*[out]*/BSTR* lpbstrName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	FindFirstKey(
									/*[in]*/long lDeviceID, 
									/*[in]*/long lRootKeyID, 
									/*[out]*/long* lpKeyID, 
									/*[out]*/BSTR* lpbstrName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	FindNextRootKey(
									/*[in]*/long lDeviceID, 
									/*[out]*/long* lpRootKeyID, 
									/*[out]*/BSTR* lpbstrName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	FindFirstRootKey(
									/*[in]*/long lDeviceID, 
									/*[out]*/long* lpRootKeyID, 
									/*[out]*/BSTR* lpbstrName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	SetOwnerName(
									/*[in]*/long lDeviceID, 
									/*[in]*/BSTR bstrName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	GetOwnerName(
									/*[in]*/long lDeviceID, 
									/*[out]*/BSTR* lpbstrName) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	SetDateTime(
									/*[in]*/long lDeviceID, 
									/*[out]*/IUnknown* lpDateTime) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	GetDateTime(
									/*[in]*/long lDeviceID, 
									/*[out]*/IUnknown* lpDateTime) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	FindNextFormatSupport(
									/*[in]*/long lDeviceID, 
									/*[out]*/IUnknown* lpFormatInfo) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	FindFirstFormatSupport(
									/*[in]*/long lDeviceID, 
									/*[out]*/IUnknown* lpFormatInfo) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	GetStorageInfo(
									/*[in]*/long lDeviceID, 
									/*[out]*/unsigned long* lpTotalMemHigh, 
									/*[out]*/unsigned long* lpTotalMemLow, 
									/*[out]*/unsigned long* lpFreeMemHigh, 
									/*[out]*/unsigned long* lpFreeMemLow ) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	GetDeviceInfo(
									/*[in]*/long lDeviceID, 
									/*[out]*/IUnknown* lpDeviceInfo) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	GetDeviceCount(
									/*[out]*/long* lpDeviceCount) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	SetCallbackWindow(
									/*[in]*/long lDeviceID, 
									/*[in]*/long hWnd) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	GetSDKVersion(
									/*[out]*/long* lpVersion) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	ShutDown() = 0;
	virtual HRESULT		STDMETHODCALLTYPE	Initialize() = 0;

};


//////////////////////////////////////////////////////////////////////////
//					ICTJukebox2 (Interface 2) Methods					//
//////////////////////////////////////////////////////////////////////////

DEFINE_GUID(IID_ICTJukebox2, 
0xdfc92080, 0x4b64, 0x11d4, 0xa4, 0xed, 0x00, 0xa0, 0xc9, 0x8e, 0x46, 0xcc);

interface ICTJukebox2 : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE Initialize2() = 0;
	virtual HRESULT STDMETHODCALLTYPE ShutDown2() = 0;
	virtual HRESULT STDMETHODCALLTYPE SetCallbackWindow2(
							long		lDeviceID,
							long		hWnd ) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDeviceCount2(
							long*		lpDeviceCount ) = 0;

	//-- Canceling I/O --//
	virtual HRESULT STDMETHODCALLTYPE CancelTransfer2(
							long		lDeviceID ) = 0;


	//-- GetSDKVersion2() Fucntion Descriptions --//
	// This function overides the same function in interface 1, this is to
	// return a more meaningful version information to the client which is
	// clearly specified in the DAPSDK_VERSION structure
	virtual HRESULT STDMETHODCALLTYPE GetSDKVersion2(
							IUnknown* 	lpSDKVersion ) = 0;


	//_______________________ Querying RootItem __________________________//

	virtual HRESULT STDMETHODCALLTYPE FindFirstRootItem	(
							long		lDeviceID,
							IUnknown*	lpRootItemID ) = 0;
	virtual HRESULT STDMETHODCALLTYPE FindNextRootItem	(
							long		lDeviceID,
							IUnknown*	lpRootItemID ) = 0;

	//______________________ Querying ParentItem __________________________//

	virtual HRESULT STDMETHODCALLTYPE FindFirstParentItem	(
							long		lDeviceID,
							IUnknown*	lRootItemID,
							IUnknown*	lpParentItemID ) = 0;
	virtual HRESULT STDMETHODCALLTYPE FindNextParentItem	(
							long		lDeviceID,
							IUnknown*	lRootItemID,
							IUnknown*	lpParentItemID ) = 0;


 	//______________________ ParentItem Management __________________________	//

	//-- AddParentItem() Fucntion Descriptions --//
	// Client can only add Parent Item of the same type as the RootItem. For
	// example, client cannot add a kDataFileType Parent item into a RootItem
	// type of kAudioTrackType.
	//
	// As of current firmware implementation, client can only add ParentItem
	// into a RootItem that has a type of kPlaylistType. Adding ParentItem into
	// other RootItem type will fail.
	virtual HRESULT STDMETHODCALLTYPE AddParentItem	(
							long 		lDeviceID,
							IUnknown*	lRootItemID,
							IUnknown*	lParentItemID ) = 0;

	//-- DeleteParentItem() Function Description --//
	// Client can only delete ParentItem of type kPlaylistType. Deleting other
	// ParentItem type will fail.
	virtual HRESULT STDMETHODCALLTYPE DeleteParentItem	(
							long		lDeviceID,
							IUnknown*	lParentItemID ) = 0;

	//-- RenameParentItem() Function Description --//
	// Client can only rename ParentItem of type kPlaylistType. Renaming other
	// ParentItem type will fail.
	//
	// The updated DAPSDK_PARENTITEM_ID structure is returned in lParentItemID
	// parameter.
	virtual HRESULT STDMETHODCALLTYPE RenameParentItem	(
							long		lDeviceID,
							IUnknown*	lParentItemID,
							BSTR	bstrNewParentItemName ) = 0;


	//__________________________ Querying Item ______________________________//

	virtual HRESULT STDMETHODCALLTYPE FindFirstItem	(
							long		lDeviceID,
							IUnknown*	lParentItemID,
							IUnknown*	lpItemID ) = 0;
	virtual HRESULT STDMETHODCALLTYPE FindNextItem		(
							long		lDeviceID,
							IUnknown*	lParentItemID,
							IUnknown*	lpItemID ) = 0;


	//_____________________ Getting Item Attributes __________________________//

	//-- GetItemAttribute() Function Description --//
	// This function returns the TrackInfo data for all Item type. But for
	// kDataFileType and kPlaylistFileType, the TrackInfo data contain only
	// file name, file type and file size. In contrast, the item attribute
	// for a kAudioTrackType may contain all information similar to the ID3tag
	// information that a typical MP3 file has.
	virtual HRESULT STDMETHODCALLTYPE GetItemAttribute	(
							long		lDeviceID,
							IUnknown*	lpItemID,
							long		lInItemInfoSize,
							long*		lOutItemInfoSize,
							IUnknown*	lpItemInfo ) = 0;

	
	//-- SetItemAttribute() Function Description --//
	// This function allows client to set TrackInfo attributes of a
	// kAudioTrackItem only. Setting other item type will fail.
	//
	// It allows client to set a particular attribute data according to the name
	// and type given in the parameters. Note that only one attribute can be set
	// at a time.
	virtual HRESULT STDMETHODCALLTYPE SetItemAttribute	(
							long		lDeviceID,
							IUnknown*	lpItemID,
							BSTR		bstrAttributeName,
							long		lAttributeType,
							long		lAttributeDataSize,
							IUnknown*	lpAttributeData ) = 0;


	//____________________________ Item Management ___________________________//

	//-- AddItem() Function Description --//
	// This function initiates the file download from the computer to the
	// device. Client should call this function to download audio tracks
	// like MP3, WMA and WAVE and data files. Client are not allow
	// to download an item of kPlaylistType, use AddItemsToParentItem to add
	// item of type kPlaylistType.
	virtual HRESULT STDMETHODCALLTYPE AddItem		(
							long		lDeviceID,
							long		lItemType,
							BSTR		bstrSrcFileName,
							long		lItemInfoSize,
							IUnknown*	lpItemInfo) = 0;

	//-- AddItemsToParentItem() Function Description --//
	// Client call this function to add kAudioTrackType item into the ParentItem
	// of type kPlaylistType. Only kAudioTrackType items are accepted, and only
	// ParentItem of type kPlaylistType can accept such addition.
	virtual HRESULT STDMETHODCALLTYPE AddItemsToParentItem(
							long		lDeviceID,
							IUnknown*	lpParentItemID,
							long		lItemIDCount,
							IUnknown*	lpItemIDList ) = 0;

	//-- DeleteItem() Function Description --//
	// Client can call this function to remove item of type kAudioTrackType and
	// kDataFileType. You cannot remove kPlaylistType item here.
	virtual HRESULT STDMETHODCALLTYPE DeleteItem		(
							long		lDeviceID,
							IUnknown*	lpItemID ) = 0;

	//-- GetItem() Function Description --//
	// Client can call this function to retrieve item that are kAudioTrackType
	// and kDataFileType from the device to the computer. Note that client 
	// cannot retrieve item of type kPlaylistItem
	virtual HRESULT STDMETHODCALLTYPE GetItem		(
							long		lDeviceID,
							BSTR	bstrDestinationFileName,
							IUnknown*	lpItemID ) = 0;


	//________________________ Device Properties ____________________________//

	virtual HRESULT STDMETHODCALLTYPE GetDeviceProperties	(
							long		lDeviceID,
							long		lDevicePropertyType,
							IUnknown*	lpValue ) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetDeviceProperties	(
							long		lDeviceID,
							long		lDevicePropertyType,
							IUnknown*	lpValue ) = 0;


	//_______________________ Playback control ______________________________//

	virtual HRESULT STDMETHODCALLTYPE PlayControl		(
							long		lDeviceID,
							long		lPlayOperationType,
							IUnknown*	lpValue ) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueueControl		(
							long		lDeviceID,
							long		lQueueOperationType,
							IUnknown*	lpValue ) = 0;

};

//////////////////////////////////////////////////////////////
//				ICTNomad2 Methods							//
//////////////////////////////////////////////////////////////

// {368953D4-6A2F-4787-BC6F-4047A39A7557}
DEFINE_GUID(IID_ICTNomad2, 
0x368953d4, 0x6a2f, 0x4787, 0xbc, 0x6f, 0x40, 0x47, 0xa3, 0x9a, 0x75, 0x57);
 
interface ICTNomad2 : public IUnknown
{
	virtual HRESULT		STDMETHODCALLTYPE	Initialize() = 0;
	virtual HRESULT		STDMETHODCALLTYPE	ShutDown() = 0;

	virtual HRESULT		STDMETHODCALLTYPE	SetCallbackWindow(
									/*[in]*/long lDeviceID, 
									/*[in]*/long hWnd) = 0;
	virtual HRESULT		STDMETHODCALLTYPE	GetDeviceCount(
									/*[out]*/long* lpDeviceCount) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	CancelTransfer(
									/*[in]*/long lDeviceID) = 0;

	virtual HRESULT		STDMETHODCALLTYPE	GetSDKVersion(
									/*[out]*/IUnknown* lpVersion) = 0;


	//__________________________ Querying Item ______________________________//

	virtual HRESULT STDMETHODCALLTYPE FindFirstItem	(
							long		lDeviceID,
							IUnknown*	lParentItemID,
							IUnknown*	lpItemID ) = 0;
	virtual HRESULT STDMETHODCALLTYPE FindNextItem		(
							long		lDeviceID,
							IUnknown*	lParentItemID,
							IUnknown*	lpItemID ) = 0;


	//_____________________ Getting Item Attributes __________________________//

	//-- GetItemAttribute() Function Description --//
	// This function returns the TrackInfo data for all Item type. But for
	// kDataFileType and kPlaylistFileType, the TrackInfo data contain only
	// file name, file type and file size. In contrast, the item attribute
	// for a kAudioTrackType may contain all information similar to the ID3tag
	// information that a typical MP3 file has.
	virtual HRESULT STDMETHODCALLTYPE GetItemAttribute	(
							long		lDeviceID,
							IUnknown*	lpItemID,
							long		lInItemInfoSize,
							long*		lOutItemInfoSize,
							IUnknown*	lpItemInfo ) = 0;


	//____________________________ Item Management ___________________________//

	//-- AddItem() Function Description --//
	// This function initiates the file download from the computer to the
	// device. Client should call this function to download audio tracks
	// like MP3, WMA and WAVE and data files. Client are not allow
	// to download an item of kPlaylistType, use AddItemsToParentItem to add
	// item of type kPlaylistType.
	virtual HRESULT STDMETHODCALLTYPE AddItem		(
							long		lDeviceID,
							long		lItemType,
							BSTR		bstrSrcFileName,
							long		lItemInfoSize,
							IUnknown*	lpItemInfo) = 0;

	//-- DeleteItem() Function Description --//
	// Client can call this function to remove item of type kAudioTrackType and
	// kDataFileType. You cannot remove kPlaylistType item here.
	virtual HRESULT STDMETHODCALLTYPE DeleteItem		(
							long		lDeviceID,
							IUnknown*	lpItemID ) = 0;

	//-- GetItem() Function Description --//
	// Client can call this function to retrieve item that are kAudioTrackType
	// and kDataFileType from the device to the computer. Note that client 
	// cannot retrieve item of type kPlaylistItem
	virtual HRESULT STDMETHODCALLTYPE GetItem		(
							long		lDeviceID,
							BSTR		bstrDestinationFileName,
							IUnknown*	lpItemID ) = 0;


	//________________________ Device Properties ____________________________//
	
	virtual HRESULT STDMETHODCALLTYPE GetDeviceProperties	(
							long		lDeviceID,
							long		lDevicePropertyType,
							IUnknown*	lpValue ) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetDeviceProperties	(
							long		lDeviceID,
							long		lDevicePropertyType,
							IUnknown*	lpValue ) = 0;

};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

