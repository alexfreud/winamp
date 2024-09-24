#pragma once
#include <windows.h>
#include <Dbt.h>

#include "../../Library/ml_pmp/transcoder.h"
#include "../../Library/ml_pmp/pmp.h"

#include "../nde/nde_c.h"
#include "../nu/AutoLock.h"

#include <vector>

#define NDE_CACHE_DAT L"winamp_metadata.dat"
#define NDE_CACHE_IDX L"winamp_metadata.idx"

//Filename="E:\Howling Bells - Into The Chaos.MP3" 
//Artist="Howling Bells" 
//Album="E:" 
//Title="Into The Chaos" 
//Genre="" 
//AlbumArtist="" 
//Publisher="" 
//Composer="" 
//Year="0" 
//Track="0" 
//Bitrate="0" 
//Playcount="0" 
//Discnum="0" 
//Length="0" 
//Size="7767879" 
enum
{
	NDE_USB_FAILURE=0,
	NDE_USB_SUCCESS=1,
};

enum
{
	DEVICEVIEW_COL_FILENAME = 0,
	DEVICEVIEW_COL_ARTIST=1,
	DEVICEVIEW_COL_ALBUM=2,
	DEVICEVIEW_COL_TITLE=3,
	DEVICEVIEW_COL_GENRE=4,
	DEVICEVIEW_COL_ALBUM_ARTIST=5,
	DEVICEVIEW_COL_PUBLISHER=6,
	DEVICEVIEW_COL_COMPOSER=7,
	DEVICEVIEW_COL_YEAR=8,
	DEVICEVIEW_COL_TRACK=9,
	DEVICEVIEW_COL_BITRATE=10,
	DEVICEVIEW_COL_DISC_NUMBER=11,
	DEVICEVIEW_COL_LENGTH=12,
	DEVICEVIEW_COL_SIZE=13,
	DEVICEVIEW_COL_PLAY_COUNT=14,
};

#define TAG_CACHE L"winamp_metadata.dat"
#define FIELD_LENGTH 1024

class UsbSong {
public:
	UsbSong();
	wchar_t filename[MAX_PATH];
	wchar_t artist[FIELD_LENGTH];
	wchar_t album[FIELD_LENGTH];
	wchar_t title[FIELD_LENGTH];
	wchar_t genre[FIELD_LENGTH];
	wchar_t albumartist[FIELD_LENGTH];
	wchar_t publisher[FIELD_LENGTH];
	wchar_t composer[FIELD_LENGTH];
	int year,track,length,discnum,bitrate,playcount;
	__int64 size;
	BOOL filled;
	wchar_t ext[ 6 ];
};

enum DeviceType {
  TYPE_OTHER,
  TYPE_PSP,
};

class USBPlaylist;

class USBDevice : public Device
{
public:
	USBDevice(wchar_t drive, pmpDeviceLoading * load);
	~USBDevice();
	USBDevice();
	void fileProbe(wchar_t * indir);
	void tag(void); //load ID3 tags from cache or mp3 file
	void createDeviceFields();
	int openDeviceDatabase();
	int openDeviceTable();
	void closeDeviceTable();
	static void CloseDatabase();

	void refreshNDECache(void);
	void fillMetaData(UsbSong *s);
	static int getFileInfoW(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, size_t len);
	void setupTranscoder();
	USBPlaylist* getMasterPlaylist();
	UsbSong* findSongInMasterPlaylist(const wchar_t *songfn);
	void writeRecordToDB(UsbSong* songToPrint);
	//////////////////////////////////////////

	virtual __int64 getDeviceCapacityAvailable(); // in bytes
	virtual __int64 getDeviceCapacityTotal(); // in bytes

	virtual void Eject(); // if you ejected successfully, you MUST call PMP_IPC_DEVICEDISCONNECTED and delete this;
	virtual void Close(); // save any changes, and call PMP_IPC_DEVICEDISCONNECTED AND delete this;

	// return 0 for success, -1 for failed or cancelled
	virtual int transferTrackToDevice(const itemRecordW * track, // the track to transfer
	void * callbackContext, //pass this to the callback
	void (*callback)(void *callbackContext, wchar_t *status),  // call this every so often so the GUI can be updated. Including when finished!
											songid_t * songid, // fill in the songid when you are finished
											int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
	);
	virtual int trackAddedToTransferQueue(const itemRecordW *track); // return 0 to accept, -1 for "not enough space", -2 for "incorrect format"
	virtual void trackRemovedFromTransferQueue(const itemRecordW *track); 

	// return the amount of space that will be taken up on the device by the track (once it has been tranferred)
	// or 0 for incompatable. This is usually the filesize, unless you are transcoding. An estimate is acceptable.
	virtual __int64 getTrackSizeOnDevice(const itemRecordW *track); 

	virtual void deleteTrack(songid_t songid); // physically remove from device. Be sure to remove it from all the playlists!

	virtual void commitChanges(); // optional. Will be called at a good time to save changes

	virtual int getPlaylistCount(); // always at least 1. playlistnumber 0 is the Master Playlist containing all tracks.
	// PlaylistName(0) should return the name of the device.
	virtual void getPlaylistName(int playlistnumber, wchar_t *buf, int len);
	virtual int getPlaylistLength(int playlistnumber);
	virtual songid_t getPlaylistTrack(int playlistnumber,int songnum); // returns a songid

	virtual void setPlaylistName(int playlistnumber, const wchar_t *buf); // with playlistnumber==0, set the name of the device.
	virtual void playlistSwapItems(int playlistnumber, int posA, int posB); // swap the songs at position posA and posB
	virtual void sortPlaylist(int playlistnumber, int sortBy);
	virtual void addTrackToPlaylist(int playlistnumber, songid_t songid); // adds songid to the end of the playlist
	virtual void removeTrackFromPlaylist(int playlistnumber, int songnum); //where songnum is the position of the track in the playlist

	virtual void deletePlaylist(int playlistnumber);
	virtual int newPlaylist(const wchar_t *name); // create empty playlist, returns playlistnumber. -1 for failed.

	virtual void getTrackArtist(songid_t songid, wchar_t *buf, int len);
	virtual void getTrackAlbum(songid_t songid, wchar_t *buf, int len);
	virtual void getTrackTitle(songid_t songid, wchar_t *buf, int len);
	virtual int getTrackTrackNum(songid_t songid);
	virtual int getTrackDiscNum(songid_t songid);
	virtual void getTrackGenre(songid_t songid, wchar_t * buf, int len);
	virtual int getTrackYear(songid_t songid);
	virtual __int64 getTrackSize(songid_t songid); // in bytes
	virtual int getTrackLength(songid_t songid); // in millisecs
	virtual int getTrackBitrate(songid_t songid); // in kbps
	virtual int getTrackPlayCount(songid_t songid);
	virtual int getTrackRating(songid_t songid); //0-5
	virtual __time64_t getTrackLastPlayed(songid_t songid); // in unix time format
	virtual __time64_t getTrackLastUpdated(songid_t songid); // in unix time format
	virtual void getTrackAlbumArtist(songid_t songid, wchar_t *buf, int len);
	virtual void getTrackPublisher(songid_t songid, wchar_t *buf, int len);
	virtual void getTrackComposer(songid_t songid, wchar_t *buf, int len);
	virtual int getTrackType(songid_t songid);
	virtual void getTrackExtraInfo(songid_t songid, const wchar_t *field, wchar_t *buf, int len) ; //optional

	// feel free to ignore any you don't support
	virtual void setTrackArtist(songid_t songid, const wchar_t *value);
	virtual void setTrackAlbum(songid_t songid, const wchar_t *value);
	virtual void setTrackTitle(songid_t songid, const wchar_t *value);
	virtual void setTrackTrackNum(songid_t songid, int value);
	virtual void setTrackDiscNum(songid_t songid, int value);
	virtual void setTrackGenre(songid_t songid, const wchar_t *value);
	virtual void setTrackYear(songid_t songid, int year);
	virtual void setTrackPlayCount(songid_t songid, int value);
	virtual void setTrackRating(songid_t songid, int value);
	virtual void setTrackLastPlayed(songid_t songid, __time64_t value); // in unix time format
	virtual void setTrackLastUpdated(songid_t songid, __time64_t value); // in unix time format
	virtual void setTrackAlbumArtist(songid_t songid, const wchar_t *value);
	virtual void setTrackPublisher(songid_t songid, const wchar_t *value);
	virtual void setTrackComposer(songid_t songid, const wchar_t *value);
	virtual void setTrackExtraInfo(songid_t songid, const wchar_t *field, const wchar_t *value) ; //optional

	virtual bool playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue); // return false if unsupported

	virtual intptr_t extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4);

	virtual bool copyToHardDriveSupported();

	virtual __int64 songSizeOnHardDrive(songid_t song); // how big a song will be when copied back. Return -1 for not supported.

	virtual int copyToHardDrive(songid_t song, // the song to copy
		wchar_t * path, // path to copy to, in the form "c:\directory\song". The directory will already be created, you must append ".mp3" or whatever to this string! (there is space for at least 10 new characters).
		void * callbackContext, //pass this to the callback
		void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
		int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
	); // -1 for failed/not supported. 0 for success.

	// art functions
	virtual void setArt(songid_t songid, void *buf, int w, int h); //buf is in format ARGB32*
	virtual pmpart_t getArt(songid_t songid);
	virtual void releaseArt(pmpart_t art);
	virtual int drawArt(pmpart_t art, HDC dc, int x, int y, int w, int h);
	virtual void getArtNaturalSize(pmpart_t art, int *w, int *h);
	virtual void setArtNaturalSize(pmpart_t art, int w, int h);
	virtual void getArtData(pmpart_t art, void* data); // data ARGB32* is at natural size
	virtual bool artIsEqual(pmpart_t a, pmpart_t b);

	// Additional attributes
	Transcoder *transcoder;
	wchar_t drive;
	DeviceType devType;
	wchar_t iniFile[MAX_PATH];
	wchar_t pldir[MAX_PATH];
	wchar_t songFormat[MAX_PATH];
	wchar_t supportedFormats[MAX_PATH];
	wchar_t purgeFolders[2];
	int pl_write_mode; // used to determine how the playlists are stored
	__int64 transferQueueLength;
	std::vector<USBPlaylist*> usbPlaylists;
	bool cacheUpToDate; //whether or not to output on device disconnect
	bool loadedUpToDate; //whether or not songs in memory are tagged and correct

	static nde_database_t discDB;
	nde_table_t deviceTable;
	Nullsoft::Utility::LockGuard dbcs;
	wchar_t ndeDataFile[100];
	wchar_t ndeIndexFile[100];

private:
	// update a track with new metadata (string)
	void updateTrackField(UsbSong* song, unsigned int col, const void* newValue, int fieldType);
	bool readRecordFromDB(UsbSong* song);
	bool songChanged(UsbSong* song);
};

class USBArt
{
public:
	USBArt(ARGB32 *bits, int w, int h);
	~USBArt();
	ARGB32 *bits;
	int w,h;
};