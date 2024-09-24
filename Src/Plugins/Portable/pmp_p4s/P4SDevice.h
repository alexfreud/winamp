#ifndef _P4SDEVICE_H_
#define _P4SDEVICE_H_

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <tchar.h>
#include <shlobj.h>
#include "..\..\General\gen_ml/itemlist.h"
#include "..\..\General\gen_ml/ml.h"
#include "..\..\Library\ml_pmp/pmp.h"
#include "..\..\Library\ml_pmp/transcoder.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/ipc_pe.h"
#include "../Agave/Language/api_language.h"
#include <api/service/waServiceFactory.h>
#include "resource1.h"

#include <tataki/bitmap/bitmap.h>
#include <tataki/canvas/bltcanvas.h>
#include <api/service/svcs/svc_imgload.h>
#include <api/service/svcs/svc_imgwrite.h>

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memoryManager;
#define WASABI_API_MEMMGR memoryManager

#include "../Agave/AlbumArt/api_albumart.h"
extern api_albumart *albumArtApi;
#define AGAVE_API_ALBUMART albumArtApi

#include "../devices/api_devicemanager.h"
extern api_devicemanager *deviceManagerApi;
#define AGAVE_API_DEVICEMANAGER deviceManagerApi


class P4SDevice;
class MyProgress;
class TransferItem;

#ifndef __RPC__in
#define __RPC__in
#define __RPC__in_opt
#define __RPC__in_ecount_full(x)
#define __RPC__in_ecount_full_opt(x)
#define __RPC__in_ecount_full_string(x)
#define __RPC__inout
#define __RPC__inout_opt
#define __RPC__deref_opt_inout_opt
#define __RPC__deref_inout_ecount_full_opt_string(x)
#define __RPC__inout_ecount_full(x)
#define __RPC__out
#define __RPC__out_ecount_full(x)
#define __RPC__out_ecount_full_string(x)
#define __RPC__out_ecount_part(x,y)
#define __RPC__deref_out_ecount_full_opt(x)
#define __RPC__deref_out_opt_string
#define __RPC__deref_out_opt
#endif

#include "WMDRMDeviceApp.h"
#include "msWMDM.h" // Include headers for Windows Media Device Manager.
#include "Sac.h"      // Include authentication headers.
#include "Scclient.h" // Include authentication client headers.
#include "MyProgress.h"
#include "resource1.h"

extern PMPDevicePlugin plugin;

class Playlist {
public:
	wchar_t name[128];
	IWMDMStorage4 * storage;
	C_ItemList songs;
	IWMDMMetaData * meta;
	bool modified;
	Playlist() : storage(0),meta(0),modified(false){ name[0]=0; }
};

class Song {
public:
	IWMDMStorage4 * storage;
	IWMDMMetaData * meta;
	bool video;
	bool modified;
	wchar_t * artist, * album;
	IWMDMStorage4 * alb;
	IWMDMMetaData * albmeta;
	Song() {artist=album=NULL; storage=0; meta=0; video = 0; modified = 0; alb=0; albmeta=0;}
	virtual ~Song() {if(artist) free(artist); if(album) free(album); if(alb) alb->Release(); if(albmeta) albmeta->Release();}
};

class TransferItem {
public:
	int phase;
	void * callbackContext;
	void (*callback)(void * callbackContext, wchar_t * status);
	wchar_t *file;
	const itemRecordW * track;
	songid_t * songid;
	int * killswitch;
	P4SDevice * dev;
	IWMDMMetaData * meta;
	IWMDMProgress * progress;
	int pc;
	bool video;
};

class P4SDevice : public Device {
public:
	bool requiresALB;
	int error;
	Transcoder * transcoder;
	wchar_t *musicDir, *videoDir;
	bool noMetadata, supportsVideo;
	__int64 transferQueueSize;
	C_ItemList playlists;
	C_ItemList albfiles;
	IWMDMStorage4 * playlistsDir;
	wchar_t name[100];
	IWMDMDevice3* WMDevice;
	P4SDevice(IWMDMDevice3* pIDevice,bool noMetadata);
	virtual void Load();
	virtual ~P4SDevice();
  
	void traverseStorage(IWMDMStorage * store, int level=0, wchar_t * artist=0, wchar_t * album=0);
	void foundSong(IWMDMStorage4 * store, IWMDMMetaData * meta, bool video, int pl=0,wchar_t * artist=0, wchar_t * album=0, IWMDMStorage4 * alb=0, IWMDMMetaData * albmeta=0);
	void foundPlaylist(IWMDMStorage4 * store, IWMDMMetaData * meta);
	bool songsEqual(songid_t a, songid_t b);
	int songsCmp(songid_t a, songid_t b);

	virtual __int64 getDeviceCapacityAvailable(); // in bytes
	virtual __int64 getDeviceCapacityTotal(); // in bytes

	virtual void Eject(){Close();}; // if you ejected successfully, you MUST call plugin.deviceDisconnected(this) and delete this;
	virtual void Close(); // save any changes, and call plugin.deviceDisconnected(this) AND delete this;
  
	virtual void doTransfer(TransferItem * t);

	// return the songid when transfer is finished.
	// call the callback with a percentage and the number of bytes copied so far every so often (to update the GUI)
	// return 0 for success, -1 for failed or cancelled
	virtual int transferTrackToDevice(const itemRecordW * track, // the track to transfer
		void * callbackContext, //pass this to the callback
	    void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
		songid_t * songid, // fill in the songid when you are finished
		int * killswitch // if this gets set to 1, the transfer has been cancelled by the user
    );
	virtual int trackAddedToTransferQueue(const itemRecordW * track); // return 0 to accept, -1 for "not enough space", -2 for "incorrect format"
	virtual void trackRemovedFromTransferQueue(const itemRecordW * track);  
	virtual __int64 getTrackSizeOnDevice(const itemRecordW * track); // return the amount of space taken up on the device by the track, or 0 for incompatable (usually the filesize, unless you are transcoding)

	virtual void deleteTrack(songid_t songid); // physically remove from device. Be sure to remove it from all the playlists!

	virtual void commitChanges(); // optional. Will be called at a good time to save changes

	virtual int getPlaylistCount(); // always at least 1. playlistnumber 0 is the Master Playlist containing all tracks.
	// PlaylistName(0) should return the name of the device.
	virtual void getPlaylistName(int playlistnumber, wchar_t * buf, int len);
	virtual int getPlaylistLength(int playlistnumber);
	virtual songid_t getPlaylistTrack(int playlistnumber,int songnum); // returns a songid

	virtual void setPlaylistName(int playlistnumber, const wchar_t *buf); // playlistnumber != 0!!
	virtual void playlistSwapItems(int playlistnumber, int posA, int posB); // swap the songs at position posA and posB
	virtual void sortPlaylist(int playlistnumber, int sortBy);
	virtual void addTrackToPlaylist(int playlistnumber, songid_t songid); // adds songid to the end of the playlist
	virtual void removeTrackFromPlaylist(int playlistnumber, int songnum); //where songnum is the position of the track in the playlist

	virtual void deletePlaylist(int playlistnumber);
	virtual int newPlaylist(const wchar_t * name); // create empty playlist, returns playlistnumber

	virtual void getTrackArtist(songid_t songid, wchar_t * buf, int len);
	virtual void getTrackAlbum(songid_t songid, wchar_t * buf, int len);
	virtual void getTrackTitle(songid_t songid, wchar_t * buf, int len);
	virtual int getTrackTrackNum(songid_t songid);
	virtual int getTrackDiscNum(songid_t songid){return -1;}
	virtual void getTrackGenre(songid_t songid, wchar_t * buf, int len);
	virtual int getTrackYear(songid_t songid);
	virtual __int64 getTrackSize(songid_t songid); // in bytes
	virtual int getTrackLength(songid_t songid); // in millisecs
	virtual int getTrackBitrate(songid_t songid); // in kbps
	virtual int getTrackPlayCount(songid_t songid);
	virtual int getTrackRating(songid_t songid); //0-5
	virtual __time64_t getTrackLastPlayed(songid_t songid); // in unix time format
	virtual __time64_t getTrackLastUpdated(songid_t songid); // in unix time format
	virtual void getTrackAlbumArtist(songid_t songid, wchar_t * buf, int len);
	virtual void getTrackComposer(songid_t songid, wchar_t * buf, int len);
	virtual int getTrackType(songid_t songid);
	virtual void getTrackExtraInfo(songid_t songid, const wchar_t *field, wchar_t * buf, int len);

	Song * lastChange;
	void PreCommit(Song * song);

	// feel free to ignore any you don't support
	virtual void setTrackArtist(songid_t songid, const wchar_t * value);
	virtual void setTrackAlbum(songid_t songid, const wchar_t * value);
	virtual void setTrackTitle(songid_t songid, const wchar_t * value);
	virtual void setTrackTrackNum(songid_t songid, int value);
	virtual void setTrackDiscNum(songid_t songid, int value){};
	virtual void setTrackGenre(songid_t songid, const wchar_t * value);
	virtual void setTrackYear(songid_t songid, int year);
	virtual void setTrackPlayCount(songid_t songid, int value);
	virtual void setTrackRating(songid_t songid, int value);
	virtual void setTrackLastPlayed(songid_t songid, __time64_t value); // in unix time format
	virtual void setTrackLastUpdated(songid_t songid, __time64_t value); // in unix time format
	virtual void setTrackAlbumArtist(songid_t songid, const wchar_t * value);
	virtual void setTrackComposer(songid_t songid, const wchar_t * value);
	virtual void setTrackExtraInfo(songid_t songid, const wchar_t * field, const wchar_t * value) {}; //optional

	virtual bool playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue){return false;}; // return false if unsupported

	virtual intptr_t extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4);

	virtual bool copyToHardDriveSupported() {return true;}

	virtual __int64 songSizeOnHardDrive(songid_t song) {return getTrackSize(song);} // how big a song will be when copied back. Return -1 for not supported.

	virtual int copyToHardDrive(songid_t song, // the song to copy
		wchar_t * path, // path to copy to, in the form "c:\directory\song". The directory will already be created, you must append ".mp3" or whatever to this string! (there is space for at least 10 new characters).
		void * callbackContext, //pass this to the callback
		void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
		int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
    ); // -1 for failed/not supported. 0 for success.

	virtual void setArt(songid_t songid, void *buf, int w, int h); //buf is in format ARGB32*
	virtual pmpart_t getArt(songid_t songid);
	virtual void releaseArt(pmpart_t art);
	virtual int drawArt(pmpart_t art, HDC dc, int x, int y, int w, int h);
	virtual void getArtNaturalSize(pmpart_t art, int *w, int *h);
	virtual void setArtNaturalSize(pmpart_t art, int w, int h);
	virtual void getArtData(pmpart_t art, void* data); // data ARGB32* is at natural size
	virtual bool artIsEqual(pmpart_t a, pmpart_t b);
};

#endif // _P4SDEVICE_H_