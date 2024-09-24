#pragma once
#include "../../Library/ml_pmp/pmp.h"
#include "../../Library/ml_pmp/transcoder.h"
#include <vector>
#include "WifiDevice.h"
#include "WifiPlaylist.h"
#include "main.h"
#include <bfc/platform/types.h>

class WifiDevice;
class TemplateDevice : public Device
{
public:
	typedef std::vector<WifiTrack*> TrackList;
	typedef std::vector<WifiPlaylist*> PlaylistsList;
	TemplateDevice(WifiDevice *device, const char *root_url, DeviceInfo *device_info, TemplateDevice::TrackList *track_list, TemplateDevice::PlaylistsList *playlists_list);
	~TemplateDevice();
	virtual __int64 getDeviceCapacityAvailable(); // in bytes
	virtual __int64 getDeviceCapacityTotal(); // in bytes

	virtual void Eject(); // if you ejected successfully, you MUST call PMP_IPC_DEVICEDISCONNECTED and delete this;
	virtual void Close(); // save any changes, and call PMP_IPC_DEVICEDISCONNECTED AND delete this;
	void CloseAsync();

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

	PlaylistsList playlists;
	TrackList tracks;
	DeviceInfo device_info; 
	char *url;
	int image16, image160;
	Transcoder *transcoder;
	int64_t transferQueueLength;
};