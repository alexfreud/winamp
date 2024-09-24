#include "usbdevice.h"
#include "resource.h"
#include "usbplaylist.h"
#include "usbplaylistsaver.h"
#include "api.h"
#include "../winamp/wa_ipc.h"
#include <tataki/bitmap/bitmap.h>
#include <tataki/canvas/bltcanvas.h>
#include <shlobj.h>
#include <strsafe.h>
#include <shlwapi.h>

// from main.cpp
extern PMPDevicePlugin plugin;
extern std::vector<USBDevice*> devices;
extern bool loading_devices[26];
// from utils.cpp
extern BOOL RecursiveCreateDirectory(wchar_t* buf);
extern bool supportedFormat(wchar_t * file, wchar_t * supportedFormats);
extern DeviceType detectDeviceType(wchar_t drive);
extern __int64 fileSize(wchar_t * filename);
extern void removebadchars(wchar_t *s);
extern wchar_t * fixReplacementVars(wchar_t *str, int str_size, Device * dev, songid_t song);
static INT_PTR CALLBACK prefs_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
int CopyFile(const wchar_t *infile, const wchar_t *outfile, void * callbackContext, void (*callback)(void * callbackContext, wchar_t * status), int * killswitch);
// called from ml_pmp
extern BOOL EjectVolume(TCHAR cDriveLetter);
void CopyAlbumArt(const wchar_t *source, const wchar_t *destination);

__int64 USBDevice::getDeviceCapacityAvailable()  // in bytes
{
	ULARGE_INTEGER tfree={0,}, total={0,}, freeb={0,};
	wchar_t path[4]=L"x:\\";
	path[0]=drive;
	GetDiskFreeSpaceEx(path,  &tfree, &total, &freeb);
	return freeb.QuadPart;
}

// called from ml_pmp
__int64 USBDevice::getDeviceCapacityTotal()
{
	// in bytes
	ULARGE_INTEGER tfree={0,}, total={0,}, freeb={0,};
	wchar_t path[4]=L"x:\\";
	path[0]=drive;
	GetDiskFreeSpaceEx(path,  &tfree, &total, &freeb);
	return total.QuadPart;
}

// called from ml_pmp
void USBDevice::Eject()
{
	// if you ejected successfully, you MUST call PMP_IPC_DEVICEDISCONNECTED and delete this
	for(size_t i=0; i < devices.size(); i++) 
	{
		USBDevice *device = devices.at(i);
		if (device == this)
		{
			if (EjectVolume(drive))
			{
				devices.erase(devices.begin() + i); 
				SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
				delete this;
				break;
			}
			else
			{
				wchar_t titleStr[128] = {0};
				MessageBox(plugin.hwndLibraryParent,WASABI_API_LNGSTRINGW(IDS_FAILED_TO_EJECT_DRIVE), WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,titleStr,128),0);
				break;
			}
		}
	}
}

// called from ml_pmp
void USBDevice::Close()
{
	// save any changes, and call PMP_IPC_DEVICEDISCONNECTED AND delete this
	for(size_t i=0; i < devices.size(); i++) 
	{
		if(((USBDevice*)devices.at(i)) == this)
		{
			devices.erase(devices.begin() + i); 
			SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICEDISCONNECTED);
			delete this;
			break;
		}
	}
} 

// called from ml_pmp
// return 0 for success, -1 for failed or cancelled
int USBDevice::transferTrackToDevice(const itemRecordW * track, // the track to transfer
									void * callbackContext,		//pass this to the callback
									void (*callback)(void *callbackContext, wchar_t *status),  // call this every so often so the GUI can be updated. Including when finished!
									songid_t * songid,			// fill in the songid when you are finished
									int * killswitch)			// if this gets set to anything other than zero, the transfer has been cancelled by the user
{
	wchar_t fn[MAX_PATH] = L"X:\\";
	lstrcpyn(fn, songFormat, MAX_PATH);
	fn[0] = drive;
	wchar_t * src = track->filename;
	wchar_t ext[10] = {0};
	wchar_t *e = wcsrchr(src,L'.');
	if (e) lstrcpyn(ext, e, 10);

	bool transcodefile = false;
	if (transcoder && transcoder->ShouldTranscode(src))
	{
		int r = transcoder->CanTranscode(src, ext);
		if (r != 0 && r != -1) transcodefile = true;
	}

	UsbSong *s = new UsbSong();
	lstrcpyn(s->filename, src, MAX_PATH);  //this will get written over, but for now we have this so that the user can keep the old filename

	fillMetaData(s); // TODO: benski> used cached info inside track (itemRecordW) if available
	fixReplacementVars(fn, MAX_PATH, this, (songid_t)s);

	StringCchCat(fn, MAX_PATH, ext); //place extension
	StringCchCopy(s->filename, MAX_PATH, fn);

	wchar_t * dir = wcsrchr(fn,L'\\');
	wchar_t * dir2 = wcsrchr(fn,L'/');
	wchar_t slash;
	if (dir2 > dir)
	{
		dir = dir2;
		slash=L'/';
	}
	else slash = L'\\';
	if (dir) *dir = 0;
	RecursiveCreateDirectory(fn);
	if (dir) *dir = slash;
	int r;
	if (transcodefile)
	{
		r = transcoder->TranscodeFile(src, fn, killswitch, callback, callbackContext);
	}
	else
	{
		r = CopyFile(src, fn, callbackContext, callback, killswitch);
	}

	if (r == 0)
	{
		// TODO: benski> do we need to update any fields from the transcoded filed?
		CopyAlbumArt(src, fn);
		writeRecordToDB(s);
		callback(callbackContext, WASABI_API_LNGSTRINGW(IDS_DONE));
		*songid = (songid_t)s;
	}
	else
	{
		callback(callbackContext, WASABI_API_LNGSTRINGW(IDS_TRANSFER_FAILED));
		delete s;
	}
	return r;
}

// called from ml_pmp
int USBDevice::trackAddedToTransferQueue(const itemRecordW *track)
{
	// return 0 to accept, -1 for "not enough space", -2 for "incorrect format"
	__int64 k = getTrackSizeOnDevice(track);
	if(!k) return -2;
	__int64 l = (__int64)k;
	__int64 avail = getDeviceCapacityAvailable();
	__int64 cmp = transferQueueLength;
	cmp += l;
	if(cmp > avail) return -1;
	else {
		transferQueueLength += l;
		return 0;
	}
} 

// called from ml_pmp
void USBDevice::trackRemovedFromTransferQueue(const itemRecordW *track)
{
	transferQueueLength -= (__int64)getTrackSizeOnDevice(track);
} 

// called from ml_pmp
// return the amount of space that will be taken up on the device by the track (once it has been tranferred)
// or 0 for incompatable. This is usually the filesize, unless you are transcoding. An estimate is acceptable.
__int64 USBDevice::getTrackSizeOnDevice(const itemRecordW *track)
{
	if(transcoder) {
		if(transcoder->ShouldTranscode(track->filename)) {
			int k = transcoder->CanTranscode(track->filename);
			if(k != -1 && k != 0) return k;
			return 0;
		} else return fileSize(track->filename);
	} else {
		if(!supportedFormat(track->filename,supportedFormats)) return 0;
		return fileSize(track->filename);
	}
} 

// called from ml_pmp
void USBDevice::deleteTrack(songid_t songid)
{
	// physically remove from device. Be sure to remove it from all the playlists!
	UsbSong * s = (UsbSong*)songid;

	//errno == 2 is ENOENT
	if(!_wunlink(s->filename) || errno == 2) { //will continue delete if file was deleted successfully or file path does not exist in the first place (errno==2)
		for(size_t i=0; i < usbPlaylists.size(); i++) {

			USBPlaylist * pl = usbPlaylists.at(i);
			size_t l = pl->songs.size();
			while(l--)
			{
				if(((UsbSong*)pl->songs.at(l)) == s) 
				{
					// remove the track and rewrite the playlist
					removeTrackFromPlaylist((int)i, (int)l);
				}
			}

			if(purgeFolders[0]=='1') 
			{
				RemoveDirectory(s->filename);
			}
		}
		delete (UsbSong*)songid;
	} else {
		char titleStr[32] = {0};
		MessageBoxA(plugin.hwndLibraryParent,WASABI_API_LNGSTRING(IDS_TRACK_IN_USE),
		WASABI_API_LNGSTRING_BUF(IDS_ERROR,titleStr,32),0);
	}
} 

// called from ml_pmp
// optional. Will be called at a good time to save changes
void USBDevice::commitChanges()
{
	//update cache
	tag();
	cacheUpToDate = true;
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)0,IPC_WRITE_EXTENDED_FILE_INFO);
}

// called from ml_pmp
int USBDevice::getPlaylistCount()
{
	// always at least 1. playlistnumber 0 is the Master Playlist containing all tracks.
	return (int)usbPlaylists.size();
} 

// called from ml_pmp
// PlaylistName(0) should return the name of the device.
void USBDevice::getPlaylistName(int playlistnumber, wchar_t *buf, int len)
{
	wchar_t * pathName = usbPlaylists.at(playlistnumber)->filename;
	if(playlistnumber != 0) 
	{
		if(pathName[0]) 
		{
			wchar_t * playlistName = PathFindFileNameW(pathName);
			lstrcpyn(buf,playlistName,len);
			PathRemoveExtension(buf);
		}
	}
	else //playlist number = 0 -> this is the device
	{
		if(pathName[0]) 
		{ //if we have a custom device name
			lstrcpyn(buf,pathName,len);
		}
		else 
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_USB_DRIVE_X,buf,len);
			wchar_t * x = wcsrchr(buf,L'X');
			if(x) *x = drive;
		}
	}
}

// called from ml_pmp
int USBDevice::getPlaylistLength(int playlistnumber)
{
	return (int)usbPlaylists.at(playlistnumber)->songs.size();
}

// called from ml_pmp
songid_t USBDevice::getPlaylistTrack(int playlistnumber,int songnum)
{
	// returns a songid
	return (songid_t) usbPlaylists.at(playlistnumber)->songs.at(songnum);
} 

// called from ml_pmp
void USBDevice::setPlaylistName(int playlistnumber, const wchar_t *buf)
{
	// with playlistnumber==0, set the name of the device.
	USBPlaylist * pl = usbPlaylists.at(playlistnumber);
	if(playlistnumber==0)
	{
		WritePrivateProfileString(L"pmp_usb",L"customName",buf,iniFile);
		lstrcpyn(pl->filename,buf,sizeof(pl->filename)/sizeof(wchar_t));
	}
	else 
	{
		wchar_t currentFilename[MAX_PATH] = {0};
		lstrcpynW(currentFilename, pl->filename, MAX_PATH);
		
		wchar_t * newFilename = const_cast<wchar_t *>(buf);
		if(wcslen(buf) >= MAX_PATH-1) newFilename[MAX_PATH-1]=0;
		while(newFilename && *newFilename && *newFilename == L'.') newFilename++;
		removebadchars(newFilename);
		StringCchPrintf(pl->filename,MAX_PATH,L"%s\\%s.m3u",pldir,newFilename);
		pl->filename[0]=drive;
		MoveFile(currentFilename, pl->filename);

		pl->dirty=true;
	}
} 

// called from ml_pmp
void USBDevice::playlistSwapItems(int playlistnumber, int posA, int posB)
{
	// swap the songs at position posA and posB
	USBPlaylist * pl = (USBPlaylist*)usbPlaylists.at(playlistnumber);
	UsbSong* a = pl->songs.at(posA);
	UsbSong* b = pl->songs.at(posB);
	pl->songs.insert(pl->songs.begin() + posA, b);
	pl->songs.erase(pl->songs.begin() + posA + 1);
	pl->songs.insert(pl->songs.begin() + posB, a);
	pl->songs.erase(pl->songs.begin() + posB + 1);
	pl->dirty = true;
}

// called from ml_pmp
void USBDevice::sortPlaylist(int playlistnumber, int sortBy)
{
// TODO: implement
}

// called from ml_pmp
void USBDevice::addTrackToPlaylist(int playlistnumber, songid_t songid)
{
	// adds songid to the end of the playlist
	UsbSong* song = (UsbSong *) songid;
	USBPlaylist * pl = (USBPlaylist*)usbPlaylists.at(playlistnumber);
	pl->songs.push_back(song);

	pl->dirty=true;
}

// called from ml_pmp
void USBDevice::removeTrackFromPlaylist(int playlistnumber, int songnum)
{
	//where songnum is the position of the track in the playlist
	USBPlaylist * pl = (USBPlaylist*)usbPlaylists.at(playlistnumber);
	pl->songs.erase(pl->songs.begin() + songnum);

	pl->dirty=true;
} 

// called from ml_pmp
void USBDevice::deletePlaylist(int playlistnumber)
{
	USBPlaylist * pl = (USBPlaylist*)usbPlaylists.at(playlistnumber);
	_wunlink(pl->filename);
	usbPlaylists.erase(usbPlaylists.begin() + playlistnumber);
	delete pl;
}

// called from ml_pmp
int USBDevice::newPlaylist(const wchar_t *name)
{
	// create empty playlist, returns playlistnumber. -1 for failed.
	wchar_t plname[MAX_PATH] = {0};
	StringCchCopy(plname, MAX_PATH, name);
	removebadchars(plname);
	wchar_t buff[MAX_PATH] = {0};
	StringCchPrintf(buff, MAX_PATH, L"%s\\%s.m3u",pldir,plname);

	USBPlaylist * pl = new USBPlaylist(*this, buff, false);
	pl->filename[0]=drive;

	//Lets save the playlist right away
	USBPlaylistSaver playlistSaver(pl->filename, L"autosaved", pl);
	playlistSaver.Save();

	usbPlaylists.push_back(pl);
	return (int)usbPlaylists.size()-1;
} 

// called from ml_pmp
void USBDevice::getTrackArtist(songid_t songid, wchar_t *buf, int len)
{
	UsbSong* song = (UsbSong*)songid;
	if (!song) return;

	buf[0] = L'\0';
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	StringCchCopy(buf, len, song->artist);
}

// called from ml_pmp
void USBDevice::getTrackAlbum(songid_t songid, wchar_t *buf, int len)
{
	UsbSong* song = (UsbSong*)songid;
	if (!song) return;
	
	buf[0] = L'\0';
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	StringCchCopy(buf, len, song->album);
}

// called from ml_pmp
void USBDevice::getTrackTitle(songid_t songid, wchar_t *buf, int len)
{
	UsbSong* song = (UsbSong*)songid;
	if (!song) return;
	
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	StringCchCopy(buf, len, song->title);
}

// called from ml_pmp
int USBDevice::getTrackTrackNum(songid_t songid)
{
	UsbSong* song = (UsbSong*)songid;
	if (!song) return 0;
	
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	return song->track;
}

// called from ml_pmp
int USBDevice::getTrackDiscNum(songid_t songid)
{
	UsbSong* song = (UsbSong*)songid;
	if (!song) return 0;
	
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	return song->discnum;
}

// called from ml_pmp
void USBDevice::getTrackGenre(songid_t songid, wchar_t * buf, int len)
{
	UsbSong* song = (UsbSong*)songid;
	if (!song) return;
	
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	StringCchCopy(buf, len, song->genre);
}

// called from ml_pmp
int USBDevice::getTrackYear(songid_t songid)
{
	UsbSong* song = (UsbSong*)songid;
	if (!song) return 0;
	
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	return song->year;
}

// called from ml_pmp
__int64 USBDevice::getTrackSize(songid_t songid)
{
	// in bytes
	UsbSong* song = (UsbSong*)songid;
	if (!song) return 0;
	
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	return song->size;
}

// called from ml_pmp
int USBDevice::getTrackLength(songid_t songid)
{
	// in millisecs
	UsbSong* song = (UsbSong*)songid;
	if (!song) return 0;
	
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	return song->length;
} 

// called from ml_pmp
int USBDevice::getTrackBitrate(songid_t songid)
{
	// in kbps
	UsbSong* song = (UsbSong*)songid;
	if (!song) return 0;
	
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	return song->bitrate;
} 

// called from ml_pmp
int USBDevice::getTrackPlayCount(songid_t songid)
{
	UsbSong* song = (UsbSong*)songid;
	if (!song) return 0;
	
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	return song->playcount;
}

// called from ml_pmp
int USBDevice::getTrackRating(songid_t songid)
{
	 //0-5
// TODO: implement
  return 0;
}

// called from ml_pmp
__time64_t USBDevice::getTrackLastPlayed(songid_t songid)
{
	// in unix time format
// TODO: implement
	return 0;

} 

// called from ml_pmp
__time64_t USBDevice::getTrackLastUpdated(songid_t songid)
{
	// in unix time format
// TODO: implement
	return 0;
} 

// called from ml_pmp
void USBDevice::getTrackAlbumArtist(songid_t songid, wchar_t *buf, int len)
{
	UsbSong* song = (UsbSong*)songid;
	if (!song) return;
	
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	StringCchCopy(buf, len, song->albumartist);
}

// called from ml_pmp
void USBDevice::getTrackPublisher(songid_t songid, wchar_t *buf, int len)
{
	UsbSong* song = (UsbSong*)songid;
	if (!song) return;
	
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	StringCchCopy(buf, len, song->publisher);
}

// called from ml_pmp
void USBDevice::getTrackComposer(songid_t songid, wchar_t *buf, int len)
{
	UsbSong* song = (UsbSong*)songid;
	if (!song) return;
	
	if(!loadedUpToDate)
	{
		this->fillMetaData(song);
	}
	StringCchCopy(buf, len, song->composer);
}

// called from ml_pmp
int USBDevice::getTrackType(songid_t songid)
{
// TODO: implement
 return 0;
}

// called from ml_pmp
void USBDevice::getTrackExtraInfo(songid_t songid, const wchar_t *field, wchar_t *buf, int len) 
{
// TODO: implement
//optional
} 

// called from ml_pmp
// feel free to ignore any you don't support
void USBDevice::setTrackArtist(songid_t songid, const wchar_t *value)
{
	UsbSong *song = (UsbSong *) songid;
	if (song)
	{
		updateTrackField(song, DEVICEVIEW_COL_ARTIST, value, FIELD_STRING);
		StringCchCopy(song->artist, FIELD_LENGTH, value);
		AGAVE_API_METADATA->SetExtendedFileInfo(song->filename, L"artist", value);
	}
}

// called from ml_pmp
void USBDevice::setTrackAlbum(songid_t songid, const wchar_t *value)
{
	UsbSong *song = (UsbSong *) songid;
	if (song)
	{
		updateTrackField(song, DEVICEVIEW_COL_ALBUM, value, FIELD_STRING);
		StringCchCopy(song->album, FIELD_LENGTH, value);
		AGAVE_API_METADATA->SetExtendedFileInfo(song->filename, L"album", value);
	}
}

// called from ml_pmp
void USBDevice::setTrackTitle(songid_t songid, const wchar_t *value)
{
	UsbSong *song = (UsbSong *) songid;
	if (song)
	{
		updateTrackField(song, DEVICEVIEW_COL_TITLE, value, FIELD_STRING);
		StringCchCopy(song->title, FIELD_LENGTH, value);
		AGAVE_API_METADATA->SetExtendedFileInfo(song->filename, L"title", value);
	}
}

// called from ml_pmp
void USBDevice::setTrackTrackNum(songid_t songid, int value)
{
	UsbSong *song = (UsbSong *) songid;

	if (song)
	{
		wchar_t track[FIELD_LENGTH] = {0};
		updateTrackField(song, DEVICEVIEW_COL_TRACK, &value, FIELD_INTEGER);
		song->track = value;
		StringCchPrintf(track, FIELD_LENGTH, L"%d", value);
		AGAVE_API_METADATA->SetExtendedFileInfo(song->filename, L"track", track);
	}
}

// called from ml_pmp
void USBDevice::setTrackDiscNum(songid_t songid, int value)
{
	UsbSong *song = (UsbSong *) songid;

	if (song)
	{
		wchar_t discNum[FIELD_LENGTH] = {0};
		updateTrackField(song, DEVICEVIEW_COL_DISC_NUMBER, &value, FIELD_INTEGER);
		song->discnum = value;
		StringCchPrintf(discNum, FIELD_LENGTH, L"%d", value);
		AGAVE_API_METADATA->SetExtendedFileInfo(song->filename, L"disc", discNum);
	}
}

// called from ml_pmp
void USBDevice::setTrackGenre(songid_t songid, const wchar_t *value)
{
	UsbSong *song = (UsbSong *) songid;
	if (song)
	{
		updateTrackField(song, DEVICEVIEW_COL_GENRE, value, FIELD_STRING);
		StringCchCopy(song->genre, FIELD_LENGTH, value);
		AGAVE_API_METADATA->SetExtendedFileInfo(song->filename, L"genre", value);
	}
}

// called from ml_pmp
void USBDevice::setTrackYear(songid_t songid, int year)
{
	UsbSong *song = (UsbSong *) songid;

	if (song)
	{
		wchar_t yearStr[FIELD_LENGTH] = {0};
		updateTrackField(song, DEVICEVIEW_COL_YEAR, &year, FIELD_INTEGER);
		song->year = year;
		StringCchPrintf(yearStr, FIELD_LENGTH, L"%d", year);
		AGAVE_API_METADATA->SetExtendedFileInfo(song->filename, L"year", yearStr);
	}
}

// called from ml_pmp
void USBDevice::setTrackPlayCount(songid_t songid, int value)
{
	UsbSong *song = (UsbSong *) songid;

	if (song)
	{
		wchar_t playCount[FIELD_LENGTH] = {0};
		updateTrackField(song, DEVICEVIEW_COL_PLAY_COUNT, &value, FIELD_INTEGER);
		song->playcount = value;
		StringCchPrintf(playCount, FIELD_LENGTH, L"%d", value);
		AGAVE_API_METADATA->SetExtendedFileInfo(song->filename, L"playcount", playCount);
	}
}

// called from ml_pmp
void USBDevice::setTrackRating(songid_t songid, int value)
{
	UsbSong *song = (UsbSong *) songid;

	if (song)
	{
		wchar_t rating[FIELD_LENGTH] = {0};
		updateTrackField(song, DEVICEVIEW_COL_PLAY_COUNT, &value, FIELD_INTEGER);
		song->playcount = value;
		StringCchPrintf(rating, FIELD_LENGTH, L"%d", value);
		AGAVE_API_METADATA->SetExtendedFileInfo(song->filename, L"rating", rating);
	}
}

// called from ml_pmp
void USBDevice::setTrackLastPlayed(songid_t songid, __time64_t value)
{
// TODO: implement

} // in unix time format

// called from ml_pmp
void USBDevice::setTrackLastUpdated(songid_t songid, __time64_t value)
{
// TODO: implement

} // in unix time format

// called from ml_pmp
void USBDevice::setTrackAlbumArtist(songid_t songid, const wchar_t *value)
{
	UsbSong *song = (UsbSong *) songid;
	if (song)
	{
		updateTrackField(song, DEVICEVIEW_COL_ALBUM_ARTIST, value, FIELD_STRING);
		StringCchCopy(song->albumartist, FIELD_LENGTH, value);
		AGAVE_API_METADATA->SetExtendedFileInfo(song->filename, L"albumartist", value);
	}
}

// called from ml_pmp
void USBDevice::setTrackPublisher(songid_t songid, const wchar_t *value)
{
	UsbSong *song = (UsbSong *) songid;
	if (song)
	{
		updateTrackField(song, DEVICEVIEW_COL_PUBLISHER, value, FIELD_STRING);
		StringCchCopy(song->publisher, FIELD_LENGTH, value);
		AGAVE_API_METADATA->SetExtendedFileInfo(song->filename, L"publisher", value);
	}
}

// called from ml_pmp
void USBDevice::setTrackComposer(songid_t songid, const wchar_t *value)
{
	UsbSong *song = (UsbSong *) songid;
	if (song)
	{
		updateTrackField(song, DEVICEVIEW_COL_COMPOSER, value, FIELD_STRING);
		StringCchCopy(song->composer, FIELD_LENGTH, value);
		AGAVE_API_METADATA->SetExtendedFileInfo(song->filename, L"composer", value);
	}
}

// called from ml_pmp
void USBDevice::setTrackExtraInfo(songid_t songid, const wchar_t *field, const wchar_t *value) 
{
// TODO: implement

} //optional

// called from ml_pmp
bool USBDevice::playTracks(songid_t * songidList, int listLength, int startPlaybackAt, bool enqueue)
{
	// return false if unsupported
	if(!enqueue) //clear playlist
	{ 
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_DELETE);
	}

	for(int i=0; i<listLength; i++) 
	{
		UsbSong *curSong = (UsbSong*)songidList[i];

		if (curSong)
		{
			enqueueFileWithMetaStructW s={0};
			s.filename = _wcsdup(curSong->filename);
			s.title    = _wcsdup(curSong->title);
			s.ext      = NULL;
			s.length   = curSong->length/1000;

			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW);
		}
		else
		{
			char titleStr[32] = {0};
			MessageBoxA(plugin.hwndWinampParent,WASABI_API_LNGSTRING(IDS_CANNOT_OPEN_FILE),
						WASABI_API_LNGSTRING_BUF(IDS_ERROR,titleStr,32),0);
		}
	}

	if(!enqueue) 
	{ 
		//play item startPlaybackAt
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,startPlaybackAt,IPC_SETPLAYLISTPOS);
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); //stop
		SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0); //play
	}
	return true;
}

// called from ml_pmp
intptr_t USBDevice::extraActions(intptr_t param1, intptr_t param2, intptr_t param3,intptr_t param4)
{
	switch(param1) 
	{
	case DEVICE_SET_ICON:
		{
			MLTREEIMAGE * i = (MLTREEIMAGE*)param2;
			i->hinst = plugin.hDllInstance;
			switch(devType) 
			{
			case TYPE_PSP: 
				i->resourceId = IDR_PSP_ICON; 
				break;
			default:       
				i->resourceId = IDR_USB_ICON;
				break;
			}
		}
		break;
	case DEVICE_GET_ICON:
		{
			if (param2 <= 16 && param3 <= 16)
			{
				// TODO: get the name of the DLL at load time 
				StringCchPrintfW((wchar_t *)param4, 260, L"res://%s/PNG/#%u", L"pmp_usb.dll", (devType==TYPE_PSP)?IDR_PSP_ICON:IDR_USB_ICON);
			}

		}
		break;

	case DEVICE_SUPPORTED_METADATA:
		{
			intptr_t supported = SUPPORTS_ARTIST | SUPPORTS_ALBUM | SUPPORTS_TITLE | SUPPORTS_TRACKNUM | SUPPORTS_DISCNUM | SUPPORTS_GENRE | 
				SUPPORTS_YEAR | SUPPORTS_SIZE | SUPPORTS_LENGTH | SUPPORTS_BITRATE | SUPPORTS_LASTUPDATED | SUPPORTS_ALBUMARTIST | 
				SUPPORTS_COMPOSER | SUPPORTS_PUBLISHER | SUPPORTS_ALBUMART;
			return supported;
		}
		break;

	case DEVICE_CAN_RENAME_DEVICE:
		return 1;

	case DEVICE_GET_INI_FILE:
		StringCchCopy((wchar_t*)param2, MAX_PATH, iniFile);
		break;
	case DEVICE_GET_PREFS_DIALOG:
		if(param3 == 0) 
		{
			pref_tab * p = (pref_tab *)param2;
			p->hinst = WASABI_API_LNG_HINST;
			p->dlg_proc = prefs_dialogProc;
			p->res_id = IDD_CONFIG;
			WASABI_API_LNGSTRINGW_BUF(IDS_ADVANCED,p->title,100);
		}
		break;
	case DEVICE_DONE_SETTING:
		UsbSong * song = (UsbSong *) param2;
		AGAVE_API_METADATA->WriteExtendedFileInfo(song->filename);
		return true;
		break;
	}
	return false;
}

// called from ml_pmp
bool USBDevice::copyToHardDriveSupported()
{
	return true;
}

// called from ml_pmp
__int64 USBDevice::songSizeOnHardDrive(songid_t song)
{
	// how big a song will be when copied back. Return -1 for not supported.
// TODO: implement
	return 0;

} 

// called from ml_pmp
int USBDevice::copyToHardDrive(songid_t song, // the song to copy
								wchar_t * path, // path to copy to, in the form "c:\directory\song". The directory will already be created, you must append ".mp3" or whatever to this string! (there is space for at least 10 new characters).
								void * callbackContext, //pass this to the callback
								void (*callback)(void * callbackContext, wchar_t * status),  // call this every so often so the GUI can be updated. Including when finished!
								int * killswitch // if this gets set to anything other than zero, the transfer has been cancelled by the user
								)
{
	// -1 for failed/not supported. 0 for success.
	UsbSong* track = (UsbSong*)song;
	wchar_t * ext = PathFindExtensionW(track->filename);
	if(ext && (lstrlen(ext)<10)) StringCchCat(path,MAX_PATH, ext); // append correct extention
	return CopyFile(track->filename,path,callbackContext, callback, killswitch);
} 

// called from ml_pmp
// art functions
void USBDevice::setArt(songid_t songid, void *buf, int w, int h)
{
	//buf is in format ARGB32*
	// TODO: implement
}

// called from ml_pmp
pmpart_t USBDevice::getArt(songid_t songid)
{
	UsbSong *song = (UsbSong *)songid;
	ARGB32 *bits;
	int w, h;
	if (AGAVE_API_ALBUMART && AGAVE_API_ALBUMART->GetAlbumArt(song->filename, L"cover", &w, &h, &bits) == ALBUMART_SUCCESS && bits)
	{
		return (pmpart_t) new USBArt(bits, w, h);
	}
	return 0;
}

// called from ml_pmp
void USBDevice::releaseArt(pmpart_t art)
{
	USBArt *image = (USBArt *)art;
	delete image;
}

// called from ml_pmp
int USBDevice::drawArt(pmpart_t art, HDC dc, int x, int y, int w, int h)
{
	USBArt *image = (USBArt *)art;
	if (image)
	{
		HQSkinBitmap temp(image->bits, image->w, image->h); // wrap into a SkinBitmap (no copying involved)
		DCCanvas canvas(dc);
		temp.stretch(&canvas,x,y,w,h);
		return 1;
	}
	return 0;
}

// called from ml_pmp
void USBDevice::getArtNaturalSize(pmpart_t art, int *w, int *h)
{
	// TODO: implement
	USBArt *image = (USBArt *)art;
	if (image)
	{
		*w = image->w;
		*h = image->h;
	}
}

// called from ml_pmp
void USBDevice::setArtNaturalSize(pmpart_t art, int w, int h)
{
	// TODO: implement
}

// called from ml_pmp
void USBDevice::getArtData(pmpart_t art, void* data)
{
	USBArt *image = (USBArt *)art;
	if (image)
		memcpy(data, image->bits, image->w*image->h*sizeof(ARGB32));
	// data ARGB32* is at natural size
} 

// called from ml_pmp
bool USBDevice::artIsEqual(pmpart_t a, pmpart_t b)
{
	if (a == b)
		return true;
	// TODO: implement
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Initialize class statics
nde_database_t USBDevice::discDB = 0;

// The getter that returns the master playlist
// the playlist vector always carries a master playlist
USBPlaylist* USBDevice::getMasterPlaylist()
{
	for (std::vector<USBPlaylist*>::const_iterator e = usbPlaylists.begin(); e != usbPlaylists.end(); e++) 
	{
		USBPlaylist* playlist = (*e);
		if (playlist->isMaster()) return playlist;
	}
	return NULL;
}

// constructor
USBDevice::USBDevice(wchar_t drive, pmpDeviceLoading * load): transcoder(NULL) {
	deviceTable = 0;
	StringCchPrintf(ndeDataFile, 100, L"%c:\\winamp_metadata.dat", drive);
	StringCchPrintf(ndeIndexFile, 100, L"%c:\\winamp_metadata.idx", drive);
	load->dev = this;
	load->UpdateCaption = NULL;
	//pass load to ml_pmp, ml updates load->UpdateCaption and context
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)load,PMP_IPC_DEVICELOADING);
	if(load->UpdateCaption) {
		wchar_t buf[100] = L"";
		WASABI_API_LNGSTRINGW_BUF(IDS_LOADING_DRIVE_X,buf,100);
		wchar_t * x = wcsrchr(buf,L'X');
		if(x) *x = drive;
		load->UpdateCaption(buf,load->context);
	}
	
	devType = detectDeviceType(drive);
	// load settings
	StringCchCopy(iniFile,MAX_PATH,L"x:\\pmp_usb.ini");
	iniFile[0]=drive;

	wchar_t customName[FIELD_LENGTH] = {0};
	GetPrivateProfileString(L"pmp_usb",L"pldir",devType==TYPE_PSP?L"X:\\PSP\\MUSIC":L"X:",pldir,sizeof(pldir)/sizeof(wchar_t),iniFile);
	GetPrivateProfileString(L"pmp_usb",L"songFormat",devType==TYPE_PSP?L"X:\\PSP\\MUSIC\\<Artist> - <Album>\\## - <Title>":L"X:\\<Artist>\\<Album>\\## - <Title>",songFormat,sizeof(songFormat)/sizeof(wchar_t),iniFile);
	GetPrivateProfileString(L"pmp_usb",L"supportedFormats",L"mp3;wav;wma;m4a;aac;ogg;flac",supportedFormats,sizeof(supportedFormats)/sizeof(wchar_t),iniFile);
	GetPrivateProfileString(L"pmp_usb",L"purgeFolders",L"1",purgeFolders,sizeof(purgeFolders)/sizeof(wchar_t),iniFile);
	GetPrivateProfileString(L"pmp_usb",L"customName",devType==TYPE_PSP?L"Sony PSP":L"",customName,sizeof(customName)/sizeof(wchar_t),iniFile);
	pl_write_mode = GetPrivateProfileInt(L"pmp_usb",L"pl_write_mode",0,iniFile);
	pldir[0] = drive;
	songFormat[0] = drive;

	transferQueueLength = 0;
	this->drive = drive;
	USBPlaylist * mpl = new USBPlaylist(*this, customName, true);
	usbPlaylists.push_back(mpl);
	wchar_t * pl = _wcsdup(pldir);
	pl[0] = drive;
	RecursiveCreateDirectory(pl);
	wchar_t root[3] = L"X:";
	root[0] = drive;

	openDeviceTable();

	fileProbe(root);

	// sort out and read playlists....
	if (WASABI_API_PLAYLISTMNGR != NULL && WASABI_API_PLAYLISTMNGR != (api_playlistmanager *)1)
	{
		for (std::vector<USBPlaylist*>::const_iterator e = usbPlaylists.begin(); e != usbPlaylists.end(); e++) 
		{
			USBPlaylist* playlist = (*e);
			if (playlist && playlist->isMaster() == false)
			{
				WASABI_API_PLAYLISTMNGR->Load(playlist->getFilename(), playlist);
			}
		}
	}

	tag();
	devices.push_back(this);
	extern HWND config;
	if(config) PostMessage(config,WM_USER,0,0);

	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)this,PMP_IPC_DEVICECONNECTED);
	setupTranscoder();
	loading_devices[drive-'A']=false;
}

USBDevice::USBDevice() : transcoder(NULL), drive(0), devType(TYPE_OTHER), pl_write_mode(0), transferQueueLength(0), cacheUpToDate(false), loadedUpToDate(false)
{
	deviceTable = 0;
	ZeroMemory(ndeDataFile, sizeof(ndeDataFile));
	ZeroMemory(ndeIndexFile, sizeof(ndeIndexFile));

	ZeroMemory(iniFile, sizeof(iniFile));
	ZeroMemory(pldir, sizeof(pldir));
	ZeroMemory(songFormat, sizeof(songFormat));
	ZeroMemory(supportedFormats, sizeof(supportedFormats));
	ZeroMemory(purgeFolders, sizeof(purgeFolders));
}

USBDevice::~USBDevice() 
{
	closeDeviceTable();
	if (transcoder) SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)transcoder,PMP_IPC_RELEASE_TRANSCODER);
}

//read files from device's folder 'indir'
void USBDevice::fileProbe(wchar_t * indir) {
	wchar_t dir[MAX_PATH] = {0};
	WIN32_FIND_DATA FindFileData = {0};
	StringCchPrintf(dir,MAX_PATH,L"%s\\*",indir);
	HANDLE hFind = FindFirstFile(dir, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) return;
	do {
		if(wcscmp(FindFileData.cFileName,L".") && wcscmp(FindFileData.cFileName,L"..")) {
			wchar_t fullfile[MAX_PATH] = {0};
			StringCchPrintf(fullfile,MAX_PATH,L"%s\\%s",indir,FindFileData.cFileName);
			
			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //file is directory
			{ 
				fileProbe(fullfile);	//call until we have found a file
			} 
			else // found a file!
			{ 
				wchar_t * ext = wcsrchr(FindFileData.cFileName,'.'); 
				if(!ext) continue; //no files with extensions in the directory
				ext++;
				if(!_wcsicmp(ext,L"m3u")) // its a playlist
				{ 
					USBPlaylist *playlist = new USBPlaylist(*this, fullfile, false);
					usbPlaylists.push_back(playlist);
					continue;
				}	//its a file
				if(supportedFormat(fullfile,supportedFormats)) //check extension
				{
					UsbSong *s = new UsbSong(); 
					lstrcpynW(s->filename, fullfile, MAX_PATH);
					this->getMasterPlaylist()->songs.push_back(s); //add track to alltrack list (playlist 0)
				}
			}
		}
	} while(FindNextFile(hFind, &FindFileData) != 0);
	FindClose(hFind);
}

int USBDevice::getFileInfoW(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, size_t len)
{
	dest[0]=0;
	return AGAVE_API_METADATA->GetExtendedFileInfo(filename, metadata, dest, len);
}

// read all metadata from the metadata wasabi service
void USBDevice::fillMetaData(UsbSong *t)
{
	if (!t->filled)
	{
		wchar_t tmp[1024] = {0};
		if (getFileInfoW(t->filename,L"artist",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			StringCchCopyW(t->artist, FIELD_LENGTH, tmp);
			t->filled = true;
		}

		if (getFileInfoW(t->filename,L"title",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			StringCchCopyW(t->title, FIELD_LENGTH, tmp);
			t->filled = true;
		}

		if (getFileInfoW(t->filename,L"album",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			StringCchCopyW(t->album, FIELD_LENGTH, tmp);
			t->filled = true;
		}

		if (getFileInfoW(t->filename,L"composer",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			StringCchCopyW(t->composer, FIELD_LENGTH, tmp);
			t->filled = true;
		}

		if (getFileInfoW(t->filename,L"publisher",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			StringCchCopyW(t->publisher, FIELD_LENGTH, tmp);
			t->filled = true;
		}

		if (getFileInfoW(t->filename,L"albumartist",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			StringCchCopyW(t->albumartist, FIELD_LENGTH, tmp);
			t->filled = true;
		}

		if (getFileInfoW(t->filename, L"length",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			t->length = _wtoi(tmp);
			t->filled = true;
		}

		if (getFileInfoW(t->filename, L"track",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			t->track = _wtoi(tmp);
			t->filled = true;
		}

		if (getFileInfoW(t->filename, L"disc",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			t->discnum = _wtoi(tmp);
			t->filled = true;
		}

		if (getFileInfoW(t->filename, L"genre",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			StringCchCopyW(t->genre, FIELD_LENGTH, tmp);
			t->filled = true;
		}

		if (getFileInfoW(t->filename, L"year",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			if (!wcsstr(tmp,L"__") && !wcsstr(tmp,L"/") && !wcsstr(tmp,L"\\") && !wcsstr(tmp,L"."))
			{
				wchar_t *p = tmp;
				while (p && *p)
				{
					if (*p == L'_') *p=L'0';
					p++;
				}
				t->year = _wtoi(tmp);
				t->filled = true;
			}
		}

		if (getFileInfoW(t->filename, L"bitrate",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			t->bitrate = _wtoi(tmp);
			t->filled = true;
		}

		if (getFileInfoW(t->filename, L"size",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			t->size = _wtoi(tmp);
			t->filled = true;
		}
		else
		{
			t->size = fileSize(t->filename);
			t->filled = true;
		}

		if (getFileInfoW(t->filename, L"playcount",tmp,sizeof(tmp)/sizeof(wchar_t)) && tmp[0])
		{
			t->playcount = _wtoi(tmp);
			t->filled = true;
		}
	}
}

int USBDevice::openDeviceDatabase() 
{
	Nullsoft::Utility::AutoLock lock(dbcs);
	if (!discDB) 
	{
		discDB = NDE_CreateDatabase(plugin.hDllInstance);
	}
	return NDE_USB_SUCCESS;
}

void USBDevice::createDeviceFields() 
{
	// create defaults
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_FILENAME, L"filename", FIELD_FILENAME);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_ARTIST, L"artist", FIELD_STRING);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_ALBUM, L"album", FIELD_STRING);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_TITLE, L"title", FIELD_STRING);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_GENRE, L"genre", FIELD_STRING);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_ALBUM_ARTIST, L"albumartist", FIELD_STRING);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_PUBLISHER, L"publisher", FIELD_STRING);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_COMPOSER, L"composer", FIELD_STRING);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_YEAR, L"year", FIELD_INTEGER);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_TRACK, L"track", FIELD_INTEGER);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_BITRATE, L"bitrate", FIELD_INTEGER);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_DISC_NUMBER, L"discnumber", FIELD_INTEGER);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_LENGTH, L"length", FIELD_INTEGER);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_SIZE, L"size", FIELD_INTEGER);
	NDE_Table_NewColumnW(deviceTable, DEVICEVIEW_COL_PLAY_COUNT, L"playcount", FIELD_INTEGER);
	NDE_Table_PostColumns(deviceTable);
	NDE_Table_AddIndexByIDW(deviceTable, 0, L"filename");
}

int USBDevice::openDeviceTable() {
	Nullsoft::Utility::AutoLock lock(dbcs);
	int ret = openDeviceDatabase();
	if (ret != NDE_USB_SUCCESS)
		return ret;

	if (!deviceTable)
	{
		deviceTable = NDE_Database_OpenTable(discDB, ndeDataFile, ndeIndexFile,NDE_OPEN_ALWAYS,NDE_CACHE); 
		if (deviceTable) {
			createDeviceFields();
		}
	}
	return deviceTable?NDE_USB_SUCCESS:NDE_USB_FAILURE;
}

void USBDevice::closeDeviceTable()
{
	if (deviceTable)
	{
		NDE_Table_Sync(deviceTable);
		NDE_Database_CloseTable(discDB, deviceTable);
		deviceTable=0;
	}

}

void USBDevice::CloseDatabase()
{
	if (discDB)
		NDE_DestroyDatabase(discDB);
	discDB=0;
}

static void db_setFieldInt(nde_scanner_t s, unsigned char id, int data)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (!f)	f = NDE_Scanner_NewFieldByID(s, id);
	NDE_IntegerField_SetValue(f, data);
}

static void db_setFieldString(nde_scanner_t s, unsigned char id, const wchar_t *data)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (!f)	f = NDE_Scanner_NewFieldByID(s, id);
	NDE_StringField_SetString(f, data);
}

static void db_removeField(nde_scanner_t s, unsigned char id)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (f)
	{
		NDE_Scanner_DeleteField(s, f);
	}
}

static int db_getFieldInt(nde_scanner_t s, unsigned char id, int defaultVal)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (f)
	{
		return NDE_IntegerField_GetValue(f);
	}
	else
	{
		return defaultVal;
	}
}

static wchar_t* db_getFieldString(nde_scanner_t s, unsigned char id)
{
	nde_field_t f = NDE_Scanner_GetFieldByID(s, id);
	if (f)
	{
		return NDE_StringField_GetString(f);
	}
	else
	{
		return 0;
	}
}

void USBDevice::refreshNDECache(void) {
	tag();
	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)0,IPC_WRITE_EXTENDED_FILE_INFO);
}

int filenamecmp(const wchar_t *f1, const wchar_t *f2)
{
	for (;;)
	{
		wchar_t c1 = *f1++;
		wchar_t c2 = *f2++;
		if (!c1 && !c2)
			return 0;
		if (!c1)
			return -1;
		if (!c2)
			return 1;
		c1 = towupper(c1);
		c2 = towupper(c2);
		if (c1 == '\\')
			c1 = '/';
		if (c2 == '\\')
			c2 = '/';
		if (c1 < c2)
			return -1;
		else if (c1 > c2)
			return 1;
	}
}
//
UsbSong *USBDevice::findSongInMasterPlaylist(const wchar_t *songfn) {
	USBPlaylist* mpl = this->getMasterPlaylist();
	for (std::vector<UsbSong*>::const_iterator e = mpl->songs.begin(); e != mpl->songs.end(); e++) 
	{
		if (filenamecmp(songfn, (*e)->filename) == 0) {
			return (*e);
		}
	}
	return NULL;
}

void USBDevice::tag(void) 
{
	/**
		loop thru the newly probed disk
		check for updates on each of the songs
		if there is an update or if metadata does not exist for the file, re-read the metadata
		if there is no update and the song is found in the master playlist, just read from the db
	*/
	USBPlaylist *mpl = this->getMasterPlaylist();
	int top = (int)mpl->songs.size();

	//first load in all songs data from ID3 - this is what we were trying to avoid
	for(int i = 0; i < top; i++) 
	{
		UsbSong *t = (UsbSong *)mpl->songs.at(i);

		// now check if this song has changed
		// check if the nde cache exists in the first place
		if (songChanged(t))
		{
			this->fillMetaData(t);
			// now since we've refreshed the metadata write to NDE
			this->writeRecordToDB(t);
		}
		else
		{
			// read the record from NDE
			if (this->readRecordFromDB(t) == false)
			{
				this->fillMetaData(t);
				this->writeRecordToDB(t);
			}
		}
	}

	for (size_t i=0;i<usbPlaylists.size();i++)
	{
		USBPlaylist *pl = usbPlaylists[i];
		if (pl->dirty)
		{
			// Lets delete the current playlist file
			_wunlink(pl->filename);

			USBPlaylistSaver playlistSaver(pl->filename, L"autosaved", pl);
			playlistSaver.Save();
			pl->dirty = false;
		}
	}

	cacheUpToDate = true; 
	loadedUpToDate = true;
}

// check change in filetimes for the song
bool USBDevice::songChanged(UsbSong* song)
{
	if (!song) return true;
	if (!PathFileExists(ndeDataFile)) return true;

	//For fLastAccess/LastWrite information, use GetFileAttributesEx
	WIN32_FILE_ATTRIBUTE_DATA cacheFileInfo, tempInfo;
	GetFileAttributesExW(ndeDataFile, GetFileExInfoStandard, (LPVOID)&cacheFileInfo);

	if(song->filename)
	{
		GetFileAttributesExW(song->filename, GetFileExInfoStandard, (LPVOID)&tempInfo);
	}
	else
	{
		return true;
	}

	//cachetime - song time
	if(CompareFileTime(&cacheFileInfo.ftLastWriteTime, &tempInfo.ftLastWriteTime) < 0) 
	{
		return true;
	}
	return false;
}

// read metadata for a specific song from the NDE cache
bool USBDevice::readRecordFromDB(UsbSong* song)
{
	if (!song) return false;

	Nullsoft::Utility::AutoLock lock(dbcs);
	openDeviceTable();
	nde_scanner_t scanner = NDE_Table_CreateScanner(deviceTable);

	if (NDE_Scanner_LocateFilename(scanner, DEVICEVIEW_COL_FILENAME, FIRST_RECORD, song->filename))
	{
		nde_field_t artist = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_ARTIST);
		wchar_t* artistString = NDE_StringField_GetString(artist);
		lstrcpyn(song->artist, artistString, FIELD_LENGTH);

		nde_field_t album = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_ALBUM);
		wchar_t* albumString = NDE_StringField_GetString(album);
		lstrcpyn(song->album, albumString, FIELD_LENGTH);

		nde_field_t albumArtist = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_ALBUM_ARTIST);
		wchar_t* albumArtistString = NDE_StringField_GetString(albumArtist);
		lstrcpyn(song->albumartist, albumArtistString, FIELD_LENGTH);

		nde_field_t publisher = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_PUBLISHER);
		wchar_t* publisherString = NDE_StringField_GetString(publisher);
		lstrcpyn(song->publisher, publisherString, FIELD_LENGTH);

		nde_field_t composer = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_COMPOSER);
		wchar_t* composerString = NDE_StringField_GetString(composer);
		lstrcpyn(song->composer, composerString, FIELD_LENGTH);

		nde_field_t title = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_TITLE);
		wchar_t* titleString = NDE_StringField_GetString(title);
		lstrcpyn(song->title, titleString, FIELD_LENGTH);

		nde_field_t genre = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_GENRE);
		wchar_t* genreString = NDE_StringField_GetString(genre);
		lstrcpyn(song->genre, genreString, FIELD_LENGTH);

		nde_field_t track = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_TRACK);
		int trackInt= NDE_IntegerField_GetValue(track);
		song->track = trackInt;

		nde_field_t year = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_YEAR);
		int yearInt= NDE_IntegerField_GetValue(year);
		song->year = yearInt;

		nde_field_t discNumber = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_DISC_NUMBER);
		int discNumberInt= NDE_IntegerField_GetValue(discNumber);
		song->discnum = discNumberInt;

		nde_field_t length = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_LENGTH);
		int lengthInt= NDE_IntegerField_GetValue(length);
		song->length = lengthInt;

		nde_field_t bitrate = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_BITRATE);
		int bitrateInt= NDE_IntegerField_GetValue(bitrate);
		song->bitrate = bitrateInt;

		nde_field_t size = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_SIZE);
		int sizeInt= NDE_IntegerField_GetValue(size);
		song->size = sizeInt;

		nde_field_t playcount = NDE_Scanner_GetFieldByID(scanner, DEVICEVIEW_COL_PLAY_COUNT);
		int playcountInt = NDE_IntegerField_GetValue(playcount);
		song->playcount = playcountInt;
	}
	else
	{
		return false;
	}

	NDE_Table_DestroyScanner(deviceTable, scanner);
	//closeDeviceTable();
	return true;
}



// write a single record to the nde database
void USBDevice::writeRecordToDB(UsbSong* songToPrint) {
	Nullsoft::Utility::AutoLock lock(dbcs);
	openDeviceTable();
	nde_scanner_t s = NDE_Table_CreateScanner(deviceTable);

	if (! NDE_Scanner_LocateFilename(s, DEVICEVIEW_COL_FILENAME, FIRST_RECORD, songToPrint->filename))
	{
		NDE_Scanner_New(s);
	}

	if (songToPrint->filename) 
	{
		db_setFieldString(s, DEVICEVIEW_COL_FILENAME, songToPrint->filename);
	}

	if (songToPrint->artist) 
	{
		db_setFieldString(s, DEVICEVIEW_COL_ARTIST, songToPrint->artist);
	}

	if (songToPrint->albumartist) 
	{
		db_setFieldString(s, DEVICEVIEW_COL_ALBUM_ARTIST, songToPrint->albumartist);
	}

	if (songToPrint->publisher) 
	{
		db_setFieldString(s, DEVICEVIEW_COL_PUBLISHER, songToPrint->publisher);
	}

	if (songToPrint->composer) 
	{
		db_setFieldString(s, DEVICEVIEW_COL_COMPOSER, songToPrint->composer);
	}

	if (songToPrint->album) 
	{
		db_setFieldString(s, DEVICEVIEW_COL_ALBUM, songToPrint->album);
	}

	if (songToPrint->title) 
	{
		db_setFieldString(s, DEVICEVIEW_COL_TITLE, songToPrint->title);
	}

	if (songToPrint->genre) 
	{
		db_setFieldString(s, DEVICEVIEW_COL_GENRE, songToPrint->genre);
	}

	if (songToPrint->year) 
	{
		db_setFieldInt(s, DEVICEVIEW_COL_YEAR, songToPrint->year);
	}

	if (songToPrint->track) 
	{
		db_setFieldInt(s, DEVICEVIEW_COL_TRACK, songToPrint->track);
	}

	if (songToPrint->bitrate) 
	{
		db_setFieldInt(s, DEVICEVIEW_COL_BITRATE, songToPrint->bitrate);
	}

	if (songToPrint->discnum) 
	{
		db_setFieldInt(s, DEVICEVIEW_COL_DISC_NUMBER, songToPrint->discnum);
	}

	if (songToPrint->length) 
	{
		db_setFieldInt(s, DEVICEVIEW_COL_LENGTH, songToPrint->length);
	}

	if (songToPrint->size) 
	{
		db_setFieldInt(s, DEVICEVIEW_COL_SIZE, (int)songToPrint->size);
	}

	if (songToPrint->playcount) 
	{
		db_setFieldInt(s, DEVICEVIEW_COL_PLAY_COUNT, songToPrint->playcount);
	}
	NDE_Scanner_Post(s);
	NDE_Table_DestroyScanner(deviceTable, s);
	//NDE_Table_Sync(deviceTable);
	//closeDeviceTable();
}

void USBDevice::setupTranscoder() {
  if(transcoder) SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)transcoder,PMP_IPC_RELEASE_TRANSCODER);
  transcoder = (Transcoder*)SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)this,PMP_IPC_GET_TRANSCODER);
  if(!transcoder) return;
  
  wchar_t * p = supportedFormats;
	while(p && *p) {
		wchar_t * np = wcschr(p,L';');
		if(np) *np = 0;
		transcoder->AddAcceptableFormat(p);
		if(np) { *np = L';'; p=np+1; } 
		else return;
	}
}

BOOL CALLBACK browseEnumProc(HWND hwnd, LPARAM lParam)
{
	wchar_t cl[32] = {0};
	GetClassNameW(hwnd, cl, ARRAYSIZE(cl));
	if (!lstrcmpiW(cl, WC_TREEVIEW))
	{
		PostMessage(hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection(hwnd));
		return FALSE;
	}

	return TRUE;
}

wchar_t pldir[MAX_PATH] = {0};
int CALLBACK WINAPI BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED)
	{
		SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)pldir);

		// this is not nice but it fixes the selection not working correctly on all OSes
		EnumChildWindows(hwnd, browseEnumProc, 0);
	}
	return 0;
}

static INT_PTR CALLBACK prefs_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
  static USBDevice * dev;
  switch(uMsg) {
	case WM_INITDIALOG:
		{
			prefsParam* p = (prefsParam*)lParam;
			dev = (USBDevice*)p->dev;
			p->config_tab_init(hwndDlg,p->parent);
			SetDlgItemTextW(hwndDlg,IDC_NAMEFORMAT,dev->songFormat);
			SetDlgItemTextW(hwndDlg,IDC_PLDIR,dev->pldir);
			SetDlgItemTextW(hwndDlg,IDC_SUPPORTEDFORMATS,dev->supportedFormats);
			if(dev->purgeFolders[0]=='1') CheckDlgButton(hwndDlg,IDC_PURGEFOLDERS,BST_CHECKED);
			else CheckDlgButton(hwndDlg,IDC_PURGEFOLDERS,BST_UNCHECKED);

			SendDlgItemMessageW(hwndDlg,IDC_PL_WRITE_COMBO,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_SLASH_AT_START));
			SendDlgItemMessageW(hwndDlg,IDC_PL_WRITE_COMBO,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_DOT_AT_START));
			SendDlgItemMessageW(hwndDlg,IDC_PL_WRITE_COMBO,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_NO_SLASH_OR_DOT));
			SendDlgItemMessage(hwndDlg,IDC_PL_WRITE_COMBO,CB_SETCURSEL,dev->pl_write_mode,0);
			SetDlgItemTextW(hwndDlg,IDC_PL_WRITE_EG,WASABI_API_LNGSTRINGW(IDS_EG_SLASH+dev->pl_write_mode));
		}
	break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{
			case IDC_NAMEFORMAT:
				if(HIWORD(wParam)==EN_CHANGE) 
				{
					GetDlgItemTextW(hwndDlg,IDC_NAMEFORMAT,dev->songFormat,sizeof(dev->songFormat)/sizeof(wchar_t));
					WritePrivateProfileStringW(L"pmp_usb",L"songFormat",dev->songFormat,dev->iniFile);
				}
			break;
			
			case IDC_PLDIR:
				if(HIWORD(wParam)==EN_CHANGE) 
				{
					GetDlgItemTextW(hwndDlg,IDC_PLDIR,dev->pldir,sizeof(dev->pldir)/sizeof(wchar_t));
					WritePrivateProfileStringW(L"pmp_usb",L"pldir",dev->pldir,dev->iniFile);
				}
			break;

			case IDC_SUPPORTEDFORMATS:
				if(HIWORD(wParam)==EN_CHANGE) 
				{
					GetDlgItemTextW(hwndDlg,IDC_SUPPORTEDFORMATS,dev->supportedFormats,sizeof(dev->supportedFormats)/sizeof(wchar_t));
					WritePrivateProfileStringW(L"pmp_usb",L"supportedFormats",dev->supportedFormats,dev->iniFile);
				}
			break;

			case IDC_REFRESHCACHE:
				{
					char titleStr[32] = {0};
					dev->refreshNDECache();
					MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_CACHE_UPDATED),
								WASABI_API_LNGSTRING_BUF(IDS_SUCCESS,titleStr,32),MB_OK);
					break;
				}

			case IDC_PL_WRITE_COMBO:
				{
					dev->pl_write_mode = (int)SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
					SetDlgItemTextW(hwndDlg,IDC_PL_WRITE_EG,WASABI_API_LNGSTRINGW(IDS_EG_SLASH+dev->pl_write_mode));
					
					wchar_t tmp[16] = {0};
					StringCchPrintf(tmp, 16, L"%d", dev->pl_write_mode);
					WritePrivateProfileStringW(L"pmp_usb",L"pl_write_mode",tmp,dev->iniFile);
					break;
				}

			case IDC_FILENAMEHELP:
				{
					char titleStr[64] = {0};
					MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_FILENAME_FORMATTING_INFO),
					WASABI_API_LNGSTRING_BUF(IDS_FILENAME_FORMAT_HELP,titleStr,64),MB_OK);
				}
			break;
		
			case IDC_PLBROWSE:
				{	
					wchar_t *tempWS;
					BROWSEINFO bi = {0};
					LPMALLOC lpm = 0;
					wchar_t bffFileName[MAX_PATH] = {0};

					bi.hwndOwner = hwndDlg;
					bi.pszDisplayName = bffFileName;
					bi.lpszTitle = WASABI_API_LNGSTRINGW(IDS_SELECT_FOLDER_TO_LOAD_PLAYLISTS);
					bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
					bi.lpfn = BrowseCallbackProc;
					lstrcpynW(pldir, dev->pldir, MAX_PATH);
					LPITEMIDLIST iil = SHBrowseForFolder(&bi);
					if(iil)
					{
						SHGetPathFromIDListW(iil,bffFileName);
						SHGetMalloc(&lpm);
						// path is now in bffFileName
					}

					tempWS = _wcsdup(bffFileName);
					if(tempWS[0] == dev->drive) 
					{
						lstrcpynW(dev->pldir, tempWS, MAX_PATH);
						SetDlgItemText(hwndDlg,IDC_PLDIR,tempWS);
					}
					else
					{
						if(bffFileName[0] != 0) //dont print error if the user selected 'cancel'
						{
							char titleStr[32] = {0};
							MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_ERR_SELECTED_PATH_NOT_ON_DEVICE),
							WASABI_API_LNGSTRING_BUF(IDS_ERROR,titleStr,32), MB_OK);
						}
					}			
					free(tempWS);
				}
				break;
      
			case IDC_FORMATSHELP:
				{
					char titleStr[64] = {0};
					MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_SUPPORTED_FORMAT_INFO),
					WASABI_API_LNGSTRING_BUF(IDS_SUPPORTED_FORMAT_HELP,titleStr,64),MB_OK);
				}
				break;

			case IDC_PURGEFOLDERS:
		        {
					StringCchCopy(dev->purgeFolders, ARRAYSIZE(dev->purgeFolders),
						((IsDlgButtonChecked(hwndDlg,IDC_PURGEFOLDERS) == BST_CHECKED) ? L"1" : L"0"));
					WritePrivateProfileStringW(L"pmp_usb",L"purgeFolders",dev->purgeFolders,dev->iniFile);
				}
				break;

			case IDC_RESCAN:
			{
				//update changes
				SetFileAttributesW(dev->iniFile,FILE_ATTRIBUTE_HIDDEN);

				wchar_t driveletter = dev->drive; //hold on to driveletter before it goes away
				//disconnect
				dev->Close();

				//connect
				pmpDeviceLoading load;
				dev = new USBDevice(driveletter,&load);
				char titleStr[64] = {0};
				MessageBoxA(hwndDlg,WASABI_API_LNGSTRING(IDS_RESCAN_COMPLETE_SAVED),
							WASABI_API_LNGSTRING_BUF(IDS_RESCAN_COMPLETE,titleStr,64),MB_OK);
			}
			break;
		}
	}
  return 0;
}

// update a track with new metadata (string)
void USBDevice::updateTrackField(UsbSong* song, unsigned int col, const void* newValue, int fieldType)
{
	if (!song) return;

	Nullsoft::Utility::AutoLock lock(dbcs);
	openDeviceTable();
	nde_scanner_t s = NDE_Table_CreateScanner(deviceTable);

	if (NDE_Scanner_LocateFilename(s, DEVICEVIEW_COL_FILENAME, FIRST_RECORD, song->filename))
	{
		switch (fieldType)
		{
			case FIELD_STRING:
				db_setFieldString(s, col, (wchar_t *)(newValue));
				break;
			case FIELD_INTEGER:
				db_setFieldInt(s, col, *((int *)newValue));
			default:
				break;
		}
	}

	NDE_Scanner_Post(s);
	NDE_Table_DestroyScanner(deviceTable, s);
	//NDE_Table_Sync(deviceTable);
	//closeDeviceTable();
}

UsbSong::UsbSong() {
  filename[0]=artist[0]=album[0]=title[0]=genre[0]=albumartist[0]=publisher[0]=composer[0]=0;
  filled=year=track=length=discnum=bitrate=playcount=(int)(size=0);
}

USBArt::USBArt(ARGB32 *bits, int w, int h) :bits(bits), w(w), h(h)
{
}

USBArt::~USBArt()
{
	if (bits)
		WASABI_API_MEMMGR->sysFree(bits);
}