#define _WIN32_DCOM

#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <rapi2.h>
#include <shlobj.h>
//#include <vector>
#include <algorithm>

#include <vector>

#include "../Winamp/wa_ipc.h"
#include "../Plugins/General/gen_ml/ml.h"
#include "../Plugins/Library/ml_pmp/pmp.h"
#include "../Plugins/Library/ml_pmp/transcoder.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"

#include <api/service/waservicefactory.h>
#include "../playlist/ifc_playlistloader.h"
#include "../xml/ifc_xmlreadercallback.h"
#include "../xml/obj_xml.h"
#include "../xml/api__xml.h"
#include "../Agave/Language/api_language.h"

#include "resource.h"

typedef struct  {
	RAPIDEVICEID id;
	bool marked;
} ejectedDevice;

extern PMPDevicePlugin plugin;
extern std::vector<ejectedDevice*> ejected;

#define fieldlen 128

class Song {
public:
	wchar_t artist[fieldlen],album[fieldlen],title[fieldlen];
	wchar_t fn[MAX_PATH];
	int track;
	__int64 size;
	bool video;
	Song(const wchar_t * path, LPCE_FIND_DATA f, bool video);
	Song();
};

class Playlist {
public:
	std::vector<Song*> songs;
	wchar_t name[fieldlen];
	wchar_t fn[MAX_PATH];
	bool dirty;
	Playlist(const wchar_t * path, LPCE_FIND_DATA f);
	Playlist(const wchar_t * name);
};

class ASDevice : public Device {
public:
	
	wchar_t musicFolder[MAX_PATH];
	wchar_t videoFolder[MAX_PATH];
	wchar_t playlistFolder[MAX_PATH];
	wchar_t playlistFormat[16];

	IRAPIDevice *pIDevice;
	IRAPISession *pISession;
	
	Transcoder * transcoder;
	RAPI_DEVICEINFO devInfo;
	__int64 transferQueueSize;
	std::vector<Playlist*> playlists;
	void Find(const wchar_t * dir);
	void FoundFile(const wchar_t * path, LPCE_FIND_DATA f);
	
	void WritePlaylist(Playlist * pl);
	void ReadPlaylist(Playlist * pl);

	ASDevice(IRAPIDevice *pIDevice,IRAPISession *pISession);
	~ASDevice();

	virtual __int64 getDeviceCapacityAvailable(); // in bytes
	virtual __int64 getDeviceCapacityTotal(); // in bytes

	virtual void Eject();
	virtual void Close(); // save any changes, and call PMP_IPC_DEVICEDISCONNECTED AND delete this;
  
  
  // return 0 for success, -1 for failed or cancelled
  virtual int transferTrackToDevice(const itemRecordW * track, // the track to transfer
    void * callbackContext, //pass this to the callback
    void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
    songid_t * songid, // fill in the songid when you are finished
    int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
    );
  virtual int trackAddedToTransferQueue(const itemRecordW * track); // return 0 to accept, -1 for "not enough space", -2 for "incorrect format"
  virtual void trackRemovedFromTransferQueue(const itemRecordW * track); 

  // return the amount of space that will be taken up on the device by the track (once it has been tranferred)
  // or 0 for incompatable. This is usually the filesize, unless you are transcoding. An estimate is acceptable.
  virtual __int64 getTrackSizeOnDevice(const itemRecordW * track);
  
  virtual void deleteTrack(songid_t songid); // physically remove from device. Be sure to remove it from all the playlists!

  virtual void commitChanges(); // optional. Will be called at a good time to save changes

	virtual int getPlaylistCount(); // always at least 1. playlistnumber 0 is the Master Playlist containing all tracks.
  // PlaylistName(0) should return the name of the device.
  virtual void getPlaylistName(int playlistnumber, wchar_t * buf, int len);
  virtual int getPlaylistLength(int playlistnumber);
  virtual songid_t getPlaylistTrack(int playlistnumber,int songnum); // returns a songid

  virtual void setPlaylistName(int playlistnumber, const wchar_t * buf); // with playlistnumber==0, set the name of the device.
  virtual void playlistSwapItems(int playlistnumber, int posA, int posB); // swap the songs at position posA and posB
  virtual void sortPlaylist(int playlistnumber, int sortBy);
  virtual void addTrackToPlaylist(int playlistnumber, songid_t songid); // adds songid to the end of the playlist
  virtual void removeTrackFromPlaylist(int playlistnumber, int songnum); //where songnum is the position of the track in the playlist

  virtual void deletePlaylist(int playlistnumber);
  virtual int newPlaylist(const wchar_t * name); // create empty playlist, returns playlistnumber. -1 for failed.
  
  virtual void getTrackArtist(songid_t songid, wchar_t * buf, int len);
  virtual void getTrackAlbum(songid_t songid, wchar_t * buf, int len);
  virtual void getTrackTitle(songid_t songid, wchar_t * buf, int len);
  virtual int getTrackTrackNum(songid_t songid);
  virtual int getTrackDiscNum(songid_t songid){return -1;}
  virtual void getTrackGenre(songid_t songid, wchar_t * buf, int len){buf[0]=0;}
  virtual int getTrackYear(songid_t songid){return -1;}
  virtual __int64 getTrackSize(songid_t songid); // in bytes
  virtual int getTrackLength(songid_t songid){return -1;} // in millisecs
  virtual int getTrackBitrate(songid_t songid){return -1;} // in kbps
  virtual int getTrackPlayCount(songid_t songid){return 0;}
  virtual int getTrackRating(songid_t songid){return 0;} //0-5
  virtual __time64_t getTrackLastPlayed(songid_t songid){return 0;} // in unix time format
  virtual __time64_t getTrackLastUpdated(songid_t songid){return 0;} // in unix time format
	virtual int getTrackType(songid_t songid) { return ((Song *)songid)->video?1:0; }
  virtual void getTrackExtraInfo(songid_t songid, const wchar_t * field, wchar_t * buf, int len); //optional
  
  // feel free to ignore any you don't support
  virtual void setTrackArtist(songid_t songid, const wchar_t * value){}
  virtual void setTrackAlbum(songid_t songid, const wchar_t * value){}
  virtual void setTrackTitle(songid_t songid, const wchar_t * value){}
  virtual void setTrackTrackNum(songid_t songid, int value){}
  virtual void setTrackDiscNum(songid_t songid, int value){}
  virtual void setTrackGenre(songid_t songid, const wchar_t * value){}
  virtual void setTrackYear(songid_t songid, int year){}
  virtual void setTrackPlayCount(songid_t songid, int value){}
  virtual void setTrackRating(songid_t songid, int value){}
  virtual void setTrackLastPlayed(songid_t songid, __time64_t value){} // in unix time format
  virtual void setTrackLastUpdated(songid_t songid, __time64_t value){} // in unix time format
  virtual void setTrackExtraInfo(songid_t songid, const wchar_t * field, const wchar_t * value) {}; //optional

  virtual bool playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue){return false;} // return false if unsupported

  virtual intptr_t extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4);

  // new methods as of PMPHDR_VER 0x002
  virtual bool copyToHardDriveSupported() {return true;}
  
  virtual __int64 songSizeOnHardDrive(songid_t song) {return getTrackSize(song);} // how big a song will be when copied back. Return -1 for not supported.

  virtual int copyToHardDrive(songid_t song, // the song to copy
    wchar_t * path, // path to copy to, in the form "c:\directory\song". The directory will already be created, you must append ".mp3" or whatever to this string! (there is space for at least 10 new characters).
    void * callbackContext, //pass this to the callback
    void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
    int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
    ); // -1 for failed/not supported. 0 for success.
};

extern std::vector<ASDevice*> devices;
