#ifndef _NJBDEVICE_H_
#define _NJBDEVICE_H_

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <Objbase.h>
#include <initguid.h>
#include <stdio.h>
#include <shlobj.h>
#include <wchar.h>
#include "../Plugins/General/gen_ml/ml.h"
#include "../Plugins/Library/ml_pmp/pmp.h"
#include "../Plugins/Library/ml_pmp/transcoder.h"
#include "../Plugins/General/gen_ml/itemlist.h"
#include "Nmsdk.h"
#include "resource.h"

#include "../Agave/Language/api_language.h"
#include <api/service/waServiceFactory.h>

extern PMPDevicePlugin plugin;
extern LRESULT CALLBACK CallbackWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern C_ItemList devices;
extern ICTJukebox2 * m_pCTJukebox2;

#define fieldlen 256

class Playlist {
public:
  bool dirty;
  int plid;
  wchar_t name[fieldlen];
  C_ItemList songs;
};

class Song {
public:
  char codec[5];
  int trackid;
  wchar_t artist[fieldlen];
  wchar_t album[fieldlen];
  wchar_t title[fieldlen];
  wchar_t genre[fieldlen];
  int year,track,size,length;
};

class TransferItem {
public:
  const itemRecordW * track;
  void* callbackContext;
  void (*callback)(void * callbackContext, wchar_t * status);
  songid_t * songid;
  int * killswitch;
  int status;
  BYTE * meta;
  wchar_t * file;
};

class NJBDevice : public Device {
public:
  Transcoder * transcoder;
  BYTE serial[16];
  __int64 transferQueueLength;
  DAPSDK_ID playlistRoot;
  C_ItemList playlists;
  int id;
  HWND messageWindow;
  TransferItem transferItem;
  TransferItem revTransferItem;
  CRITICAL_SECTION csTransfer;
  CRITICAL_SECTION csRevTransfer;

  NJBDevice(long id);
  virtual ~NJBDevice();

  virtual BOOL WindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  virtual __int64 getDeviceCapacityAvailable(); // in bytes
  virtual __int64 getDeviceCapacityTotal(); // in bytes

  virtual void Eject(); // if you ejected successfully, you MUST call plugin.deviceDisconnected(this) and delete this;
  virtual void Close(); // save any changes, and call plugin.deviceDisconnected(this) AND delete this;
  
  
  // return 0 for success, -1 for failed or cancelled
  virtual int transferTrackToDevice(const itemRecordW * track, // the track to transfer
    void * callbackContext, //pass this to the callback
    void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
    songid_t * songid, // fill in the songid when you are finished
    int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
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

  virtual void setPlaylistName(int playlistnumber, const wchar_t * buf); // with playlistnumber={}, set the name of the device.
  virtual void playlistSwapItems(int playlistnumber, int posA, int posB); // swap the songs at position posA and posB
  virtual void sortPlaylist(int playlistnumber, int sortBy); // only implement if you are a non-cached device!!!
  virtual void addTrackToPlaylist(int playlistnumber, songid_t songid); // adds songid to the end of the playlist
  virtual void removeTrackFromPlaylist(int playlistnumber, int songnum); //where songnum is the position of the track in the playlist

  virtual void deletePlaylist(int playlistnumber);
  virtual int newPlaylist(const wchar_t * name); // create empty playlist, returns playlistnumber
  
  virtual void getTrackArtist(songid_t songid, wchar_t * buf, int len);
  virtual void getTrackAlbum(songid_t songid, wchar_t * buf, int len);
  virtual void getTrackTitle(songid_t songid, wchar_t * buf, int len);
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
  virtual void getTrackExtraInfo(songid_t songid, const wchar_t * field, wchar_t * buf, int len); //optional
  
  // feel free to ignore any you don't support
  virtual void setTrackArtist(songid_t songid, const wchar_t * value);
  virtual void setTrackAlbum(songid_t songid, const wchar_t * value);
  virtual void setTrackTitle(songid_t songid, const wchar_t * value);
  virtual void setTrackTrackNum(songid_t songid, int value);
  virtual void setTrackDiscNum(songid_t songid, int value){};
  virtual void setTrackGenre(songid_t songid, const wchar_t * value);
  virtual void setTrackYear(songid_t songid, int year);
  virtual void setTrackPlayCount(songid_t songid, int value){};
  virtual void setTrackRating(songid_t songid, int value){};
  virtual void setTrackLastPlayed(songid_t songid, __time64_t value){}; // in unix time format
  virtual void setTrackLastUpdated(songid_t songid, __time64_t value){}; // in unix time format
  virtual void setTrackExtraInfo(songid_t songid, const wchar_t * field, const wchar_t * value) {}; //optional

  virtual bool playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue){return false;}; // return false if unsupported

  virtual intptr_t extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4);

  virtual bool copyToHardDriveSupported() {return true;} // for now...
  
  virtual __int64 songSizeOnHardDrive(songid_t song) {return getTrackSize(song);} // how big a song will be when copied back. Return -1 for not supported.

  virtual int copyToHardDrive(songid_t song, // the song to copy
    wchar_t * path, // path to copy to, in the form "c:\directory\song". The directory will already be created, you must append ".mp3" or whatever to this string! (there is space for at least 10 new characters).
    void * callbackContext, //pass this to the callback
    void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
    int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
    ); // -1 for failed/not supported. 0 for success.
};

#endif //_NJBDEVICE_H_